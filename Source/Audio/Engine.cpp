#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace cc {
NoteSet noteSet(const Chord& chord) { auto r=resolve(chord);return {r.notes,r.count}; }
AudioFrame audioFrame(const Timeline& t,uint64_t revision) {
    if(!validate(t)) throw std::invalid_argument("Invalid playback timeline");
    AudioFrame frame;frame.end=t.bars*bar;frame.revision=revision;
    auto blocks=t.blocks;std::sort(blocks.begin(),blocks.end(),[](auto& a,auto& b){return a.start<b.start;});
    for(auto& b:blocks) frame.blocks[frame.blockCount++]={b.id,b.start,b.end,noteSet(b.chord)};
    return frame;
}
void FrameMailbox::publish(const AudioFrame& value) noexcept {
    slots[back]=value;
    back=middle.exchange(back|4u,std::memory_order_acq_rel)&3u;
}
bool FrameMailbox::consume(AudioFrame& value) noexcept {
    if((middle.load(std::memory_order_acquire)&4u)==0) return false;
    front=middle.exchange(front,std::memory_order_acq_rel)&3u;
    value=slots[front];return true;
}
void Engine::prepare(double sr) noexcept { rate=std::isfinite(sr) && sr>=8000 ? sr : 48000;reset(); }
void Engine::reset() noexcept {
    for(auto& v:voices) v={};sounding={};source=0;running=false;activeVoices=0;
}
void Engine::release() noexcept { for(auto& v:voices) if(v.active) v.held=false;sounding={};source=0; }
void Engine::trigger(NoteSet notes,uint64_t owner) noexcept {
    release();sounding=notes;source=owner;
    if(notes.count) ++articulationCount;
    for(int i=0;i<notes.count && i<4;++i) {
        if(notes.notes[i]<0 || notes.notes[i]>127) continue;
        auto it=std::find_if(voices.begin(),voices.end(),[](auto& v){return !v.active;});
        if(it==voices.end()) it=std::min_element(voices.begin(),voices.end(),[](auto& a,auto& b){return a.release<b.release;});
        *it={true,true,notes.notes[i],frame.sound,0,0,1};
    }
}
const PlaybackBlock* Engine::blockAt(double time) const noexcept {
    int lo=0,hi=frame.blockCount;
    while(lo<hi){int mid=(lo+hi)/2;if(frame.blocks[mid].start<=time)lo=mid+1;else hi=mid;}
    if(lo>0 && time<frame.blocks[lo-1].end) return &frame.blocks[lo-1];
    return nullptr;
}
float Engine::sample(Voice& v) noexcept {
    constexpr double tau=2*std::numbers::pi;
    double frequency=440*std::exp2((v.note-69)/12.0);
    v.phase+=frequency/rate;if(v.phase>=1)v.phase-=std::floor(v.phase);
    v.age+=1/rate;
    if(!v.held) v.release*=std::exp(-1/(rate*(v.sound==Sound::strings || v.sound==Sound::pad ? 0.07 : 0.025)));
    if(v.release<0.00005){v.active=false;return 0;}
    double output=0,attack=1-std::exp(-v.age/(v.sound==Sound::strings ? 0.07 : v.sound==Sound::pad ? 0.12 : 0.002));
    // Original additive voices. Harmonics above Nyquist are never generated.
    for(int h=1;h<=12 && frequency*h<rate*0.45;++h) {
        double amplitude=0,angle=tau*v.phase*h;
        switch(v.sound) {
            case Sound::piano: amplitude=std::exp(-v.age*(0.6+h*0.32))/(h*h*0.65);break;
            case Sound::guitar: amplitude=std::exp(-v.age*(1.0+h*0.65))*std::sin(h*1.13)/h;break;
            case Sound::strings: amplitude=0.5/h;angle+=0.015*h*std::sin(tau*5.2*v.age);break;
            case Sound::pad: amplitude=std::exp(-h*0.7)*1.3;angle+=0.18*std::sin(tau*0.8*v.age+h);break;
        }
        output+=amplitude*std::sin(angle);
    }
    return static_cast<float>(output*attack*v.release*0.15);
}
void Engine::process(float* left,float* right,int count,HostClock host,bool bypass) noexcept {
    if(count<=0)return;
    if(bypass){reset();std::fill_n(left,count,0);std::fill_n(right,count,0);uiRunning=false;uiOverride=false;uiBlock=0;return;}
    if(std::isfinite(host.bpm) && host.bpm>0 && host.bpm<1000){bpm=host.bpm;knownTempo=true;}
    meterValid=host.numerator==4 && host.denominator==4;
    AudioFrame latest;
    if(input.consume(latest)) {
        bool transport=latest.transportSerial!=frame.transportSerial;
        bool seeking=latest.seekSerial!=frame.seekSerial;
        bool syncChanged=latest.sync!=frame.sync;
        bool repeatReset=latest.previewOwner!=frame.previewOwner || latest.repeatTicks!=frame.repeatTicks || latest.repeating!=frame.repeating;
        bool soundChanged=latest.sound!=frame.sound;
        frame=latest;
        if(frame.end<bar || frame.end>hardEnd || frame.blockCount<0 || frame.blockCount>512 || frame.preview.count<0 || frame.preview.count>4){reset();frame={};}
        if(seeking)tick=std::clamp(frame.seekTick,0,frame.end-1);
        if(tick>=frame.end)tick=0;
        if(syncChanged){running=frame.sync && host.playing;if(running)tick=0;}
        if(transport && !frame.sync)running=frame.localRun;
        if(repeatReset)repeatPhase=0;
        if(soundChanged)trigger(sounding,source);
    }
    if(frame.sync && host.playing!=lastHost){running=host.playing;if(running)tick=0;}
    lastHost=host.playing;
    if(!meterValid)running=false;
    double step=bpm*ppq/(60*rate);
    for(int i=0;i<count;++i) {
        NoteSet desired;uint64_t owner=0;bool retrigger=false;
        if(frame.previewOwner && frame.preview.count) {
            desired=frame.preview;owner=frame.previewOwner|0x8000000000000000ull;
            if(frame.repeating) {
                int interval=std::max(240,frame.repeatTicks);
                if(repeatPhase>=interval){repeatPhase-=interval;retrigger=true;}
                if(repeatPhase>=interval*0.95){desired={};owner=0;}
                repeatPhase+=step;
            }
        } else if(running) if(auto b=blockAt(tick)){desired=b->chord;owner=b->id;}
        if(desired!=sounding || owner!=source || retrigger)trigger(desired,owner);
        float value=0;activeVoices=0;
        for(auto& v:voices)if(v.active){value+=sample(v);++activeVoices;}
        float target=std::isfinite(frame.gain) ? std::clamp(frame.gain,0.0f,1.0f) : 0.0f;
        smoothGain+=(target-smoothGain)*static_cast<float>(1-std::exp(-1/(rate*0.01)));
        value=std::tanh(value*smoothGain);left[i]=value;right[i]=value;
        if(running){tick+=step;if(tick>=frame.end)tick-=frame.end;}
    }
    auto b=running ? blockAt(tick) : nullptr;
    uiTick=tick;uiTempo=bpm;uiBlock=b ? b->id : 0;uiRunning=running;uiOverride=frame.previewOwner!=0;uiMeter=meterValid;uiTempoKnown=knownTempo;
}
EngineStatus Engine::status() const noexcept { auto b=running ? blockAt(tick) : nullptr;return {tick,bpm,b ? b->id : 0,running,frame.previewOwner!=0,meterValid,knownTempo,activeVoices}; }
}

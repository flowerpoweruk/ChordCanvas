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
void Engine::prepare(double sr) noexcept {
    rate=std::isfinite(sr) && sr>=8000 ? sr : 48000;
    gainFactor=static_cast<float>(1-std::exp(-1/(rate*0.01)));
    if(!waveReady){for(int i=0;i<=waveSize;++i)wave[i]=std::sin(2*std::numbers::pi*i/waveSize);waveReady=true;}
    reset();
}
void Engine::reset() noexcept {
    for(auto& v:voices) v={};sounding={};source=0;running=false;activeVoices=0;
    frame.previewOwner=0;frame.preview={};frame.localRun=false;
}
void Engine::release() noexcept { for(auto& v:voices) if(v.active) v.held=false;sounding={};source=0; }
void Engine::trigger(NoteSet notes,uint64_t owner) noexcept {
    uint64_t previousSource=source;
    release();sounding=notes;source=owner;
    if(auto* sink=diagnostics.load(std::memory_order_acquire)){auto instance=diagnosticInstance.load(std::memory_order_relaxed);if(previousSource)sink->audioEvent({2,instance,frame.revision,previousSource,static_cast<int>(tick),0});if(notes.count)sink->audioEvent({1,instance,frame.revision,owner,static_cast<int>(tick),notes.count});}
    if(notes.count) ++articulationCount;
    for(int i=0;i<notes.count && i<4;++i) {
        if(notes.notes[i]<0 || notes.notes[i]>127) continue;
        auto it=std::find_if(voices.begin(),voices.end(),[](auto& v){return !v.active;});
        if(it==voices.end()) it=std::min_element(voices.begin(),voices.end(),[](auto& a,auto& b){return a.release<b.release;});
        *it={};it->active=true;it->held=true;it->sound=frame.sound;
        double frequency=440*std::exp2((notes.notes[i]-69)/12.0);
        it->increment=frequency/rate;
        it->releaseFactor=std::exp(-1/(rate*(it->sound==Sound::strings || it->sound==Sound::pad ? .07 : .025)));
        it->attackFactor=std::exp(-1/(rate*(it->sound==Sound::strings ? .07 : it->sound==Sound::pad ? .12 : .002)));
        for(int h=1;h<=12 && frequency*h<rate*.45;++h){
            auto index=h-1;++it->harmonics;it->decay[index]=1;
            switch(it->sound){
                case Sound::piano:it->amplitude[index]=1/(h*h*.65);it->decay[index]=std::exp(-(0.6+h*.32)/rate);break;
                case Sound::guitar:it->amplitude[index]=std::sin(h*1.13)/h;it->decay[index]=std::exp(-(1.0+h*.65)/rate);break;
                case Sound::strings:it->amplitude[index]=.5/h;break;
                case Sound::pad:it->amplitude[index]=std::exp(-h*.7)*1.3;break;
            }
        }
    }
}
const PlaybackBlock* Engine::blockAt(double time) const noexcept {
    int lo=0,hi=frame.blockCount;
    while(lo<hi){int mid=(lo+hi)/2;if(frame.blocks[mid].start<=time)lo=mid+1;else hi=mid;}
    if(lo>0 && time<frame.blocks[lo-1].end) return &frame.blocks[lo-1];
    return nullptr;
}
double Engine::sine(double cycles) const noexcept {
    cycles-=std::floor(cycles);double position=cycles*waveSize;
    int index=static_cast<int>(position);
    if(index>=waveSize)return wave[0]; // Negative values very near zero can round up during wrapping.
    // A fixed table, prepared outside processing, bounds the oscillator work.
    return wave[index]+(wave[index+1]-wave[index])*(position-index);
}
float Engine::sample(Voice& v) noexcept {
    constexpr double tau=2*std::numbers::pi;
    v.phase+=v.increment;if(v.phase>=1)v.phase-=std::floor(v.phase);
    v.age+=1/rate;
    if(!v.held) v.release*=v.releaseFactor;
    if(v.release<0.00005){v.active=false;return 0;}
    v.attackRemaining*=v.attackFactor;if(v.attackRemaining<1e-20)v.attackRemaining=0;
    double output=0,attack=1-v.attackRemaining;
    double vibrato=v.sound==Sound::strings ? .015*sine(5.2*v.age)/tau : 0;
    // Original additive voices. Harmonics above Nyquist are never generated.
    for(int h=1;h<=v.harmonics;++h) {
        auto index=h-1;v.amplitude[index]*=v.decay[index];
        if(std::abs(v.amplitude[index])<1e-20)v.amplitude[index]=0; // No inaudible denormal tails.
        double cycles=(v.phase+vibrato)*h;
        if(v.sound==Sound::pad)cycles+=.18*sine(.8*v.age+h/tau)/tau;
        output+=v.amplitude[index]*sine(cycles);
    }
    return static_cast<float>(output*attack*v.release*0.15);
}
void Engine::process(float* left,float* right,int count,HostClock host,bool bypass) noexcept {
    if(count<=0)return;
    bool validTempo=std::isfinite(host.bpm) && host.bpm>0 && host.bpm<1000;
    bool becameKnown=validTempo && !knownTempo;
    if(validTempo){bpm=host.bpm;knownTempo=true;}
    meterValid=host.numerator==4 && host.denominator==4;
    uiTempoAvailable=validTempo;uiTempo=bpm;uiTempoKnown=knownTempo;uiMeter=meterValid;
    if(bypass){reportClock(host,count,true);AudioFrame discarded;input.consume(discarded);reset();std::fill_n(left,count,0.0f);std::fill_n(right,count,0.0f);uiRunning=false;uiOverride=false;uiBlock=0;return;}
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
    if(frame.sync && (host.playing!=lastHost || becameKnown)){running=host.playing;if(running)tick=0;}
    lastHost=host.playing;
    if(!meterValid || !knownTempo)running=false; // Initial 120 BPM fallback is preview-only.
    reportClock(host,count,false);
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
        smoothGain+=(target-smoothGain)*gainFactor;
        value=std::tanh(value*smoothGain);left[i]=value;right[i]=value;
        if(running){tick+=step;if(tick>=frame.end)tick-=frame.end;}
    }
    auto b=running ? blockAt(tick) : nullptr;
    uiTick=tick;uiTempo=bpm;uiBlock=b ? b->id : 0;uiRunning=running;uiOverride=frame.previewOwner!=0;uiMeter=meterValid;uiTempoKnown=knownTempo;
}
void Engine::reportClock(HostClock clock,int count,bool bypass) noexcept {
    auto* sink=diagnostics.load(std::memory_order_acquire);
    if(!sink){reportedSink=nullptr;return;}
    AudioLogEvent event;event.kind=3;event.instance=diagnosticInstance.load(std::memory_order_relaxed);event.revision=frame.revision;event.tick=static_cast<int>(tick);
    event.tempo=bpm;event.sampleRate=rate;event.bufferFrames=count;event.numerator=clock.numerator;event.denominator=clock.denominator;
    event.tempoAvailable=std::isfinite(clock.bpm) && clock.bpm>0 && clock.bpm<1000;event.tempoEverKnown=knownTempo;event.hostPlaying=clock.playing;event.bypassed=bypass;
    if(reportedSink==sink && reportedClock.tempo==event.tempo && reportedClock.sampleRate==event.sampleRate && reportedClock.bufferFrames==event.bufferFrames && reportedClock.numerator==event.numerator && reportedClock.denominator==event.denominator && reportedClock.tempoAvailable==event.tempoAvailable && reportedClock.tempoEverKnown==event.tempoEverKnown && reportedClock.hostPlaying==event.hostPlaying && reportedClock.bypassed==event.bypassed)return;
    // A refused bounded queue admission is counted by the sink. Coalesce the
    // observation anyway so an overflow never creates a per-callback retry storm.
    sink->audioEvent(event);reportedSink=sink;reportedClock=event;
}
EngineStatus Engine::status() const noexcept { auto b=running ? blockAt(tick) : nullptr;return {tick,bpm,b ? b->id : 0,running,frame.previewOwner!=0,meterValid,knownTempo,activeVoices}; }
}

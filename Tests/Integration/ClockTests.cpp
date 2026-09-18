#include "Audio/Engine.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace cc;
struct Sink final:AudioEventSink {
    AudioLogQueue queue;int admitted=0,refused=0;
    bool audioEvent(AudioLogEvent event) noexcept override {if(queue.push(event)){++admitted;return true;}++refused;return false;}
    AudioLogEvent clock(){AudioLogEvent event;while(queue.pop(event))if(event.kind==3)return event;throw std::runtime_error("Missing clock observation");}
    bool empty(){AudioLogEvent event;return !queue.pop(event);}
};
void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
int main(){try{
    Sink sink;Engine engine;engine.prepare(48000);engine.diagnostics=&sink;engine.diagnosticInstance=9;float left[256]{},right[256]{};
    auto process=[&](HostClock clock,int frames=256,bool bypass=false){engine.process(left,right,frames,clock,bypass);};
    AudioFrame frame;frame.sync=true;engine.input.publish(frame);
    process({NAN,true});auto initial=sink.clock();check(initial.instance==9 && initial.tempo==120 && !initial.tempoAvailable && !initial.tempoEverKnown && initial.sampleRate==48000 && initial.bufferFrames==256 && !engine.status().running,"initial fallback is preview-only, with explicit unknown tempo and actual format");
    for(int i=0;i<10000;++i)process({0,true});check(sink.empty() && sink.admitted==1 && engine.status().tick==0,"unchanged missing tempo leaves timeline armed without normal-callback noise");
    process({91.125,true});auto valid=sink.clock();check(valid.tempo==91.125 && valid.tempoAvailable && valid.tempoEverKnown && engine.uiTempoKnown && engine.uiTempoAvailable && engine.status().running && engine.status().tick<20,"first valid host tempo starts already-playing Sync at its own zero");
    process({INFINITY,true});auto lost=sink.clock();check(lost.tempo==91.125 && !lost.tempoAvailable && lost.tempoEverKnown && !engine.uiTempoAvailable && engine.status().running,"tempo validity loss retains last valid running clock instead of falling back");
    process({112.875,true,3,8},127);auto changed=sink.clock();check(changed.tempo==112.875 && changed.hostPlaying && changed.numerator==3 && changed.denominator==8 && changed.bufferFrames==127 && !engine.uiMeter,"tempo, transport, explicit meter and actual buffer changes observed together");
    process({112.875,true,3,8},127,true);check(sink.clock().bypassed && !engine.uiRunning && !engine.uiOverride,"bypass transition observed while preserving cleanup");
    process({112.875,true,3,8},127,true);check(sink.empty(),"unchanged bypass callback coalesced");
    process({112.875,false,4,4},127);auto resumed=sink.clock();check(!resumed.bypassed && !resumed.hostPlaying && engine.uiMeter,"unbypass/host stop/supported meter recovery recorded");
    engine.prepare(96000);process({112.875,false,4,4},64);auto format=sink.clock();check(format.sampleRate==96000 && format.bufferFrames==64,"sample-rate reprepare and actual buffer change observed");
    Sink second;engine.diagnostics=&second;process({112.875,false,4,4},64);check(second.clock().tempo==112.875,"new interactive sink receives current metadata even without clock change");
    engine.diagnostics=nullptr;process({112.875,false,4,4},64);engine.diagnostics=&second;process({112.875,false,4,4},64);check(second.clock().tempo==112.875,"sink reattachment receives initial metadata");
    for(int i=0;i<256;++i)check(second.queue.push({1}),"fill only own bounded diagnostic ring");auto refused=second.refused;process({113,false});for(int i=0;i<10000;++i)process({113,false});check(second.refused==refused+1,"full queue refusal never creates normal-callback retry storm");
    std::cout<<"PASS: unknown fallback, exact valid tempo, last-valid retention, explicit meter/transport/buffer/rate/bypass observations, sink attachment and bounded overflow coalescing; 20000 quiet callbacks\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

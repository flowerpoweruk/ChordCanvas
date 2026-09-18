#include "Audio/Engine.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <malloc.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>

// Instrument only the thread executing Engine::process. Setup, mailbox
// publication, measurement and result formatting happen outside this guard.
namespace {
thread_local bool guarded=false;
thread_local uint64_t allocations=0,deallocations=0;
void* allocate(size_t size,size_t alignment=0){
    if(guarded)++allocations;
    void* result=alignment ? _aligned_malloc(size ? size : 1,alignment) : std::malloc(size ? size : 1);
    if(!result)throw std::bad_alloc();return result;
}
void dispose(void* value,bool aligned=false) noexcept{
    if(value && guarded)++deallocations;
    if(aligned)_aligned_free(value);else std::free(value);
}
struct Sink final:cc::AudioEventSink {
    cc::AudioLogQueue queue;
    bool audioEvent(cc::AudioLogEvent event) noexcept override{return queue.push(event);}
};
}
void* operator new(size_t n){return allocate(n);}
void* operator new[](size_t n){return allocate(n);}
void* operator new(size_t n,std::align_val_t a){return allocate(n,static_cast<size_t>(a));}
void* operator new[](size_t n,std::align_val_t a){return allocate(n,static_cast<size_t>(a));}
void operator delete(void* p) noexcept{dispose(p);}
void operator delete[](void* p) noexcept{dispose(p);}
void operator delete(void* p,size_t) noexcept{dispose(p);}
void operator delete[](void* p,size_t) noexcept{dispose(p);}
void operator delete(void* p,std::align_val_t) noexcept{dispose(p,true);}
void operator delete[](void* p,std::align_val_t) noexcept{dispose(p,true);}
void operator delete(void* p,size_t,std::align_val_t) noexcept{dispose(p,true);}
void operator delete[](void* p,size_t,std::align_val_t) noexcept{dispose(p,true);}

int main(){
    using namespace cc;using Clock=std::chrono::steady_clock;
    bool valid=true;unsigned row=0;std::cout<<std::setprecision(9);
    std::cout<<"{\"source\":\""<<CC_SOURCE_COMMIT<<"\",\"scope\":\"Offline engine measurements; not host callback or listening acceptance\",\"cases\":[\n";
    for(double rate:{44100.0,48000.0,96000.0})for(int buffer:{64,256,1024})for(auto sound:{Sound::piano,Sound::guitar,Sound::strings,Sound::pad})for(bool repeat:{false,true}){
        Engine engine;engine.prepare(rate);Sink sink;engine.diagnostics=&sink;
        AudioFrame frame;frame.end=32*bar;frame.sound=sound;
        frame.localRun=true;frame.transportSerial=1;frame.blockCount=128;
        for(int i=0;i<128;++i)frame.blocks[i]={static_cast<uint64_t>(i+1),i*ppq,(i+1)*ppq,{{48+i%12,52+i%12,55+i%12,59+i%12},4}};
        if(repeat){frame.preview={{48,52,55,59},4};frame.previewOwner=1;frame.repeating=true;frame.repeatTicks=480;}
        engine.input.publish(frame);float left[1024]{},right[1024]{};
        uint64_t total=0;double sum=0,maximum=0,power=0,peak=0;int maximumVoices=0;
        const int warm=static_cast<int>(rate*.08/buffer),measured=static_cast<int>(std::ceil(rate*.5/buffer));
        allocations=0;deallocations=0;
        for(int b=0;b<warm+measured;++b){
            if(b%11==0){++frame.revision;frame.gain=b%22==0 ? .18f : .3f;frame.blocks[(b/11)%128].chord.notes[0]=48+b%12;engine.input.publish(frame);}
            auto before=Clock::now();guarded=true;
            engine.process(left,right,buffer,{999,false});
            guarded=false;auto after=Clock::now();
            if(b>=warm){double elapsed=std::chrono::duration<double>(after-before).count();sum+=elapsed;maximum=std::max(maximum,elapsed);total+=buffer;}
            for(int i=0;i<buffer;++i){valid=valid && std::isfinite(left[i]) && left[i]==right[i];power+=left[i]*left[i];peak=std::max(peak,std::abs(static_cast<double>(left[i])));}
            maximumVoices=std::max(maximumVoices,engine.status().voices);
            AudioLogEvent event;while(sink.queue.pop(event)){}
        }
        // Bypass and a variable final buffer are covered by the allocation guard.
        guarded=true;engine.process(left,right,17,{120,false},true);guarded=false;
        valid=valid && allocations==0 && deallocations==0 && power>0 && peak<1 && engine.status().voices==0;
        if(row++)std::cout<<",\n";
        std::cout<<"{\"rate\":"<<rate<<",\"buffer\":"<<buffer<<",\"sound\":"<<static_cast<int>(sound)<<",\"scenario\":\""<<(repeat ? "rapid-repeat-over-dense-timeline" : "dense-timeline-with-edits")<<"\",\"bpm\":999,\"measuredFrames\":"<<total<<",\"cpuFraction\":"<<sum/(total/rate)<<",\"maxCallbackDeadlineFraction\":"<<maximum/(buffer/rate)<<",\"peak\":"<<peak<<",\"maxVoices\":"<<maximumVoices<<",\"allocations\":"<<allocations<<",\"deallocations\":"<<deallocations<<"}";
    }
    std::cout<<"\n],\"signalAndAllocationChecks\":\""<<(valid ? "PASS" : "FAIL")<<"\"}\n";
    return valid ? 0 : 1;
}

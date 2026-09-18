#include "Audio/Engine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>

// Independent direct evaluation of the original synthesis equations. This
// deliberately uses no production lookup table or recursive envelopes.
struct Reference {
    double phase=0,age=0,release=1;
    float next(int note,cc::Sound sound,double rate,bool held){
        constexpr double tau=2*std::numbers::pi;
        double frequency=440*std::exp2((note-69)/12.0);
        phase+=frequency/rate;if(phase>=1)phase-=std::floor(phase);age+=1/rate;
        if(!held)release*=std::exp(-1/(rate*(sound==cc::Sound::strings || sound==cc::Sound::pad ? .07 : .025)));
        if(release<.00005)return 0;
        double attack=1-std::exp(-age/(sound==cc::Sound::strings ? .07 : sound==cc::Sound::pad ? .12 : .002)),output=0;
        for(int h=1;h<=12 && frequency*h<rate*.45;++h){
            double amplitude=0,angle=tau*phase*h;
            switch(sound){
                case cc::Sound::piano:amplitude=std::exp(-age*(.6+h*.32))/(h*h*.65);break;
                case cc::Sound::guitar:amplitude=std::exp(-age*(1+h*.65))*std::sin(h*1.13)/h;break;
                case cc::Sound::strings:amplitude=.5/h;angle+=.015*h*std::sin(tau*5.2*age);break;
                case cc::Sound::pad:amplitude=std::exp(-h*.7)*1.3;angle+=.18*std::sin(tau*.8*age+h);break;
            }
            output+=amplitude*std::sin(angle);
        }
        return static_cast<float>(output*attack*release*.15);
    }
};
int main(){
    using namespace cc;double worst=0,totalError=0;uint64_t samples=0;unsigned cases=0;
    for(double rate:{44100.0,48000.0,96000.0})for(auto sound:{Sound::piano,Sound::guitar,Sound::strings,Sound::pad})for(int note:{36,60,84,108,125,-1}){
        Engine engine;engine.prepare(rate);AudioFrame frame;frame.sound=sound;frame.previewOwner=1;
        frame.preview=note<0 ? NoteSet{{48,52,55,59},4} : NoteSet{{note,0,0,0},1};
        auto notes=frame.preview;engine.input.publish(frame);std::array<Reference,4> reference;
        float left[256]{},right[256]{};int held=static_cast<int>(rate*.04),total=static_cast<int>(rate*.25),offset=0,block=0;
        while(offset<total){
            bool holding=offset<held;
            if(!holding && frame.previewOwner){frame.previewOwner=0;engine.input.publish(frame);}
            int pattern=block++%3;
            int n=std::min({pattern==0 ? 17 : pattern==1 ? 127 : 256,total-offset,holding ? held-offset : total-offset});
            engine.process(left,right,n,{120,false});
            for(int i=0;i<n;++i){
                float sum=0;for(int voice=0;voice<notes.count;++voice)sum+=reference[voice].next(notes.notes[voice],sound,rate,holding);
                float expected=std::tanh(sum*.25118864f);double error=std::abs(static_cast<double>(left[i])-expected);
                if(!std::isfinite(left[i]) || left[i]!=right[i] || error>2e-6){std::cerr<<"FAIL: synthesis fidelity at rate "<<rate<<", sound "<<static_cast<int>(sound)<<", note "<<note<<", frame "<<offset+i<<", error "<<error<<'\n';return 1;}
                worst=std::max(worst,error);totalError+=error*error;++samples;
            }
            offset+=n;
        }
        ++cases;
    }
    std::cout<<"PASS: "<<cases<<" direct-equation held/released cases; "<<samples<<" samples, maximum absolute error "<<worst<<", RMS error "<<std::sqrt(totalError/samples)<<"; nonuniform buffers, all sounds, five registers and four-note chords\n";
}

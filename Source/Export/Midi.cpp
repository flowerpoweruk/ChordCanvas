#include "Midi.h"
#include <algorithm>
#include <stdexcept>
namespace cc {
namespace {
void be(std::vector<uint8_t>& out,uint32_t v,int count) { for(int n=count-1;n>=0;--n) out.push_back(static_cast<uint8_t>(v>>(n*8))); }
void vlq(std::vector<uint8_t>& out,uint32_t v) {
    uint8_t bytes[4];int count=0;bytes[count++]=static_cast<uint8_t>(v&127);
    while((v>>=7)!=0) bytes[count++]=static_cast<uint8_t>((v&127)|128);
    while(count) out.push_back(bytes[--count]);
}
}
std::vector<uint8_t> midi(const Timeline& t) {
    if(!validate(t)) throw std::invalid_argument("Invalid export timeline");
    struct Event { int tick,note;bool on; };
    std::vector<Event> events;
    for(auto& b:t.blocks) { auto r=resolve(b.chord); for(int n=0;n<r.count;++n){events.push_back({b.start,r.notes[n],true});events.push_back({b.end,r.notes[n],false});} }
    std::sort(events.begin(),events.end(),[](auto& a,auto& b){if(a.tick!=b.tick)return a.tick<b.tick;if(a.on!=b.on)return !a.on;return a.note<b.note;});
    std::vector<uint8_t> track;int tick=0;
    for(auto e:events){vlq(track,static_cast<uint32_t>(e.tick-tick));tick=e.tick;track.push_back(e.on ? 0x90 : 0x80);track.push_back(static_cast<uint8_t>(e.note));track.push_back(e.on ? 100 : 0);}
    vlq(track,static_cast<uint32_t>(t.bars*bar-tick));track.insert(track.end(),{0xff,0x2f,0});
    std::vector<uint8_t> result {'M','T','h','d'};be(result,6,4);be(result,0,2);be(result,1,2);be(result,ppq,2);
    result.insert(result.end(),{'M','T','r','k'});be(result,static_cast<uint32_t>(track.size()),4);result.insert(result.end(),track.begin(),track.end());return result;
}
}

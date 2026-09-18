#include "Timeline.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace cc {
bool intersects(const Block& a,const Block& b) { return a.start<b.end && b.start<a.end; }
namespace { bool gridValid(int grid) { return grid==240 || grid==480 || grid==960 || grid==1920 || grid==3840; } }
int snap(int t,int grid) {
    if(!gridValid(grid))return t;
    auto result=static_cast<int64_t>(std::floor(static_cast<double>(t)/grid+0.5))*grid;
    return static_cast<int>(std::clamp(result,static_cast<int64_t>(std::numeric_limits<int>::min()),static_cast<int64_t>(std::numeric_limits<int>::max())));
}
bool validate(const Timeline& t) {
    if(t.bars<1 || t.bars>32 || t.blocks.size()>512) return false;
    std::set<uint64_t> ids;
    for(size_t i=0;i<t.blocks.size();++i) {
        auto& b=t.blocks[i];
        if(!b.id || b.id==std::numeric_limits<uint64_t>::max() || !ids.insert(b.id).second || b.start<0 || b.end<=b.start || b.end>t.bars*bar || b.end-b.start<ppq || !valid(b.chord)) return false;
        for(size_t j=0;j<i;++j) if(intersects(b,t.blocks[j])) return false;
    }
    return true;
}
const Block* Document::find(uint64_t id) const {
    auto i=std::find_if(current.blocks.begin(),current.blocks.end(),[&](const auto& b){return b.id==id;});
    return i==current.blocks.end() ? nullptr : &*i;
}
bool Document::commit(Timeline next,std::string name) {
    std::sort(next.blocks.begin(),next.blocks.end(),[](const auto& a,const auto& b){return a.start<b.start;});
    if(!validate(next) || next==current) return false;
    past.push_back({current,next,name}); if(past.size()>20) past.pop_front();
    future.clear(); current=std::move(next); action=std::move(name); ++rev;
    for(auto& b:current.blocks) nextId=std::max(nextId,b.id+1);
    return true;
}
bool Document::insert(std::vector<Block> incoming,const std::vector<uint64_t>& removed,std::string name) {
    if(incoming.empty()) return false;
    for(auto& b:incoming) if(b.start<0 || b.start>=hardEnd || b.end<=b.start || b.end>hardEnd || b.end-b.start<ppq || !valid(b.chord) || b.id==std::numeric_limits<uint64_t>::max()) return false;
    Timeline next=current;
    std::erase_if(next.blocks,[&](const Block& b){
        return std::find(removed.begin(),removed.end(),b.id)!=removed.end()
            || std::any_of(incoming.begin(),incoming.end(),[&](const Block& n){return intersects(b,n);});
    });
    for(auto& b:incoming) { if(!b.id) b.id=nextId++; next.bars=std::max(next.bars,(b.end+bar-1)/bar); next.blocks.push_back(b); }
    return commit(std::move(next),std::move(name));
}
bool Document::add(const Chord& c,int start,int grid) {
    if(!gridValid(grid) || start<0 || start>=hardEnd || !valid(c))return false;
    start=snap(start,grid);
    if(start<0 || start>=hardEnd || hardEnd-start<ppq) return false;
    return insert({{0,start,std::min(hardEnd,start+bar),c}}, {},"Add chord");
}
bool Document::append(const Chord& c,int grid) {
    if(!gridValid(grid))return false;
    int end=0; for(auto& b:current.blocks) end=std::max(end,b.end);
    return add(c,((end+grid-1)/grid)*grid,grid);
}
std::vector<Block> Document::copy(const std::vector<uint64_t>& ids) const {
    std::vector<Block> result;
    for(auto& b:current.blocks) if(std::find(ids.begin(),ids.end(),b.id)!=ids.end()) result.push_back(b);
    if(!result.empty()) { int anchor=result.front().start; for(auto& b:result) { b.start-=anchor; b.end-=anchor; b.id=0; } }
    return result;
}
bool Document::paste(const std::vector<Block>& copied,int playhead,int grid) {
    if(!gridValid(grid) || playhead<0 || playhead>=hardEnd)return false;
    int anchor=snap(playhead,grid); if(anchor<0 || anchor>=hardEnd) return false;
    std::vector<Block> result;
    for(auto b:copied) {
        if(b.start<0 || b.start>=hardEnd || b.end<=b.start || b.end>hardEnd || b.end-b.start<ppq || !valid(b.chord)) return false;
        b.id=0; b.start+=anchor; b.end=std::min(hardEnd,b.end+anchor);
        if(b.start<hardEnd && b.end-b.start>=ppq) result.push_back(b);
    }
    return insert(std::move(result),{},"Paste");
}
bool Document::duplicate(const std::vector<uint64_t>& ids,int grid) {
    int end=0; for(auto id:ids) if(auto b=find(id)) end=std::max(end,b->end);
    return paste(copy(ids),end,grid);
}
bool Document::move(const std::vector<uint64_t>& ids,int candidate,int grid) {
    if(!gridValid(grid))return false;
    std::vector<Block> group; for(auto id:ids) if(auto b=find(id)) group.push_back(*b);
    if(group.empty()) return false;
    int first=hardEnd,last=0; for(auto& b:group){first=std::min(first,b.start);last=std::max(last,b.end);}
    int maxAnchor=((hardEnd-(last-first))/grid)*grid;
    int anchor=std::clamp(snap(candidate,grid),0,maxAnchor);
    for(auto& b:group){b.start+=anchor-first;b.end+=anchor-first;}
    return insert(std::move(group),ids,"Move");
}
bool Document::resize(uint64_t id,bool left,int tick,int grid) {
    if(!gridValid(grid))return false;
    auto ptr=find(id); if(!ptr) return false;
    Block b=*ptr;
    if(left) b.start=std::clamp(snap(tick,grid),0,((b.end-ppq)/grid)*grid);
    else b.end=std::clamp(snap(tick,grid),((b.start+ppq+grid-1)/grid)*grid,(hardEnd/grid)*grid);
    return insert({b},{id},left ? "Resize start" : "Resize end");
}
bool Document::slice(uint64_t id,int tick,int grid) {
    if(!gridValid(grid))return false;
    auto ptr=find(id); if(!ptr) return false;
    Block a=*ptr,b=*ptr; tick=snap(tick,grid);
    if(tick<=a.start || tick>=a.end || tick-a.start<ppq || a.end-tick<ppq) return false;
    a.id=0;b.id=0;a.end=tick;b.start=tick;
    return insert({a,b},{id},"Slice");
}
bool Document::erase(const std::vector<uint64_t>& ids) {
    Timeline next=current; std::erase_if(next.blocks,[&](auto& b){return std::find(ids.begin(),ids.end(),b.id)!=ids.end();});
    return commit(std::move(next),"Delete");
}
bool Document::length(int bars) {
    if(bars<1 || bars>32) return false;
    Timeline next=current;next.bars=bars;
    for(auto& b:next.blocks) b.end=std::min(b.end,bars*bar);
    std::erase_if(next.blocks,[](auto& b){return b.end-b.start<ppq;});
    return commit(std::move(next),"Timeline length");
}
bool Document::voicing(uint64_t id,const Chord& c) {
    if(!valid(c)) return false;
    Timeline next=current; for(auto& b:next.blocks) if(b.id==id) b.chord=c;
    return commit(std::move(next),"Chord voicing");
}
bool Document::load(const Timeline& t) { return commit(t,"Load progression"); }
bool Document::undo() {
    if(past.empty()) return false;
    auto tx=std::move(past.back());past.pop_back();current=tx.before;action="Undo "+tx.action;future.push_back(std::move(tx));++rev;return true;
}
bool Document::redo() {
    if(future.empty()) return false;
    auto tx=std::move(future.back());future.pop_back();current=tx.after;action="Redo "+tx.action;past.push_back(std::move(tx));++rev;return true;
}
}

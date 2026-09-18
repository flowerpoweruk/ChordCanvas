#pragma once
#include "Model/Model.h"
#include <deque>
#include <functional>
#include <string>

namespace cc {
struct Transaction { Timeline before,after; std::string action; };
class Document {
public:
    const Timeline& state() const { return current; }
    uint64_t revision() const { return rev; }
    const std::string& lastAction() const { return action; }
    bool insert(std::vector<Block> incoming,const std::vector<uint64_t>& removed={},std::string name="Insert");
    bool add(const Chord& chord,int start,int grid=ppq);
    bool append(const Chord& chord,int grid=ppq);
    bool move(const std::vector<uint64_t>& ids,int candidateAnchor,int grid=ppq);
    bool resize(uint64_t id,bool left,int tick,int grid=ppq);
    bool slice(uint64_t id,int tick,int grid=ppq);
    bool erase(const std::vector<uint64_t>& ids);
    bool length(int bars);
    bool voicing(uint64_t id,const Chord& chord);
    bool load(const Timeline& state);
    std::vector<Block> copy(const std::vector<uint64_t>& ids) const;
    bool paste(const std::vector<Block>& copied,int playhead,int grid=ppq);
    bool duplicate(const std::vector<uint64_t>& ids,int grid=ppq);
    bool undo();
    bool redo();
    size_t undoCount() const { return past.size(); }
    const Block* find(uint64_t id) const;
private:
    bool commit(Timeline next,std::string name);
    Timeline current;
    std::deque<Transaction> past,future;
    uint64_t nextId=1,rev=0;
    std::string action;
};
}

#pragma once
#include "Theory/Theory.h"
#include <cstdint>
#include <vector>

namespace cc {
struct Block {
    uint64_t id=0;
    int start=0, end=bar;
    Chord chord;
    bool operator==(const Block&) const = default;
};
struct Timeline {
    int bars=8;
    std::vector<Block> blocks;
    bool operator==(const Timeline&) const = default;
};
bool validate(const Timeline& timeline);
bool intersects(const Block& a,const Block& b);
int snap(int tick,int grid);
}

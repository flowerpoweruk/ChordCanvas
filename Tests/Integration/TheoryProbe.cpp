#include "Theory/Theory.h"
#include <iostream>
int main() {
    using namespace cc;
    for(auto k:keys())for(int d=0;d<7;++d)for(int o=1;o<=5;++o)for(int sus=0;sus<3;++sus)for(int seventh=0;seventh<2;++seventh)for(int inv=0;inv<(seventh ? 4 : 3);++inv) {
        auto r=resolve({k,d,o,inv,seventh!=0,static_cast<Suspension>(sus)});
        std::cout<<k.letter<<'\t'<<k.accidental<<'\t'<<static_cast<int>(k.mode)<<'\t'<<d<<'\t'<<o<<'\t'<<sus<<'\t'<<seventh<<'\t'<<inv<<'\t';
        for(int n=0;n<r.count;++n){if(n)std::cout<<',';std::cout<<r.notes[n];}
        std::cout<<'\t';auto s=scale(k);for(int n=0;n<7;++n){if(n)std::cout<<',';std::cout<<s[n].letter<<':'<<s[n].accidental;}
        std::cout<<'\t'<<r.label<<'\n';
    }
}

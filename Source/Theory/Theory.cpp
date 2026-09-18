#include "Theory.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cc {
namespace {
constexpr std::array<int, 7> naturals {0,2,4,5,7,9,11};
constexpr std::array<int, 7> major {0,2,4,5,7,9,11};
constexpr std::array<int, 7> minor {0,2,3,5,7,8,10};
constexpr char letters[] = "CDEFGAB";
const auto& offsets(const Key& k) { return k.mode == Mode::major ? major : minor; }
int accidentalFor(int pc, int letter) {
    int a = mod12(pc - naturals.at(static_cast<size_t>(letter)));
    return a > 6 ? a - 12 : a;
}
}
int mod12(int value) { return (value % 12 + 12) % 12; }
bool valid(const Key& k) {
    return k.letter >= 0 && k.letter < 7 && k.accidental >= -1 && k.accidental <= 1
        && (k.mode == Mode::major || k.mode == Mode::minor);
}
bool valid(const Chord& c) {
    return valid(c.origin) && c.degree >= 0 && c.degree < 7 && c.octave >= 1 && c.octave <= 5
        && c.inversion >= 0 && c.inversion < (c.seventh ? 4 : 3)
        && (c.suspension == Suspension::none || c.suspension == Suspension::sus2 || c.suspension == Suspension::sus4);
}
std::vector<Key> keys() {
    std::vector<Key> result;
    for (int l=0; l<7; ++l) for (int a : {0,1,-1}) for (auto m : {Mode::major,Mode::minor}) result.push_back({l,a,m});
    return result;
}
std::string written(PitchName n) {
    std::string result(1, letters[n.letter]);
    for (int i=0; i<std::abs(n.accidental); ++i) result += n.accidental > 0 ? "♯" : "♭";
    return result;
}
std::string display(PitchName n, int direction) {
    if (std::abs(n.accidental)<=1) return written(n);
    int pc = mod12(naturals[n.letter]+n.accidental);
    PitchName best {0, 12};
    for (int l=0;l<7;++l) {
        PitchName next {l, accidentalFor(pc,l)};
        const bool tiePreferred = next.accidental == (direction < 0 ? -1 : 1);
        if (std::abs(next.accidental)<std::abs(best.accidental)
            || (std::abs(next.accidental)==std::abs(best.accidental) && tiePreferred)) best=next;
    }
    return written(best); // Natural first; sharp fallback for a natural originating tonic.
}
std::string keyLabel(const Key& k) {
    if (!valid(k)) throw std::invalid_argument("Invalid written key");
    return written({k.letter,k.accidental}) + (k.mode==Mode::major ? " Major" : " Minor");
}
std::array<PitchName,7> scale(const Key& k) {
    if (!valid(k)) throw std::invalid_argument("Invalid key");
    std::array<PitchName,7> result {};
    int tonic=naturals[k.letter]+k.accidental;
    for(int d=0;d<7;++d) {
        int l=(k.letter+d)%7;
        result[d]={l, accidentalFor(tonic+offsets(k)[d],l)};
    }
    return result;
}
ResolvedChord resolve(const Chord& c) {
    if(!valid(c)) throw std::invalid_argument("Invalid chord definition");
    ResolvedChord r;
    auto s=scale(c.origin);
    auto ext=[&](int d) { return 60+mod12(naturals[c.origin.letter]+c.origin.accidental)+offsets(c.origin)[d%7]+12*(d/7); };
    int root=ext(c.degree), third=ext(c.degree+2)-root, fifth=ext(c.degree+4)-root;
    r.count=c.seventh ? 4 : 3;
    for(int n=0;n<r.count;++n) { r.notes[n]=ext(c.degree+n*2); r.names[n]=s[(c.degree+n*2)%7]; }
    r.rootLabel=display(s[c.degree],c.origin.accidental);
    std::string quality;
    int seventh=ext(c.degree+6)-root;
    if(c.suspension!=Suspension::none) {
        r.notes[1]=root+(c.suspension==Suspension::sus2 ? 2 : 5);
        r.notes[2]=root+7;
        int secondLetter=(s[c.degree].letter+(c.suspension==Suspension::sus2 ? 1 : 3))%7;
        r.names[1]={secondLetter,accidentalFor(r.notes[1],secondLetter)};
        int fifthLetter=(s[c.degree].letter+4)%7;
        r.names[2]={fifthLetter,accidentalFor(r.notes[2],fifthLetter)};
        if(c.seventh) quality=seventh==11 ? "maj7" : "7";
        quality += c.suspension==Suspension::sus2 ? "sus2" : "sus4";
    } else if(c.seventh) {
        if(third==4) quality=seventh==11 ? "maj7" : "7";
        else quality=fifth==6 ? "m7♭5" : "m7";
    } else quality=third==4 ? "" : (fifth==6 ? "dim" : "m");
    r.label=r.rootLabel+quality;
    for(int i=0;i<c.inversion;++i) {
        int n=r.notes[0]+12; PitchName name=r.names[0];
        for(int j=0;j<r.count-1;++j) { r.notes[j]=r.notes[j+1]; r.names[j]=r.names[j+1]; }
        r.notes[r.count-1]=n; r.names[r.count-1]=name;
    }
    for(int n=0;n<r.count;++n) {
        r.notes[n]+=12*(c.octave-3);
        if(r.notes[n]<0 || r.notes[n]>127) throw std::out_of_range("MIDI note range");
    }
    return r;
}
void setSeventh(Chord& c,bool enabled) { c.seventh=enabled; c.inversion=std::min(c.inversion,enabled ? 3 : 2); }
}

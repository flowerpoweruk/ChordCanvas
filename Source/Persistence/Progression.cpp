#include "Progression.h"
#include <charconv>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

namespace cc {
namespace {
class Reader {
public:
    explicit Reader(std::string_view text):input(text) { if(text.size()>1024*1024)fail(); }
    [[noreturn]] void fail() const { throw std::invalid_argument("Invalid or unsupported ChordCanvas progression"); }
    void space() { while(pos<input.size() && (input[pos]==' ' || input[pos]=='\r' || input[pos]=='\n' || input[pos]=='\t'))++pos; }
    bool take(char c) { space();if(pos<input.size() && input[pos]==c){++pos;return true;}return false; }
    void need(char c) { if(!take(c))fail(); }
    std::string key() {
        need('"');size_t first=pos;
        while(pos<input.size() && input[pos]!='"') { char c=input[pos++];if(c<' ' || c=='\\' || pos-first>32)fail(); }
        if(pos==input.size())fail();std::string result(input.substr(first,pos-first));++pos;need(':');return result;
    }
    int64_t integer() {
        space();size_t first=pos;if(pos<input.size() && input[pos]=='-')++pos;
        size_t digits=pos;while(pos<input.size() && input[pos]>='0' && input[pos]<='9')++pos;
        if(pos==digits || (pos-digits>1 && input[digits]=='0'))fail();
        int64_t result=0;auto r=std::from_chars(input.data()+first,input.data()+pos,result);
        if(r.ec!=std::errc{})fail();return result;
    }
    void finish() { space();if(pos!=input.size())fail(); }
private:
    std::string_view input;size_t pos=0;
};
int field(const std::map<std::string,int64_t>& fields,const char* name,int lo,int hi,Reader& r) {
    auto i=fields.find(name);if(i==fields.end() || i->second<lo || i->second>hi)r.fail();return static_cast<int>(i->second);
}
Block readBlock(Reader& r) {
    r.need('{');std::map<std::string,int64_t> fields;
    if(!r.take('}'))do { auto name=r.key();auto value=r.integer();if(!fields.emplace(name,value).second)r.fail();if(r.take('}'))break;r.need(','); }while(true);
    if(fields.size()!=12)r.fail();
    Block b;
    auto id=fields.find("id");if(id==fields.end() || id->second<1 || id->second==std::numeric_limits<int64_t>::max())r.fail();b.id=static_cast<uint64_t>(id->second);
    b.start=field(fields,"start",0,hardEnd,r);b.end=field(fields,"end",0,hardEnd,r);
    b.chord.origin.letter=field(fields,"tonicLetter",0,6,r);b.chord.origin.accidental=field(fields,"tonicAccidental",-1,1,r);
    b.chord.origin.mode=static_cast<Mode>(field(fields,"mode",0,1,r));
    b.chord.degree=field(fields,"degree",0,6,r);b.chord.octave=field(fields,"octave",1,5,r);
    b.chord.inversion=field(fields,"inversion",0,3,r);b.chord.seventh=field(fields,"seventh",0,1,r)!=0;
    b.chord.suspension=static_cast<Suspension>(field(fields,"suspension",0,2,r));
    if(field(fields,"theorySchema",1,1,r)!=1 || !valid(b.chord))r.fail();return b;
}
}
std::string saveProgression(const Timeline& t) {
    if(!validate(t))throw std::invalid_argument("Cannot save invalid progression");
    std::ostringstream out;out.imbue(std::locale::classic());out<<"{\"schema\":1,\"bars\":"<<t.bars<<",\"blocks\":[";
    bool first=true;
    for(auto& b:t.blocks){if(!first)out<<',';first=false;auto& c=b.chord;
        out<<"{\"id\":"<<b.id<<",\"start\":"<<b.start<<",\"end\":"<<b.end<<",\"tonicLetter\":"<<c.origin.letter
           <<",\"tonicAccidental\":"<<c.origin.accidental<<",\"mode\":"<<static_cast<int>(c.origin.mode)<<",\"degree\":"<<c.degree
           <<",\"octave\":"<<c.octave<<",\"inversion\":"<<c.inversion<<",\"seventh\":"<<(c.seventh ? 1 : 0)
           <<",\"suspension\":"<<static_cast<int>(c.suspension)<<",\"theorySchema\":1}";
    }
    out<<"]}\n";return out.str();
}
Timeline loadProgression(std::string_view json) {
    Reader r(json);Timeline result;std::map<std::string,bool> seen;r.need('{');
    do {
        auto name=r.key();if(!seen.emplace(name,true).second)r.fail();
        if(name=="schema"){if(r.integer()!=1)r.fail();}
        else if(name=="bars"){auto bars=r.integer();if(bars<1 || bars>32)r.fail();result.bars=static_cast<int>(bars);}
        else if(name=="blocks") {
            r.need('[');if(!r.take(']'))do {if(result.blocks.size()>=512)r.fail();result.blocks.push_back(readBlock(r));if(r.take(']'))break;r.need(',');}while(true);
        } else r.fail();
        if(r.take('}'))break;r.need(',');
    }while(true);
    r.finish();if(seen.size()!=3 || !validate(result))r.fail();return result;
}
}

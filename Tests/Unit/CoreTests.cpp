#include "Theory/Theory.h"
#include "Commands/Timeline.h"
#include "Export/Midi.h"
#include "Audio/Engine.h"
#include "Persistence/Progression.h"
#include "Model/Session.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <thread>
#include <filesystem>
#include <climits>

using namespace cc;
int assertions=0;
void check(bool value,const char* name) { ++assertions;if(!value){std::cerr<<"FAIL: "<<name<<'\n';std::exit(1);} }
void notes(const ResolvedChord& r,std::initializer_list<int> expected) { check(r.count==static_cast<int>(expected.size()),"note count");check(std::equal(expected.begin(),expected.end(),r.notes.begin()),"exact pitches"); }
void theory() {
    auto inventory=keys();check(inventory.size()==42,"42 written keys");
    std::set<std::string> names;for(auto k:inventory)names.insert(keyLabel(k));check(names.size()==42,"unique identities");
    notes(resolve({}),{60,64,67});Chord c;c.inversion=1;notes(resolve(c),{64,67,72});c.inversion=2;notes(resolve(c),{67,72,76});
    c.seventh=true;c.inversion=3;notes(resolve(c),{71,72,76,79});setSeventh(c,false);check(c.inversion==2,"seventh clamp");
    std::array<std::string,7> labels {"C","Dm","Em","F","G","Am","Bdim"};
    for(int d=0;d<7;++d)check(resolve({{},d}).label==labels[d],"C major palette");
    check(resolve({{0,1,Mode::major},2}).label=="E♯m","retain E sharp");
    check(resolve({{0,-1,Mode::major},3}).label=="F♭","retain F flat");
    check(scale({4,1,Mode::major})[6]==PitchName{3,2},"theoretical F double sharp");
    check(resolve({{4,1,Mode::major},6}).label=="Gdim","display simplification");
    c={};c.seventh=true;c.suspension=Suspension::sus4;check(resolve(c).label=="Cmaj7sus4","suspended major seventh name");notes(resolve(c),{60,65,67,71});
    // Independently tabulated degree qualities from the acceptance fixtures.
    constexpr int rootClasses[2][7]={{0,2,4,5,7,9,11},{0,2,3,5,7,8,10}};
    constexpr int thirds[2][7]={{4,3,3,4,4,3,3},{3,3,4,3,3,4,4}};
    constexpr int fifths[2][7]={{7,7,7,7,7,7,6},{7,6,7,7,7,7,7}};
    constexpr int sevenths[2][7]={{11,10,10,11,10,10,10},{10,10,11,10,10,11,10}};
    constexpr int tonicClasses[7]={0,2,4,5,7,9,11};
    const std::string seventhNames[2][7]={{"maj7","m7","m7","maj7","7","m7","m7♭5"},{"m7","m7♭5","maj7","m7","m7","maj7","7"}};
    int combinations=0,minNote=127,maxNote=0;
    for(auto k:inventory)for(int d=0;d<7;++d)for(int o=1;o<=5;++o)for(int sus=0;sus<3;++sus)for(int seventh=0;seventh<2;++seventh)for(int inv=0;inv<(seventh ? 4 : 3);++inv) {
        Chord chord{k,d,o,inv,seventh!=0,static_cast<Suspension>(sus)};auto r=resolve(chord);
        int m=k.mode==Mode::minor ? 1 : 0;
        if(seventh && !sus)check(r.label==r.rootLabel+seventhNames[m][d],"independent degree seventh quality names");
        int tonic=(tonicClasses[k.letter]+k.accidental+12)%12;
        int base=60+tonic+rootClasses[m][d]+12*(o-3);
        std::vector<int> oracle {base,base+(sus==1 ? 2 : sus==2 ? 5 : thirds[m][d]),base+(sus ? 7 : fifths[m][d])};
        if(seventh)oracle.push_back(base+sevenths[m][d]);
        for(int i=0;i<inv;++i){int first=oracle.front()+12;oracle.erase(oracle.begin());oracle.push_back(first);}
        check(std::equal(oracle.begin(),oracle.end(),r.notes.begin()),"exhaustive independent interval oracle");
        for(int n=0;n<r.count;++n){check(r.notes[n]>=0 && r.notes[n]<=127,"MIDI range");if(n)check(r.notes[n]>r.notes[n-1],"ordered distinct voices");minNote=std::min(minNote,r.notes[n]);maxNote=std::max(maxNote,r.notes[n]);}
        ++combinations;
    }
    check(combinations==30870,"Cartesian coverage");check(minNote==36 && maxNote==125,"specified register extrema");
    std::cout<<"Theory: "<<combinations<<" combinations; MIDI "<<minNote<<".."<<maxNote<<'\n';
}
void timeline() {
    Document d;check(d.state().bars==8,"initial eight bars");check(d.add({},0),"add A");check(d.add({},bar),"adjacent B");
    check(!intersects(d.state().blocks[0],d.state().blocks[1]),"shared endpoint has no overlap");
    check(intersects({1,0,3840,{}},{2,3839,7680,{}}),"positive one-tick overlap");
    auto original=d.state();check(d.add({},2880),"replace intersections");check(d.state().blocks.size()==1 && d.state().blocks[0].end==6720,"whole victims original length");check(d.undo() && d.state()==original,"atomic undo");
    check(d.move({original.blocks[0].id},960),"self collision exclusion");check(d.state().blocks.size()==1,"other overlap removed");
    Document gaps;gaps.add({},0);gaps.add({},7680);auto copied=gaps.copy({gaps.state().blocks[0].id,gaps.state().blocks[1].id});gaps.add({},3840);
    check(gaps.paste(copied,0),"paste group");check(gaps.state().blocks.size()==3,"gap destination survives");
    check(gaps.paste(copied,120960),"partial paste");check(gaps.state().blocks.back().end==hardEnd && gaps.state().blocks.back().start==120960,"crossing clipboard clipped");
    auto stable=gaps.state();check(!gaps.paste(copied,hardEnd-240,240) && gaps.state()==stable,"subminimum paste no-op");
    auto bad=stable;bad.blocks[0].end=bad.blocks[0].start;check(!gaps.load(bad) && gaps.state()==stable,"failed document validation leaves old snapshot intact");
    Document trim;trim.length(16);trim.insert({{0,28800,32640,{}},{0,34560,38400,{}}});auto before=trim.state();check(trim.length(8),"shorten");check(trim.state().blocks.size()==1 && trim.state().blocks[0].end==30720,"destructive trim");trim.length(16);check(trim.state().blocks[0].end==30720,"extension never resurrects");trim.undo();trim.undo();check(trim.state()==before,"undo restores trim");
    Document cut;cut.add({},0);uint64_t id=cut.state().blocks[0].id;check(!cut.slice(id,480,240),"slice minimum");check(cut.slice(id,1920),"slice halves");check(cut.state().blocks.size()==2 && cut.state().blocks[0].end==1920,"slice geometry");cut.undo();check(cut.state().blocks.size()==1,"slice undo");
    Document history;for(int i=0;i<21;++i)check(history.add({},i*bar),"history edits");check(history.undoCount()==20,"history cap");for(int i=0;i<20;++i)check(history.undo(),"history undo");check(!history.undo() && history.state().blocks.size()==1,"oldest transaction unavailable");history.add({},0);check(!history.redo(),"redo branch invalidation");
    std::mt19937 random(20260918);Document fuzz;
    for(int i=0;i<20000;++i) {
        auto old=fuzz.state();
        int op=static_cast<int>(random()%9),pos=static_cast<int>(random()%(hardEnd+3840));
        std::vector<uint64_t> ids;for(auto& b:fuzz.state().blocks)if(random()%3==0)ids.push_back(b.id);
        if(op==0)fuzz.add({{},static_cast<int>(random()%7)},pos,240);
        if(op==1)fuzz.length(1+static_cast<int>(random()%32));
        if(op==2)fuzz.move(ids,pos,480);
        if(op==3 && !ids.empty())fuzz.resize(ids[0],random()%2!=0,pos,240);
        if(op==4 && !ids.empty())fuzz.slice(ids[0],pos,240);
        if(op==5)fuzz.paste(fuzz.copy(ids),pos,240);
        if(op==6)fuzz.erase(ids);
        if(op==7)fuzz.undo();if(op==8)fuzz.redo();
        check(validate(fuzz.state()),"fuzz invariant");
        for(auto& b:fuzz.state().blocks){auto pitches=resolve(b.chord);for(int n=0;n<pitches.count;++n)check(pitches.notes[n]>=0 && pitches.notes[n]<=127,"fuzz note range");}
        if(op<7 && old!=fuzz.state()) {
            auto changed=fuzz.state();check(fuzz.undo() && fuzz.state()==old,"fuzz undo restores complete old snapshot");
            check(fuzz.redo() && fuzz.state()==changed,"fuzz redo restores complete new snapshot");
        }
    }
    std::cout<<"Timeline: 20,000 deterministic random commands\n";
}
void mailbox() {
    FrameMailbox queue;std::atomic<bool> done=false;
    std::thread producer([&]{for(uint64_t n=1;n<=30000;++n){AudioFrame f;f.revision=n;f.previewOwner=n;f.seekSerial=n;queue.publish(f);}done=true;});
    AudioFrame f;uint64_t previous=0;
    while(!done.load()){if(queue.consume(f)){check(f.revision==f.previewOwner && f.revision==f.seekSerial,"untorn frame");check(f.revision>=previous,"monotonic publication");previous=f.revision;}}
    producer.join();std::cout<<"Mailbox: 30,000 concurrent publications\n";
}
void persistence() {
    Document d;d.add({{3,-1,Mode::minor},4,2,3,true,Suspension::sus2},1920);
    d.add({{0,1,Mode::major},6,5,1,false,Suspension::none},7680);
    auto saved=saveProgression(d.state());check(loadProgression(saved)==d.state(),"mixed origin progression round trip");
    Document fresh;check(fresh.load(loadProgression(saved)),"manual load");check(fresh.undo() && fresh.state()==Timeline{},"load single undo");
    for(auto invalid:{std::string("{}"),std::string("{\"schema\":2,\"bars\":8,\"blocks\":[]}"),std::string("{\"schema\":1,\"bars\":8,\"blocks\":[],\"schema\":1}"),saved+"extra",std::string(1024*1024+1,' ')}) {
        bool rejected=false;try{loadProgression(invalid);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"malformed progression rejected");
    }
    auto overlap=d.state();overlap.blocks[1].start=1920;bool rejected=false;try{saveProgression(overlap);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"invalid document save rejected");
}
void session() {
    Session s;Engine e;e.prepare(48000);s.publish=[&](const AudioFrame& f){e.input.publish(f);};float left[256],right[256];
    auto process=[&]{e.process(left,right,256,{120,false});};
    check(s.key==Key{} && s.document.state()==Timeline{} && s.pads[0].octave==3 && !s.repeats && !s.sync,"fresh session defaults");
    s.pressPad(0,true);process();check(s.activePad()==0 && e.status().overrideActive,"keyboard hold");auto count=e.articulations();s.pressPad(0,true);process();check(e.articulations()==count,"suppress OS repeats");
    s.pressPad(1,true);s.releasePad(0,true);process();check(s.activePad()==1,"older key release preserves newest owner");s.loseFocus();process();check(!e.status().overrideActive,"focus releases momentary");
    s.setRepeats(true);s.pressPad(0);process();s.releasePad(0);check(s.repeatLatched(),"mouse up retains repeat");s.loseFocus();check(s.repeatLatched(),"focus retains latch");
    s.cancelPreview();process();check(!s.repeatLatched() && s.activePad()==-1 && !e.status().overrideActive,"gesture/lifecycle cancellation releases latched preview");s.pressPad(0);process();
    auto ownerBefore=e.articulations();s.pressPad(0);s.pressPad(0);process();check(e.articulations()>ownerBefore,"coalesced stop-start rearticulates");
    s.edit([&](auto& d){return d.add(s.pads[0],0);});auto block=s.document.state().blocks[0];s.changeKey({3,0,Mode::minor});check(s.document.state().blocks[0]==block && s.pads[0].origin==s.key,"key changes preserve block snapshot");
    s.play();process();check(!s.repeatLatched() && e.status().running && !e.status().overrideActive,"local play clears preview latch");
    s.selectAll();s.copy();s.paste(7680);check(s.selected.size()==1 && s.document.find(s.selected[0])->start==7680,"paste selects incoming");
    auto live=s.document.state();s.receiveHostState(Session::hostState());check(s.document.state()==live,"host state never erases live work");Session fresh;fresh.receiveHostState(Session::hostState());check(fresh.document.state()==Timeline{},"host restoration starts fresh");
    s.pressBlock(s.selected[0]);s.removeSelected();process();check(s.activeBlock()==0,"deleting preview owner releases source");
    AudioFrame f;f.preview=noteSet({});f.previewOwner=1;f.repeating=true;e.input.publish(f);process();e.process(left,right,256,{120,false},true);process();check(!e.status().overrideActive && e.status().voices==0,"bypass clears stale latch");
    auto stable=s.document.state();check(!s.document.add({},0,0) && s.document.state()==stable,"invalid grid no-op");check(!s.document.insert({{0,0,INT_MIN,{}}}),"malformed negative interval rejected");
    check(!validate({8,{{1,1,INT_MIN,{}}}}),"malformed interval rejected before signed subtraction");
    check(!validate({8,{{UINT64_MAX,0,ppq,{}}}}),"maximum identifier cannot wrap allocator");
    check(!s.document.slice(s.document.state().blocks[0].id,INT_MIN),"extreme slice position rejected before subtraction");
}
void audio(const std::filesystem::path& output) {
    float left[256],right[256];
    for(auto sound:{Sound::piano,Sound::guitar,Sound::strings,Sound::pad}) {
        Engine engine;engine.prepare(48000);AudioFrame frame;frame.preview=noteSet({});frame.previewOwner=1;frame.sound=sound;engine.input.publish(frame);
        double power=0,peak=0;std::ofstream raw(output/(std::to_string(static_cast<int>(sound))+".f32"),std::ios::binary);
        for(int b=0;b<375;++b){engine.process(left,right,256,{120,false});for(auto value:left){check(std::isfinite(value),"finite audio");power+=value*value;peak=std::max(peak,std::abs(static_cast<double>(value)));}raw.write(reinterpret_cast<char*>(left),sizeof(left));}
        check(power>0.01 && peak<1,"audible bounded sound");frame.previewOwner=0;engine.input.publish(frame);for(int b=0;b<400;++b)engine.process(left,right,256,{120,false});check(engine.status().voices==0,"released voices expire");
        std::cout<<"Sound "<<static_cast<int>(sound)<<": RMS "<<std::sqrt(power/96000)<<", peak "<<peak<<'\n';
    }
    for(double sr:{44100.0,48000.0,96000.0})for(double bpm:{60.0,120.0,180.0})for(int interval:{480,960,1920,3840,7680}) {
        Engine engine;engine.prepare(sr);AudioFrame f;f.preview=noteSet({});f.previewOwner=1;f.repeating=true;f.repeatTicks=interval;engine.input.publish(f);
        int total=static_cast<int>(sr*interval/ppq*60/bpm*3.2),processed=0;
        while(processed<total){int n=std::min(127,total-processed);engine.process(left,right,n,{bpm,false});processed+=n;}
        check(engine.articulations()==4,"repeat exact sample timing");
    }
    Document d;d.add({},0);auto f=audioFrame(d.state(),d.revision());Engine e;e.prepare(48000);f.sync=true;e.input.publish(f);e.process(left,right,256,{120,false});check(!e.status().running,"sync armed");e.process(left,right,256,{120,true});check(e.status().running && e.status().tick<20,"start at own zero");for(int i=0;i<100;++i)e.process(left,right,256,{120,true});double phase=e.status().tick;e.process(left,right,256,{120,false});check(!e.status().running && e.status().tick==phase,"host stop freezes");e.process(left,right,256,{120,true});check(e.status().tick<20,"restart own zero");e.process(left,right,256,{120,true,3,4});check(!e.status().running,"unsupported meter stops");e.process(left,right,256,{NAN,false});check(std::isfinite(e.status().bpm),"retain valid tempo");
}
int main(int argc,char** argv) {
    std::filesystem::path out=argc>1 ? argv[1] : ".";
    theory();timeline();mailbox();persistence();session();audio(out);
    Document d;d.add({},bar);Chord c;c.inversion=1;d.insert({{0,2*bar,3*bar,c}});
    Document extent;extent.add({},bar);extent.insert({{0,3*bar,4*bar,c}});
    for(auto pair:std::initializer_list<std::pair<std::string,Timeline>>{{"chords.mid",d.state()},{"empty.mid",Timeline{}},{"extent-gate.mid",extent.state()}}){auto data=midi(pair.second);std::ofstream file(out/pair.first,std::ios::binary);file.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));check(file.good(),"MIDI fixture write");}
    std::cout<<"PASS: "<<assertions<<" assertions\n";
}

#include "Model/Session.h"
#include <iostream>
#include <stdexcept>
#include <string>
using namespace cc;
void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
struct Observation {std::string name;int degree,activePad;uint64_t targetBlock,revision,owner;bool keyboard,latched;std::vector<uint64_t> selected;};
int main(){try{
    Session session;AudioFrame published;std::vector<Observation> events;
    session.publish=[&](const AudioFrame& frame){published=frame;};
    session.semanticEvent=[&](const char* name,int degree,uint64_t block,bool keyboard){events.push_back({name,degree,session.activePad(),block,session.document.revision(),published.previewOwner,keyboard,session.repeatLatched(),session.selected});};
    session.pressPad(0,true);auto first=events.back();session.pressPad(0,true);check(events.size()==1 && first.name=="pad.press" && first.keyboard && first.degree==0 && first.owner!=0,"one semantic keyboard press despite OS repeat");
    session.pressPad(1,true);auto latest=events.back();session.releasePad(0,true);check(events.back().name=="pad.release" && events.back().activePad==1 && events.back().owner==latest.owner,"older release explains target while preserving newer active ownership");
    session.releasePad(1,true);check(events.back().activePad==-1 && published.previewOwner==0,"owning key release publishes cleanup before observation");
    session.setRepeats(true);session.pressPad(2);auto latched=events.back();session.releasePad(2);check(latched.latched && events.back().latched,"release event distinguishes continuing latch");
    session.setRepeatRate(480);check(events.back().name=="repeat.rate" && published.repeatTicks==480,"rate observation follows actual frame change");session.pressPad(2);check(!events.back().latched && published.previewOwner==0,"same-pad latch stop has explicit final ownership");
    session.edit([](auto& document){return document.add({},0);});auto id=session.document.state().blocks.front().id;
    session.setSelection({id,id,UINT64_MAX});check(events.back().selected==std::vector<uint64_t>{id},"selection observation validates IDs and removes duplicates");
    session.setSelection(std::vector<uint64_t>(513,id));check(events.back().name=="selection.invalid" && session.selected==std::vector<uint64_t>{id},"oversized selection input preserves last valid selection");
    session.copy();check(events.back().name=="clipboard.copy" && session.clipboard.size()==1,"clipboard event occurs after complete copied state");
    check(session.paste(bar*2),"paste complete copied block");check(events.back().name=="selection.inserted" && events.back().selected.size()==1 && events.back().selected.front()!=id && events.back().revision==session.document.revision(),"post-paste observation captures actual new selection instead of pre-selection snapshot");
    auto newId=session.selected.front();session.pressBlock(newId);check(events.back().name=="block.press" && events.back().targetBlock==newId && published.previewOwner!=0,"block preview identifies immutable target");session.releaseMomentary();
    session.play();check(events.back().name=="transport.play" && published.localRun,"local play observation follows request");session.seek(bar+ppq);check(events.back().name=="transport.seek" && published.seekTick==bar+ppq,"seek reflects snapped local tick");session.stop();session.setSync(true);auto count=events.size();session.play();check(events.size()==count && !published.localRun && published.sync,"disabled host-owned local command creates no fake operation");
    session.semanticEvent=[](const char*,int,uint64_t,bool){throw std::runtime_error("synthetic diagnostic failure");};session.setRepeats(false);session.pressPad(4);session.releasePad(4);check(session.diagnosticCallbackFailures()==3 && session.activePad()==-1 && published.previewOwner==0,"failed diagnostic callback cannot interrupt valid release/model/audio publication");
    auto before=session.document.revision();session.actionEvent=[](const std::string&,const Timeline&,uint64_t){throw std::runtime_error("synthetic transaction diagnostic failure");};check(session.edit([](auto& document){return document.add({},bar*4);}) && session.document.revision()==before+1 && published.revision==before+1 && session.diagnosticCallbackFailures()==4,"transaction diagnostic failure cannot falsely fail an already committed musical edit");
    std::cout<<"PASS: message-thread semantic ownership, keyboard-repeat suppression, latch/rate/transport, validated selection, post-paste context and diagnostic exception isolation; no GUI/host claim\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

#include "Session.h"
#include <algorithm>

namespace cc {
Session::Session() { for(int d=0;d<7;++d)pads[d]={key,d}; }
void Session::observe(const char* name,int degree,uint64_t block,bool keyboard) noexcept {
    if(!semanticEvent)return;
    try{semanticEvent(name,degree,block,keyboard);}catch(...){if(observationFailures!=UINT64_MAX)++observationFailures;}
}
void Session::reconcile() {
    std::erase_if(selected,[&](auto id){return !document.find(id);});
    if(previewBlock && !document.find(previewBlock))previewBlock=0;
}
bool Session::edit(const std::function<bool(Document&)>& op) {
    if(!op(document))return false;reconcile();send();
    if(actionEvent)try{actionEvent(document.lastAction(),document.state(),document.revision());}catch(...){if(observationFailures!=UINT64_MAX)++observationFailures;}return true;
}
void Session::setSelection(std::vector<uint64_t> ids) {
    if(ids.size()>512){observe("selection.invalid");return;}
    std::vector<uint64_t> next;next.reserve(ids.size());for(auto id:ids)if(document.find(id) && std::find(next.begin(),next.end(),id)==next.end())next.push_back(id);selected=std::move(next);observe("selection.change");
}
void Session::select(uint64_t id,bool toggle) {
    if(!document.find(id))return;
    if(!toggle){selected={id};observe("selection.change",-1,id);return;}
    auto i=std::find(selected.begin(),selected.end(),id);if(i==selected.end())selected.push_back(id);else selected.erase(i);
    observe("selection.change",-1,id);
}
void Session::selectAll() { selected.clear();for(auto& b:document.state().blocks)selected.push_back(b.id);observe("selection.all"); }
bool Session::removeSelected() { return edit([&](Document& d){return d.erase(selected);}); }
void Session::copy() { clipboard=document.copy(selected);observe("clipboard.copy"); }
void Session::selectNew(const std::vector<uint64_t>& old) { selected.clear();for(auto& b:document.state().blocks)if(std::find(old.begin(),old.end(),b.id)==old.end())selected.push_back(b.id);observe("selection.inserted"); }
bool Session::paste(int cursor) {
    std::vector<uint64_t> old;for(auto& b:document.state().blocks)old.push_back(b.id);
    if(!edit([&](auto& d){return d.paste(clipboard,cursor,settings.editGrid);}))return false;selectNew(old);return true;
}
bool Session::duplicate() {
    std::vector<uint64_t> old;for(auto& b:document.state().blocks)old.push_back(b.id);
    if(!edit([&](auto& d){return d.duplicate(selected,settings.editGrid);}))return false;selectNew(old);return true;
}
bool Session::undo() { return edit([](auto& d){return d.undo();}); }
bool Session::redo() { return edit([](auto& d){return d.redo();}); }
void Session::changeKey(Key next) {
    if(!valid(next)){observe("key.invalid");return;}key=next;for(int d=0;d<7;++d)pads[d]={key,d};send();observe("key.change");
}
void Session::resetPad(int d) { if(d<0 || d>=7)return;pads[d]={key,d};send();observe("pad.reset",d); }
void Session::changePad(int d,Chord next) {
    if(d<0 || d>=7)return;next.origin=key;next.degree=d;if(!valid(next)){observe("pad.invalid",d);return;}pads[d]=next;send();observe("pad.voicing",d);
}
void Session::pressPad(int d,bool keyboard) {
    if(d<0 || d>=7)return;
    if(keyboard){if(heldKeys[d])return;heldKeys[d]=true;}
    if(repeats && latched && previewPad==d){previewPad=-1;latched=false;}
    else {previewPad=d;previewBlock=0;latched=repeats;keyOwner=keyboard;++previewSerial;}
    send();observe("pad.press",d,0,keyboard);
}
void Session::releasePad(int d,bool keyboard) {
    if(d<0 || d>=7)return;if(keyboard)heldKeys[d]=false;
    if(!latched && previewPad==d && keyOwner==keyboard){previewPad=-1;send();}observe("pad.release",d,0,keyboard);
}
void Session::pressBlock(uint64_t id) { if(!document.find(id))return;previewBlock=id;previewPad=-1;latched=false;++previewSerial;send();observe("block.press",-1,id); }
void Session::releaseMomentary() { if(latched)return;previewPad=-1;previewBlock=0;heldKeys={};send();observe("preview.release"); }
void Session::cancelPreview() { previewPad=-1;previewBlock=0;latched=false;heldKeys={};send();observe("preview.cancel"); }
void Session::loseFocus() { heldKeys={};if(!latched)releaseMomentary();observe("preview.focus_cleanup"); }
void Session::setRepeats(bool enabled) { repeats=enabled;if(!enabled && latched){latched=false;previewPad=-1;}send();observe("repeat.mode"); }
void Session::setRepeatRate(int ticks) { if(ticks!=480 && ticks!=960 && ticks!=1920 && ticks!=3840 && ticks!=7680){observe("repeat.invalid_rate");return;}repeatTicks=ticks;send();observe("repeat.rate"); }
void Session::play() { if(sync)return;previewPad=-1;previewBlock=0;latched=false;localRun=true;++transportSerial;send();observe("transport.play"); }
void Session::stop() { if(sync)return;localRun=false;++transportSerial;send();observe("transport.stop"); }
void Session::seek(int position) { seekTick=std::clamp(snap(position,settings.editGrid),0,document.state().bars*bar-1);++seekSerial;send();observe("transport.seek"); }
void Session::setSync(bool enabled) { sync=enabled;localRun=false;++transportSerial;send();observe("transport.sync"); }
void Session::send() {
    if(!publish)return;auto f=audioFrame(document.state(),document.revision());
    f.sound=sound;f.gain=gain;f.sync=sync;f.localRun=localRun;f.transportSerial=transportSerial;f.seekSerial=seekSerial;f.seekTick=seekTick;
    if(previewPad>=0){f.preview=noteSet(pads[previewPad]);f.previewOwner=previewSerial;}
    else if(auto b=document.find(previewBlock)){f.preview=noteSet(b->chord);f.previewOwner=previewSerial;}
    f.repeating=latched;f.repeatTicks=repeatTicks;publish(f);
}
std::string Session::hostState() { return "{\"product\":\"ChordCanvas\",\"hostSchema\":1}"; }
}

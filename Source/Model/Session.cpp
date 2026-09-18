#include "Session.h"
#include <algorithm>

namespace cc {
Session::Session() { for(int d=0;d<7;++d)pads[d]={key,d}; }
void Session::reconcile() {
    std::erase_if(selected,[&](auto id){return !document.find(id);});
    if(previewBlock && !document.find(previewBlock))previewBlock=0;
}
bool Session::edit(const std::function<bool(Document&)>& op) {
    if(!op(document))return false;reconcile();send();
    if(actionEvent)actionEvent(document.lastAction(),document.state(),document.revision());return true;
}
void Session::select(uint64_t id,bool toggle) {
    if(!document.find(id))return;
    if(!toggle){selected={id};return;}
    auto i=std::find(selected.begin(),selected.end(),id);if(i==selected.end())selected.push_back(id);else selected.erase(i);
}
void Session::selectAll() { selected.clear();for(auto& b:document.state().blocks)selected.push_back(b.id); }
bool Session::removeSelected() { return edit([&](Document& d){return d.erase(selected);}); }
void Session::copy() { clipboard=document.copy(selected); }
void Session::selectNew(const std::vector<uint64_t>& old) { selected.clear();for(auto& b:document.state().blocks)if(std::find(old.begin(),old.end(),b.id)==old.end())selected.push_back(b.id); }
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
    if(!valid(next))return;key=next;for(int d=0;d<7;++d)pads[d]={key,d};send();
}
void Session::resetPad(int d) { if(d<0 || d>=7)return;pads[d]={key,d};send(); }
void Session::changePad(int d,Chord next) {
    if(d<0 || d>=7)return;next.origin=key;next.degree=d;if(!valid(next))return;pads[d]=next;send();
}
void Session::pressPad(int d,bool keyboard) {
    if(d<0 || d>=7)return;
    if(keyboard){if(heldKeys[d])return;heldKeys[d]=true;}
    if(repeats && latched && previewPad==d){previewPad=-1;latched=false;}
    else {previewPad=d;previewBlock=0;latched=repeats;keyOwner=keyboard;++previewSerial;}
    send();
}
void Session::releasePad(int d,bool keyboard) {
    if(d<0 || d>=7)return;if(keyboard)heldKeys[d]=false;
    if(!latched && previewPad==d && keyOwner==keyboard){previewPad=-1;send();}
}
void Session::pressBlock(uint64_t id) { if(!document.find(id))return;previewBlock=id;previewPad=-1;latched=false;++previewSerial;send(); }
void Session::releaseMomentary() { if(latched)return;previewPad=-1;previewBlock=0;heldKeys={};send(); }
void Session::cancelPreview() { previewPad=-1;previewBlock=0;latched=false;heldKeys={};send(); }
void Session::loseFocus() { heldKeys={};if(!latched)releaseMomentary(); }
void Session::setRepeats(bool enabled) { repeats=enabled;if(!enabled && latched){latched=false;previewPad=-1;}send(); }
void Session::setRepeatRate(int ticks) { if(ticks!=480 && ticks!=960 && ticks!=1920 && ticks!=3840 && ticks!=7680)return;repeatTicks=ticks;send(); }
void Session::play() { if(sync)return;previewPad=-1;previewBlock=0;latched=false;localRun=true;++transportSerial;send(); }
void Session::stop() { if(sync)return;localRun=false;++transportSerial;send(); }
void Session::seek(int position) { seekTick=std::clamp(snap(position,settings.editGrid),0,document.state().bars*bar-1);++seekSerial;send(); }
void Session::setSync(bool enabled) { sync=enabled;localRun=false;++transportSerial;send(); }
void Session::send() {
    if(!publish)return;auto f=audioFrame(document.state(),document.revision());
    f.sound=sound;f.gain=gain;f.sync=sync;f.localRun=localRun;f.transportSerial=transportSerial;f.seekSerial=seekSerial;f.seekTick=seekTick;
    if(previewPad>=0){f.preview=noteSet(pads[previewPad]);f.previewOwner=previewSerial;}
    else if(auto b=document.find(previewBlock)){f.preview=noteSet(b->chord);f.previewOwner=previewSerial;}
    f.repeating=latched;f.repeatTicks=repeatTicks;publish(f);
}
std::string Session::hostState() { return "{\"product\":\"ChordCanvas\",\"hostSchema\":1}"; }
}

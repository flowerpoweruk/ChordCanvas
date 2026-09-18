#pragma once
#include "Commands/Timeline.h"
#include "Audio/Engine.h"
#include <functional>

namespace cc {
// Processor-owned live session: editor creation does not reconstruct/reset it.
// Host state deliberately has no progression or session-settings representation.
struct Settings {
    bool showSeventh=false,showSus2=false,showSus4=false;
    int editGrid=ppq,razorGrid=ppq;
};
class Session {
public:
    Session();
    Document document;
    Key key;
    std::array<Chord,7> pads;
    Settings settings;
    Sound sound=Sound::piano;
    float gain=0.25118864f;
    bool repeats=false,sync=false;
    int repeatTicks=ppq;
    std::vector<uint64_t> selected;
    std::vector<Block> clipboard;
    std::function<void(const AudioFrame&)> publish;
    std::function<void(const std::string&,const Timeline&,uint64_t)> actionEvent;
    bool edit(const std::function<bool(Document&)>& operation);
    void select(uint64_t id,bool toggle);
    void selectAll();
    bool removeSelected();
    void copy();
    bool paste(int cursor);
    bool duplicate();
    bool undo();bool redo();
    void changeKey(Key next);
    void resetPad(int degree);
    void changePad(int degree,Chord next);
    void pressPad(int degree,bool keyboard=false);
    void releasePad(int degree,bool keyboard=false);
    void pressBlock(uint64_t id);
    void releaseMomentary();
    void loseFocus();
    void setRepeats(bool enabled);
    void setRepeatRate(int ticks);
    void play();void stop();void seek(int tick);void setSync(bool enabled);
    void send();
    int activePad() const { return previewPad; }
    uint64_t activeBlock() const { return previewBlock; }
    bool repeatLatched() const { return latched; }
    static std::string hostState();
    void receiveHostState(std::string_view) {} // Host recalls never mutate a live session.
private:
    int previewPad=-1;
    uint64_t previewBlock=0;
    bool latched=false,keyOwner=false,localRun=false;
    uint64_t transportSerial=0,seekSerial=0;
    uint64_t previewSerial=0;
    int seekTick=0;
    std::array<bool,7> heldKeys {};
    void reconcile();
    void selectNew(const std::vector<uint64_t>& old);
};
}

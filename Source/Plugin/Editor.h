#pragma once
#include "Processor.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace cc {
class Skin;
class Pad;
class Canvas;
class Voicing;
class Editor final : public juce::AudioProcessorEditor, private juce::Timer, private juce::AsyncUpdater, private juce::ScrollBar::Listener {
public:
    explicit Editor(Processor&);
    ~Editor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool) override;
    void focusOfChildComponentChanged(juce::Component::FocusChangeType) override;
    void changed();
    void popover(uint64_t block);
    void fail(const char* operation,const juce::String& message);
    void beginPadDrag(Chord chord,const juce::MouseEvent&);
    void updatePadDrag(const juce::MouseEvent&);
    void finishPadDrag(const juce::MouseEvent&);
    Processor& processor;
private:
    std::unique_ptr<Skin> skin;
    std::array<std::unique_ptr<Pad>,7> pads;
    std::unique_ptr<Canvas> canvas;
    juce::ComboBox key,sound,rate,editGrid,sliceGrid;
    juce::Slider volume;
    juce::TextEditor length;
    juce::TextButton repeats {"Repeats"},settings {"Settings"},play {"Play"},stop {"Stop"},start {"Return to Start"},sync {"Sync"};
    juce::TextButton select {"Select"},razor {"Razor"},undo {"Undo"},redo {"Redo"},minus {"−"},plus {"+"},zoomMinus {"−"},zoomPlus {"+"},fit {"Fit"};
    juce::TextButton save {"Save Progression"},load {"Load Progression"},editSelected {"Edit Selected Chord"},logs {"Open Logs Folder"},back {"Back to Canvas"},closePopover {"Close"};
    juce::ToggleButton seventh {"Seventh controls"},sus2 {"Sus2 controls"},sus4 {"Sus4 controls"};
    juce::Label status,about,logging,popoverTitle;
    std::unique_ptr<Voicing> popup;
    struct PopoverBackground;
    std::unique_ptr<PopoverBackground> popupBackground;
    uint64_t popupOwner=0;
    juce::ScrollBar scrollbar {false};
    struct ExportHandle;
    std::unique_ptr<ExportHandle> exporter;
    std::unique_ptr<juce::FileChooser> chooser;
    bool settingsOpen=false;
    std::array<bool,7> heldKeys {};
    void timerCallback() override;
    void handleAsyncUpdate() override;
    void manualFile(bool saving);
    void applyLength();
    void scrollBarMoved(juce::ScrollBar*,double) override;
    friend class Canvas;
    friend struct ExportHandle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
};
}

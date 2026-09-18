#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Model/Session.h"

namespace cc {
class Processor final : public juce::AudioProcessor, private juce::AsyncUpdater {
public:
    Processor();
    ~Processor() override;
    Session session; // Message-thread model, preserved across editor close/reopen.
    Engine engine;
    void prepareToPlay(double sampleRate,int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    const juce::String getName() const override { return "ChordCanvas"; }
    // Live requires an event input bus for instruments. Incoming MIDI is ignored;
    // audition and playback remain controlled exclusively by the canvas.
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.8; }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override; // Recall intentionally leaves live model unchanged.
    void interactive();
    void event(const char* name,juce::var details=juce::var());
    void snapshot();
    std::shared_ptr<LogService> logs;
    uint64_t instance=0;
    std::atomic<bool> bypassPreviewCleanup {false};
    juce::String loggingFailure;
private:
    struct PublishedState {uint64_t revision=0,previewOwner=0,transportSerial=0,seekSerial=0;int seekTick=0;bool localRun=false;} published;
    juce::String observedKey;
    void semantic(const char*,int degree,uint64_t block,bool keyboard);
    std::atomic<int> lifecycle {0},preparedFrames {0};
    std::atomic<double> preparedRate {0};
    void environment(const char* event);
    HostClock clock() const noexcept;
    void handleAsyncUpdate() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
}

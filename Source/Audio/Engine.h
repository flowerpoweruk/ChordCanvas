#pragma once
#include "Model/Model.h"
#include <atomic>
#include <array>
#include <cstdint>

namespace cc {
enum class Sound { piano, guitar, strings, pad };
struct NoteSet {
    std::array<int,4> notes {};
    int count=0;
    bool operator==(const NoteSet&) const = default;
};
NoteSet noteSet(const Chord& chord);
struct PlaybackBlock { uint64_t id=0;int start=0,end=0;NoteSet chord; };
struct AudioFrame {
    std::array<PlaybackBlock,512> blocks {};
    int blockCount=0,end=8*bar;
    uint64_t revision=0,transportSerial=0,seekSerial=0;
    bool localRun=false,sync=false;
    int seekTick=0;
    NoteSet preview;
    uint64_t previewOwner=0; // zero is no preview; identity changes reset repeat phase
    bool repeating=false;
    int repeatTicks=ppq;
    Sound sound=Sound::piano;
    float gain=0.25118864f;
};
AudioFrame audioFrame(const Timeline& timeline,uint64_t revision);
// Single message-thread producer, single audio-thread consumer. Slot ownership is
// transferred by exchange; neither side accesses the other's mutable slot.
class FrameMailbox {
public:
    void publish(const AudioFrame& value) noexcept;
    bool consume(AudioFrame& value) noexcept;
private:
    std::array<AudioFrame,3> slots {};
    std::atomic<unsigned> middle {1};
    unsigned back=2,front=0;
};
struct HostClock { double bpm=0;bool playing=false;int numerator=4,denominator=4; };
struct EngineStatus { double tick=0,bpm=120;uint64_t block=0;bool running=false,overrideActive=false,meterValid=true,tempoKnown=false;int voices=0; };
class Engine {
public:
    FrameMailbox input;
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void process(float* left,float* right,int count,HostClock clock,bool bypass=false) noexcept;
    EngineStatus status() const noexcept; // Only call from the audio thread; UI uses published atomics.
    std::atomic<double> uiTick {0},uiTempo {120};
    std::atomic<uint64_t> uiBlock {0};
    std::atomic<bool> uiRunning {false},uiOverride {false},uiMeter {true},uiTempoKnown {false};
    uint64_t articulations() const noexcept { return articulationCount; }
private:
    struct Voice { bool active=false,held=false;int note=0;Sound sound=Sound::piano;double phase=0,age=0,release=1; };
    std::array<Voice,32> voices {};
    AudioFrame frame;
    NoteSet sounding;
    uint64_t source=0,articulationCount=0;
    double rate=48000,tick=0,bpm=120,repeatPhase=0;
    float smoothGain=0.25118864f;
    bool running=false,lastHost=false,knownTempo=false,meterValid=true;
    int activeVoices=0;
    void release() noexcept;
    void trigger(NoteSet notes,uint64_t owner) noexcept;
    float sample(Voice& v) noexcept;
    const PlaybackBlock* blockAt(double time) const noexcept;
};
}

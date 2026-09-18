#pragma once
#include <array>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

namespace cc {
struct AudioLogEvent {
    uint32_t kind=0;uint64_t instance=0,revision=0,owner=0;int tick=0,value=0;
    // Kind 3: coalesced callback clock/format observation; no JSON/audio I/O.
    double tempo=0,sampleRate=0;int bufferFrames=0,numerator=0,denominator=0;
    bool tempoAvailable=false,tempoEverKnown=false,hostPlaying=false,bypassed=false;
};
class AudioEventSink {
public:
    virtual ~AudioEventSink()=default;
    virtual bool audioEvent(AudioLogEvent event) noexcept=0;
};
// Bounded MPMC ring: each slot's sequence grants exclusive producer/consumer ownership.
class AudioLogQueue {
public:
    AudioLogQueue();
    bool push(AudioLogEvent event) noexcept;
    bool pop(AudioLogEvent& event) noexcept;
private:
    struct Slot { std::atomic<size_t> sequence {0};AudioLogEvent event; };
    std::array<Slot,256> slots;
    std::atomic<size_t> write {0},read {0};
};
std::filesystem::path logsFolder();
std::string jsonQuote(std::string_view text);
struct LogStatus { bool available=false;std::string reason;uint64_t dropped=0; };
class LogService final : public AudioEventSink {
public:
    explicit LogService(std::filesystem::path root=logsFolder(),size_t storageCap=32*1024*1024,std::string host="unknown",std::string format="unknown");
    ~LogService();
    LogService(const LogService&)=delete;
    LogService& operator=(const LogService&)=delete;
    static std::shared_ptr<LogService> interactive(std::string host="unknown",std::string format="unknown"); // Actual interactive use, never scan.
    void post(uint64_t instance,std::string event,std::string details="{}"); // Non-real-time only.
    bool audioEvent(AudioLogEvent event) noexcept override;
    LogStatus status() const;
    uint64_t instanceId() noexcept { return nextInstance.fetch_add(1); }
private:
    struct Record {uint64_t instance;std::string event,details;};
    std::filesystem::path root;
    size_t cap;
    std::string host,format;
    mutable std::mutex mutex;
    std::condition_variable wake;
    std::deque<Record> pending;
    LogStatus report;
    AudioLogQueue audio;
    std::atomic<uint64_t> dropped {0},nextInstance {1};
    std::atomic<bool> stopping {false};
    std::thread worker;
    void run() noexcept;
};
}

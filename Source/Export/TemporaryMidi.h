#pragma once
#include <filesystem>
#include <memory>
#include <vector>
#include <cstdint>

namespace cc {
// Message/worker-thread operation only. An exclusive lease outlives the native drag.
class TemporaryMidi final {
public:
    static std::shared_ptr<TemporaryMidi> prepare(const std::vector<uint8_t>& bytes,
        const std::filesystem::path& root, size_t capacity=256);
    ~TemporaryMidi();
    const std::filesystem::path& file() const { return path; }
    TemporaryMidi(const TemporaryMidi&)=delete;
    TemporaryMidi& operator=(const TemporaryMidi&)=delete;
private:
    TemporaryMidi()=default;
    std::filesystem::path path;
    void* lease=nullptr;
};
}

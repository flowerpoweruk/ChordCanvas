#pragma once
#include "Version.h"
#include <filesystem>
#include <string>
#include <vector>

namespace cc::packaging {
inline constexpr const char* productIdentity="EE56A5B5-24E6-4F83-B3AF-1C73BCD93594";
inline constexpr const wchar_t* receiptName=L"chordcanvas.payload";
struct OwnedFile { std::string sha256;std::filesystem::path relative; };
struct Payload {
    std::string version;
    std::string receiptHash;
    std::filesystem::path binary;
    std::vector<OwnedFile> files;
    static Payload read(const std::filesystem::path& root);
    void validate(const std::filesystem::path& root,bool hashes=true) const;
};
std::string sha256(const std::filesystem::path& file);
bool plainPath(const std::filesystem::path& path);
}

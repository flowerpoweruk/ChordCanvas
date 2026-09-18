#pragma once
#include <filesystem>
#include <string>

namespace cc {
struct ExecutableVersion { std::string value="unknown",source="unavailable"; };
// Non-real-time, bounded version-resource inspection. No path enters diagnostics.
ExecutableVersion executableProductVersion(const std::filesystem::path& executable);
}

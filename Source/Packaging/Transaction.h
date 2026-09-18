#pragma once
#include "Payload.h"
#include <functional>

namespace cc::packaging {
enum class InstallMode { setup,update };
enum class InstallResult { installed,alreadyCurrent };
// Test-only injection points execute after real operations. Production supplies none.
enum class Boundary { staged,previousRetained,replaced,validated };
InstallResult installBundle(const std::filesystem::path& source,const std::filesystem::path& parent,
    InstallMode mode,const std::function<void(Boundary)>& injection={});
}

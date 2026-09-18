#pragma once
#include "Model/Model.h"
#include <string_view>
namespace cc {
std::string saveProgression(const Timeline& timeline);
Timeline loadProgression(std::string_view json);
}

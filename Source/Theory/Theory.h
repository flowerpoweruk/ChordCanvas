#pragma once
#include <array>
#include <string>
#include <vector>

namespace cc {
constexpr int ppq = 960, bar = 3840, hardEnd = 32 * bar;
enum class Mode { major, minor };
enum class Suspension { none, sus2, sus4 };
struct Key {
    int letter = 0, accidental = 0;
    Mode mode = Mode::major;
    bool operator==(const Key&) const = default;
};
struct PitchName {
    int letter = 0, accidental = 0;
    bool operator==(const PitchName&) const = default;
};
struct Chord {
    Key origin;
    int degree = 0, octave = 3, inversion = 0;
    bool seventh = false;
    Suspension suspension = Suspension::none;
    bool operator==(const Chord&) const = default;
};
struct ResolvedChord {
    std::array<int, 4> notes {};
    std::array<PitchName, 4> names {};
    int count = 3;
    std::string label, rootLabel;
};
int mod12(int value);
bool valid(const Key& key);
bool valid(const Chord& chord);
std::vector<Key> keys();
std::string written(PitchName name);
std::string display(PitchName name, int direction);
std::string keyLabel(const Key& key);
std::array<PitchName, 7> scale(const Key& key);
ResolvedChord resolve(const Chord& chord);
void setSeventh(Chord& chord, bool enabled);
}

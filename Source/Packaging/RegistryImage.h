#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
#include <vector>

namespace cc::packaging {
struct RegistryValue {std::wstring name;DWORD type=0;std::vector<unsigned char> bytes;bool operator==(const RegistryValue&) const=default;};
// Bounded images of one leaf key in the native 64-bit registry view. No recursive
// key operations, ACL changes or arbitrary registry-tree snapshots.
struct RegistryImage {
    bool exists=false;
    std::vector<RegistryValue> values;
    static RegistryImage capture(HKEY hive,const std::wstring& key);
    static RegistryImage read(const std::filesystem::path& file);
    void write(const std::filesystem::path& file) const;
    // New values must belong to the caller's explicit forward-operation list.
    // Unexpected values/subkeys stop before any restoration changes.
    void restore(HKEY hive,const std::wstring& key,const std::vector<std::wstring>& forwardValues) const;
    bool operator==(const RegistryImage&) const=default;
};
}

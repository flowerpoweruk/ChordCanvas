#pragma once
#include <windows.h>
#include <objbase.h>
#include <filesystem>
#include <stdexcept>
inline std::filesystem::path ownedWorkspace(const std::filesystem::path& build,const std::string& prefix){
    GUID id;wchar_t text[40]{};
    if(FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id,text,40))throw std::runtime_error("Synthetic workspace identity failed");
    auto path=build/(prefix+std::to_string(GetCurrentProcessId())+"-"+std::string(text+1,text+37));
    if(!std::filesystem::create_directory(path))throw std::runtime_error("Synthetic workspace already exists; existing files preserved");
    return path;
}

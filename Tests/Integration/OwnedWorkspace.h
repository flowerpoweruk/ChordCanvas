#pragma once
#include <windows.h>
#include <objbase.h>
#include <filesystem>
#include <stdexcept>
inline std::filesystem::path ownedWorkspace(const std::filesystem::path& build,const std::string& prefix){
    GUID id;wchar_t text[40]{};
    if(FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id,text,40))throw std::runtime_error("Synthetic workspace identity failed");
    std::string token;for(auto* p=text+1;p!=text+37;++p)token.push_back(static_cast<char>(*p));
    auto path=build/(prefix+std::to_string(GetCurrentProcessId())+"-"+token);
    if(!std::filesystem::create_directory(path))throw std::runtime_error("Synthetic workspace already exists; existing files preserved");
    return path;
}

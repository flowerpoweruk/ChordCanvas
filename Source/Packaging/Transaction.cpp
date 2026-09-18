#include "Transaction.h"
#include <windows.h>
#include <objbase.h>
#include <algorithm>
#include <set>
#include <exception>

namespace cc::packaging {
namespace {
struct Lock {
    HANDLE mutex=nullptr;
    Lock(){mutex=CreateMutexW(nullptr,FALSE,L"Local\\ChordCanvas.InstallBundle.v1");if(!mutex)throw std::runtime_error("Installation lock failed");
        auto result=WaitForSingleObject(mutex,1000);if(result!=WAIT_OBJECT_0 && result!=WAIT_ABANDONED){CloseHandle(mutex);mutex=nullptr;throw std::runtime_error("Another installation is running");}}
    ~Lock(){if(mutex){ReleaseMutex(mutex);CloseHandle(mutex);}}
};
std::wstring identifier(){GUID id;wchar_t text[40]{};if(FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id,text,40))throw std::runtime_error("Install identity failed");return std::wstring(text+1,36);}
void renameOwned(const std::filesystem::path& from,const std::filesystem::path& to){if(!MoveFileExW(from.c_str(),to.c_str(),MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Installation rename failed; save and close the host, then retry");}
void available(const std::filesystem::path& path){
    if(!std::filesystem::exists(path))return;
    auto handle=CreateFileW(path.c_str(),GENERIC_READ|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("Installed files are in use or inaccessible; save and close the host, then retry");CloseHandle(handle);
}
void removeOwned(const std::filesystem::path& root,const Payload& payload){
    payload.validate(root,false); // Never traverse/delete an unknown file or a reparse point.
    std::set<std::filesystem::path> directories;
    for(auto& file:payload.files){auto path=root/file.relative;std::filesystem::remove(path);auto parent=path.parent_path();while(parent!=root){directories.insert(parent);parent=parent.parent_path();}}
    std::filesystem::remove(root/receiptName);
    std::vector<std::filesystem::path> ordered(directories.begin(),directories.end());
    std::sort(ordered.begin(),ordered.end(),[](auto& a,auto& b){return std::distance(a.begin(),a.end())>std::distance(b.begin(),b.end());});
    for(auto& path:ordered)std::filesystem::remove(path);std::filesystem::remove(root);
}
void ancestorSafety(const std::filesystem::path& path){auto current=path;for(;;){if(!plainPath(current))throw std::runtime_error("Unsafe installation ancestor");auto next=current.parent_path();if(next==current || next.empty())break;current=next;}}
}
InstallResult installBundle(const std::filesystem::path& source,const std::filesystem::path& supplied,InstallMode mode,const std::function<void(Boundary)>& injection){
    Lock lock;auto parent=std::filesystem::absolute(supplied).lexically_normal();ancestorSafety(parent);
    auto payload=Payload::read(source);payload.validate(source);
    auto target=parent/L"ChordCanvas.vst3";
    bool existed=std::filesystem::exists(target);
    if(!existed && mode==InstallMode::update)throw std::runtime_error("ChordCanvas is not installed; run Setup.exe first");
    Payload previous;
    if(existed){
        previous=Payload::read(target);previous.validate(target,false);
        auto installedVersion=Version::parse(previous.version),incomingVersion=Version::parse(payload.version);
        if(installedVersion>incomingVersion)throw std::runtime_error("A newer ChordCanvas version is installed; downgrade refused");
        if(installedVersion==incomingVersion){
            if(sha256(target/receiptName)!=sha256(source/receiptName))throw std::runtime_error("Conflicting payloads use the same release version; replacement refused");
            try{previous.validate(target);return InstallResult::alreadyCurrent;}catch(const std::runtime_error&){/* Owned same-version damaged payload: controlled repair. */}
        }else previous.validate(target);
        for(auto& entry:previous.files)available(target/entry.relative);available(target/receiptName);
    }
    uintmax_t bytes=0;for(auto& file:payload.files)bytes+=std::filesystem::file_size(source/file.relative);
    if(std::filesystem::space(parent).available<bytes+16*1024*1024)throw std::runtime_error("Insufficient installation space");
    auto token=identifier();auto stage=parent/(L".ChordCanvas.stage."+token),backup=parent/(L".ChordCanvas.previous."+token);
    bool staged=false,retained=false,replaced=false;
    try {
        if(!std::filesystem::create_directory(stage))throw std::runtime_error("Staging directory collision");staged=true;
        // Copy the ownership receipt first so interrupted/partial stages remain recognisable.
        std::filesystem::copy_file(source/receiptName,stage/receiptName);
        for(auto& entry:payload.files){auto destination=stage/entry.relative;std::filesystem::create_directories(destination.parent_path());std::filesystem::copy_file(source/entry.relative,destination);}
        payload.validate(stage);if(injection)injection(Boundary::staged);
        if(existed){for(auto& entry:previous.files)available(target/entry.relative);renameOwned(target,backup);retained=true;}
        if(injection)injection(Boundary::previousRetained);
        renameOwned(stage,target);staged=false;replaced=true;if(injection)injection(Boundary::replaced);
        payload.validate(target);if(injection)injection(Boundary::validated);
    }catch(...){
        auto original=std::current_exception();
        try{if(replaced)removeOwned(target,payload);if(retained)renameOwned(backup,target);if(staged){if(std::filesystem::exists(stage/receiptName))removeOwned(stage,payload);else if(std::filesystem::is_empty(stage))std::filesystem::remove(stage);}}
        catch(...){throw std::runtime_error("Installation failed and rollback needs recovery; the retained previous bundle has not been discarded");}
        std::rethrow_exception(original);
    }
    // Cleanup cannot turn a successfully validated swap into destructive rollback.
    // A locked backup is retained rather than forcing its deletion.
    if(retained){try{removeOwned(backup,previous);}catch(const std::exception&){}}
    return InstallResult::installed;
}
}

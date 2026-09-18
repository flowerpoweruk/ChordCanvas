#include "InstallMetadata.h"
#include <shlobj.h>
#include <memory>
#include <string>

namespace {
using namespace cc::packaging;
// Explicit lifetime: never perform filesystem rollback from DLL_PROCESS_DETACH
// under Windows' loader lock. Inno calls Abort on cancellation/normal teardown;
// abrupt termination is recovered from the durable journal on the next launch.
BundleTransaction* active=nullptr;
InstallMetadata* metadata=nullptr;
DWORD ownerThread=0;
constexpr auto key=L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{EE56A5B5-24E6-4F83-B3AF-1C73BCD93594}_is1";
std::filesystem::path known(REFKNOWNFOLDERID id){PWSTR value=nullptr;if(FAILED(SHGetKnownFolderPath(id,KF_FLAG_DEFAULT,nullptr,&value)))throw std::runtime_error("Installation folder unavailable");std::filesystem::path result(value);CoTaskMemFree(value);return result;}
std::filesystem::path product(){return known(FOLDERID_ProgramFilesX64)/L"ChordCanvas";}
std::filesystem::path vst(){return known(FOLDERID_ProgramFilesCommonX64)/L"VST3";}
std::filesystem::path module(){HMODULE dll=nullptr;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&module),&dll))throw std::runtime_error("Installer helper unavailable");std::wstring name(32768,L'\0');auto length=GetModuleFileNameW(dll,name.data(),static_cast<DWORD>(name.size()));if(!length || length>=name.size())throw std::runtime_error("Installer helper path unavailable");name.resize(length);return name;}
bool same(const std::filesystem::path& left,const std::filesystem::path& right){auto a=std::filesystem::absolute(left).lexically_normal().wstring(),b=std::filesystem::absolute(right).lexically_normal().wstring();return CompareStringOrdinal(a.c_str(),static_cast<int>(a.size()),b.c_str(),static_cast<int>(b.size()),TRUE)==CSTR_EQUAL;}
int error(const std::exception& fault){std::string message=fault.what();if(message.find("Setup.exe first")!=std::string::npos)return -2;if(message.find("downgrade refused")!=std::string::npos)return -3;if(message.find("rollback needs recovery")!=std::string::npos)return -5;if(message.find("in use")!=std::string::npos || message.find("close the host")!=std::string::npos)return -4;return -1;}
bool caller(){return !active || ownerThread==GetCurrentThreadId();}
void dispose(){auto* previous=active;active=nullptr;metadata=nullptr;ownerThread=0;delete previous;}
}
extern "C" __declspec(dllexport) int WINAPI CC_Supported(){
    SYSTEM_INFO system{};GetNativeSystemInfo(&system);if(system.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_AMD64)return -10;
    // Same exported native version-query route used by the pinned JUCE Windows
    // implementation; the installer executable also carries a supported-OS manifest.
    auto library=GetModuleHandleW(L"ntdll.dll");if(!library)return -11;
    using Query=LONG(WINAPI*)(OSVERSIONINFOEXW*);auto query=reinterpret_cast<Query>(GetProcAddress(library,"RtlGetVersion"));OSVERSIONINFOEXW info{};info.dwOSVersionInfoSize=sizeof(info);
    if(!query || query(&info)!=0 || info.dwMajorVersion!=10 || info.dwBuildNumber<22000 || info.wProductType!=VER_NT_WORKSTATION)return -11;return 0;
}
extern "C" __declspec(dllexport) const char* WINAPI CC_Version(){return CC_VERSION;}
extern "C" __declspec(dllexport) int WINAPI CC_Begin(const wchar_t* source,const wchar_t* appDirectory,int update){
    try{
        if(CC_Supported()!=0)return -10;if(active || !source || !appDirectory || !caller())return -1;
        auto root=product(),parent=vst();if(!same(appDirectory,root))return -12;
        if(update && !std::filesystem::exists(parent/L"ChordCanvas.vst3") && !std::filesystem::exists(parent/L".ChordCanvas.transaction")){
            if(std::filesystem::exists(root) || RegistryImage::capture(HKEY_LOCAL_MACHINE,key).exists)return -1;return -2;
        }
        auto payload=Payload::read(source);payload.validate(source);if(payload.version!=CC_VERSION)return -13;
        // Validate shared ancestors before creating only the standard VST3 folder.
        auto ancestor=parent.parent_path();for(;;){if(!plainPath(ancestor))return -1;auto next=ancestor.parent_path();if(next.empty() || next==ancestor)break;ancestor=next;}
        if(!std::filesystem::exists(parent))std::filesystem::create_directory(parent);
        auto participant=std::make_shared<InstallMetadata>(root,HKEY_LOCAL_MACHINE,key,module(),payload.version);
        auto transaction=BundleTransaction::begin(source,parent,update ? InstallMode::update : InstallMode::setup,{},participant);
        if(transaction->result()==InstallResult::alreadyCurrent)return 1;
        metadata=participant.get();active=transaction.release();ownerThread=GetCurrentThreadId();return 0;
    }catch(const std::exception& fault){return error(fault);}catch(...){return -1;}
}
extern "C" __declspec(dllexport) int WINAPI CC_Finalise(){
    if(!caller() || !active || !metadata)return -1;
    try{metadata->seal();active->commit();dispose();return 0;}catch(const std::exception& fault){return error(fault);}catch(...){return -1;}
}
extern "C" __declspec(dllexport) int WINAPI CC_Abort(){
    if(!caller())return -1;if(!active)return 0;
    try{active->rollback();dispose();return 0;}catch(...){return -5;}
}
extern "C" __declspec(dllexport) int WINAPI CC_UninstallCheck(const wchar_t* appDirectory){
    try{
        if(CC_Supported()!=0 || !appDirectory || !same(appDirectory,product()))return -1;
        // Serialise the preflight with installation. Uninstall never force-closes
        // a host. Inno unloads this helper before deleting its owned DLL.
        HANDLE lock=CreateMutexW(nullptr,FALSE,L"Global\\ChordCanvas.InstallBundle.EE56A5B5-24E6-4F83-B3AF-1C73BCD93594.v1");if(!lock)return -1;
        struct Unlock {HANDLE value;bool held=false;~Unlock(){if(held)ReleaseMutex(value);CloseHandle(value);}}guard{lock};auto wait=WaitForSingleObject(lock,1000);if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED)return -4;guard.held=true;
        auto parent=vst();if(std::filesystem::exists(parent/L".ChordCanvas.transaction"))return -5;
        auto root=parent/L"ChordCanvas.vst3";auto payload=Payload::read(root);payload.validate(root);InstallMetadata current(product(),HKEY_LOCAL_MACHINE,key,module(),payload.version);current.checkUninstall(payload.version);
        std::vector<std::filesystem::path> files={root/receiptName,product()/L"chordcanvas.install"};
        if(std::filesystem::exists(product()/L"unins000.msg"))files.push_back(product()/L"unins000.msg");for(auto& file:payload.files)files.push_back(root/file.relative);
        // Match update's image-section check; no write operation is performed.
        for(auto& file:files){auto handle=CreateFileW(file.c_str(),GENERIC_READ|GENERIC_WRITE|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)return -4;CloseHandle(handle);}return 0;
    }catch(const std::exception& fault){return error(fault);}catch(...){return -1;}
}

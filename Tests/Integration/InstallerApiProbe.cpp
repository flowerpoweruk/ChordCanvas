#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <iostream>
#include <cstring>
int main(int argc,char** argv){
    if(argc!=2)return 2;auto file=std::filesystem::absolute(std::filesystem::path(argv[1]));auto dll=LoadLibraryExW(file.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);if(!dll){std::cerr<<"FAIL: native installer helper could not load\n";return 1;}
    auto supported=reinterpret_cast<int(WINAPI*)()>(GetProcAddress(dll,"CC_Supported"));auto version=reinterpret_cast<const char*(WINAPI*)()>(GetProcAddress(dll,"CC_Version"));auto begin=reinterpret_cast<int(WINAPI*)(const wchar_t*,const wchar_t*,int)>(GetProcAddress(dll,"CC_Begin"));auto abort=reinterpret_cast<int(WINAPI*)()>(GetProcAddress(dll,"CC_Abort"));
    if(!supported || !version || !begin || !abort || std::strcmp(version(),CC_VERSION)!=0){std::cerr<<"FAIL: API exports or running helper version\n";FreeLibrary(dll);return 1;}
    // Server hosted CI must prove refusal, rather than weaken the shipping
    // Windows 11 workstation restriction or call a Server test a Win11 pass.
    auto query=reinterpret_cast<LONG(WINAPI*)(OSVERSIONINFOEXW*)>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"RtlGetVersion"));
    OSVERSIONINFOEXW os{};os.dwOSVersionInfoSize=sizeof(os);
    if(!query || query(&os)!=0){FreeLibrary(dll);return 1;}
    if(os.wProductType!=VER_NT_WORKSTATION){
        if(supported()!=-11 || begin(nullptr,nullptr,1)!=-10 || abort()!=0){std::cerr<<"FAIL: unsupported Windows Server preflight\n";FreeLibrary(dll);return 1;}
        std::cout<<"PASS: native API load/version; Windows Server refused before installation. Windows 11 installation acceptance NOT RUN.\n";
        FreeLibrary(dll);return 0;
    }
    if(supported()!=0){std::cerr<<"FAIL: actual supported workstation environment\n";FreeLibrary(dll);return 1;}
    PWSTR folder=nullptr;if(FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesX64,KF_FLAG_DEFAULT,nullptr,&folder))){FreeLibrary(dll);return 1;}auto product=std::filesystem::path(folder)/L"ChordCanvas";CoTaskMemFree(folder);
    // Deliberately absent source: even if a product appears concurrently, payload
    // validation must fail before any installation mutation. Never tests Setup.
    auto absent=file.parent_path()/L"intentionally-absent-installer-probe-payload";if(std::filesystem::exists(absent)){FreeLibrary(dll);return 2;}
    auto result=begin(absent.c_str(),product.c_str(),1);if((result!=-2 && result!=-1) || abort()!=0){std::cerr<<"FAIL: absent-source updater preflight\n";FreeLibrary(dll);return 1;}
    std::cout<<"PASS: actual native DLL load, exports, Windows 11 native AMD64 support and version "<<version()<<"; read-only absent-source updater returned "<<result<<"; no installation started\n";FreeLibrary(dll);return 0;
}

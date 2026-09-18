#include "BinaryVersion.h"
#include <windows.h>
#include <winver.h>
#include <vector>
#include <cstdint>
#include <cwchar>
#include <string_view>

namespace cc {
ExecutableVersion executableProductVersion(const std::filesystem::path& executable) {
    DWORD ignored=0;auto bytes=GetFileVersionInfoSizeW(executable.c_str(),&ignored);
    if(!bytes || bytes>4*1024*1024)return {};
    std::vector<BYTE> resource(bytes);
    if(!GetFileVersionInfoW(executable.c_str(),0,bytes,resource.data()))return {};
    auto contained=[&](const void* pointer,size_t length){auto start=reinterpret_cast<uintptr_t>(resource.data()),address=reinterpret_cast<uintptr_t>(pointer);return pointer && address>=start && address-start<=resource.size() && length<=resource.size()-(address-start);};
    struct Translation { WORD language,codepage; };
    Translation* translations=nullptr;UINT translationBytes=0;
    if(VerQueryValueW(resource.data(),L"\\VarFileInfo\\Translation",reinterpret_cast<void**>(&translations),&translationBytes) && contained(translations,translationBytes) && translationBytes%sizeof(Translation)==0) {
        auto count=translationBytes/sizeof(Translation);
        for(auto field:{L"ProductVersion",L"FileVersion"})for(size_t i=0;i<count && i<64;++i) {
            wchar_t key[80]{};std::swprintf(key,80,L"\\StringFileInfo\\%04x%04x\\%ls",static_cast<unsigned>(translations[i].language),static_cast<unsigned>(translations[i].codepage),field);
            wchar_t* value=nullptr;UINT characters=0;
            if(!VerQueryValueW(resource.data(),key,reinterpret_cast<void**>(&value),&characters) || characters<2 || characters>129 || !contained(value,characters*sizeof(wchar_t)) || value[characters-1]!=L'\0')continue;
            // Capture only a numeric version, never arbitrary executable strings.
            std::string version;bool valid=true,needsDigit=true;
            for(UINT j=0;j+1<characters;++j){auto c=value[j];if(c>=L'0' && c<=L'9'){version+=static_cast<char>(c);needsDigit=false;}else if(c==L'.' && !needsDigit){version+='.';needsDigit=true;}else{valid=false;break;}}
            if(valid && !needsDigit)return {version,std::wstring_view(field)==L"ProductVersion" ? "process executable ProductVersion string resource" : "process executable FileVersion string resource"};
        }
    }
    VS_FIXEDFILEINFO* fixed=nullptr;UINT size=0;
    if(!VerQueryValueW(resource.data(),L"\\",reinterpret_cast<void**>(&fixed),&size) || size<sizeof(*fixed) || !contained(fixed,sizeof(*fixed)) || fixed->dwSignature!=0xfeef04bd)return {};
    return {std::to_string(HIWORD(fixed->dwProductVersionMS))+'.'+std::to_string(LOWORD(fixed->dwProductVersionMS))+'.'+std::to_string(HIWORD(fixed->dwProductVersionLS))+'.'+std::to_string(LOWORD(fixed->dwProductVersionLS)),"process executable fixed product version resource"};
}
}

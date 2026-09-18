#include "Payload.h"
#include <windows.h>
#include <bcrypt.h>
#include <fstream>
#include <set>
#include <algorithm>
#include <array>
#include <cwctype>

namespace cc::packaging {
namespace {
void checked(NTSTATUS status){if(status<0)throw std::runtime_error("Payload hash failed");}
struct Hash {
    BCRYPT_ALG_HANDLE algorithm=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;
    std::vector<unsigned char> object;
    ~Hash(){if(hash)BCryptDestroyHash(hash);if(algorithm)BCryptCloseAlgorithmProvider(algorithm,0);}
};
bool lowerHex(const std::string& value){return value.size()==64 && std::all_of(value.begin(),value.end(),[](char c){return (c>='0' && c<='9') || (c>='a' && c<='f');});}
std::filesystem::path relative(const std::string& value){
    // Format v1 uses printable ASCII bundle filenames. User progression paths
    // are separate and unrestricted; this prevents Windows Unicode device-name
    // and case aliases from entering the owned installation manifest.
    if(value.empty() || value.size()>240 || value.front()=='/' || value.find_first_of("\\:\t\r\n")!=std::string::npos || std::any_of(value.begin(),value.end(),[](unsigned char c){return c<32 || c>=127;}))throw std::runtime_error("Unsafe payload path");
    auto path=std::filesystem::path(std::u8string(value.begin(),value.end()));
    if(path.is_absolute() || path.has_root_path())throw std::runtime_error("Unsafe payload path");
    for(auto segment:path){auto name=segment.wstring();
        if(name.empty() || name==L"." || name==L".." || name.back()==L'.' || name.back()==L' ' || name.find_first_of(L"<>\"|?*")!=std::wstring::npos || std::any_of(name.begin(),name.end(),[](wchar_t c){return c<32;}))throw std::runtime_error("Unsafe payload path");
        auto stem=name.substr(0,name.find(L'.'));std::transform(stem.begin(),stem.end(),stem.begin(),[](wchar_t c){return static_cast<wchar_t>(towupper(c));});
        if(stem==L"CON" || stem==L"PRN" || stem==L"AUX" || stem==L"NUL" || (stem.size()==4 && (stem.substr(0,3)==L"COM" || stem.substr(0,3)==L"LPT") && stem[3]>=L'1' && stem[3]<=L'9'))throw std::runtime_error("Reserved Windows payload path");}
    return path;
}
std::wstring foldPath(const std::filesystem::path& path){auto value=path.generic_wstring();std::transform(value.begin(),value.end(),value.begin(),[](wchar_t c){return static_cast<wchar_t>(towlower(c));});return value;}
void architecture(const std::filesystem::path& file){
    std::ifstream input(file,std::ios::binary);std::array<unsigned char,64> header{};
    if(!input.read(reinterpret_cast<char*>(header.data()),header.size()) || header[0]!='M' || header[1]!='Z')throw std::runtime_error("Payload is not a PE binary");
    uint32_t offset=header[60]|(uint32_t(header[61])<<8)|(uint32_t(header[62])<<16)|(uint32_t(header[63])<<24);
    if(offset<64 || offset>1024*1024)throw std::runtime_error("Invalid PE header offset");
    input.seekg(offset);std::array<unsigned char,26> pe{};
    if(!input.read(reinterpret_cast<char*>(pe.data()),pe.size()) || pe[0]!='P' || pe[1]!='E' || pe[2]!=0 || pe[3]!=0 || pe[4]!=0x64 || pe[5]!=0x86 || pe[24]!=0x0b || pe[25]!=0x02)throw std::runtime_error("Payload is not an AMD64 PE32+ binary");
}
}
bool plainPath(const std::filesystem::path& path){auto attributes=GetFileAttributesW(path.c_str());return attributes!=INVALID_FILE_ATTRIBUTES && !(attributes&FILE_ATTRIBUTE_REPARSE_POINT);}
std::string sha256(const std::filesystem::path& file){
    Hash state;checked(BCryptOpenAlgorithmProvider(&state.algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0));
    DWORD length=0,actual=0;checked(BCryptGetProperty(state.algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&length),sizeof(length),&actual,0));
    state.object.resize(length);checked(BCryptCreateHash(state.algorithm,&state.hash,state.object.data(),length,nullptr,0,0));
    std::ifstream input(file,std::ios::binary);if(!input)throw std::runtime_error("Payload file unreadable");
    std::array<unsigned char,65536> buffer{};
    while(input){input.read(reinterpret_cast<char*>(buffer.data()),buffer.size());auto count=input.gcount();if(count)checked(BCryptHashData(state.hash,buffer.data(),static_cast<ULONG>(count),0));}
    if(!input.eof())throw std::runtime_error("Payload file read failed");
    std::array<unsigned char,32> digest{};checked(BCryptFinishHash(state.hash,digest.data(),digest.size(),0));
    static constexpr char hex[]="0123456789abcdef";std::string result;result.reserve(64);for(auto byte:digest){result+=hex[byte>>4];result+=hex[byte&15];}return result;
}
void validateAmd64Pe(const std::filesystem::path& file){if(!plainPath(file))throw std::runtime_error("Unsafe binary path");architecture(file);}
Payload Payload::read(const std::filesystem::path& root){
    auto file=root/receiptName;
    if(!plainPath(root) || !plainPath(file) || !std::filesystem::is_regular_file(file) || std::filesystem::file_size(file)>1024*1024)throw std::runtime_error("Installation has no recognised ownership receipt");
    const auto receiptBefore=sha256(file);
    std::ifstream input(file,std::ios::binary);std::string line;auto next=[&]{if(!std::getline(input,line) || (!line.empty() && line.back()=='\r'))throw std::runtime_error("Invalid payload receipt");return line;};
    if(next()!="ChordCanvasPayload1" || next()!=productIdentity)throw std::runtime_error("Conflicting product identity");
    Payload result;result.version=next();Version::parse(result.version);result.binary=relative(next());
    if(result.binary.generic_wstring()!=L"Contents/x86_64-win/ChordCanvas.vst3")throw std::runtime_error("Unexpected instrument binary path");
    std::set<std::wstring> names;
    while(std::getline(input,line)){
        auto delimiter=line.find('\t');if(delimiter!=64 || !lowerHex(line.substr(0,delimiter)))throw std::runtime_error("Invalid payload hash entry");
        auto path=relative(line.substr(delimiter+1));if(foldPath(path)==foldPath(receiptName) || !names.insert(foldPath(path)).second || names.size()>4096)throw std::runtime_error("Duplicate or excessive payload entries");
        result.files.push_back({line.substr(0,delimiter),path});
    }
    if(!input.eof() || result.files.empty() || !names.contains(foldPath(result.binary)))throw std::runtime_error("Incomplete payload receipt");
    result.receiptHash=sha256(file);
    if(result.receiptHash!=receiptBefore)throw std::runtime_error("Payload receipt changed during validation");
    return result;
}
void Payload::validate(const std::filesystem::path& root,bool hashes) const{
    if(!plainPath(root))throw std::runtime_error("Unsafe installation directory");
    if(!plainPath(root/receiptName) || receiptHash.empty() || sha256(root/receiptName)!=receiptHash)throw std::runtime_error("Payload receipt changed after validation");
    std::set<std::wstring> expected;expected.insert(foldPath(receiptName));
    std::set<std::wstring> directories;
    for(auto& entry:files){expected.insert(foldPath(entry.relative));auto parent=entry.relative.parent_path();while(!parent.empty()){directories.insert(foldPath(parent));parent=parent.parent_path();}}
    std::set<std::wstring> found;
    for(auto& entry:std::filesystem::recursive_directory_iterator(root)){
        if(!plainPath(entry.path()))throw std::runtime_error("Reparse point in installation");
        auto name=foldPath(entry.path().lexically_relative(root));
        if(entry.is_directory()){if(!directories.contains(name))throw std::runtime_error("Unowned directory in installation");}
        else if(!entry.is_regular_file() || !expected.contains(name))throw std::runtime_error("Unowned file in installation");
        else found.insert(name);
    }
    if(hashes){if(found!=expected)throw std::runtime_error("Incomplete installed payload");for(auto& entry:files)if(sha256(root/entry.relative)!=entry.sha256)throw std::runtime_error("Payload integrity mismatch");architecture(root/binary);}
}
}

#include "RegistryImage.h"
#include "Payload.h"
#include <algorithm>
#include <array>
#include <cwctype>
#include <fstream>
#include <set>
#include <stdexcept>

namespace cc::packaging {
namespace {
constexpr size_t cap=1024*1024;
constexpr char magic[]="ChordCanvasRegistry1\n";
struct Key {HKEY value=nullptr;~Key(){if(value)RegCloseKey(value);}};
struct File {HANDLE value=INVALID_HANDLE_VALUE;~File(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);}};
void require(bool test,const char* message){if(!test)throw std::runtime_error(message);}
std::wstring folded(std::wstring value){for(auto& c:value)c=static_cast<wchar_t>(towlower(c));return value;}
void plainAncestors(std::filesystem::path path){for(;;){require(plainPath(path),"Unsafe registry snapshot ancestor");auto parent=path.parent_path();if(parent.empty() || parent==path)break;path=parent;}}
void validate(const RegistryImage& image){
    require(image.values.size()<=128 && (image.exists || image.values.empty()),"Invalid registry image");
    size_t total=0;std::set<std::wstring> names;
    for(auto& value:image.values){
        require(value.name.size()<=256 && std::all_of(value.name.begin(),value.name.end(),[](wchar_t c){return c>=32 && c<127;}) && names.insert(folded(value.name)).second,"Duplicate or invalid registry value name");
        require(value.bytes.size()<=65536,"Oversized registry value");total+=value.bytes.size()+value.name.size()*2+12;
        require(total<=cap,"Oversized registry image");
        switch(value.type){
            case REG_SZ:case REG_EXPAND_SZ:case REG_MULTI_SZ:
                require(value.bytes.size()%2==0 && value.bytes.size()>=2 && value.bytes[value.bytes.size()-1]==0 && value.bytes[value.bytes.size()-2]==0,"Invalid registry string");break;
            case REG_DWORD:require(value.bytes.size()==4,"Invalid registry DWORD");break;
            case REG_QWORD:require(value.bytes.size()==8,"Invalid registry QWORD");break;
            case REG_BINARY:break;
            default:throw std::runtime_error("Unsupported registry value type; installation metadata preserved");
        }
    }
}
void appendWord(std::vector<unsigned char>& out,uint32_t value){for(int i=0;i<4;++i)out.push_back(static_cast<unsigned char>(value>>(i*8)));}
uint32_t readWord(const std::vector<unsigned char>& in,size_t& offset){require(offset<=in.size() && in.size()-offset>=4,"Truncated registry image");uint32_t value=0;for(int i=0;i<4;++i)value|=static_cast<uint32_t>(in[offset++])<<(i*8);return value;}
void keyName(const std::wstring& value){require(!value.empty() && value.front()!=L'\\' && value.back()!=L'\\' && value.find(L'\0')==std::wstring::npos,"Invalid registry key");}
}
RegistryImage RegistryImage::capture(HKEY hive,const std::wstring& path){
    keyName(path);Key key;auto error=RegOpenKeyExW(hive,path.c_str(),0,KEY_READ|KEY_WOW64_64KEY,&key.value);
    if(error==ERROR_FILE_NOT_FOUND || error==ERROR_PATH_NOT_FOUND)return {};
    require(error==ERROR_SUCCESS,"Cannot read installation registry metadata");
    FILETIME before{},after{};DWORD subkeys=0,count=0;require(RegQueryInfoKeyW(key.value,nullptr,nullptr,nullptr,&subkeys,nullptr,nullptr,&count,nullptr,nullptr,nullptr,&before)==ERROR_SUCCESS && subkeys==0 && count<=128,"Unexpected registry subkeys or excessive values; metadata preserved");
    RegistryImage image;image.exists=true;size_t total=0;
    for(DWORD i=0;i<count;++i){
        std::array<wchar_t,257> name{};DWORD nameSize=static_cast<DWORD>(name.size()),type=0,size=0;
        require(RegEnumValueW(key.value,i,name.data(),&nameSize,nullptr,&type,nullptr,&size)==ERROR_SUCCESS && size<=65536,"Invalid or oversized installation registry value");
        total+=size+nameSize*2+12;require(total<=cap,"Oversized installation registry metadata");
        RegistryValue value;value.name.assign(name.data(),nameSize);value.type=type;value.bytes.resize(size);DWORD actual=size,actualType=0;
        require(RegQueryValueExW(key.value,value.name.c_str(),nullptr,&actualType,value.bytes.data(),&actual)==ERROR_SUCCESS && actual==size && actualType==type,"Registry metadata changed during capture");
        image.values.push_back(std::move(value));
    }
    DWORD finalCount=0;require(RegQueryInfoKeyW(key.value,nullptr,nullptr,nullptr,&subkeys,nullptr,nullptr,&finalCount,nullptr,nullptr,nullptr,&after)==ERROR_SUCCESS && subkeys==0 && count==finalCount && CompareFileTime(&before,&after)==0,"Registry metadata changed during capture");
    std::sort(image.values.begin(),image.values.end(),[](auto& a,auto& b){return folded(a.name)<folded(b.name);});validate(image);return image;
}
void RegistryImage::write(const std::filesystem::path& path) const{
    validate(*this);plainAncestors(path.parent_path());
    std::vector<unsigned char> bytes(magic,magic+sizeof(magic)-1);appendWord(bytes,exists ? 1 : 0);appendWord(bytes,static_cast<uint32_t>(values.size()));
    for(auto& value:values){appendWord(bytes,static_cast<uint32_t>(value.name.size()));for(auto c:value.name){bytes.push_back(static_cast<unsigned char>(c));bytes.push_back(static_cast<unsigned char>(c>>8));}appendWord(bytes,value.type);appendWord(bytes,static_cast<uint32_t>(value.bytes.size()));bytes.insert(bytes.end(),value.bytes.begin(),value.bytes.end());}
    require(bytes.size()<=cap,"Oversized registry snapshot");File file;file.value=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    require(file.value!=INVALID_HANDLE_VALUE,"Cannot create registry snapshot; existing data preserved");DWORD written=0;
    require(WriteFile(file.value,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr) && written==bytes.size() && FlushFileBuffers(file.value),"Cannot persist registry snapshot");
}
RegistryImage RegistryImage::read(const std::filesystem::path& path){
    plainAncestors(path);
    require(std::filesystem::is_regular_file(path),"Unsafe registry snapshot");
    const auto byteCount=std::filesystem::file_size(path);require(byteCount<=cap,"Oversized registry snapshot");
    auto before=sha256(path);std::ifstream file(path,std::ios::binary);std::vector<unsigned char> bytes(static_cast<size_t>(byteCount));
    file.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    require(file.good() && file.peek()==std::char_traits<char>::eof() && bytes.size()>=sizeof(magic)-1 && std::equal(magic,magic+sizeof(magic)-1,bytes.begin()) && sha256(path)==before,"Invalid or changed registry snapshot");
    size_t offset=sizeof(magic)-1;RegistryImage image;auto presence=readWord(bytes,offset);require(presence<=1,"Invalid registry snapshot presence");image.exists=presence!=0;auto count=readWord(bytes,offset);require(count<=128,"Excessive registry snapshot values");
    for(uint32_t i=0;i<count;++i){
        RegistryValue value;auto length=readWord(bytes,offset);require(length<=256 && offset<=bytes.size() && bytes.size()-offset>=length*2,"Invalid registry snapshot name");
        for(uint32_t j=0;j<length;++j){auto c=static_cast<wchar_t>(bytes[offset]|static_cast<unsigned>(bytes[offset+1])<<8);value.name+=c;offset+=2;}
        value.type=readWord(bytes,offset);auto size=readWord(bytes,offset);require(size<=65536 && offset<=bytes.size() && bytes.size()-offset>=size,"Invalid registry snapshot data");value.bytes.assign(bytes.begin()+offset,bytes.begin()+offset+size);offset+=size;image.values.push_back(std::move(value));
    }
    require(offset==bytes.size(),"Trailing registry snapshot data");validate(image);return image;
}
void RegistryImage::restore(HKEY hive,const std::wstring& path,const std::vector<std::wstring>& forward) const{
    validate(*this);keyName(path);auto current=capture(hive,path);std::set<std::wstring> allowed;
    for(auto& name:forward){require(name.size()<=256 && name.find(L'\0')==std::wstring::npos,"Invalid forward registry value");allowed.insert(folded(name));}
    for(auto& value:values)allowed.insert(folded(value.name));
    for(auto& value:current.values)require(allowed.contains(folded(value.name)),"Unexpected registry value; restoration stopped before changing metadata");
    if(!exists && !current.exists)return;
    Key key;DWORD disposition=0;
    require(RegCreateKeyExW(hive,path.c_str(),0,nullptr,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE|KEY_WOW64_64KEY,nullptr,&key.value,&disposition)==ERROR_SUCCESS,"Cannot restore installation registry metadata");
    for(auto& value:current.values){auto error=RegDeleteValueW(key.value,value.name.c_str());require(error==ERROR_SUCCESS || error==ERROR_FILE_NOT_FOUND,"Cannot remove forward registry value");}
    for(auto& value:values)require(RegSetValueExW(key.value,value.name.c_str(),0,value.type,value.bytes.data(),static_cast<DWORD>(value.bytes.size()))==ERROR_SUCCESS,"Cannot restore registry value; durable image must be retained for retry");
    require(RegFlushKey(key.value)==ERROR_SUCCESS,"Cannot persist restored installation registry metadata");
    if(!exists){RegCloseKey(key.value);key.value=nullptr;require(RegDeleteKeyExW(hive,path.c_str(),KEY_WOW64_64KEY,0)==ERROR_SUCCESS,"Cannot remove owned empty installation key");}
}
}

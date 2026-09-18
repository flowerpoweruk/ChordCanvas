#include "TemporaryMidi.h"
#include <windows.h>
#include <objbase.h>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <algorithm>

namespace cc {
namespace {
struct Handle {
    HANDLE value=INVALID_HANDLE_VALUE;
    ~Handle(){if(value!=INVALID_HANDLE_VALUE && value!=nullptr)CloseHandle(value);}
};
struct Guard {
    Handle mutex;
    Guard(){mutex.value=CreateMutexW(nullptr,FALSE,L"Local\\ChordCanvas.ExportCache.v1");
        if(!mutex.value)throw std::runtime_error("Export cache mutex failed");
        auto result=WaitForSingleObject(mutex.value,1000);
        if(result!=WAIT_OBJECT_0 && result!=WAIT_ABANDONED)throw std::runtime_error("Export cache busy");}
    ~Guard(){ReleaseMutex(mutex.value);}
};
bool plain(const std::filesystem::path& path) {
    auto attributes=GetFileAttributesW(path.c_str());
    return attributes!=INVALID_FILE_ATTRIBUTES && !(attributes&FILE_ATTRIBUTE_REPARSE_POINT);
}
std::wstring identifier(){GUID id;wchar_t result[40]{};
    if(FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id,result,40))throw std::runtime_error("Export identity failed");
    return std::wstring(result+1,36);
}
bool ownedName(const std::wstring& name){
    if(name.size()!=36)return false;
    for(size_t i=0;i<36;++i){auto c=name[i];if(i==8 || i==13 || i==18 || i==23){if(c!=L'-')return false;}
        else if(!((c>=L'0' && c<=L'9') || (c>=L'A' && c<=L'F') || (c>=L'a' && c<=L'f')))return false;}
    return true;
}
bool clean(const std::filesystem::path& dir){
    if(!plain(dir))return false;
    for(auto& entry:std::filesystem::directory_iterator(dir))
        if(!entry.is_regular_file() || !plain(entry.path()) || (entry.path().filename()!=L"lease" && entry.path().filename()!=L"chords.mid"))return false;
    auto lease=dir/L"lease",file=dir/L"chords.mid";
    Handle lock;lock.value=CreateFileW(lease.c_str(),GENERIC_READ|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(lock.value==INVALID_HANDLE_VALUE)return false; // An active drag or unknown directory stays untouched.
    std::error_code ec;std::filesystem::remove(file,ec);if(ec)return false;
    FILE_DISPOSITION_INFO disposition {TRUE};
    if(!SetFileInformationByHandle(lock.value,FileDispositionInfo,&disposition,sizeof(disposition)))return false;
    CloseHandle(lock.value);lock.value=INVALID_HANDLE_VALUE;
    return std::filesystem::remove(dir,ec) && !ec;
}
}
std::shared_ptr<TemporaryMidi> TemporaryMidi::prepare(const std::vector<uint8_t>& bytes,const std::filesystem::path& supplied,size_t capacity){
    if(bytes.empty() || bytes.size()>65536 || !capacity)throw std::invalid_argument("Invalid MIDI cache payload");
    Guard guard;
    auto root=std::filesystem::absolute(supplied).lexically_normal();
    std::filesystem::create_directories(root);if(!plain(root))throw std::runtime_error("Unsafe export cache directory");
    static const auto session=identifier();
    size_t count=0;
    for(auto& folder:std::filesystem::directory_iterator(root)){
        if(!folder.is_directory() || !ownedName(folder.path().filename().wstring()) || !plain(folder.path()))continue;
        for(auto& entry:std::filesystem::directory_iterator(folder.path())){
            if(!entry.is_directory() || !ownedName(entry.path().filename().wstring()) || !plain(entry.path()))continue;
            auto age=std::filesystem::file_time_type::clock::now()-std::filesystem::last_write_time(entry.path());
            if(age>std::chrono::hours(24) && clean(entry.path()))continue;
            ++count;
        }
        std::error_code ec;std::filesystem::remove(folder.path(),ec); // Only succeeds for an empty owned session directory.
    }
    if(count>=capacity)throw std::runtime_error("Export cache capacity reached; retained files are still within the safe grace period");
    auto result=std::shared_ptr<TemporaryMidi>(new TemporaryMidi);
    auto sessionRoot=root/session;std::filesystem::create_directories(sessionRoot);
    if(!plain(sessionRoot))throw std::runtime_error("Unsafe export session directory");
    auto dir=sessionRoot/identifier();if(!std::filesystem::create_directory(dir))throw std::runtime_error("Export directory collision");
    result->path=dir/L"chords.mid";
    auto handle=CreateFileW((dir/L"lease").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error("Export lease failed");result->lease=handle;
    std::ofstream output(result->path,std::ios::binary);output.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));output.flush();output.close();
    if(!output)throw std::runtime_error("MIDI write failed");
    std::ifstream input(result->path,std::ios::binary);std::vector<uint8_t> actual{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
    if(input.bad() || actual!=bytes)throw std::runtime_error("MIDI read-back differs");
    return result;
}
TemporaryMidi::~TemporaryMidi(){if(lease)CloseHandle(static_cast<HANDLE>(lease));}
}

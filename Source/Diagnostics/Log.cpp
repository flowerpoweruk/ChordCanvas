#include "Log.h"
#include <windows.h>
#include <shlobj.h>
#include <winver.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <cwctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <charconv>
#include <cmath>

namespace cc {
AudioLogQueue::AudioLogQueue() { for(size_t i=0;i<slots.size();++i)slots[i].sequence=i; }
bool AudioLogQueue::push(AudioLogEvent event) noexcept {
    size_t pos=write.load(std::memory_order_relaxed);
    for(int attempt=0;attempt<8;++attempt) {
        auto& slot=slots[pos%slots.size()];auto sequence=slot.sequence.load(std::memory_order_acquire);
        auto diff=static_cast<intptr_t>(sequence)-static_cast<intptr_t>(pos);
        if(diff==0){if(write.compare_exchange_weak(pos,pos+1,std::memory_order_relaxed)){slot.event=event;slot.sequence.store(pos+1,std::memory_order_release);return true;}}
        else if(diff<0)return false;else pos=write.load(std::memory_order_relaxed);
    }
    return false;
}
bool AudioLogQueue::pop(AudioLogEvent& event) noexcept {
    size_t pos=read.load(std::memory_order_relaxed);
    for(;;) {
        auto& slot=slots[pos%slots.size()];auto sequence=slot.sequence.load(std::memory_order_acquire);
        auto diff=static_cast<intptr_t>(sequence)-static_cast<intptr_t>(pos+1);
        if(diff==0){if(read.compare_exchange_weak(pos,pos+1,std::memory_order_relaxed)){event=slot.event;slot.sequence.store(pos+slots.size(),std::memory_order_release);return true;}}
        else if(diff<0)return false;else pos=read.load(std::memory_order_relaxed);
    }
}
std::filesystem::path logsFolder() {
    PWSTR path=nullptr;
    if(FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData,KF_FLAG_CREATE,nullptr,&path)))throw std::runtime_error("Local application data unavailable");
    std::filesystem::path root(path);CoTaskMemFree(path);return root/L"ChordCanvas"/L"Logs";
}
std::string jsonQuote(std::string_view text) {
    std::string result="\"";
    constexpr char hex[]="0123456789abcdef";
    for(unsigned char c:text){if(c=='"' || c=='\\'){result+='\\';result+=static_cast<char>(c);}else if(c<32){result+="\\u00";result+=hex[c>>4];result+=hex[c&15];}else result+=static_cast<char>(c);}
    result+='"';return result;
}
namespace {
std::string number(double value){if(!std::isfinite(value))return "null";std::array<char,64> buffer{};auto result=std::to_chars(buffer.data(),buffer.data()+buffer.size(),value);return result.ec==std::errc() ? std::string(buffer.data(),result.ptr) : "null";}
std::string nativeArchitecture(){SYSTEM_INFO info{};GetNativeSystemInfo(&info);switch(info.wProcessorArchitecture){case PROCESSOR_ARCHITECTURE_AMD64:return "x64";case PROCESSOR_ARCHITECTURE_ARM64:return "arm64";case PROCESSOR_ARCHITECTURE_INTEL:return "x86";default:return "unknown";}}
std::string windowsVersion(){auto dll=GetModuleHandleW(L"ntdll.dll");if(!dll)return "unknown";using Query=LONG(WINAPI*)(OSVERSIONINFOEXW*);auto query=reinterpret_cast<Query>(GetProcAddress(dll,"RtlGetVersion"));OSVERSIONINFOEXW info{};info.dwOSVersionInfoSize=sizeof(info);if(!query || query(&info)!=0)return "unknown";return std::to_string(info.dwMajorVersion)+'.'+std::to_string(info.dwMinorVersion)+'.'+std::to_string(info.dwBuildNumber);}
std::string hostBinaryVersion(){
    // Read only the current host executable's fixed version resource. Its path
    // is a local API input, never part of the header or diagnostic payload.
    std::wstring path(32768,L'\0');auto length=GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));if(!length || length>=path.size())return "unknown";path.resize(length);
    DWORD ignored=0;auto bytes=GetFileVersionInfoSizeW(path.c_str(),&ignored);if(!bytes || bytes>4*1024*1024)return "unknown";std::vector<BYTE> resource(bytes);
    if(!GetFileVersionInfoW(path.c_str(),0,bytes,resource.data()))return "unknown";VS_FIXEDFILEINFO* info=nullptr;UINT size=0;
    if(!VerQueryValueW(resource.data(),L"\\",reinterpret_cast<void**>(&info),&size) || size<sizeof(*info) || !info || info->dwSignature!=0xfeef04bd)return "unknown";
    return std::to_string(HIWORD(info->dwProductVersionMS))+'.'+std::to_string(LOWORD(info->dwProductVersionMS))+'.'+std::to_string(HIWORD(info->dwProductVersionLS))+'.'+std::to_string(LOWORD(info->dwProductVersionLS));
}
std::string audioDetails(const AudioLogEvent& event){
    auto result="{\"kind\":"+std::to_string(event.kind)+",\"revision\":"+std::to_string(event.revision)+",\"owner\":"+std::to_string(event.owner)+",\"tick\":"+std::to_string(event.tick)+",\"value\":"+std::to_string(event.value);
    if(event.kind==3)result+=",\"tempo_bpm\":"+number(event.tempo)+",\"sample_rate_hz\":"+number(event.sampleRate)+",\"buffer_frames\":"+std::to_string(event.bufferFrames)+",\"meter_numerator\":"+std::to_string(event.numerator)+",\"meter_denominator\":"+std::to_string(event.denominator)+",\"tempo_available\":"+(event.tempoAvailable ? "true" : "false")+",\"tempo_ever_known\":"+(event.tempoEverKnown ? "true" : "false")+",\"fallback_120_bpm\":"+(!event.tempoEverKnown ? "true" : "false")+",\"host_playing\":"+(event.hostPlaying ? "true" : "false")+",\"bypassed\":"+(event.bypassed ? "true" : "false");
    return result+'}';
}
struct Handle {
    HANDLE h=INVALID_HANDLE_VALUE;
    ~Handle(){close();}
    void close(){if(h!=INVALID_HANDLE_VALUE && h!=nullptr)CloseHandle(h);h=INVALID_HANDLE_VALUE;}
};
std::string utc(bool filename=false) {
    using namespace std::chrono;auto now=system_clock::now();auto time=system_clock::to_time_t(now);std::tm value{};gmtime_s(&value,&time);
    std::ostringstream out;out<<std::put_time(&value,filename ? "%Y%m%dT%H%M%S" : "%Y-%m-%dT%H:%M:%S");
    out<<'.'<<std::setfill('0')<<std::setw(3)<<(duration_cast<milliseconds>(now.time_since_epoch()).count()%1000);out<<'Z';return out.str();
}
bool writeText(HANDLE file,const std::string& text) {
    DWORD written=0;return WriteFile(file,text.data(),static_cast<DWORD>(text.size()),&written,nullptr) && written==text.size();
}
std::wstring mutexName(const std::filesystem::path& root) {
    uint64_t hash=1469598103934665603ull;for(wchar_t c:root.wstring()){hash^=static_cast<uint64_t>(towlower(c));hash*=1099511628211ull;}
    return L"Local\\ChordCanvas.LogRotation.v1."+std::to_wstring(hash);
}
std::string readTail(const std::filesystem::path& file) {
    std::ifstream input(file,std::ios::binary);if(!input)return {};
    input.seekg(0,std::ios::end);auto end=input.tellg();input.seekg(std::max<std::streamoff>(0,static_cast<std::streamoff>(end)-4096));
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
}
LogService::LogService(std::filesystem::path folder,size_t storageCap,std::string hostName,std::string pluginFormat)
    :root(std::move(folder)),cap(std::max<size_t>(storageCap,16384)),host(std::move(hostName)),format(std::move(pluginFormat)),worker([this]{run();}){}
LogService::~LogService(){stopping=true;wake.notify_all();if(worker.joinable())worker.join();}
std::shared_ptr<LogService> LogService::interactive(std::string host,std::string format) {
    static auto service=std::make_shared<LogService>(logsFolder(),32*1024*1024,std::move(host),std::move(format));return service;
}
void LogService::post(uint64_t instance,std::string event,std::string details) {
    if(details.size()>256*1024 || event.size()>96){++dropped;return;}
    // Internal payloads are already JSON. Strip formatting line breaks so nested
    // progression snapshots remain one JSON Lines record; string data is escaped
    // by jsonQuote before reaching this boundary.
    std::erase_if(details,[](char c){return c=='\r' || c=='\n';});
    {std::lock_guard guard(mutex);if(pending.size()>=256){++dropped;return;}pending.push_back({instance,std::move(event),std::move(details)});}
    wake.notify_one();
}
bool LogService::audioEvent(AudioLogEvent event) noexcept { if(audio.push(event))return true;++dropped;return false; }
LogStatus LogService::status() const { std::lock_guard guard(mutex);auto value=report;value.dropped=dropped.load();return value; }
void LogService::run() noexcept {
    try {
    using namespace std::chrono;
    Handle lease,file,rotation;
    auto started=steady_clock::now();uint64_t seq=0,lostSeen=0;
    std::deque<std::string> recent;size_t recentBytes=0,total=0,windowDiscarded=0;
    std::string snapshot;
    GUID guid{};CoCreateGuid(&guid);
    std::string session=utc(true)+"_"+std::to_string(GetCurrentProcessId())+"_"+std::to_string(guid.Data1);
    auto path=root/("ChordCanvas_"+session+".txt");auto lockPath=path;lockPath+=L".lock";
    std::string header="ChordCanvas diagnostic session; UTF-8 JSON Lines; schema 1\n";
    auto binaryVersion=hostBinaryVersion();
    header+="{\"schema\":1,\"event\":\"session.header\",\"session\":"+jsonQuote(session)+",\"version\":"+jsonQuote(CC_VERSION)+",\"host\":"+jsonQuote(host)+",\"host_version\":"+jsonQuote(binaryVersion)+",\"host_version_source\":"+jsonQuote(binaryVersion=="unknown" ? "unavailable" : "process executable fixed product version resource")+",\"plugin_format\":"+jsonQuote(format)+",\"source\":"+jsonQuote(CC_SOURCE_COMMIT)+",\"build_number\":"+std::to_string(CC_BUILD_NUMBER)+",\"build_configuration\":"+jsonQuote(CC_BUILD_CONFIGURATION)+",\"build_id\":"+jsonQuote(std::string(CC_VERSION)+'.'+std::to_string(CC_BUILD_NUMBER)+'@'+CC_SOURCE_COMMIT)+",\"theory_schema\":1,\"windows_version\":"+jsonQuote(windowsVersion())+",\"native_architecture\":"+jsonQuote(nativeArchitecture())+"}\n";
    auto status=[&](bool ok,const char* reason){std::lock_guard guard(mutex);report.available=ok;report.reason=reason;};
    auto admit=[&]() {
        std::filesystem::create_directories(root);
        rotation.close();rotation.h=CreateMutexW(nullptr,FALSE,mutexName(root).c_str());if(!rotation.h){status(false,"Cannot coordinate log rotation");return false;}
        auto waited=WaitForSingleObject(rotation.h,1000);if(waited!=WAIT_OBJECT_0 && waited!=WAIT_ABANDONED){rotation.close();status(false,"Log rotation busy");return false;}
        struct Unlock {HANDLE h;~Unlock(){ReleaseMutex(h);}}unlock{rotation.h};
        std::vector<std::filesystem::path> files;
        for(auto& entry:std::filesystem::directory_iterator(root)) if(entry.is_regular_file() && entry.path().extension()==L".txt" && entry.path().filename().wstring().starts_with(L"ChordCanvas_"))files.push_back(entry.path());
        std::sort(files.begin(),files.end());
        while(files.size()>=5) {
            bool removed=false;
            for(size_t i=0;i<files.size();++i) {
                auto oldLock=files[i];oldLock+=L".lock";Handle oldLease;
                oldLease.h=CreateFileW(oldLock.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_FLAG_DELETE_ON_CLOSE,nullptr);
                if(oldLease.h==INVALID_HANDLE_VALUE)continue;
                if(DeleteFileW(files[i].c_str())){files.erase(files.begin()+static_cast<ptrdiff_t>(i));removed=true;break;}
            }
            if(!removed){status(false,"Five active sessions; retaining bounded in-memory diagnostics");return false;}
        }
        bool unclean=false;for(auto& prior:files){auto oldLock=prior;oldLock+=L".lock";Handle test;test.h=CreateFileW(oldLock.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_FLAG_DELETE_ON_CLOSE,nullptr);if(test.h!=INVALID_HANDLE_VALUE && readTail(prior).find("\"session.end\"")==std::string::npos)unclean=true;}
        lease.h=CreateFileW(lockPath.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_FLAG_DELETE_ON_CLOSE,nullptr);
        if(lease.h==INVALID_HANDLE_VALUE){status(false,"Cannot lease session log");return false;}
        file.h=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file.h==INVALID_HANDLE_VALUE || !writeText(file.h,header)){file.close();lease.close();status(false,"Cannot create session log");return false;}
        total=header.size();status(true,"");
        if(unclean)post(0,"session.previous_unclean","{\"attribution\":\"unknown; missing orderly end is not proof of a plugin crash\"}");
        return true;
    };
    auto line=[&](const Record& record,const char* role="message") {
        auto elapsed=duration_cast<milliseconds>(steady_clock::now()-started).count();
        const char* level=record.event.ends_with(".error") ? "error" : record.event.starts_with("log.") || record.event=="session.previous_unclean" ? "warning" : "info";
        return "{\"schema\":1,\"utc\":"+jsonQuote(utc())+",\"mono_ms\":"+std::to_string(elapsed)+",\"seq\":"+std::to_string(++seq)+",\"session\":"+jsonQuote(session)+",\"instance\":"+std::to_string(record.instance)+",\"level\":"+jsonQuote(level)+",\"thread_role\":"+jsonQuote(role)+",\"event\":"+jsonQuote(record.event)+",\"details\":"+record.details+"}\n";
    };
    auto emit=[&](const std::string& text,bool forceCompact=false) {
        recent.push_back(text);recentBytes+=text.size();while(recent.size()>1 && recentBytes>std::min<size_t>(cap/2,512*1024)){recentBytes-=recent.front().size();recent.pop_front();++windowDiscarded;}
        if(file.h==INVALID_HANDLE_VALUE)return;
        if(total+text.size()>cap || forceCompact) {
            // Replay retained records in their original sequence order. A snapshot
            // outside the recent window predates it; one inside must not be duplicated.
            std::string compact=header;
            if(!snapshot.empty() && std::find(recent.begin(),recent.end(),snapshot)==recent.end())compact+=snapshot;
            for(auto& item:recent)compact+=item;
            auto marker=line({0,"log.history_truncated","{\"retained_records\":"+std::to_string(recent.size())+",\"from_sequence\":"+std::to_string(seq-recent.size()+1)+",\"through_sequence\":"+std::to_string(seq)+",\"discarded_window_records\":"+std::to_string(windowDiscarded)+",\"retained_window_bytes\":"+std::to_string(recentBytes)+"}"},"worker");
            compact+=marker;
            recent.push_back(marker);recentBytes+=marker.size();
            if(compact.size()>cap){status(false,"Snapshot exceeds log storage limit");return;}
            auto tmp=path;tmp+=L".compact";Handle replacement;replacement.h=CreateFileW(tmp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
            if(replacement.h==INVALID_HANDLE_VALUE || !writeText(replacement.h,compact) || !FlushFileBuffers(replacement.h)){status(false,"Log compaction write failed");return;}
            replacement.close();file.close();
            if(!MoveFileExW(tmp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){file.h=CreateFileW(path.c_str(),FILE_APPEND_DATA,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);status(false,"Log compaction replacement failed");return;}
            file.h=CreateFileW(path.c_str(),FILE_APPEND_DATA,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);total=compact.size();status(file.h!=INVALID_HANDLE_VALUE,file.h==INVALID_HANDLE_VALUE ? "Cannot reopen compacted log" : "");
        } else if(!writeText(file.h,text)){status(false,"Log write failed");}else total+=text.size();
    };
    auto lastFlush=steady_clock::now(),lastAdmission=started-seconds(2);
    try {
        while(!stopping.load()) {
            if(file.h==INVALID_HANDLE_VALUE && steady_clock::now()-lastAdmission>=seconds(1)) {
                admit();lastAdmission=steady_clock::now();
                if(file.h!=INVALID_HANDLE_VALUE) {
                    std::string restored;
                    if(!snapshot.empty() && std::find(recent.begin(),recent.end(),snapshot)==recent.end())restored+=snapshot;
                    for(auto& previous:recent)restored+=previous;
                    if(total+restored.size()>cap)status(false,"Retained diagnostic snapshot exceeds storage limit");
                    else if(!restored.empty()) {
                        if(writeText(file.h,restored)) {
                            total+=restored.size();
                            if(windowDiscarded)emit(line({0,"log.history_truncated","{\"origin\":\"memory_window\",\"discarded_records\":"+std::to_string(windowDiscarded)+"}"},"worker"));
                        }else status(false,"Cannot restore in-memory diagnostic trail");
                    }
                }
            }
            std::deque<Record> work;
            {std::unique_lock guard(mutex);wake.wait_for(guard,milliseconds(50),[&]{return stopping.load() || !pending.empty();});work.swap(pending);}
            for(auto& record:work){auto text=line(record);if(record.event=="state.snapshot")snapshot=text;emit(text);}
            AudioLogEvent event;for(int budget=0;budget<256 && audio.pop(event);++budget){emit(line({event.instance,event.kind==3 ? "audio.clock" : "audio.transition",audioDetails(event)},"audio-summary"));}
            auto loss=dropped.load();if(loss!=lostSeen){emit(line({0,"log.events_dropped","{\"count\":"+std::to_string(loss-lostSeen)+"}"},"worker"));lostSeen=loss;}
            if(file.h!=INVALID_HANDLE_VALUE && steady_clock::now()-lastFlush>=seconds(1)){FlushFileBuffers(file.h);lastFlush=steady_clock::now();}
        }
        std::deque<Record> work;{std::lock_guard guard(mutex);work.swap(pending);}for(auto& record:work){auto text=line(record);if(record.event=="state.snapshot")snapshot=text;emit(text);}
        AudioLogEvent event;while(audio.pop(event))emit(line({event.instance,event.kind==3 ? "audio.clock" : "audio.transition",audioDetails(event)},"audio-summary"));
        auto finalLoss=dropped.load();if(finalLoss!=lostSeen)emit(line({0,"log.events_dropped","{\"count\":"+std::to_string(finalLoss-lostSeen)+"}"},"worker"));
        // Reserve room before assigning the terminal record's sequence. A
        // compaction marker must never follow the orderly session.end record.
        if(file.h!=INVALID_HANDLE_VALUE && total+1024>cap)emit(line({0,"session.closing","{}"},"worker"),true);
        emit(line({0,"session.end","{\"termination\":\"orderly\"}"},"worker"));if(file.h!=INVALID_HANDLE_VALUE)FlushFileBuffers(file.h);
    }catch(...){status(false,"Diagnostic storage unavailable; no host fault attribution");}
    }catch(...){
        // Header/resource initialization is also outside the audio thread and
        // must not escape a noexcept worker into the shared host process.
        try{std::lock_guard guard(mutex);report.available=false;report.reason="Diagnostic initialization unavailable";}catch(...){ }
    }
}
}

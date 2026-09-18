#include "Transaction.h"
#include <windows.h>
#include <objbase.h>
#include <algorithm>
#include <set>
#include <exception>
#include <fstream>
#include <array>

namespace cc::packaging {
namespace {
constexpr auto journalName=L".ChordCanvas.transaction";
struct Lock {
    HANDLE mutex=nullptr;
    Lock(){mutex=CreateMutexW(nullptr,FALSE,L"Global\\ChordCanvas.InstallBundle.EE56A5B5-24E6-4F83-B3AF-1C73BCD93594.v1");if(!mutex)throw std::runtime_error("Installation lock failed");
        auto result=WaitForSingleObject(mutex,1000);if(result!=WAIT_OBJECT_0 && result!=WAIT_ABANDONED){CloseHandle(mutex);mutex=nullptr;throw std::runtime_error("Another installation is running");}}
    ~Lock(){if(mutex){ReleaseMutex(mutex);CloseHandle(mutex);}}
};
struct Handle { HANDLE value=INVALID_HANDLE_VALUE;~Handle(){if(value!=INVALID_HANDLE_VALUE)CloseHandle(value);} };
std::wstring identifier(){GUID id;wchar_t text[40]{};if(FAILED(CoCreateGuid(&id)) || !StringFromGUID2(id,text,40))throw std::runtime_error("Install identity failed");return std::wstring(text+1,36);}
bool tokenValid(const std::string& token){
    if(token.size()!=36)return false;
    for(size_t i=0;i<36;++i){char c=token[i];if(i==8 || i==13 || i==18 || i==23){if(c!='-')return false;}
        else if(!((c>='0' && c<='9') || (c>='A' && c<='F') || (c>='a' && c<='f')))return false;}
    return true;
}
bool hashValid(const std::string& value){return value.size()==64 && std::all_of(value.begin(),value.end(),[](char c){return (c>='0' && c<='9') || (c>='a' && c<='f');});}
void renameOwned(const std::filesystem::path& from,const std::filesystem::path& to){if(!MoveFileExW(from.c_str(),to.c_str(),MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Installation rename failed; save and close the host, then retry");}
void available(const std::filesystem::path& path){
    if(!std::filesystem::exists(path))return;
    Handle file;file.value=CreateFileW(path.c_str(),GENERIC_READ|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file.value==INVALID_HANDLE_VALUE)throw std::runtime_error("Installed files are in use or inaccessible; save and close the host, then retry");
}
void ancestorSafety(const std::filesystem::path& path){auto current=path;for(;;){if(!plainPath(current))throw std::runtime_error("Unsafe installation ancestor");auto next=current.parent_path();if(next==current || next.empty())break;current=next;}}
std::filesystem::path safeParent(const std::filesystem::path& path){auto parent=std::filesystem::absolute(path).lexically_normal();ancestorSafety(parent);return parent;}
Payload recognised(const std::filesystem::path& root,const std::string& expected,bool hashes=false){
    auto payload=Payload::read(root);if(payload.receiptHash!=expected)throw std::runtime_error("Recovery found a conflicting ownership receipt; retained files preserved");payload.validate(root,hashes);return payload;
}
void removeOwned(const std::filesystem::path& root,const Payload& payload){
    payload.validate(root,false);
    std::set<std::filesystem::path> directories;
    for(auto& file:payload.files){auto path=root/file.relative;std::filesystem::remove(path);auto parent=path.parent_path();while(parent!=root){directories.insert(parent);parent=parent.parent_path();}}
    // Keep the receipt through directory cleanup so failed cleanup can retry.
    std::vector<std::filesystem::path> ordered(directories.begin(),directories.end());
    std::sort(ordered.begin(),ordered.end(),[](auto& a,auto& b){return std::distance(a.begin(),a.end())>std::distance(b.begin(),b.end());});
    for(auto& path:ordered)std::filesystem::remove(path);
    std::filesystem::remove(root/receiptName);std::filesystem::remove(root);
}
struct Journal {
    bool committed=false;
    std::string token,incoming,previous,participant="-";
    std::filesystem::path stage(const std::filesystem::path& parent) const {return parent/(L".ChordCanvas.stage."+std::wstring(token.begin(),token.end()));}
    std::filesystem::path backup(const std::filesystem::path& parent) const {return parent/(L".ChordCanvas.previous."+std::wstring(token.begin(),token.end()));}
    std::string text() const {return std::string(committed ? "C" : "P")+"ChordCanvasTransaction1\n"+productIdentity+'\n'+token+'\n'+incoming+'\n'+previous+'\n'+participant+'\n';}
    static Journal read(const std::filesystem::path& parent){
        auto path=parent/journalName;
        if(!plainPath(path) || !std::filesystem::is_regular_file(path) || std::filesystem::file_size(path)>1024)throw std::runtime_error("Unrecognised installation recovery marker");
        std::ifstream input(path,std::ios::binary);std::array<std::string,6> lines;
        for(auto& line:lines)if(!std::getline(input,line))throw std::runtime_error("Incomplete recovery marker; installation files preserved");
        std::string extra;if(std::getline(input,extra) || !input.eof())throw std::runtime_error("Malformed recovery marker");
        Journal result;
        if(lines[0]=="CChordCanvasTransaction1")result.committed=true;
        else if(lines[0]!="PChordCanvasTransaction1")throw std::runtime_error("Unrecognised recovery marker");
        if(lines[1]!=productIdentity || !tokenValid(lines[2]) || !hashValid(lines[3]) || (lines[4]!="-" && !hashValid(lines[4])) || (lines[5]!="-" && !hashValid(lines[5])))throw std::runtime_error("Unsafe installation recovery marker");
        result.token=lines[2];result.incoming=lines[3];result.previous=lines[4];result.participant=lines[5];return result;
    }
    void create(const std::filesystem::path& parent) const {
        auto path=parent/journalName;Handle file;
        file.value=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
        if(file.value==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot create installation recovery marker");
        auto content=text();DWORD written=0;
        if(!WriteFile(file.value,content.data(),static_cast<DWORD>(content.size()),&written,nullptr) || written!=content.size() || !FlushFileBuffers(file.value))throw std::runtime_error("Cannot persist recovery marker; installation not changed");
    }
    void decideCommit(const std::filesystem::path& parent){
        auto actual=read(parent);if(actual.token!=token || actual.incoming!=incoming || actual.previous!=previous || actual.participant!=participant || actual.committed)throw std::runtime_error("Installation recovery marker changed unexpectedly");
        Handle file;file.value=CreateFileW((parent/journalName).c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_WRITE_THROUGH,nullptr);
        DWORD written=0;const char decision='C';
        if(file.value==INVALID_HANDLE_VALUE || !WriteFile(file.value,&decision,1,&written,nullptr) || written!=1 || !FlushFileBuffers(file.value))throw std::runtime_error("Cannot persist installation completion; retained bundle preserved");
        committed=true;
    }
};
void removeStage(const std::filesystem::path& stage,const std::string& expected){
    if(!std::filesystem::exists(stage))return;
    if(!plainPath(stage) || !std::filesystem::is_directory(stage))throw std::runtime_error("Unsafe recovery stage");
    if(std::filesystem::is_empty(stage)){std::filesystem::remove(stage);return;}
    auto payload=recognised(stage,expected);removeOwned(stage,payload);
}
void participantRequired(const Journal& journal,TransactionParticipant* participant){
    if(journal.participant!="-" && !participant)throw std::runtime_error("Installation metadata participant required; bundle and recovery data preserved");
}
void rollbackOwned(const std::filesystem::path& parent,const Journal& journal,TransactionParticipant* participant){
    participantRequired(journal,participant);
    if(journal.participant!="-")participant->verify(journal.token,journal.participant);
    auto target=parent/L"ChordCanvas.vst3",stage=journal.stage(parent),backup=journal.backup(parent);
    if(std::filesystem::exists(backup)){
        if(journal.previous=="-")throw std::runtime_error("Unexpected previous bundle; recovery stopped");
        auto previous=recognised(backup,journal.previous);
        if(std::filesystem::exists(target)){
            if(plainPath(target) && std::filesystem::is_directory(target) && std::filesystem::is_empty(target))std::filesystem::remove(target);
            else{auto incoming=recognised(target,journal.incoming);for(auto& file:incoming.files)available(target/file.relative);removeOwned(target,incoming);}
        }
        renameOwned(backup,target);previous.validate(target,false);
    }else if(journal.previous!="-")recognised(target,journal.previous);
    else if(std::filesystem::exists(target)){
        if(plainPath(target) && std::filesystem::is_directory(target) && std::filesystem::is_empty(target))std::filesystem::remove(target);
        else{auto incoming=recognised(target,journal.incoming);for(auto& file:incoming.files)available(target/file.relative);removeOwned(target,incoming);}
    }
    removeStage(stage,journal.incoming);
    if(journal.participant!="-"){participant->rollback(journal.token,journal.participant);participant->cleanup(journal.token,journal.participant,false);}
    std::filesystem::remove(parent/journalName);
    if(journal.participant!="-"){try{participant->retire(journal.token,journal.participant);}catch(...){/* One descriptor retained; participant must recover it before staging. */}}
}
void finishCommitted(const std::filesystem::path& parent,const Journal& journal,TransactionParticipant* participant){
    participantRequired(journal,participant);
    if(journal.participant!="-")participant->verify(journal.token,journal.participant);
    auto installed=recognised(parent/L"ChordCanvas.vst3",journal.incoming,true);
    if(journal.participant!="-")participant->validate(journal.token,journal.participant,installed.version);
    auto backup=journal.backup(parent);
    if(std::filesystem::exists(backup)){
        if(journal.previous=="-")throw std::runtime_error("Unexpected backup; cleanup stopped");
        if(plainPath(backup) && std::filesystem::is_directory(backup) && std::filesystem::is_empty(backup))std::filesystem::remove(backup);
        else{auto previous=recognised(backup,journal.previous);for(auto& file:previous.files)available(backup/file.relative);removeOwned(backup,previous);}
    }
    removeStage(journal.stage(parent),journal.incoming);
    if(journal.participant!="-")participant->cleanup(journal.token,journal.participant,true);
    std::filesystem::remove(parent/journalName);
    if(journal.participant!="-"){try{participant->retire(journal.token,journal.participant);}catch(...){}}
}
void recoverLocked(const std::filesystem::path& parent,TransactionParticipant* participant){
    if(!std::filesystem::exists(parent/journalName))return;
    auto journal=Journal::read(parent);
    if(journal.committed)finishCommitted(parent,journal,participant);else rollbackOwned(parent,journal,participant);
}
}
struct BundleTransaction::State {
    Lock lock;
    std::filesystem::path parent;
    Journal journal;
    std::shared_ptr<TransactionParticipant> participant;
    InstallResult result=InstallResult::installed;
    bool pending=false;
};
BundleTransaction::BundleTransaction(std::unique_ptr<State> value):state(std::move(value)){}
BundleTransaction::~BundleTransaction(){if(state && state->pending){try{rollback();}catch(...){/* Durable marker and previous bundle retained for explicit recovery. */}}}
InstallResult BundleTransaction::result() const noexcept{return state->result;}
void BundleTransaction::rollback(){if(!state->pending)return;rollbackOwned(state->parent,state->journal,state->participant.get());state->pending=false;}
void BundleTransaction::commit(){
    if(!state->pending)return;
    auto installed=recognised(state->parent/L"ChordCanvas.vst3",state->journal.incoming,true);
    if(state->journal.participant!="-"){state->participant->verify(state->journal.token,state->journal.participant);state->participant->validate(state->journal.token,state->journal.participant,installed.version);}
    state->journal.decideCommit(state->parent);state->pending=false;
    // At most one retained transaction: subsequent staging requires recovery.
    try{finishCommitted(state->parent,state->journal,state->participant.get());}catch(...){ }
}
std::unique_ptr<BundleTransaction> BundleTransaction::begin(const std::filesystem::path& source,const std::filesystem::path& supplied,InstallMode mode,const std::function<void(Boundary)>& injection,std::shared_ptr<TransactionParticipant> participant){
    auto transaction=std::unique_ptr<BundleTransaction>(new BundleTransaction(std::make_unique<State>()));auto& state=*transaction->state;
    state.participant=std::move(participant);state.parent=safeParent(supplied);recoverLocked(state.parent,state.participant.get());
    auto payload=Payload::read(source);payload.validate(source);auto target=state.parent/L"ChordCanvas.vst3";
    bool existed=std::filesystem::exists(target);
    if(!existed && mode==InstallMode::update)throw std::runtime_error("ChordCanvas is not installed; run Setup.exe first");
    Payload previous;
    if(existed){
        previous=Payload::read(target);previous.validate(target,false);
        if(state.participant)state.participant->checkExisting(previous.version);
        auto installedVersion=Version::parse(previous.version),incomingVersion=Version::parse(payload.version);
        if(installedVersion>incomingVersion)throw std::runtime_error("A newer ChordCanvas version is installed; downgrade refused");
        if(installedVersion==incomingVersion){
            if(previous.receiptHash!=payload.receiptHash)throw std::runtime_error("Conflicting payloads use the same release version; replacement refused");
            try{previous.validate(target);state.result=InstallResult::alreadyCurrent;return transaction;}catch(const std::runtime_error&){ }
        }else previous.validate(target);
        for(auto& entry:previous.files)available(target/entry.relative);available(target/receiptName);
    }
    uintmax_t bytes=0;for(auto& file:payload.files)bytes+=std::filesystem::file_size(source/file.relative);
    if(std::filesystem::space(state.parent).available<bytes+16*1024*1024)throw std::runtime_error("Insufficient installation space");
    auto token=identifier();state.journal.token=std::string(token.begin(),token.end());state.journal.incoming=payload.receiptHash;state.journal.previous=existed ? previous.receiptHash : "-";
    auto stage=state.journal.stage(state.parent),backup=state.journal.backup(state.parent);
    if(std::filesystem::exists(stage) || std::filesystem::exists(backup))throw std::runtime_error("Staging identity collision");
    if(state.participant){state.journal.participant=state.participant->stage(state.journal.token);if(!hashValid(state.journal.participant))throw std::runtime_error("Invalid installation metadata recovery digest");}
    state.journal.create(state.parent);state.pending=true;
    try{
        if(state.participant){state.participant->apply(state.journal.token,state.journal.participant,payload.version);if(injection)injection(Boundary::participantApplied);}
        if(!std::filesystem::create_directory(stage))throw std::runtime_error("Staging directory collision");
        std::filesystem::copy_file(source/receiptName,stage/receiptName);
        for(auto& entry:payload.files){auto destination=stage/entry.relative;std::filesystem::create_directories(destination.parent_path());std::filesystem::copy_file(source/entry.relative,destination);}
        payload.validate(stage);if(injection)injection(Boundary::staged);
        if(existed){for(auto& entry:previous.files)available(target/entry.relative);renameOwned(target,backup);}
        if(injection)injection(Boundary::previousRetained);
        renameOwned(stage,target);if(injection)injection(Boundary::replaced);
        payload.validate(target);if(injection)injection(Boundary::validated);
    }catch(...){
        auto original=std::current_exception();
        try{transaction->rollback();}catch(...){state.pending=false;throw std::runtime_error("Installation failed and rollback needs recovery; previous bundle and marker preserved");}
        std::rethrow_exception(original);
    }
    return transaction;
}
void recoverInterruptedBundle(const std::filesystem::path& parent,std::shared_ptr<TransactionParticipant> participant){Lock lock;recoverLocked(safeParent(parent),participant.get());}
InstallResult installBundle(const std::filesystem::path& source,const std::filesystem::path& parent,InstallMode mode,const std::function<void(Boundary)>& injection){
    auto transaction=BundleTransaction::begin(source,parent,mode,injection);auto result=transaction->result();transaction->commit();return result;
}
}

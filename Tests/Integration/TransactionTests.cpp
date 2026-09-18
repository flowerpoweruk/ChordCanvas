#include "Packaging/Transaction.h"
#include "OwnedWorkspace.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <functional>
using namespace cc::packaging;
void require(bool ok,const char* why){if(!ok){std::cerr<<"FAIL: "<<why<<'\n';std::exit(1);}}
bool rejects(const std::function<void()>& fn){try{fn();return false;}catch(const std::exception&){return true;}}
void write(const std::filesystem::path& file,const std::string& value){std::ofstream out(file,std::ios::binary);out<<value;require(out.good(),"owned transaction fixture write");}
void payload(const std::filesystem::path& root,const std::filesystem::path& executable,const std::string& version){
    auto binary=root/L"Contents/x86_64-win/ChordCanvas.vst3",changes=root/L"Contents/resources/current.txt";
    std::filesystem::create_directories(binary.parent_path());std::filesystem::create_directory(changes.parent_path());
    std::filesystem::copy_file(executable,binary);write(changes,version);
    write(root/receiptName,std::string("ChordCanvasPayload1\n")+productIdentity+"\n"+version+"\nContents/x86_64-win/ChordCanvas.vst3\n"+sha256(binary)+"\tContents/x86_64-win/ChordCanvas.vst3\n"+sha256(changes)+"\tContents/resources/current.txt\n");
}
void abrupt(const std::filesystem::path& source,const std::filesystem::path& parent,int boundary,bool contend=false){
    std::wstring executable(32768,L'\0');auto length=GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size()));
    require(length>0 && length<executable.size(),"locate only this synthetic test executable");executable.resize(length);
    auto command=L"\""+executable+(contend ? L"\" --contend \"" : L"\" --abrupt \"")+source.wstring()+L"\" \""+parent.wstring()+L"\""+(contend ? L"" : L" "+std::to_wstring(boundary));
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    require(CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"launch own disposable crash-test child");
    CloseHandle(process.hThread);require(WaitForSingleObject(process.hProcess,15000)==WAIT_OBJECT_0,"crash-test child reaches the boundary promptly");
    DWORD exit=0;require(GetExitCodeProcess(process.hProcess,&exit) && exit==(contend ? 93 : 77),"actual child process reached the expected crash or mutex refusal");CloseHandle(process.hProcess);
}
int main(int argc,char** argv){
    if(argc==4 && std::string(argv[1])=="--contend"){
        try{auto transaction=BundleTransaction::begin(std::filesystem::path(argv[2]),std::filesystem::path(argv[3]),InstallMode::setup);}
        catch(const std::runtime_error& error){return std::string(error.what())=="Another installation is running" ? 93 : 3;}
        return 2;
    }
    if(argc==5 && std::string(argv[1])=="--abrupt"){
        int boundary=std::stoi(argv[4]);
        auto transaction=BundleTransaction::begin(std::filesystem::path(argv[2]),std::filesystem::path(argv[3]),InstallMode::setup,[=](Boundary point){if(static_cast<int>(point)==boundary)ExitProcess(77);});
        return 2; // The selected real boundary must be reached.
    }
    try {
    auto build=std::filesystem::path(argc>1 ? argv[1] : ".");auto root=ownedWorkspace(build,"transaction-tests-");
    auto older=root/L"source-1.9.0",newer=root/L"source-1.10.0";payload(older,build/L"core_tests.exe","1.9.0");payload(newer,build/L"core_tests.exe","1.10.0");
    auto installation=root/L"installed";std::filesystem::create_directory(installation);auto target=installation/L"ChordCanvas.vst3";
    auto unrelated=installation/L"OtherVendor.vst3";std::filesystem::create_directory(unrelated);write(unrelated/L"sentinel","preserved user sentinel");
    require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"updater refuses absent installation");
    require(installBundle(older,installation,InstallMode::setup)==InstallResult::installed,"fresh staged installation succeeds");
    auto original=sha256(target/receiptName),originalBinary=sha256(target/L"Contents/x86_64-win/ChordCanvas.vst3");
    for(auto boundary:{Boundary::staged,Boundary::previousRetained,Boundary::replaced,Boundary::validated}){
        require(rejects([&]{installBundle(newer,installation,InstallMode::update,[=](Boundary point){if(point==boundary)throw std::runtime_error("synthetic mid-transaction failure");});}),"injected failure reported");
        Payload::read(target).validate(target);require(sha256(target/receiptName)==original && sha256(target/L"Contents/x86_64-win/ChordCanvas.vst3")==originalBinary,"previous complete payload restored after each real boundary");
        for(auto& entry:std::filesystem::directory_iterator(installation))require(entry.path()==target || entry.path()==unrelated,"failed transaction leaves no owned stage or backup clutter");
    }
    auto binary=target/L"Contents/x86_64-win/ChordCanvas.vst3";
    // A real PE image mapping remains in use after both creation handles close.
    // This reproduces the native Live failure that an ordinary file lock missed.
    auto imageFile=CreateFileW(binary.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    require(imageFile!=INVALID_HANDLE_VALUE,"open owned executable for image-section regression");
    auto imageSection=CreateFileMappingW(imageFile,nullptr,PAGE_READONLY|SEC_IMAGE,0,0,nullptr);
    require(imageSection!=nullptr,"create real executable image section");
    auto imageView=MapViewOfFile(imageSection,FILE_MAP_READ,0,0,0);
    require(imageView!=nullptr,"map real executable image section");CloseHandle(imageSection);CloseHandle(imageFile);
    auto legacyProbe=CreateFileW(binary.c_str(),GENERIC_READ|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    require(legacyProbe!=INVALID_HANDLE_VALUE,"old read/delete probe actually misses the mapped image");CloseHandle(legacyProbe);
    require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"mapped PE image refuses update even without an ordinary open file handle");
    require(sha256(target/receiptName)==original && !std::filesystem::exists(installation/L".ChordCanvas.transaction"),"mapped-image refusal precedes mutation and preserves the complete installed receipt");
    require(UnmapViewOfFile(imageView)!=0,"unmap only the owned synthetic image view");
    imageView=nullptr;
    require(rejects([&]{installBundle(newer,installation,InstallMode::update,[&](Boundary point){if(point==Boundary::staged){
        auto opened=CreateFileW(binary.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        require(opened!=INVALID_HANDLE_VALUE,"open only owned late image fixture");auto section=CreateFileMappingW(opened,nullptr,PAGE_READONLY|SEC_IMAGE,0,0,nullptr);
        require(section!=nullptr,"create owned late image fixture");imageView=MapViewOfFile(section,FILE_MAP_READ,0,0,0);require(imageView!=nullptr,"map owned late image fixture");CloseHandle(section);CloseHandle(opened);
    }});}),"second pre-rename check refuses an image mapped after initial preflight");
    require(sha256(target/receiptName)==original && !std::filesystem::exists(installation/L".ChordCanvas.transaction"),"late mapped-image refusal restores owned staging without changing installed version");
    require(UnmapViewOfFile(imageView)!=0,"unmap only owned late image fixture");
    auto file=CreateFileW(binary.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(file!=INVALID_HANDLE_VALUE,"simulate receiving host's file lock");
    require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"in-use files stop update without force closure");CloseHandle(file);
    require(sha256(target/receiptName)==original,"file-lock refusal preserves installation");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::installed,"numeric 1.9 to 1.10 update succeeds");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::alreadyCurrent,"identical same-version update changes nothing");
    require(rejects([&]{installBundle(older,installation,InstallMode::setup);}),"setup also refuses downgrade");
    for(int boundary=0;boundary<4;++boundary){
        auto crash=root/("crash-update-"+std::to_string(boundary));std::filesystem::create_directory(crash);
        installBundle(older,crash,InstallMode::setup);auto before=sha256(crash/L"ChordCanvas.vst3"/receiptName);
        abrupt(newer,crash,boundary);require(std::filesystem::exists(crash/L".ChordCanvas.transaction"),"abrupt process leaves its durable marker");
        recoverInterruptedBundle(crash);Payload::read(crash/L"ChordCanvas.vst3").validate(crash/L"ChordCanvas.vst3");
        require(sha256(crash/L"ChordCanvas.vst3"/receiptName)==before,"actual abrupt-update recovery restores the previous complete bundle");
        require(std::distance(std::filesystem::directory_iterator(crash),std::filesystem::directory_iterator{})==1,"recovery removes known staging, backup and marker");
        auto empty=root/("crash-fresh-"+std::to_string(boundary));std::filesystem::create_directory(empty);
        abrupt(newer,empty,boundary);recoverInterruptedBundle(empty);require(std::filesystem::is_empty(empty),"abrupt fresh install recovers to an empty owned destination");
    }
    auto held=root/L"held-metadata-boundary";std::filesystem::create_directory(held);installBundle(older,held,InstallMode::setup);
    auto heldBefore=sha256(held/L"ChordCanvas.vst3"/receiptName);
    {auto transaction=BundleTransaction::begin(newer,held,InstallMode::update);require(Payload::read(held/L"ChordCanvas.vst3").version=="1.10.0","new bundle validated while previous retained through finalisation");
     require(std::filesystem::exists(held/L".ChordCanvas.transaction"),"finalisation still has a durable pending marker");abrupt(newer,held,0,true);}
    require(sha256(held/L"ChordCanvas.vst3"/receiptName)==heldBefore,"uncommitted finalisation rolls back when adapter scope ends");
    HANDLE retainedLock=INVALID_HANDLE_VALUE;
    {auto transaction=BundleTransaction::begin(newer,held,InstallMode::update);
     for(auto& entry:std::filesystem::directory_iterator(held))if(entry.path().filename().wstring().starts_with(L".ChordCanvas.previous."))retainedLock=CreateFileW((entry.path()/L"Contents/x86_64-win/ChordCanvas.vst3").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
     require(retainedLock!=INVALID_HANDLE_VALUE,"simulate a locked retained backup after validated replacement");transaction->commit();}
    Payload::read(held/L"ChordCanvas.vst3").validate(held/L"ChordCanvas.vst3");
    require(rejects([&]{installBundle(newer,held,InstallMode::update);}),"locked cleanup must recover before staging any additional backup");
    require(std::distance(std::filesystem::directory_iterator(held),std::filesystem::directory_iterator{})==3,"bounded retained state has only target, one backup and one marker");
    CloseHandle(retainedLock);recoverInterruptedBundle(held);
    require(std::distance(std::filesystem::directory_iterator(held),std::filesystem::directory_iterator{})==1,"committed cleanup recovers without rolling back the accepted bundle");
    write(held/L".ChordCanvas.transaction","unrelated marker\n");
    require(rejects([&]{recoverInterruptedBundle(held);}),"unrecognised journal never authorises deleting installation data");Payload::read(held/L"ChordCanvas.vst3").validate(held/L"ChordCanvas.vst3");
    auto unsafe=root/L"interrupted-with-user-file";std::filesystem::create_directory(unsafe);installBundle(older,unsafe,InstallMode::setup);
    abrupt(newer,unsafe,2);write(unsafe/L"ChordCanvas.vst3"/L"user-sentinel","preserved");
    require(rejects([&]{recoverInterruptedBundle(unsafe);}),"recovery refuses to destroy an unexpected user file");
    require(std::filesystem::exists(unsafe/L"ChordCanvas.vst3"/L"user-sentinel") && std::filesystem::exists(unsafe/L".ChordCanvas.transaction"),"failed recovery retains both user data and recovery marker");
    std::filesystem::remove(unsafe/L"ChordCanvas.vst3"/L"user-sentinel");recoverInterruptedBundle(unsafe);
    require(Payload::read(unsafe/L"ChordCanvas.vst3").version=="1.9.0","recovery succeeds after only the owned synthetic obstruction is removed");
    write(target/L"Contents/resources/current.txt","corrupted owned file");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::installed,"same-version damaged owned file repaired from validated identical manifest");Payload::read(target).validate(target);
    write(target/L"user-content.txt","preserved sentinel");require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"unknown file prevents destructive repair");
    require(std::filesystem::exists(target/L"user-content.txt") && std::filesystem::exists(unrelated/L"sentinel"),"user files and other vendor preserved");
    auto fresh=root/L"fresh-failure";std::filesystem::create_directory(fresh);
    require(rejects([&]{installBundle(newer,fresh,InstallMode::setup,[](Boundary point){if(point==Boundary::replaced)throw std::runtime_error("synthetic fresh-install failure");});}),"failed fresh installation reported");
    require(std::filesystem::is_empty(fresh),"failed fresh installation leaves no partial bundle");
    std::cout<<"PASS: fresh install, absent update, four real rollback boundaries, nine actual abrupt child-process recoveries, cross-process mutex, held finalisation rollback, bounded locked-backup cleanup, file locks, numeric update, repair, downgrade and unknown-file preservation\n";
    } catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}

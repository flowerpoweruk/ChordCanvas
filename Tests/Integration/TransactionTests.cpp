#include "Packaging/Transaction.h"
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
int main(int argc,char** argv){
    auto build=std::filesystem::path(argc>1 ? argv[1] : ".");auto root=build/("transaction-tests-"+std::to_string(GetCurrentProcessId()));
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
    auto file=CreateFileW(binary.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(file!=INVALID_HANDLE_VALUE,"simulate receiving host's file lock");
    require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"in-use files stop update without force closure");CloseHandle(file);
    require(sha256(target/receiptName)==original,"file-lock refusal preserves installation");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::installed,"numeric 1.9 to 1.10 update succeeds");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::alreadyCurrent,"identical same-version update changes nothing");
    require(rejects([&]{installBundle(older,installation,InstallMode::setup);}),"setup also refuses downgrade");
    write(target/L"Contents/resources/current.txt","corrupted owned file");
    require(installBundle(newer,installation,InstallMode::update)==InstallResult::installed,"same-version damaged owned file repaired from validated identical manifest");Payload::read(target).validate(target);
    write(target/L"user-content.txt","preserved sentinel");require(rejects([&]{installBundle(newer,installation,InstallMode::update);}),"unknown file prevents destructive repair");
    require(std::filesystem::exists(target/L"user-content.txt") && std::filesystem::exists(unrelated/L"sentinel"),"user files and other vendor preserved");
    auto fresh=root/L"fresh-failure";std::filesystem::create_directory(fresh);
    require(rejects([&]{installBundle(newer,fresh,InstallMode::setup,[](Boundary point){if(point==Boundary::replaced)throw std::runtime_error("synthetic fresh-install failure");});}),"failed fresh installation reported");
    require(std::filesystem::is_empty(fresh),"failed fresh installation leaves no partial bundle");
    std::cout<<"PASS: fresh install, absent update, four real rollback boundaries, file lock, numeric update, same version, repair, downgrade and user-file preservation\n";
}

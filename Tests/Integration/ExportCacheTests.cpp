#include "Export/TemporaryMidi.h"
#include "Export/Midi.h"
#include "OwnedWorkspace.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>

using namespace cc;
void require(bool ok,const char* why){if(!ok){std::cerr<<"FAIL: "<<why<<'\n';std::exit(1);}}
void stale(const std::filesystem::path& file){std::filesystem::last_write_time(file.parent_path(),std::filesystem::file_time_type::clock::now()-std::chrono::hours(25));}
int main(int argc,char** argv){
    auto root=ownedWorkspace(std::filesystem::path(argc>1 ? argv[1] : "."),"export-cache-tests-");
    auto bytes=midi(Timeline{});auto first=TemporaryMidi::prepare(bytes,root,2);auto firstFile=first->file();
    stale(firstFile);auto second=TemporaryMidi::prepare(bytes,root,2);auto secondFile=second->file();
    require(firstFile!=secondFile && std::filesystem::exists(firstFile),"active stale export lease survives cleanup; concurrent files distinct");
    bool rejected=false;try{TemporaryMidi::prepare(bytes,root,2);}catch(const std::exception&){rejected=true;}
    require(rejected && std::filesystem::exists(secondFile),"cache remains bounded; recent payload preserved");
    first.reset();auto third=TemporaryMidi::prepare(bytes,root,2);auto thirdFile=third->file();
    require(!std::filesystem::exists(firstFile) && std::filesystem::exists(secondFile),"only stale completed export removed; grace period preserved");
    std::ifstream read(secondFile,std::ios::binary);std::vector<uint8_t> roundTrip{std::istreambuf_iterator<char>(read),std::istreambuf_iterator<char>()};
    require(roundTrip==bytes,"MIDI payload survives native-lifetime simulation exactly");
    auto foreign=root/L"unrelated-user-folder";std::filesystem::create_directory(foreign);{std::ofstream out(foreign/L"sentinel");out<<"owned synthetic sentinel";}
    stale(secondFile);second.reset();rejected=false;
    try{TemporaryMidi::prepare(bytes,root,2);}catch(const std::exception&){rejected=true;}
    require(rejected && std::filesystem::exists(secondFile),"receiving host's open file prevents deletion even after completion");
    read.close();auto fourth=TemporaryMidi::prepare(bytes,root,2);
    require(!std::filesystem::exists(secondFile),"cleanup recovers after receiving host closes file");
    require(std::filesystem::exists(foreign/L"sentinel"),"unknown directories preserved");
    auto denied=root/L"not-a-directory";{std::ofstream out(denied);out<<"owned test file";}
    rejected=false;try{TemporaryMidi::prepare(bytes,denied);}catch(const std::exception&){rejected=true;}
    require(rejected && std::filesystem::is_regular_file(denied),"invalid cache root fails without changing user file");
    third.reset();require(std::filesystem::exists(thirdFile),"completion does not immediately delete its payload");
    std::cout<<"PASS: exclusive drag leases, exact bytes, 24-hour grace, stale cleanup, bounded admission and safe failure\n";
}

#include "Packaging/Payload.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <functional>
using namespace cc::packaging;
void require(bool ok,const char* why){if(!ok){std::cerr<<"FAIL: "<<why<<'\n';std::exit(1);}}
bool rejects(const std::function<void()>& fn){try{fn();return false;}catch(const std::exception&){return true;}}
void write(const std::filesystem::path& file,const std::string& value){std::ofstream out(file,std::ios::binary);out<<value;require(out.good(),"owned fixture write");}
int main(int argc,char** argv){
    auto build=std::filesystem::path(argc>1 ? argv[1] : ".");auto root=build/("payload-tests-"+std::to_string(GetCurrentProcessId()));
    auto binary=root/L"Contents/x86_64-win/ChordCanvas.vst3";std::filesystem::create_directories(binary.parent_path());
    std::filesystem::copy_file(build/L"core_tests.exe",binary); // Synthetic AMD64 payload, never represented as a VST3.
    auto text=root/L"Contents/resources/synthetic.txt";std::filesystem::create_directory(text.parent_path());write(text,"abc");
    require(sha256(text)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad","independent published SHA256 test vector");
    auto prefix=std::string("ChordCanvasPayload1\n")+productIdentity+"\n1.10.0\nContents/x86_64-win/ChordCanvas.vst3\n";
    auto receipt=prefix+sha256(binary)+"\tContents/x86_64-win/ChordCanvas.vst3\n"+sha256(text)+"\tContents/resources/synthetic.txt\n";
    write(root/receiptName,receipt);auto payload=Payload::read(root);payload.validate(root);
    write(text,"tampered");require(rejects([&]{payload.validate(root);}),"payload hash mismatch fails");payload.validate(root,false);
    write(text,"abc");write(root/L"unowned.txt","user sentinel");require(rejects([&]{payload.validate(root,false);}),"repair cannot overwrite unknown user files");std::filesystem::remove(root/L"unowned.txt");
    std::filesystem::create_directory(root/L"unowned-directory");require(rejects([&]{payload.validate(root);}),"unexpected empty directories fail ownership checks");std::filesystem::remove(root/L"unowned-directory");
    for(auto path:{"../escape","Contents/../escape","/absolute","C:/absolute","Contents/CON.txt","Contents/name.","Contents/name ","Contents/name:stream","Contents/back\\slash"}){
        write(root/receiptName,receipt+std::string(64,'0')+"\t"+path+"\n");require(rejects([&]{Payload::read(root);}),"unsafe manifest path rejected");
    }
    write(root/receiptName,receipt+sha256(text)+"\tContents/RESOURCES/Synthetic.txt\n");require(rejects([&]{Payload::read(root);}),"Windows case-insensitive duplicate rejected");
    write(root/receiptName,receipt);write(binary,"not a PE file");require(rejects([&]{payload.validate(root);}),"corrupt binary fails");
    write(root/receiptName,prefix+sha256(binary)+"\tContents/x86_64-win/ChordCanvas.vst3\n"+sha256(text)+"\tContents/resources/synthetic.txt\n");require(rejects([&]{Payload::read(root).validate(root);}),"hash-consistent non-AMD64 file fails architecture check");
    write(root/receiptName,"Unrelated product\n");require(rejects([&]{Payload::read(root);}),"unrecognised installation refused");
    std::cout<<"PASS: SHA256, complete owned file tree, numeric version, malformed paths, corruption and AMD64 PE validation\n";
}

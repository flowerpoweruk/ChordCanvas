#include "Packaging/Transaction.h"
#include "Packaging/RegistryImage.h"
#include "OwnedWorkspace.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
using namespace cc::packaging;
void check(bool test,const char* name){if(!test)throw std::runtime_error(name);}
template<class F> bool rejects(F&& operation){try{operation();return false;}catch(const std::runtime_error&){return true;}}
void text(const std::filesystem::path& file,const std::string& value){std::ofstream output(file,std::ios::binary);output<<value;check(output.good(),"write own synthetic fixture");}
void payload(const std::filesystem::path& root,const std::filesystem::path& exe,const std::string& version){
    auto binary=root/L"Contents/x86_64-win/ChordCanvas.vst3";std::filesystem::create_directories(binary.parent_path());std::filesystem::copy_file(exe,binary);
    text(root/receiptName,std::string("ChordCanvasPayload1\n")+productIdentity+'\n'+version+"\nContents/x86_64-win/ChordCanvas.vst3\n"+sha256(binary)+"\tContents/x86_64-win/ChordCanvas.vst3\n");
}
const std::vector<std::wstring> names={L"DisplayName",L"DisplayVersion",L"QuietUninstallString"};
std::string property(const RegistryImage& image,const std::wstring& name){
    for(auto& value:image.values)if(value.name==name){check(value.type==REG_SZ && value.bytes.size()%2==0,"typed synthetic registry string");std::wstring wide(value.bytes.size()/2,L'\0');std::memcpy(wide.data(),value.bytes.data(),value.bytes.size());wide.pop_back();std::string ascii;for(auto character:wide){check(character>=0 && character<=127,"synthetic ASCII registry property");ascii.push_back(static_cast<char>(character));}return ascii;}
    return {};
}
void metadata(const std::wstring& path,const std::string& version){
    check(path.starts_with(L"Software\\ChordCanvasTests\\Participant\\participant-tests-"),"mutate only own disposable HKCU namespace");
    HKEY key=nullptr;check(RegCreateKeyExW(HKEY_CURRENT_USER,path.c_str(),0,nullptr,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE|KEY_WOW64_64KEY,nullptr,&key,nullptr)==ERROR_SUCCESS,"open own synthetic metadata");
    auto set=[&](const wchar_t* name,const std::wstring& value){auto result=RegSetValueExW(key,name,0,REG_SZ,reinterpret_cast<const BYTE*>(value.c_str()),static_cast<DWORD>((value.size()+1)*2));if(result!=ERROR_SUCCESS){RegCloseKey(key);throw std::runtime_error("Synthetic metadata write failed");}};
    set(L"DisplayName",L"ChordCanvas");set(L"DisplayVersion",std::wstring(version.begin(),version.end()));RegCloseKey(key);
}
// A real typed registry participant in an isolated HKCU namespace. The native
// installer must additionally supply uninstaller-directory recovery; this
// fixture does not claim to be that complete production participant.
struct Participant final:TransactionParticipant {
    std::filesystem::path parent,file;std::wstring key;
    Participant(std::filesystem::path p,std::wstring k):parent(std::move(p)),file(parent/L".ChordCanvas.registry"),key(std::move(k)){}
    RegistryImage image(const std::string& digest){check(plainPath(file) && sha256(file)==digest,"changed descriptor must stop before bundle or registry recovery");return RegistryImage::read(file);}
    std::string stage(const std::string&) override{
        if(std::filesystem::exists(file)){RegistryImage::read(file);check(!std::filesystem::exists(parent/L".ChordCanvas.transaction"),"orphan retirement requires absent primary marker");std::filesystem::remove(file);}
        RegistryImage::capture(HKEY_CURRENT_USER,key).write(file);return sha256(file);
    }
    void verify(const std::string&,const std::string& digest) override{
        image(digest);auto current=RegistryImage::capture(HKEY_CURRENT_USER,key);
        for(auto& value:current.values)check(std::find(names.begin(),names.end(),value.name)!=names.end(),"unexpected metadata blocks both participant changes");
    }
    void apply(const std::string& token,const std::string& digest,const std::string&) override{verify(token,digest);check(image(digest)==RegistryImage::capture(HKEY_CURRENT_USER,key),"metadata changed after snapshot");}
    void validate(const std::string& token,const std::string& digest,const std::string& version) override{verify(token,digest);checkExisting(version);}
    void rollback(const std::string&,const std::string& digest) override{image(digest).restore(HKEY_CURRENT_USER,key,names);}
    void cleanup(const std::string&,const std::string& digest,bool) override{image(digest);}
    void retire(const std::string&,const std::string& digest) override{image(digest);std::filesystem::remove(file);}
    void checkExisting(const std::string& version) override{auto current=RegistryImage::capture(HKEY_CURRENT_USER,key);check(property(current,L"DisplayName")=="ChordCanvas" && property(current,L"DisplayVersion")==version,"bundle and registration must identify the same installed version");}
};
void install(const std::filesystem::path& source,const std::filesystem::path& parent,const std::wstring& key){auto p=std::make_shared<Participant>(parent,key);auto t=BundleTransaction::begin(source,parent,InstallMode::setup,{},p);if(t->result()==InstallResult::installed)metadata(key,Payload::read(source).version);t->commit();}
void abrupt(const std::filesystem::path& source,const std::filesystem::path& parent,const std::wstring& key,int boundary){
    std::wstring exe(32768,L'\0');auto size=GetModuleFileNameW(nullptr,exe.data(),static_cast<DWORD>(exe.size()));check(size>0 && size<exe.size(),"only own test executable");exe.resize(size);
    auto command=L"\""+exe+L"\" --kill \""+source.wstring()+L"\" \""+parent.wstring()+L"\" \""+key+L"\" "+std::to_wstring(boundary);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    check(CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"launch own disposable participant child");CloseHandle(process.hThread);
    check(WaitForSingleObject(process.hProcess,15000)==WAIT_OBJECT_0,"participant child reaches boundary");DWORD exit=0;check(GetExitCodeProcess(process.hProcess,&exit) && exit==77,"actual child stopped after metadata mutation");CloseHandle(process.hProcess);
}
int main(int argc,char** argv){try{
    if(argc==6 && std::string(argv[1])=="--kill"){
        auto source=std::filesystem::path(argv[2]),parent=std::filesystem::path(argv[3]);std::wstring key=std::filesystem::path(argv[4]).wstring();int boundary=std::stoi(argv[5]);
        auto p=std::make_shared<Participant>(parent,key);auto version=Payload::read(source).version;
        auto t=BundleTransaction::begin(source,parent,InstallMode::setup,[&](Boundary point){if(static_cast<int>(point)==boundary){metadata(key,version);ExitProcess(77);}},p);return 2;
    }
    auto build=std::filesystem::path(argc>1 ? argv[1] : "."),root=ownedWorkspace(build,"participant-tests-");
    auto old=root/L"old",next=root/L"next";payload(old,build/L"core_tests.exe","1.9.0");payload(next,build/L"core_tests.exe","1.10.0");
    auto prefix=L"Software\\ChordCanvasTests\\Participant\\"+root.filename().wstring();
    for(int boundary=0;boundary<5;++boundary)for(bool fresh:{false,true}){
        auto name=L"case-"+std::to_wstring(boundary)+(fresh ? L"-fresh" : L"-update"),key=prefix+L"\\"+name;auto parent=root/name;std::filesystem::create_directory(parent);
        if(!fresh)install(old,parent,key);
        auto before=RegistryImage::capture(HKEY_CURRENT_USER,key);abrupt(next,parent,key,boundary);
        check(property(RegistryImage::capture(HKEY_CURRENT_USER,key),L"DisplayVersion")=="1.10.0","new registry value existed at real process interruption");
        auto p=std::make_shared<Participant>(parent,key);auto count=std::distance(std::filesystem::directory_iterator(parent),std::filesystem::directory_iterator{});
        check(rejects([&]{recoverInterruptedBundle(parent);}) && count==std::distance(std::filesystem::directory_iterator(parent),std::filesystem::directory_iterator{}),"missing participant cannot change primary recovery files");
        recoverInterruptedBundle(parent,p);check(RegistryImage::capture(HKEY_CURRENT_USER,key)==before,"same decision restores exact prior metadata");
        check(fresh ? std::filesystem::is_empty(parent) : Payload::read(parent/L"ChordCanvas.vst3").version=="1.9.0","bundle version or fresh absence restored consistently");
        RegistryImage{}.restore(HKEY_CURRENT_USER,key,names);
    }
    auto parent=root/L"finalisation";auto key=prefix+L"\\finalisation";std::filesystem::create_directory(parent);install(old,parent,key);
    {auto p=std::make_shared<Participant>(parent,key);auto t=BundleTransaction::begin(next,parent,InstallMode::update,{},p);metadata(key,"wrong-version");check(rejects([&]{t->commit();}),"mismatched final metadata cannot commit bundle");}
    check(Payload::read(parent/L"ChordCanvas.vst3").version=="1.9.0" && property(RegistryImage::capture(HKEY_CURRENT_USER,key),L"DisplayVersion")=="1.9.0","failed finalisation restores both prior identities");
    abrupt(next,parent,key,2);auto snapshot=parent/L".ChordCanvas.registry";
    std::ifstream original(snapshot,std::ios::binary);std::string saved{std::istreambuf_iterator<char>(original),std::istreambuf_iterator<char>()};original.close();text(snapshot,saved+"changed");
    auto p=std::make_shared<Participant>(parent,key);check(rejects([&]{recoverInterruptedBundle(parent,p);}),"tampered descriptor refuses joint recovery");
    check(Payload::read(parent/L"ChordCanvas.vst3").version=="1.10.0" && property(RegistryImage::capture(HKEY_CURRENT_USER,key),L"DisplayVersion")=="1.10.0","descriptor failure happens before changing either current participant");
    text(snapshot,saved);recoverInterruptedBundle(parent,p);install(next,parent,key);p->checkExisting("1.10.0");
    check(!std::filesystem::exists(parent/L".ChordCanvas.transaction") && !std::filesystem::exists(snapshot),"successful commit retires both recovery records");
    RegistryImage{}.restore(HKEY_CURRENT_USER,key,names);
    check(RegDeleteKeyExW(HKEY_CURRENT_USER,prefix.c_str(),KEY_WOW64_64KEY,0)==ERROR_SUCCESS,"remove only exact empty generated fixture parent");
    std::cout<<"PASS: eleven real interrupted joint bundle/registry recoveries, fresh absence, exact previous metadata, missing participant refusal, descriptor tampering before mutation, failed finalisation rollback and coherent successful commit; synthetic HKCU only\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

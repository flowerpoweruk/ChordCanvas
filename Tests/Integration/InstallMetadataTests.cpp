#include "Packaging/InstallMetadata.h"
#include "OwnedWorkspace.h"
#include <fstream>
#include <iostream>
using namespace cc::packaging;
void check(bool test,const char* message){if(!test)throw std::runtime_error(message);}
template<class F> bool rejects(F&& operation){try{operation();return false;}catch(const std::runtime_error&){return true;}}
void write(const std::filesystem::path& file,const std::string& value){std::ofstream stream(file,std::ios::binary);stream<<value;check(stream.good(),"write only owned synthetic metadata fixture");}
void payload(const std::filesystem::path& root,const std::filesystem::path& exe,const std::string& version){auto binary=root/L"Contents/x86_64-win/ChordCanvas.vst3";std::filesystem::create_directories(binary.parent_path());std::filesystem::copy_file(exe,binary);write(root/receiptName,std::string("ChordCanvasPayload1\n")+productIdentity+'\n'+version+"\nContents/x86_64-win/ChordCanvas.vst3\n"+sha256(binary)+"\tContents/x86_64-win/ChordCanvas.vst3\n");}
void registration(const std::filesystem::path& directory,const std::wstring& path,const std::string& version){
    check(path.starts_with(L"Software\\ChordCanvasTests\\InstallMetadata\\metadata-tests-"),"registry fixture is isolated HKCU UUID namespace");HKEY key=nullptr;
    check(RegCreateKeyExW(HKEY_CURRENT_USER,path.c_str(),0,nullptr,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE|KEY_WOW64_64KEY,nullptr,&key,nullptr)==ERROR_SUCCESS,"create only own synthetic registration");
    auto set=[&](const wchar_t* name,const std::wstring& value){auto result=RegSetValueExW(key,name,0,REG_SZ,reinterpret_cast<const BYTE*>(value.c_str()),static_cast<DWORD>((value.size()+1)*2));if(result!=ERROR_SUCCESS){RegCloseKey(key);throw std::runtime_error("Fixture registry write failed");}};
    set(L"DisplayName",L"ChordCanvas");set(L"DisplayVersion",{version.begin(),version.end()});set(L"Publisher",L"ChordCanvas");set(L"UninstallString",L"\""+(directory/L"unins000.exe").wstring()+L"\"");set(L"QuietUninstallString",L"\""+(directory/L"unins000.exe").wstring()+L"\" /SILENT");RegCloseKey(key);
}
void finalise(const std::filesystem::path& directory,const std::filesystem::path& exe,const std::wstring& key,const std::string& version){std::filesystem::copy_file(exe,directory/L"unins000.exe");write(directory/L"unins000.dat","synthetic uninstaller data "+version);registration(directory,key,version);}
std::shared_ptr<InstallMetadata> participant(const std::filesystem::path& product,const std::filesystem::path& exe,const std::wstring& key,const std::string& version){return std::make_shared<InstallMetadata>(product,HKEY_CURRENT_USER,key,exe,version);}
void install(const std::filesystem::path& source,const std::filesystem::path& parent,const std::filesystem::path& product,const std::filesystem::path& exe,const std::wstring& key){auto version=Payload::read(source).version;auto p=participant(product,exe,key,version);auto t=BundleTransaction::begin(source,parent,InstallMode::setup,{},p);if(t->result()==InstallResult::installed){finalise(product,exe,key,version);p->seal();}t->commit();}
void abrupt(const std::filesystem::path& source,const std::filesystem::path& parent,const std::filesystem::path& product,const std::filesystem::path& exe,const std::wstring& key,int point){
    std::wstring current(32768,L'\0');auto length=GetModuleFileNameW(nullptr,current.data(),static_cast<DWORD>(current.size()));check(length>0 && length<current.size(),"locate only own disposable metadata test");current.resize(length);
    auto command=L"\""+current+L"\" --kill \""+source.wstring()+L"\" \""+parent.wstring()+L"\" \""+product.wstring()+L"\" \""+exe.wstring()+L"\" \""+key+L"\" "+std::to_wstring(point);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};check(CreateProcessW(current.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"launch only own synthetic metadata child");CloseHandle(process.hThread);check(WaitForSingleObject(process.hProcess,15000)==WAIT_OBJECT_0,"metadata child reaches real boundary");DWORD code=0;check(GetExitCodeProcess(process.hProcess,&code) && code==77,"metadata child actually exits at boundary");CloseHandle(process.hProcess);
}
int main(int argc,char** argv){try{
    if(argc==8 && std::string(argv[1])=="--kill"){
        auto source=std::filesystem::path(argv[2]),parent=std::filesystem::path(argv[3]),product=std::filesystem::path(argv[4]),exe=std::filesystem::path(argv[5]);auto key=std::filesystem::path(argv[6]).wstring();auto version=Payload::read(source).version;int point=std::stoi(argv[7]);auto p=std::make_shared<InstallMetadata>(product,HKEY_CURRENT_USER,key,exe,version,[=](MetadataBoundary boundary){if(point>=10 && static_cast<int>(boundary)==point-10)ExitProcess(77);});
        auto t=BundleTransaction::begin(source,parent,InstallMode::setup,[&](Boundary boundary){if(point<5 && static_cast<int>(boundary)==point)ExitProcess(77);},p);
        finalise(product,exe,key,version);if(point==5)ExitProcess(77);p->seal();if(point==6)ExitProcess(77);if(point==14)t->rollback();else if(point>=15)t->commit();return 2;
    }
    auto build=std::filesystem::absolute(std::filesystem::path(argc>1 ? argv[1] : "."));auto root=ownedWorkspace(build,"metadata-tests-");auto exe=build/L"core_tests.exe",old=root/L"old",next=root/L"next",future=root/L"future";payload(old,exe,"1.9.0");payload(next,exe,"1.10.0");payload(future,exe,"1.11.0");auto prefix=L"Software\\ChordCanvasTests\\InstallMetadata\\"+root.filename().wstring();
    for(int point=0;point<7;++point)for(bool fresh:{false,true}){
        auto name=L"case-"+std::to_wstring(point)+(fresh ? L"-fresh" : L"-update");auto base=root/name,parent=base/L"VST3",product=base/L"ChordCanvas";auto key=prefix+L"\\"+name;std::filesystem::create_directories(parent);
        if(!fresh)install(old,parent,product,exe,key);auto before=RegistryImage::capture(HKEY_CURRENT_USER,key);auto oldMetadata=fresh ? "-" : sha256(product/L"chordcanvas.install");
        abrupt(next,parent,product,exe,key,point);auto p=participant(product,exe,key,"1.10.0");recoverInterruptedBundle(parent,p);
        check(RegistryImage::capture(HKEY_CURRENT_USER,key)==before,"typed uninstall registration restored exactly after real process interruption");
        if(fresh)check(std::filesystem::is_empty(parent) && !std::filesystem::exists(product),"fresh interruption restores bundle and uninstaller absence");
        else{check(Payload::read(parent/L"ChordCanvas.vst3").version=="1.9.0" && sha256(product/L"chordcanvas.install")==oldMetadata,"both prior payload and uninstaller receipt restored");p->checkExisting("1.9.0");}
        check(!std::filesystem::exists(base/L".ChordCanvas.metadata") && !std::filesystem::exists(base/L".ChordCanvas.metadata.retired"),"owned auxiliary directory and retirement marker cleaned");RegistryImage{}.restore(HKEY_CURRENT_USER,key,InstallMetadata::registrationNames());
    }
    for(int boundary=0;boundary<9;++boundary){
        auto name=L"metadata-boundary-"+std::to_wstring(boundary);auto base=root/name,parent=base/L"VST3",product=base/L"ChordCanvas";auto key=prefix+L"\\"+name;std::filesystem::create_directories(parent);install(old,parent,product,exe,key);
        abrupt(next,parent,product,exe,key,10+boundary);auto p=participant(product,exe,key,"1.10.0");
        if(boundary>=5){
            p->checkExisting("1.10.0");auto futureParticipant=participant(product,exe,key,"1.11.0");{auto t=BundleTransaction::begin(future,parent,InstallMode::update,{},futureParticipant);}
            p->checkExisting("1.10.0");check(Payload::read(parent/L"ChordCanvas.vst3").version=="1.10.0","committed retirement interruptions retain the coherent new installation");
        }else if(boundary==0){
            p->checkExisting("1.9.0");{auto t=BundleTransaction::begin(next,parent,InstallMode::update,{},p);}p->checkExisting("1.9.0");
        }else{recoverInterruptedBundle(parent,p);p->checkExisting("1.9.0");}
        check(!std::filesystem::exists(base/L".ChordCanvas.metadata") && !std::filesystem::exists(base/L".ChordCanvas.metadata.retired") && !std::filesystem::exists(parent/L".ChordCanvas.transaction"),"interrupted metadata staging/rollback/retirement cleans up bounded owned recovery state");RegistryImage{}.restore(HKEY_CURRENT_USER,key,InstallMetadata::registrationNames());
    }
    auto base=root/L"normal",parent=base/L"VST3",product=base/L"ChordCanvas";auto key=prefix+L"\\normal";std::filesystem::create_directories(parent);install(old,parent,product,exe,key);auto before=RegistryImage::capture(HKEY_CURRENT_USER,key);
    {auto p=participant(product,exe,key,"1.10.0");auto t=BundleTransaction::begin(next,parent,InstallMode::update,{},p);finalise(product,exe,key,"1.10.0");check(rejects([&]{t->commit();}),"commit refuses unfinished uninstaller ownership receipt");}
    check(Payload::read(parent/L"ChordCanvas.vst3").version=="1.9.0" && RegistryImage::capture(HKEY_CURRENT_USER,key)==before,"failed finalisation restores bundle, directory and registry together");
    install(next,parent,product,exe,key);auto p=participant(product,exe,key,"1.10.0");p->checkExisting("1.10.0");
    HANDLE engineData=CreateFileW((product/L"unins000.dat").c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);check(engineData!=INVALID_HANDLE_VALUE,"simulate Inno's exclusive already-loaded uninstall data");
    p->checkUninstall("1.10.0");check(rejects([&]{p->checkExisting("1.10.0");}),"uninstall data exemption is unavailable to install/update integrity checks");CloseHandle(engineData);
    write(product/L"user-owned.txt","preserved user sentinel");check(rejects([&]{install(next,parent,product,exe,key);}) && std::filesystem::exists(product/L"user-owned.txt"),"unexpected metadata file blocks even same-version work");std::filesystem::remove(product/L"user-owned.txt");
    HANDLE reader=CreateFileW((product/L"unins000.exe").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);check(reader!=INVALID_HANDLE_VALUE,"simulate an active old uninstaller");install(next,parent,product,exe,key);check(rejects([&]{install(future,parent,product,exe,key);}),"loaded uninstaller replacement is refused without forced termination; identical same-version remains a no-op");CloseHandle(reader);
    RegistryImage{}.restore(HKEY_CURRENT_USER,key,InstallMetadata::registrationNames());check(RegDeleteKeyExW(HKEY_CURRENT_USER,prefix.c_str(),KEY_WOW64_64KEY,0)==ERROR_SUCCESS,"remove only exact empty owned test registry parent");
    std::cout<<"PASS: twenty-three actual abrupt processes across bundle, metadata, registration, sealing and retirement boundaries; exact old directory/registry restoration, fresh absence, interrupted orphan retirement, unfinished-finalisation rollback, coherent update, unknown-file and loaded-uninstaller replacement refusal; synthetic payloads and HKCU only\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

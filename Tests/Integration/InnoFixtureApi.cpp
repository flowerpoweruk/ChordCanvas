// Development-only adapter. This is never linked into the shipping helper.
// A build-time fixed UUID workspace and ownership marker prevent this adapter
// from accessing the product's Program Files directory or its HKLM registration.
#include "Packaging/InstallMetadata.h"
#include "InnoFixtureConfig.h"
#include <fstream>
#include <string>
using namespace cc::packaging;
namespace {
BundleTransaction* active=nullptr;
InstallMetadata* metadata=nullptr;
std::filesystem::path workspace(){
    std::filesystem::path root=CC_INNO_FIXTURE_ROOT;
    if(root.filename().wstring()!=L"inno-runtime-" CC_INNO_FIXTURE_ID)throw std::runtime_error("Fixture directory identity refused");
    for(auto ancestor=root;!ancestor.empty();){if(!plainPath(ancestor))throw std::runtime_error("Unsafe fixture ancestor");auto next=ancestor.parent_path();if(next==ancestor)break;ancestor=next;}
    std::ifstream marker(root/L"fixture.owner",std::ios::binary);std::string text{std::istreambuf_iterator<char>(marker),{}};
    if(text!="ChordCanvasInnoFixture1\n" CC_INNO_FIXTURE_ID_UTF8 "\n")throw std::runtime_error("Fixture ownership refused");return root;
}
std::wstring registration(){return L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{" CC_INNO_FIXTURE_ID L"}_is1";}
std::filesystem::path module(){HMODULE image=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&module),&image);std::wstring text(32768,L'\0');auto count=GetModuleFileNameW(image,text.data(),static_cast<DWORD>(text.size()));if(!count || count>=text.size())throw std::runtime_error("Fixture helper unavailable");text.resize(count);return text;}
void trace(const std::string& text){try{std::ofstream file(workspace()/L"api-events.txt",std::ios::binary|std::ios::app);file<<text<<'\n';}catch(...){}}
int failure(const std::exception& error){trace(std::string("error ")+error.what());auto text=std::string(error.what());if(text.find("Setup.exe first")!=std::string::npos)return -2;if(text.find("downgrade refused")!=std::string::npos)return -3;if(text.find("in use")!=std::string::npos || text.find("close the host")!=std::string::npos)return -4;return -1;}
void dispose(){auto* previous=active;active=nullptr;metadata=nullptr;delete previous;}
bool expected(const wchar_t* root){return root && std::filesystem::absolute(root).lexically_normal()==workspace()/L"ChordCanvas";}
}
extern "C" __declspec(dllexport) int WINAPI CC_Supported(){try{workspace();return 0;}catch(...){return -10;}}
extern "C" __declspec(dllexport) int WINAPI CC_Begin(const wchar_t* source,const wchar_t* appDirectory,int update){try{
    if(active || !source || !expected(appDirectory))return -12;auto root=workspace();auto parent=root/L"VST3";
    auto payload=Payload::read(source);payload.validate(source);
    auto participant=std::make_shared<InstallMetadata>(root/L"ChordCanvas",HKEY_CURRENT_USER,registration(),module(),payload.version);
    auto transaction=BundleTransaction::begin(source,parent,update ? InstallMode::update : InstallMode::setup,{},participant);
    if(transaction->result()==InstallResult::alreadyCurrent){trace("already current "+payload.version);return 1;}
    metadata=participant.get();active=transaction.release();trace("begin "+payload.version);return 0;
}catch(const std::exception& error){return failure(error);}catch(...){return -1;}}
extern "C" __declspec(dllexport) int WINAPI CC_Finalise(){try{
    if(!active || !metadata)return -1;
    if(std::filesystem::exists(workspace()/L"fail-finalise")){trace("injected finalise failure");return -1;}
    metadata->seal();active->commit();dispose();trace("committed");return 0;
}catch(const std::exception& error){return failure(error);}catch(...){return -1;}}
extern "C" __declspec(dllexport) int WINAPI CC_Abort(){try{if(active){active->rollback();dispose();trace("rolled back");}return 0;}catch(const std::exception& error){failure(error);return -5;}catch(...){return -5;}}
extern "C" __declspec(dllexport) int WINAPI CC_UninstallCheck(const wchar_t* appDirectory){try{
    if(!expected(appDirectory))return -12;auto root=workspace(),bundle=root/L"VST3/ChordCanvas.vst3";
    if(std::filesystem::exists(root/L"VST3/.ChordCanvas.transaction"))return -5;
    auto payload=Payload::read(bundle);payload.validate(bundle);InstallMetadata current(root/L"ChordCanvas",HKEY_CURRENT_USER,registration(),module(),payload.version);current.checkUninstall(payload.version);trace("uninstall checked "+payload.version);return 0;
}catch(const std::exception& error){return failure(error);}catch(...){return -1;}}

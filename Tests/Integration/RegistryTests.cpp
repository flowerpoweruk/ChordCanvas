#include "Packaging/RegistryImage.h"
#include "OwnedWorkspace.h"
#include <fstream>
#include <iostream>
#include <cstring>
using namespace cc::packaging;
void check(bool test,const char* name){if(!test){std::cerr<<"FAIL: "<<name<<'\n';std::exit(1);}}
template<class F> bool rejects(F&& operation){try{operation();return false;}catch(const std::runtime_error&){return true;}}
void stringValue(HKEY key,const wchar_t* name,const wchar_t* value){check(RegSetValueExW(key,name,0,REG_SZ,reinterpret_cast<const BYTE*>(value),static_cast<DWORD>((wcslen(value)+1)*2))==ERROR_SUCCESS,"write only synthetic registry string");}
int main(int argc,char** argv){
    auto workspace=ownedWorkspace(std::filesystem::path(argc>1 ? argv[1] : "."),"registry-tests-");
    auto path=L"Software\\ChordCanvasTests\\"+workspace.filename().wstring();
    auto absent=RegistryImage::capture(HKEY_CURRENT_USER,path);check(!absent.exists && absent.values.empty(),"new UUID leaf is absent");
    absent.write(workspace/L"absent.snapshot");check(RegistryImage::read(workspace/L"absent.snapshot")==absent,"absent image round trip");
    HKEY key=nullptr;DWORD created=0;check(RegCreateKeyExW(HKEY_CURRENT_USER,path.c_str(),0,nullptr,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE|KEY_WOW64_64KEY,nullptr,&key,&created)==ERROR_SUCCESS && created==REG_CREATED_NEW_KEY,"exclusive disposable HKCU UUID leaf");
    stringValue(key,L"DisplayName",L"ChordCanvas \u2014 synthetic \u97f3\u697d");
    DWORD number=9;uint64_t big=12000000000ull;BYTE binary[]={0,1,0,255};const wchar_t expanded[]=L"%ProgramFiles%\\ChordCanvas",multi[]=L"first\0second\0";
    check(RegSetValueExW(key,L"Build",0,REG_DWORD,reinterpret_cast<BYTE*>(&number),4)==ERROR_SUCCESS,"synthetic DWORD");
    check(RegSetValueExW(key,L"BuildLong",0,REG_QWORD,reinterpret_cast<BYTE*>(&big),8)==ERROR_SUCCESS,"synthetic QWORD");
    check(RegSetValueExW(key,L"Payload",0,REG_BINARY,binary,sizeof(binary))==ERROR_SUCCESS,"synthetic binary");
    check(RegSetValueExW(key,L"InstallLocation",0,REG_EXPAND_SZ,reinterpret_cast<const BYTE*>(expanded),sizeof(expanded))==ERROR_SUCCESS,"synthetic expandable string");
    check(RegSetValueExW(key,L"Options",0,REG_MULTI_SZ,reinterpret_cast<const BYTE*>(multi),sizeof(multi))==ERROR_SUCCESS,"synthetic multistring");
    RegCloseKey(key);key=nullptr;
    auto image=RegistryImage::capture(HKEY_CURRENT_USER,path);check(image.exists && image.values.size()==6,"capture six native 64-bit typed values");
    image.write(workspace/L"previous.snapshot");check(RegistryImage::read(workspace/L"previous.snapshot")==image,"full typed byte-exact snapshot round trip");
    check(rejects([&]{image.write(workspace/L"previous.snapshot");}),"exclusive image creation never replaces an existing snapshot");
    check(RegOpenKeyExW(HKEY_CURRENT_USER,path.c_str(),0,KEY_READ|KEY_WRITE|KEY_WOW64_64KEY,&key)==ERROR_SUCCESS,"open own fixture leaf");
    stringValue(key,L"DisplayName",L"New synthetic version");stringValue(key,L"QuietUninstallString",L"synthetic command only");RegCloseKey(key);key=nullptr;
    image.restore(HKEY_CURRENT_USER,path,{L"QuietUninstallString"});check(RegistryImage::capture(HKEY_CURRENT_USER,path)==image,"restore previous exact values and remove only allowed forward values");
    check(RegOpenKeyExW(HKEY_CURRENT_USER,path.c_str(),0,KEY_WRITE|KEY_WOW64_64KEY,&key)==ERROR_SUCCESS,"open own fixture for unexpected value");stringValue(key,L"ExternalValue",L"preserved");RegCloseKey(key);key=nullptr;
    auto obstructed=RegistryImage::capture(HKEY_CURRENT_USER,path);
    check(rejects([&]{image.restore(HKEY_CURRENT_USER,path,{});}) && RegistryImage::capture(HKEY_CURRENT_USER,path)==obstructed,"unexpected value stops before any registry mutation");
    check(RegOpenKeyExW(HKEY_CURRENT_USER,path.c_str(),0,KEY_WRITE|KEY_WOW64_64KEY,&key)==ERROR_SUCCESS,"open own synthetic obstruction");check(RegDeleteValueW(key,L"ExternalValue")==ERROR_SUCCESS,"remove only known own fixture obstruction");RegCloseKey(key);key=nullptr;
    HKEY child=nullptr;check(RegCreateKeyExW(HKEY_CURRENT_USER,(path+L"\\OwnedFixtureChild").c_str(),0,nullptr,REG_OPTION_NON_VOLATILE,KEY_WRITE|KEY_WOW64_64KEY,nullptr,&child,nullptr)==ERROR_SUCCESS,"create own child obstruction");RegCloseKey(child);
    check(rejects([&]{image.restore(HKEY_CURRENT_USER,path,{});}),"subkey blocks restoration without recursive registry deletion");
    check(RegDeleteKeyExW(HKEY_CURRENT_USER,(path+L"\\OwnedFixtureChild").c_str(),KEY_WOW64_64KEY,0)==ERROR_SUCCESS,"remove exact empty synthetic child");
    auto malformed=image;malformed.values.push_back(malformed.values.front());check(rejects([&]{malformed.write(workspace/L"duplicate.snapshot");}),"duplicate names refused");
    malformed=image;malformed.values.front().type=REG_LINK;check(rejects([&]{malformed.write(workspace/L"link.snapshot");}),"unsupported registry type refused");
    malformed=image;malformed.values.front().bytes.resize(65537);check(rejects([&]{malformed.write(workspace/L"oversized.snapshot");}),"oversized value refused");
    std::ifstream original(workspace/L"previous.snapshot",std::ios::binary);std::string text{std::istreambuf_iterator<char>(original),std::istreambuf_iterator<char>()};
    {std::ofstream trailing(workspace/L"trailing.snapshot",std::ios::binary);trailing<<text<<'x';}
    {std::ofstream truncated(workspace/L"truncated.snapshot",std::ios::binary);truncated.write(text.data(),static_cast<std::streamsize>(text.size()-1));}
    check(rejects([&]{RegistryImage::read(workspace/L"trailing.snapshot");}) && rejects([&]{RegistryImage::read(workspace/L"truncated.snapshot");}),"trailing and truncated snapshots refused");
    absent.restore(HKEY_CURRENT_USER,path,{L"DisplayName",L"Build",L"BuildLong",L"Payload",L"InstallLocation",L"Options"});
    check(!RegistryImage::capture(HKEY_CURRENT_USER,path).exists,"fresh rollback removes only exact empty owned leaf");
    image.restore(HKEY_CURRENT_USER,path,{});check(RegistryImage::capture(HKEY_CURRENT_USER,path)==image,"restoration reconstructs missing previous leaf");
    absent.restore(HKEY_CURRENT_USER,path,{L"DisplayName",L"Build",L"BuildLong",L"Payload",L"InstallLocation",L"Options"});
    std::cout<<"PASS: disposable HKCU 64-bit typed snapshots, Unicode string contents, exact restoration, allowed forward cleanup, unknown value/subkey preservation, strict bounds/format and absent rollback; no product or HKLM key touched\n";
}

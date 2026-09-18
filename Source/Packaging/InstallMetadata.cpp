#include "InstallMetadata.h"
#include <algorithm>
#include <cstring>
#include <set>
#include <stdexcept>

namespace cc::packaging {
namespace {
constexpr auto metadataReceipt=L"chordcanvas.install";
constexpr auto helperName=L"ChordCanvasInstaller.dll";
const std::set<std::wstring> generated={helperName,L"unins000.exe",L"unins000.dat",L"unins000.msg"};
void require(bool test,const char* message){if(!test)throw std::runtime_error(message);}
void safeAncestors(std::filesystem::path path){for(;;){auto attributes=GetFileAttributesW(path.c_str());if(attributes==INVALID_FILE_ATTRIBUTES){auto error=GetLastError();require(error==ERROR_FILE_NOT_FOUND || error==ERROR_PATH_NOT_FOUND,"Inaccessible installer metadata ancestor");}else require(!(attributes&FILE_ATTRIBUTE_REPARSE_POINT),"Unsafe installer metadata ancestor");auto parent=path.parent_path();if(parent.empty() || parent==path)break;path=parent;}}
std::wstring widen(const std::string& value){return {value.begin(),value.end()};}
bool sameWindowsPath(std::wstring left,std::wstring right){std::replace(left.begin(),left.end(),L'/',L'\\');std::replace(right.begin(),right.end(),L'/',L'\\');return CompareStringOrdinal(left.c_str(),static_cast<int>(left.size()),right.c_str(),static_cast<int>(right.size()),TRUE)==CSTR_EQUAL;}
std::string narrow(const std::wstring& value){require(std::all_of(value.begin(),value.end(),[](auto c){return c>=32 && c<127;}),"Invalid ASCII installer field");std::string result;result.reserve(value.size());for(auto c:value)result.push_back(static_cast<char>(c));return result;}
bool hash(const std::string& value){return value.size()==64 && std::all_of(value.begin(),value.end(),[](char c){return (c>='0' && c<='9') || (c>='a' && c<='f');});}
RegistryValue stringValue(const std::wstring& name,const std::wstring& value){RegistryValue result{name,REG_SZ,{}};result.bytes.resize((value.size()+1)*2);std::memcpy(result.bytes.data(),value.c_str(),result.bytes.size());return result;}
std::wstring field(const RegistryImage& image,const std::wstring& name){
    for(auto& value:image.values)if(value.name==name){require(value.type==REG_SZ && value.bytes.size()>=2 && value.bytes.size()%2==0,"Invalid installer metadata field");std::wstring result(value.bytes.size()/2,L'\0');std::memcpy(result.data(),value.bytes.data(),value.bytes.size());require(result.back()==0,"Invalid metadata terminator");result.pop_back();require(result.find(L'\0')==std::wstring::npos,"Embedded metadata terminator");return result;}
    throw std::runtime_error("Missing installer metadata field");
}
void available(const std::filesystem::path& file){if(!std::filesystem::exists(file))return;auto handle=CreateFileW(file.c_str(),GENERIC_READ|GENERIC_WRITE|DELETE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(handle!=INVALID_HANDLE_VALUE,"Installer metadata is in use; save and close the relevant application, then retry");CloseHandle(handle);}
void move(const std::filesystem::path& from,const std::filesystem::path& to){require(MoveFileExW(from.c_str(),to.c_str(),MOVEFILE_WRITE_THROUGH)!=0,"Installer metadata rename failed; previous files preserved");}
void inventory(const std::filesystem::path& root,const std::set<std::wstring>& names){
    safeAncestors(root);require(std::filesystem::is_directory(root),"Invalid installer metadata directory");
    for(auto& entry:std::filesystem::directory_iterator(root))require(plainPath(entry.path()) && entry.is_regular_file() && names.contains(entry.path().filename().wstring()),"Unexpected metadata file/directory; installation data preserved");
}
RegistryImage receipt(const std::filesystem::path& root,bool complete=true,bool uninstall=false){
    auto path=root/metadataReceipt;auto before=sha256(path);auto image=RegistryImage::read(path);
    require(image.exists && image.values.size()>=6 && image.values.size()<=7 && field(image,L"Kind")==L"ChordCanvasInstalled1" && field(image,L"Identity")==widen(productIdentity),"Unrecognised installed metadata ownership");
    Version::parse(narrow(field(image,L"Version")));std::set<std::wstring> names={metadataReceipt};
    for(auto& value:image.values){
        if(value.name==L"Kind" || value.name==L"Identity" || value.name==L"Version")continue;
        require(value.name.starts_with(L"File:"),"Unexpected installed metadata field");auto name=value.name.substr(5);require(generated.contains(name),"Unowned installed metadata file");
        auto digest=narrow(field(image,value.name));require(hash(digest),"Invalid metadata hash");names.insert(name);
        // Inno has already loaded and exclusively opened its .dat before any
        // uninstall script event. Only that engine-owned file is exempt from
        // a second content read; all update/install validation remains strict.
        if(complete && !(uninstall && name==L"unins000.dat"))require(sha256(root/name)==digest,"Installed metadata integrity mismatch");
    }
    for(auto name:{helperName,L"unins000.exe",L"unins000.dat"})require(names.contains(name),"Incomplete installed metadata ownership");
    inventory(root,names);require(sha256(path)==before,"Installed metadata receipt changed");
    if(complete){validateAmd64Pe(root/helperName);validateAmd64Pe(root/L"unins000.exe");}
    return image;
}
void removePartial(const std::filesystem::path& root){
    if(!std::filesystem::exists(root))return;auto names=generated;names.insert(metadataReceipt);inventory(root,names);
    for(auto& entry:std::filesystem::directory_iterator(root))available(entry.path());
    for(auto& name:names)std::filesystem::remove(root/name);std::filesystem::remove(root);
}
void removePrevious(const std::filesystem::path& root,const std::string& digest){
    if(!std::filesystem::exists(root))return;
    safeAncestors(root);if(std::filesystem::is_directory(root) && std::filesystem::is_empty(root)){std::filesystem::remove(root);return;}
    require(sha256(root/metadataReceipt)==digest,"Conflicting previous metadata receipt");auto image=receipt(root,false);
    for(auto& value:image.values)if(value.name.starts_with(L"File:"))available(root/value.name.substr(5));available(root/metadataReceipt);
    for(auto& value:image.values)if(value.name.starts_with(L"File:"))std::filesystem::remove(root/value.name.substr(5));
    std::filesystem::remove(root/metadataReceipt);std::filesystem::remove(root);
}
}
const std::vector<std::wstring>& InstallMetadata::registrationNames(){
    // Exact standard fields used by the pinned Inno 7.1.0 configuration.
    static const std::vector<std::wstring> values={L"Inno Setup: Setup Version",L"Inno Setup: App Path",L"InstallLocation",L"Inno Setup: Icon Group",L"Inno Setup: No Icons",L"Inno Setup: User",L"Inno Setup: Language",L"DisplayName",L"DisplayIcon",L"UninstallString",L"QuietUninstallString",L"DisplayVersion",L"Publisher",L"URLInfoAbout",L"NoModify",L"NoRepair",L"InstallDate",L"MajorVersion",L"MinorVersion",L"VersionMajor",L"VersionMinor",L"EstimatedSize"};return values;
}
InstallMetadata::InstallMetadata(std::filesystem::path root,HKEY rootHive,std::wstring registryKey,std::filesystem::path sourceHelper,std::string release,std::function<void(MetadataBoundary)> hook)
    :directory(std::filesystem::absolute(root).lexically_normal()),auxiliary(directory.parent_path()/L".ChordCanvas.metadata"),retired(directory.parent_path()/L".ChordCanvas.metadata.retired"),helper(std::move(sourceHelper)),hive(rootHive),key(std::move(registryKey)),version(std::move(release)),injection(std::move(hook)){
    require(hive==HKEY_LOCAL_MACHINE || hive==HKEY_CURRENT_USER,"Unsupported installation registry hive");Version::parse(version);safeAncestors(directory);safeAncestors(auxiliary);
}
RegistryImage InstallMetadata::descriptor(const std::string& token,const std::string& digest,bool retiring) const{
    auto path=retiring && std::filesystem::exists(retired) ? retired : auxiliary/L"descriptor.bin";require(hash(digest) && plainPath(path) && sha256(path)==digest,"Installer recovery descriptor changed; files preserved");auto data=RegistryImage::read(path);
    require(data.exists && data.values.size()==10 && field(data,L"Kind")==L"ChordCanvasInstallTransaction1" && field(data,L"Identity")==widen(productIdentity) && field(data,L"Token")==widen(token) && field(data,L"Root")==directory.wstring() && field(data,L"Key")==key && field(data,L"Hive")== (hive==HKEY_LOCAL_MACHINE ? L"HKLM" : L"HKCU"),"Conflicting installer recovery context");
    for(auto name:{L"RegistryHash",L"HelperHash"})require(hash(narrow(field(data,name))),"Invalid installer recovery hash");auto previous=narrow(field(data,L"PreviousHash"));require(previous=="-" || hash(previous),"Invalid previous installer hash");
    Version::parse(narrow(field(data,L"Version")));
    if(std::filesystem::exists(auxiliary/L"registry.bin")){require(sha256(auxiliary/L"registry.bin")==narrow(field(data,L"RegistryHash")),"Registry recovery snapshot changed");RegistryImage::read(auxiliary/L"registry.bin");}
    else require(retiring,"Registry recovery snapshot missing");
    if(std::filesystem::exists(auxiliary/L"descriptor.bin"))require(sha256(auxiliary/L"descriptor.bin")==digest,"Conflicting auxiliary descriptor");
    std::set<std::wstring> names={L"descriptor.bin",L"registry.bin",L"previous"};safeAncestors(auxiliary);
    if(std::filesystem::exists(auxiliary)){require(std::filesystem::is_directory(auxiliary),"Invalid auxiliary metadata directory");for(auto& entry:std::filesystem::directory_iterator(auxiliary))require(plainPath(entry.path()) && names.contains(entry.path().filename().wstring()) && (entry.path().filename()==L"previous" ? entry.is_directory() : entry.is_regular_file()),"Unknown installer recovery file");}
    else require(retiring,"Auxiliary recovery directory missing");
    return data;
}
void InstallMetadata::checkRegistration(const std::string& expected) const{
    auto current=RegistryImage::capture(hive,key);
    for(auto& value:current.values)require(std::find(registrationNames().begin(),registrationNames().end(),value.name)!=registrationNames().end(),"Unexpected uninstall registration value");
    require(current.exists && field(current,L"DisplayName")==L"ChordCanvas" && field(current,L"DisplayVersion")==widen(expected) && field(current,L"Publisher")==L"ChordCanvas" && sameWindowsPath(field(current,L"UninstallString"),L"\""+(directory/L"unins000.exe").wstring()+L"\""), "Uninstall registration does not match the owned installation");
}
void InstallMetadata::checkExisting(const std::string& expected){auto data=receipt(directory);require(field(data,L"Version")==widen(expected),"Bundle and uninstaller version differ");checkRegistration(expected);}
void InstallMetadata::checkUninstall(const std::string& expected){auto data=receipt(directory,true,true);require(field(data,L"Version")==widen(expected),"Bundle and uninstaller version differ");require(std::filesystem::is_regular_file(directory/L"unins000.dat"),"Missing engine-owned uninstall data");checkRegistration(expected);}
std::string InstallMetadata::stage(const std::string& token){
    require(!token.empty() && token.size()==36,"Invalid metadata transaction token");
    if(std::filesystem::exists(retired)){auto old=RegistryImage::read(retired);auto oldToken=narrow(field(old,L"Token"));auto oldHash=sha256(retired);descriptor(oldToken,oldHash,true);retire(oldToken,oldHash);}
    if(std::filesystem::exists(auxiliary)){
        // No primary journal is pending when stage runs. Only a fully recognised
        // descriptor without a retained directory can be an orphan.
        require(!std::filesystem::exists(auxiliary/L"previous"),"Unresolved previous installer directory; recovery required");auto old=RegistryImage::read(auxiliary/L"descriptor.bin");auto oldToken=narrow(field(old,L"Token"));auto oldHash=sha256(auxiliary/L"descriptor.bin");descriptor(oldToken,oldHash);retire(oldToken,oldHash);
    }
    validateAmd64Pe(helper);std::string previous="-";
    if(std::filesystem::exists(directory)){auto current=receipt(directory);checkRegistration(narrow(field(current,L"Version")));previous=sha256(directory/metadataReceipt);for(auto& entry:std::filesystem::directory_iterator(directory))available(entry.path());}
    else require(!RegistryImage::capture(hive,key).exists,"Existing uninstall registration without its owned directory; installation refused");
    require(std::filesystem::space(directory.parent_path()).available>16*1024*1024,"Insufficient installer metadata space");
    require(std::filesystem::create_directory(auxiliary),"Installer recovery directory collision");auto registry=RegistryImage::capture(hive,key);registry.write(auxiliary/L"registry.bin");
    RegistryImage data;data.exists=true;
    for(auto item:std::vector<std::pair<std::wstring,std::wstring>>{{L"Kind",L"ChordCanvasInstallTransaction1"},{L"Identity",widen(productIdentity)},{L"Token",widen(token)},{L"Root",directory.wstring()},{L"Key",key},{L"Hive",hive==HKEY_LOCAL_MACHINE ? L"HKLM" : L"HKCU"},{L"RegistryHash",widen(sha256(auxiliary/L"registry.bin"))},{L"PreviousHash",widen(previous)},{L"HelperHash",widen(sha256(helper))}})data.values.push_back(stringValue(item.first,item.second));
    data.values.push_back(stringValue(L"Version",widen(version)));
    // Ten fields include source version, so its context is pinned as well.
    data.write(auxiliary/L"descriptor.bin");if(injection)injection(MetadataBoundary::descriptorStaged);return sha256(auxiliary/L"descriptor.bin");
}
void InstallMetadata::verify(const std::string& token,const std::string& digest){
    auto data=descriptor(token,digest);auto current=RegistryImage::capture(hive,key);for(auto& value:current.values)require(std::find(registrationNames().begin(),registrationNames().end(),value.name)!=registrationNames().end(),"Unexpected registry metadata blocks joint recovery");
    if(std::filesystem::exists(directory)){auto names=generated;names.insert(metadataReceipt);inventory(directory,names);if(std::filesystem::exists(directory/metadataReceipt)){auto currentReceipt=receipt(directory,false);require(field(currentReceipt,L"Version")==field(data,L"Version") || sha256(directory/metadataReceipt)==narrow(field(data,L"PreviousHash")),"Conflicting current uninstaller ownership");}}
    auto backup=auxiliary/L"previous";if(std::filesystem::exists(backup)){auto previous=narrow(field(data,L"PreviousHash"));require(previous!="-" && (std::filesystem::is_empty(backup) || sha256(backup/metadataReceipt)==previous),"Conflicting retained uninstaller directory");if(!std::filesystem::is_empty(backup))receipt(backup,false);}
}
void InstallMetadata::apply(const std::string& token,const std::string& digest,const std::string& expected){
    verify(token,digest);auto data=descriptor(token,digest);require(field(data,L"Version")==widen(expected) && RegistryImage::read(auxiliary/L"registry.bin")==RegistryImage::capture(hive,key),"Installation context changed after staging");
    auto previous=narrow(field(data,L"PreviousHash"));if(previous!="-"){require(sha256(directory/metadataReceipt)==previous,"Previous metadata changed");for(auto& entry:std::filesystem::directory_iterator(directory))available(entry.path());move(directory,auxiliary/L"previous");if(injection)injection(MetadataBoundary::previousRetained);}
    require(std::filesystem::create_directory(directory),"Installer metadata directory collision");if(injection)injection(MetadataBoundary::directoryCreated);std::filesystem::copy_file(helper,directory/helperName);require(sha256(directory/helperName)==narrow(field(data,L"HelperHash")),"Installer helper changed during copy");if(injection)injection(MetadataBoundary::helperCopied);
}
void InstallMetadata::seal(){
    auto names=generated;inventory(directory,names);RegistryImage data;data.exists=true;data.values={stringValue(L"Kind",L"ChordCanvasInstalled1"),stringValue(L"Identity",widen(productIdentity)),stringValue(L"Version",widen(version))};
    for(auto name:{helperName,L"unins000.exe",L"unins000.dat"})require(std::filesystem::is_regular_file(directory/name),"Installer finalisation is incomplete");
    validateAmd64Pe(directory/helperName);validateAmd64Pe(directory/L"unins000.exe");checkRegistration(version);
    for(auto& entry:std::filesystem::directory_iterator(directory))data.values.push_back(stringValue(L"File:"+entry.path().filename().wstring(),widen(sha256(entry.path()))));data.write(directory/metadataReceipt);receipt(directory);
}
void InstallMetadata::validate(const std::string& token,const std::string& digest,const std::string& expected){verify(token,digest);auto data=descriptor(token,digest);require(field(data,L"Version")==widen(expected),"Final bundle version differs from metadata transaction");auto installed=receipt(directory);require(field(installed,L"Version")==widen(expected) && sha256(directory/helperName)==narrow(field(data,L"HelperHash")),"Final installer version or helper mismatch");checkRegistration(expected);}
void InstallMetadata::rollback(const std::string& token,const std::string& digest){
    verify(token,digest);auto data=descriptor(token,digest);auto backup=auxiliary/L"previous";auto previous=narrow(field(data,L"PreviousHash"));
    if(std::filesystem::exists(backup)){require(previous!="-" && sha256(backup/metadataReceipt)==previous,"Missing previous uninstaller ownership");receipt(backup);removePartial(directory);move(backup,directory);}
    else if(previous=="-")removePartial(directory);
    else require(sha256(directory/metadataReceipt)==previous,"Previous uninstaller directory is unavailable");
    RegistryImage::read(auxiliary/L"registry.bin").restore(hive,key,registrationNames());
    if(injection)injection(MetadataBoundary::registryRestored);
}
void InstallMetadata::cleanup(const std::string& token,const std::string& digest,bool committed){auto data=descriptor(token,digest);if(committed)removePrevious(auxiliary/L"previous",narrow(field(data,L"PreviousHash")));else require(!std::filesystem::exists(auxiliary/L"previous"),"Rollback has retained installer directory; cleanup refused");}
void InstallMetadata::retire(const std::string& token,const std::string& digest){
    auto data=descriptor(token,digest,true);require(!std::filesystem::exists(auxiliary/L"previous"),"Retained installer directory must be resolved first");
    // Keep one durable marker outside the directory until its last removal;
    // interruption cannot strand an unrecognisable empty auxiliary directory.
    if(!std::filesystem::exists(retired))data.write(retired);require(sha256(retired)==digest,"Retirement marker differs from recovery descriptor");
    if(injection)injection(MetadataBoundary::retirementMarked);
    std::filesystem::remove(auxiliary/L"registry.bin");if(injection)injection(MetadataBoundary::registrySnapshotRemoved);
    std::filesystem::remove(auxiliary/L"descriptor.bin");if(injection)injection(MetadataBoundary::descriptorRemoved);
    std::filesystem::remove(auxiliary);if(injection)injection(MetadataBoundary::auxiliaryRemoved);std::filesystem::remove(retired);
}
}

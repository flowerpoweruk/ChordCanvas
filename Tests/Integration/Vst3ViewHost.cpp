// Development-only native VST3 view host. No audio device, MIDI import, fake host
// transport or Live identity. This permits real rendering/gesture inspection of
// an explicitly identified existing binary; it is not Ableton acceptance.
#include <windows.h>
#include <objbase.h>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/gui/iplugview.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace Steinberg;
using namespace Steinberg::Vst;
namespace {
void require(bool result,const char* message){std::cout<<(result ? "OK: " : "FAIL: ")<<message<<std::endl;if(!result)throw std::runtime_error(message);}
struct Host final: IHostApplication,IPlugFrame,IComponentHandler {
    HWND window=nullptr;
    IPlugView* view=nullptr;
    bool resizing=false;
    uint32 references=1;
    tresult PLUGIN_API queryInterface(const TUID id,void** result) override {
        if(!result)return kInvalidArgument;*result=nullptr;
        if(!std::memcmp(id,IHostApplication_iid,16) || !std::memcmp(id,FUnknown_iid,16))*result=static_cast<IHostApplication*>(this);
        else if(!std::memcmp(id,IPlugFrame_iid,16))*result=static_cast<IPlugFrame*>(this);
        else if(!std::memcmp(id,IComponentHandler_iid,16))*result=static_cast<IComponentHandler*>(this);
        if(!*result)return kNoInterface;addRef();return kResultOk;
    }
    uint32 PLUGIN_API addRef() override {return ++references;}
    uint32 PLUGIN_API release() override {return --references;}
    tresult PLUGIN_API getName(String128 name) override {std::fill(name,name+128,0);constexpr wchar_t title[]=L"ChordCanvas development view host";for(size_t i=0;i<std::size(title)-1;++i)name[i]=static_cast<char16>(title[i]);return kResultOk;}
    tresult PLUGIN_API createInstance(TUID,TUID,void** result) override {if(result)*result=nullptr;return kNoInterface;}
    tresult PLUGIN_API beginEdit(ParamID) override {return kResultOk;}
    tresult PLUGIN_API performEdit(ParamID,ParamValue) override {return kResultOk;}
    tresult PLUGIN_API endEdit(ParamID) override {return kResultOk;}
    tresult PLUGIN_API restartComponent(int32) override {return kResultOk;}
    tresult PLUGIN_API resizeView(IPlugView* incoming,ViewRect* rectangle) override {
        if(!incoming || !rectangle || incoming!=view)return kInvalidArgument;
        if(window && !resizing){resizing=true;RECT bounds{0,0,rectangle->getWidth(),rectangle->getHeight()};AdjustWindowRectEx(&bounds,static_cast<DWORD>(GetWindowLongPtrW(window,GWL_STYLE)),FALSE,0);SetWindowPos(window,nullptr,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);resizing=false;}
        return incoming->onSize(rectangle);
    }
};
LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM first,LPARAM second){
    auto* host=reinterpret_cast<Host*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    if(message==WM_NCCREATE){host=static_cast<Host*>(reinterpret_cast<CREATESTRUCTW*>(second)->lpCreateParams);SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(host));host->window=window;}
    if(message==WM_SIZE && host && host->view && !host->resizing && first!=SIZE_MINIMIZED){ViewRect size{0,0,LOWORD(second),HIWORD(second)};host->view->checkSizeConstraint(&size);host->resizeView(host->view,&size);return 0;}
    if(message==WM_CLOSE){DestroyWindow(window);return 0;}
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,first,second);
}
}
int wmain(int count,wchar_t** arguments){try{
    require(count==2,"Supply an explicit development VST3 module");auto path=std::filesystem::absolute(arguments[1]);
    require(path.filename()==L"ChordCanvas.vst3" && std::filesystem::is_regular_file(path),"Expected existing ChordCanvas development VST3 module");
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    auto image=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);require(image!=nullptr,"Native VST3 module load");
    auto init=reinterpret_cast<bool(*)()>(GetProcAddress(image,"InitDll"));auto exit=reinterpret_cast<bool(*)()>(GetProcAddress(image,"ExitDll"));auto factoryFunction=reinterpret_cast<IPluginFactory*(*)()>(GetProcAddress(image,"GetPluginFactory"));
    require(factoryFunction && (!init || init()),"Native VST3 module initialization");auto* factory=factoryFunction();require(factory!=nullptr,"Native VST3 factory");
    PFactoryInfo information{};require(factory->getFactoryInfo(&information)==kResultOk && !std::strcmp(information.vendor,"ChordCanvas"),"Expected ChordCanvas factory");
    IComponent* component=nullptr;for(int i=0;i<factory->countClasses();++i){PClassInfo info{};if(factory->getClassInfo(i,&info)==kResultOk && !std::strcmp(info.category,"Audio Module Class")){require(factory->createInstance(info.cid,IComponent_iid,reinterpret_cast<void**>(&component))==kResultOk,"Create native instrument component");break;}}
    require(component!=nullptr,"Native VST3 instrument component");Host host;require(component->initialize(static_cast<IHostApplication*>(&host))==kResultOk,"Initialize native component");
    TUID controllerId{};require(component->getControllerClassId(controllerId)==kResultOk,"Native controller class identity");IEditController* controller=nullptr;require(factory->createInstance(controllerId,IEditController_iid,reinterpret_cast<void**>(&controller))==kResultOk && controller,"Create native controller");require(controller->initialize(static_cast<IHostApplication*>(&host))==kResultOk,"Initialize native controller");
    IConnectionPoint *componentPoint=nullptr,*controllerPoint=nullptr;
    require(component->queryInterface(IConnectionPoint_iid,reinterpret_cast<void**>(&componentPoint))==kResultOk && controller->queryInterface(IConnectionPoint_iid,reinterpret_cast<void**>(&controllerPoint))==kResultOk,"Native component/controller connection points");
    require(componentPoint->connect(controllerPoint)==kResultOk && controllerPoint->connect(componentPoint)==kResultOk,"Connect native component/controller");
    require(controller->setComponentHandler(static_cast<IComponentHandler*>(&host))==kResultOk,"Set native host component handler");
    host.view=controller->createView("editor");require(host.view && host.view->isPlatformTypeSupported(kPlatformTypeHWND)==kResultOk,"Create native HWND view");host.view->setFrame(&host);
    WNDCLASSW definition{};definition.lpfnWndProc=procedure;definition.hInstance=GetModuleHandleW(nullptr);definition.lpszClassName=L"ChordCanvasDevelopmentVst3View";definition.hCursor=LoadCursorW(nullptr,IDC_ARROW);require(RegisterClassW(&definition)!=0,"Register own development window");
    // The parent is created before attachment; resize callbacks cannot route
    // into an unattached plug-in window during CreateWindow's initial WM_SIZE.
    auto* pending=host.view;host.view=nullptr;auto window=CreateWindowW(definition.lpszClassName,L"ChordCanvas development VST3 view (no audio; not Live acceptance)",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1080,700,nullptr,nullptr,definition.hInstance,&host);host.view=pending;require(window!=nullptr,"Create own development window");
    require(host.view->attached(window,kPlatformTypeHWND)==kResultOk,"Attach actual native VST3 editor");ViewRect size{};require(host.view->getSize(&size)==kResultOk,"Read actual editor dimensions");host.resizeView(host.view,&size);ShowWindow(window,SW_SHOW);UpdateWindow(window);
    std::cout<<"Actual native VST3 view attached: "<<size.getWidth()<<'x'<<size.getHeight()<<" physical pixels; no audio processing or Live claim\n"<<std::flush;
    MSG message{};while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
    host.view->removed();host.view->setFrame(nullptr);host.view->release();host.view=nullptr;componentPoint->disconnect(controllerPoint);controllerPoint->disconnect(componentPoint);componentPoint->release();controllerPoint->release();controller->setComponentHandler(nullptr);controller->terminate();controller->release();component->terminate();component->release();factory->release();if(exit)exit();FreeLibrary(image);CoUninitialize();
    std::cout<<"Development view closed normally\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}

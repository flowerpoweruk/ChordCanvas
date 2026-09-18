#include "Diagnostics/Log.h"
#include "Commands/Timeline.h"
#include "Persistence/Progression.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>

using namespace cc;
void require(bool condition,const char* reason){if(!condition){std::cerr<<"FAIL: "<<reason<<'\n';std::exit(1);}}
void waitReady(LogService& log) {
    auto end=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(std::chrono::steady_clock::now()<end){if(log.status().available)return;std::this_thread::sleep_for(std::chrono::milliseconds(20));}
    require(false,log.status().reason.c_str());
}
std::string contents(std::filesystem::path file){std::ifstream in(file,std::ios::binary);return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};}
std::vector<std::filesystem::path> logs(std::filesystem::path root){std::vector<std::filesystem::path> files;for(auto& e:std::filesystem::directory_iterator(root))if(e.path().extension()==L".txt")files.push_back(e.path());return files;}
int main(int argc,char** argv) {
    auto root=std::filesystem::path(argc>1 ? argv[1] : ".")/("log-tests-"+std::to_string(GetCurrentProcessId()));
    for(int session=0;session<6;++session) {
        LogService service(root,32*1024*1024,"synthetic-test-host");waitReady(service);service.post(1,"synthetic.session.ordinal","{\"ordinal\":"+std::to_string(session)+"}");service.post(1,"state.snapshot","{\"bars\":8,\"revision\":3}");service.post(1,"timeline.replace.commit","{\"transaction\":3,\"revision_before\":2,\"revision_after\":3}");service.audioEvent({1,1,3,5,960,3});
    }
    auto files=logs(root);require(files.size()==5,"five retained completed sessions");for(auto& path:files){auto text=contents(path);require(text.find("session.end")!=std::string::npos && text.find("state.snapshot")!=std::string::npos,"useful orderly trail");}
    std::vector<std::unique_ptr<LogService>> active;
    auto activeRoot=root/L"concurrent";
    for(int i=0;i<5;++i){active.push_back(std::make_unique<LogService>(activeRoot));waitReady(*active.back());}
    {LogService sixth(activeRoot,16384);sixth.post(1,"state.snapshot","{\"revision\":99}");
     for(int i=0;i<200;++i)sixth.post(1,"synthetic.waiting","{\"padding\":"+jsonQuote(std::string(160,'y'))+"}");
     std::this_thread::sleep_for(std::chrono::milliseconds(150));require(!sixth.status().available && logs(activeRoot).size()==5,"sixth active session degrades without deleting live files");
     active.erase(active.begin());waitReady(sixth);require(logs(activeRoot).size()==5,"newly available slot retains five files");
    }
    active.clear();
    auto boundedRoot=root/L"bounded";
    {LogService service(boundedRoot,16384);waitReady(service);service.post(1,"state.snapshot","{\"revision\":7}");for(int i=0;i<1500;++i)service.post(1,"synthetic.action","{\"padding\":"+jsonQuote(std::string(160,'x'))+"}");std::this_thread::sleep_for(std::chrono::milliseconds(250));require(service.status().dropped>0,"bounded message queue reports loss");for(int i=0;i<2000;++i)service.audioEvent({1,1,7,1,i,3});}
    files=logs(boundedRoot);require(files.size()==1,"one bounded session file");require(std::filesystem::file_size(files[0])<=16384,"storage cap");auto text=contents(files[0]);require(text.find("history_truncated")!=std::string::npos && text.find("state.snapshot")!=std::string::npos,"compaction preserves snapshot with explicit loss marker");
    auto deny=root/L"not-a-folder";{std::ofstream file(deny);file<<"synthetic owned test sentinel";}
    {LogService denied(deny);std::this_thread::sleep_for(std::chrono::milliseconds(100));require(!denied.status().available,"unwritable storage stays safe");}
    // Exercise actual model operations and an actual owned file-I/O failure.
    // This is an integration harness, not a rendered plug-in/error-popup test.
    {LogService service(root/L"correlation",32*1024*1024,"synthetic-test-host");waitReady(service);
     Document document;document.add({},0);document.add({},bar);uint64_t transaction=0;
     auto record=[&](const char* event,uint64_t before){
         service.post(1,event,"{\"transaction\":"+std::to_string(++transaction)+",\"revision_before\":"+std::to_string(before)+",\"revision_after\":"+std::to_string(document.revision())+"}");
         service.post(1,"state.snapshot","{\"transaction\":"+std::to_string(transaction)+",\"revision\":"+std::to_string(document.revision())+",\"progression\":"+saveProgression(document.state())+"}");
     };
     auto before=document.revision();require(document.add({},2880),"synthetic replace");record("timeline.replace.commit",before);
     before=document.revision();require(document.resize(document.state().blocks[0].id,false,7680,960),"synthetic resize");record("timeline.resize.commit",before);
     before=document.revision();require(document.undo(),"synthetic undo");record("timeline.undo.commit",before);
     auto valid=document.state();std::ofstream output(deny/L"export.mid",std::ios::binary);require(!output,"actual export destination is not a directory");
     service.post(1,"export.error","{\"transaction\":"+std::to_string(++transaction)+",\"revision\":"+std::to_string(document.revision())+",\"operation\":\"write_midi\",\"error_kind\":\"file_open_failed\",\"state_changed\":false}");
     require(document.state()==valid,"file error preserves valid document");
    }
    AudioLogQueue queue;std::atomic<bool> done=false;std::atomic<int> successes=0;
    std::vector<std::thread> producers;for(int n=0;n<4;++n)producers.emplace_back([&,n]{for(int i=0;i<10000;++i)if(queue.push({1,static_cast<uint64_t>(n+1),static_cast<uint64_t>(i),static_cast<uint64_t>(i),i,i}))++successes;});
    int received=0;std::thread consumer([&]{AudioLogEvent event;while(!done){if(queue.pop(event)){require(event.revision==event.owner && event.tick==event.value,"untorn MPMC audio diagnostics");++received;}}while(queue.pop(event))++received;});
    for(auto& p:producers)p.join();done=true;consumer.join();require(received==successes,"no duplicated or lost admitted events");
    std::cout<<"PASS: five-session rotation, active-session preservation, bounded compaction/queues, storage failure and four-producer audio ring\n";
    std::cout<<"Synthetic evidence directory: "<<root.filename().string()<<'\n';
}

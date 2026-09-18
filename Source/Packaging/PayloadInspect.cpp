#include "Payload.h"
#include <iostream>
int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;
    try{auto root=std::filesystem::path(argv[1]);auto payload=cc::packaging::Payload::read(root);payload.validate(root);std::cout<<"PASS: complete owned AMD64 payload "<<payload.version<<"; "<<payload.files.size()<<" hash-verified files\n";}
    catch(const std::exception& fault){std::cerr<<"FAIL: "<<fault.what()<<'\n';return 1;}
}

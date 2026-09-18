#include "Packaging/Version.h"
#include <iostream>
#include <cstdlib>
using cc::packaging::Version;
void require(bool value){if(!value)std::exit(1);}
int main(){
    require(Version::parse("1.10.0")>Version::parse("1.9.0"));
    require(Version::parse("2.0.0")>Version::parse("1.65535.65535"));
    require(Version::parse("1.2.3")==Version::parse("1.2.3"));
    require(Version::parse("0.0.1")<Version::parse("0.1.0"));
    for(auto invalid:{"","1.2","1.2.3.4","1.2.3-beta.1","1.2.3+build","01.2.3","1.02.3","1.-2.3","1.2.65536","1.2.4294967296"," 1.2.3","1.2.3 ","1.2.3x"}){
        bool rejected=false;try{Version::parse(invalid);}catch(const std::invalid_argument&){rejected=true;}require(rejected);
    }
    std::cout<<"PASS: numeric release SemVer ordering, bounds and explicit prerelease refusal\n";
}

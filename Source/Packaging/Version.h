#pragma once
#include <array>
#include <charconv>
#include <compare>
#include <string_view>
#include <stdexcept>

namespace cc::packaging {
// Public packages currently accept release SemVer only. Prereleases are refused,
// so no implicit lexical ordering can accidentally allow a downgrade.
struct Version {
    std::array<unsigned,3> parts{};
    auto operator<=>(const Version&) const = default;
    static Version parse(std::string_view text) {
        Version result;
        for(size_t i=0;i<3;++i){
            auto separator=text.find('.');
            auto token=text.substr(0,separator);
            if(token.empty() || (token.size()>1 && token.front()=='0'))throw std::invalid_argument("Invalid release version");
            unsigned value=0;auto parsed=std::from_chars(token.data(),token.data()+token.size(),value);
            if(parsed.ec!=std::errc{} || parsed.ptr!=token.data()+token.size() || value>65535)throw std::invalid_argument("Invalid release version");
            result.parts[i]=value;
            if(i<2){if(separator==std::string_view::npos)throw std::invalid_argument("Invalid release version");text.remove_prefix(separator+1);}
            else if(separator!=std::string_view::npos)throw std::invalid_argument("Invalid release version");
        }
        return result;
    }
};
}

//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Configuration_hpp
#define Configuration_hpp

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>

#include "utility/timeutil.hpp"

#include "PRUConfig.hpp"

//======================================================================
struct Configuration {
    
    std::uint16_t clientPort ;
    std::string serverIP ;
    std::uint16_t serverPort ;
    
    std::string name ;
    
    std::array<PRUConfig,2> pruSetting ;
    
    std::filesystem::path musicPath ;
    std::filesystem::path lightPath ;
    std::string musicExtension ;
    std::string lightExtension ;
    
    util::HourRange connectTime ;
    util::MonthRange runSpan ;
    
    bool useAudio ;
    bool useLight ;
    
    int audioDevice ;
    
    Configuration() ;
    Configuration(const std::filesystem::path &path) ;
    
    auto load(const std::filesystem::path &path) -> bool ;
    auto processKeyValue(const std::string &key, const std::string &value) -> bool  ;
    
};

#endif /* Configuration_hpp */

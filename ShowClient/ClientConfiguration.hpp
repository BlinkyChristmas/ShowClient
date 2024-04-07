// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ClientConfiguration_hpp
#define ClientConfiguration_hpp

#include <cstdint>
#include <array>

#include "utility/BaseConfiguration.hpp"
#include "utility/timeutil.hpp"

#include "PRUConfig.hpp"

class ClientConfiguration: public BaseConfiguration {
    auto processKeyValue(const std::string &key, const std::string &value) ->void final ;
    
public:
    ClientConfiguration() ;
    ClientConfiguration(const std::filesystem::path &path) ;
    
    
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
 
};
#endif /* ClientConfiguration_hpp */

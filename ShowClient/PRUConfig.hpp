//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PRUConfig_hpp
#define PRUConfig_hpp

#include <cstdint>
#include <iostream>
#include <string>

//======================================================================
struct PRUConfig {
    static constexpr auto MODE_SSD = 0 ;
    static constexpr auto MODE_DMX = 1 ;
    static constexpr auto MODE_WS2812 = 2 ;
    
    int pru ;
    int mode ;
    int offset ;
    int length ;
    PRUConfig() ;
    PRUConfig(const std::string &line) ;
    auto describe() const -> std::string ;
};


#endif /* PRUConfig_hpp */

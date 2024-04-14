//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PRUConfig_hpp
#define PRUConfig_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include "bone/PruModes.hpp"
//======================================================================
struct PRUConfig {
    
    PruNumber pru ;
    PruModes mode;
    int length ;
    PRUConfig() ;
    PRUConfig(const std::string &line) ;
    //auto describe() const -> std::string ;
};


#endif /* PRUConfig_hpp */

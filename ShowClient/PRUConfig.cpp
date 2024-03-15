//Copyright © 2024 Charles Kerr. All rights reserved.

#include "PRUConfig.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/strutil.hpp"

using namespace std::string_literals ;

//======================================================================
PRUConfig::PRUConfig():pru(0),mode(0),offset(0),length(3072){
    
}

//======================================================================
// line = pru#,mode,offset,length
PRUConfig::PRUConfig(const std::string &line):PRUConfig() {
    auto values = util::parse(line,",") ;
    try {
        switch (values.size()) {
            default:
            case 4:{
                length = std::stoi(values[3],nullptr,0) ;
                [[fallthrough]] ;
            }
            case 3:{
                offset = std::stoi(values[2],nullptr,0) ;
                [[fallthrough]] ;
            }
            case 2:{
                mode = std::stoi(values[1],nullptr,0) ;
                [[fallthrough]] ;
            }
            case 1:{
                pru = std::stoi(values[0],nullptr,0) ;
                [[fallthrough]] ;
            }
                
            case 0:
                break;
        }
    }
    catch(...) {
        pru = 0 ;
        mode = 0 ;
        offset = 0 ;
        length = 3072 ;
    }
}

//======================================================================
auto PRUConfig::describe() const -> std::string {
    return std::to_string(pru) + ","s + std::to_string(mode) + ","s + std::to_string(offset) + ","s + std::to_string(length);
}


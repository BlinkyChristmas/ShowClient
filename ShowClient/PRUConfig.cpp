//Copyright © 2024 Charles Kerr. All rights reserved.

#include "PRUConfig.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/strutil.hpp"

using namespace std::string_literals ;
#include "bone/PruConstants.hpp"
//======================================================================
PRUConfig::PRUConfig():pru(PruNumber::zero),mode(PruModes::SSD),length(0){
    
}

//======================================================================
// line = pru#,mode,offset,length
PRUConfig::PRUConfig(const std::string &line):PRUConfig() {
    auto values = util::parse(line,",") ;
    try {
        switch (values.size()) {
            default:
            case 3:{
                length = std::stoi(values[2],nullptr,0) ;
                [[fallthrough]] ;
            }
            case 2:{
                auto utype = util::upper(values[1]) ;
                if (utype == "SSD"){
                    mode = PruModes::SSD ;
                }
                else if (utype == "DMX") {
                    mode = PruModes::DMX ;
                }
                else if (utype == "WS2812") {
                    mode = PruModes::WS2812 ;
                }
                [[fallthrough]] ;
            }
            case 1:{
                auto temp  = std::stoi(values[0],nullptr,0) ;
                pru = PruNumber::zero ;
                if (temp != 0) {
                    pru = PruNumber::one;
                }
                [[fallthrough]] ;
            }
                
            case 0:
                break;
        }
    }
    catch(...) {
        pru = PruNumber::zero ;
        mode = PruModes::SSD ;
        length = 0 ;
    }
}

/*
//======================================================================
auto PRUConfig::describe() const -> std::string {
    return std::to_string(pru) + ","s + std::to_string(mode) + ","s + std::to_string(length);
}
*/

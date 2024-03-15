//Copyright © 2024 Charles Kerr. All rights reserved.

#include "StatusController.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
// LEDInfo
//======================================================================
//======================================================================
const std::unordered_map<LEDInfo::ledtype,std::string> LEDInfo::LEDLOCATION{
    {LEDInfo::RUN,"/sys/class/leds/beaglebone:green:usr0"s},
    {LEDInfo::CONNECT,"/sys/class/leds/beaglebone:green:usr1"s},
    {LEDInfo::SHOW,"/sys/class/leds/beaglebone:green:usr2"s},
    {LEDInfo::PLAY,"/sys/class/leds/beaglebone:green:usr3"s}
};

const std::unordered_map<LEDInfo::ledtype,std::string> LEDInfo::LEDNAME{
    {LEDInfo::RUN,"RunState"s},
    {LEDInfo::CONNECT,"ConnectState"s},
    {LEDInfo::SHOW,"ShowState"s},
    {LEDInfo::PLAY,"PlayState"s}
};

//======================================================================
auto LEDInfo::locationForLED(ledtype led) -> const std::string& {
    static const std::string empty = "" ;
    auto iter = LEDLOCATION.find(led) ;
    if (iter == LEDLOCATION.end()) {
        return empty ;
    }
    return iter->second ;
}
//======================================================================
auto LEDInfo::nameForLED(ledtype led) -> const std::string& {
    static const std::string unknown = "Unknown" ;
    if (LEDNAME.empty()) {
        return unknown;
    }
    auto iter = LEDNAME.find(led) ;
    if (iter == LEDNAME.end()) {
        return unknown ;
    }
    return iter->second ;
}
//======================================================================
auto LEDInfo::stateNameFor(ledstate state) -> const std::string {
    switch (state) {
        case LEDInfo::ON:
            return "ON"s ;
        case LEDInfo::FLASH:
            return "FLASH"s ;
        case LEDInfo::OFF:
            return "OFF"s ;
        default:
            return "UNKNOWN"s ;
    }
}
//======================================================================
// StatusController
//======================================================================


//======================================================================
auto StatusController::setTrigger(LEDInfo::ledtype led, LEDInfo::ledstate state) -> bool {
    auto value = "none"s ;
    if (state == LEDInfo::FLASH) {
        value = "timer"s ;
    }
    auto filename = LEDInfo::locationForLED(led) + "/trigger"s ;
    auto output = std::ofstream(filename) ;
    if (!output.is_open()){
        std::cerr << "Error opening trigger file for led: " << LEDInfo::nameForLED(led) << std::endl;
        return false ;
    }
    output << value ;
    output.close() ;
    return true ;
}
//======================================================================
auto StatusController::setBrightness(LEDInfo::ledtype led, LEDInfo::ledstate state) -> bool {
    auto value = "1"s ;
    if (state == LEDInfo::OFF) {
        value = "0"s ;
    }
    auto filename = LEDInfo::locationForLED(led) + "/brightness"s ;
    auto output = std::ofstream(filename) ;
    if (!output.is_open()){
        std::cerr << "Error opening brightness file for led: " << LEDInfo::nameForLED(led) << std::endl;
        return false ;
    }
    output << value ;
    output.close() ;
    return true ;
}

//======================================================================
auto StatusController::setState(LEDInfo::ledtype led, LEDInfo::ledstate state) -> void {
#if defined(STATUSLED)
    DBGMSG(std::cout, "LED: "s+  LEDInfo::nameForLED(led)  + " State: "s + LEDInfo::stateNameFor(state) );
    if (setTrigger(led, state)) {
        setBrightness(led, state) ;
    }
#else
    std::cout << "LED: " << LEDInfo::nameForLED(led) << " State: " << LEDInfo::stateNameFor(state) << std::endl;
#endif
    
}
//======================================================================
StatusController::StatusController() {
    ledState = std::unordered_map<LEDInfo::ledtype, LEDInfo::ledstate>{
        {LEDInfo::RUN,LEDInfo::OFF},
        {LEDInfo::CONNECT,LEDInfo::OFF},
        {LEDInfo::SHOW,LEDInfo::OFF},
        {LEDInfo::PLAY,LEDInfo::OFF},
    };
    setAll(LEDInfo::OFF, true) ;

}

//======================================================================
auto StatusController::setState(LEDInfo::ledtype led, LEDInfo::ledstate state, bool force  ) -> void {
    if ((ledState.at(led) != state ) || force) {
        setState(led, state) ;
        ledState.at(led) = state ;
    }
}
//======================================================================
auto StatusController::setAll(LEDInfo::ledstate state, bool force) -> void {
    setState(LEDInfo::RUN, state,force) ;
    setState(LEDInfo::CONNECT,state,force) ;
    setState(LEDInfo::SHOW, state,force) ;
    setState(LEDInfo::PLAY,state,force) ;
}

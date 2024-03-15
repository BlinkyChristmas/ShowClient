//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef StatusController_hpp
#define StatusController_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

struct LEDInfo {
    enum ledtype {
        RUN = 0, CONNECT ,SHOW, PLAY
    };
    enum ledstate {
        OFF, ON, FLASH
    } ;
    static const std::unordered_map<ledtype,std::string> LEDLOCATION ;
    static const std::unordered_map<ledtype,std::string> LEDNAME;
    
    static auto locationForLED(ledtype led) -> const std::string& ;
    static auto nameForLED(ledtype led) -> const std::string& ;
    static auto stateNameFor(ledstate state) -> const std::string ;

};

//======================================================================
class StatusController {
    std::unordered_map<LEDInfo::ledtype,LEDInfo::ledstate> ledState ;
    
    auto setBrightness(LEDInfo::ledtype led, LEDInfo::ledstate state) -> bool ;
    auto setTrigger(LEDInfo::ledtype led, LEDInfo::ledstate state) -> bool ;

    auto setState(LEDInfo::ledtype led, LEDInfo::ledstate state) -> void ;
public:
    StatusController()  ;
    
    auto setState(LEDInfo::ledtype led, LEDInfo::ledstate state, bool force  ) -> void ;
    auto setAll(LEDInfo::ledstate state, bool force) -> void ;
    
};

#endif /* StatusController_hpp */

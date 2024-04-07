// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef StatusController_hpp
#define StatusController_hpp

#include <string>
#include <vector>

#include "bone/BeagleLed.hpp"
enum class StatusLed{
    RUN=0,CONNECT,SHOW,PLAY
};

class StatusController {
    std::vector<BeagleLed> leds ;
    
public:
    StatusController() ;
    auto operator[](StatusLed led) const -> const BeagleLed& ;
    auto operator[](StatusLed led) -> BeagleLed& ;
    
    auto clear() -> void ;
    auto flash() -> void ;
    auto describe(StatusLed led) const -> std::string ;
    auto setState(StatusLed led, LedState state) -> void ;
};

#endif /* StatusController_hpp */

// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BeagleLed_hpp
#define BeagleLed_hpp

#include <iostream>
#include <string>

enum class LedState {
    OFF,ON,FLASH,UNKNOWN
};

class BeagleLed {
    static const std::string led_location ;
    int led_number ;
    std::string led_name ;
    LedState currentState ;
    
#if !defined(BEAGLEBONE)
    bool currentOnOffState ;
    bool currentFlashState ;
#endif

    auto readOnOff() -> bool ;
    auto readFlash() -> bool ;
    auto setOnOff(bool state ) -> bool ;
    auto setFlash(bool state) -> bool ;
public:
    // Led numbers 0 - 3
    BeagleLed(int number,const std::string &name = "") ;
    auto setState(LedState state,bool force = false ) -> bool ;
    auto state() const -> LedState ;
    auto number() const -> int ;
    auto describe() const -> std::string ;
};
#endif /* BeagleLed_hpp */

// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BeagleLed.hpp"

#include <fstream>
#include <vector>

#include "utility/strutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;
const std::string BeagleLed::led_location = "/sys/class/leds/beaglebone:green:usr%i"s;
// =======================================================================================
BeagleLed::BeagleLed(int number,const std::string &name):led_name(name),led_number(number){
#if defined(BEAGLEBONE)
    currentState = LedState::OFF ;
    
    if (readOnOff()) {
        if (readFlash()){
            currentState = LedState::FLASH ;
        }
        else {
            currentState = LedState::ON ;
        }
    }
#else
    currentFlashState = false ;
    currentOnOffState = false ;
    currentState = LedState::OFF ;
#endif
}

// =================================================================================
auto BeagleLed::readFlash() -> bool {
#if defined(BEAGLE)
    auto path = util::format(led_location,led_number) ;
    path += "/trigger"s ;
    auto input = std::ifstream(path);
    if (!input.is_open()){
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1);
    if (input.gcount() == 0){
        return false ;
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string line = buffer.data();
    return line == "timer" ;

#else
    return false;
#endif
}

// =================================================================================
auto BeagleLed::readOnOff() -> bool {
#if defined(BEAGLE)
    auto path = util::format(led_location,led_number) ;
    path += "/brightness"s ;
    auto input = std::ifstream(path);
    if (!input.is_open()){
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1);
    if (input.gcount() == 0){
        return false ;
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string line = buffer.data();
    return line == "1";

#else
    return false;
#endif
}

// =======================================================================================
auto BeagleLed::setOnOff(bool state ) -> bool {
#if defined(BEAGLE)
        auto path = util::format(led_location,led_number) ;
        path += "/brightness"s ;
        auto output = std::ofstream(path);
        if (!output.is_open()){
            return false ;
        }
        output << (state?"1"s:"0"s);
        output.close();
#else
    currentOnOffState = state ;
#endif
    return true ;
}
// =======================================================================================
auto BeagleLed::setFlash(bool state ) -> bool {
#if defined(BEAGLE)
    auto path = util::format(led_location,led_number) ;
    path += "/trigger"s ;
    auto output = std::ofstream(path);
    if (!output.is_open()){
        return false ;
    }
    output << (state?"timer"s:"none"s);
    output.close();
#else
    currentFlashState = state ;
#endif
    return true ;
    
}

// =======================================================================================
auto BeagleLed::state() const -> LedState {
    return currentState ;
}
// =======================================================================================
auto BeagleLed::number() const -> int {
    return led_number ;
}
// =======================================================================================
auto BeagleLed::describe() const -> std::string {
    
    std::string state = "Off" ;
    if (currentState == LedState::FLASH) {
        state = "Flash";
    }
    else if (currentState == LedState::ON){
        state = "On";
    }
    return "Led: "s + led_name + " - "s + state ;
    
}

// =======================================================================================
auto BeagleLed::setState(LedState state,bool force) -> bool {
    if (state == currentState && !force) {
        return false ; // We didn't have to set anything
    }
    if (state == LedState::ON) {
        setFlash(false);
        setOnOff(true);
    }
    else if (state ==  LedState::OFF) {
        setOnOff(false);
    }
    else if (state == LedState::FLASH){
        setFlash(true) ;
        setOnOff(true );
    }
    currentState = state ;
#if !defined(BEAGLEBONE)
    std::cout << this->describe() << std::endl;
#endif
    return true ;
}

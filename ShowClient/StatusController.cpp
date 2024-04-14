// Copyright © 2024 Charles Kerr. All rights reserved.

#include "StatusController.hpp"
#include <algorithm>
#include "utility/dbgutil.hpp"
using namespace std::string_literals;
// ==================================================================
StatusController::StatusController() {
    leds.push_back(BeagleLed(static_cast<int>(StatusLed::RUN),"Run status"));
    leds.push_back(BeagleLed(static_cast<int>(StatusLed::CONNECT),"Connect status"));
    leds.push_back(BeagleLed(static_cast<int>(StatusLed::SHOW),"Show status"));
    leds.push_back(BeagleLed(static_cast<int>(StatusLed::PLAY),"Play status"));
}


// ==================================================================
auto StatusController::operator[](StatusLed led) const -> const BeagleLed& {
    auto iter = std::find_if(leds.begin(),leds.end(),[led](const BeagleLed &entry){
        return entry.number() == static_cast<int>(led) ;
    });
    if (iter == leds.end()) {
        throw std::runtime_error("Invalid status led was requested: "s + std::to_string(static_cast<int>(led)));
    }
    return *iter ;
}

// ==================================================================
auto StatusController::operator[](StatusLed led) -> BeagleLed& {
    auto iter = std::find_if(leds.begin(),leds.end(),[led](const BeagleLed &entry){
        return entry.number() == static_cast<int>(led) ;
    });
    if (iter == leds.end()) {
        throw std::runtime_error("Invalid status led was requested: "s + std::to_string(static_cast<int>(led)));
    }
    return *iter ;

}

// ==================================================================
auto StatusController::clear() -> void {
    for (auto &entry:leds) {

        entry.setState(LedState::OFF,true);
    }
}

// ==================================================================
auto StatusController::flash() -> void {
    for (auto &entry:leds) {
        entry.setState(LedState::FLASH,true);
    }
}

// ==================================================================
auto StatusController::describe(StatusLed led) const -> std::string {
    auto iter = std::find_if(leds.begin(),leds.end(),[led](const BeagleLed &entry){
        return entry.number() == static_cast<int>(led) ;
    });
    if (iter == leds.end()) {
        throw std::runtime_error("Invalid status led was requested: "s + std::to_string(static_cast<int>(led)));
    }
    return iter->describe();
}

// ==================================================================
auto StatusController::setState(StatusLed led, LedState state,bool force) -> void {
    this->operator[](led).setState(state,force);
}

// Copyright © 2024 Charles Kerr. All rights reserved.
#include "BlinkPru.hpp"
#include <algorithm>
#include <vector>
#include "utility/dbgutil.hpp"

using namespace std::string_literals;

const std::string BlinkPru::BLINK_FIRMWARE = "blinkylight-fw" ;

// =====================================================================================
auto BlinkPru::setDataReady(bool state) -> void {
    auto value = (state ? std::int32_t(1) : std::int32_t(0)) ;
    BeaglePru::setData(reinterpret_cast<const std::uint8_t*>(&value), 4, INDEX_DATAREADY);
}

// =====================================================================================
BlinkPru::BlinkPru(PruNumber pruNumber):BeaglePru(pruNumber),length(0),current_mode(PruModes::SSD),current_mode_size(static_cast<int>(PruModeSize::SSD)){
#if defined(BEAGLE)
    setMode(PruModes::SSD) ;
#endif
}

// =====================================================================================
BlinkPru::~BlinkPru(){
    this->clear();
}
// ============================================================================
// This setups the PRU mode , for its largeset output, and formattting
auto BlinkPru::setMode(PruModes mode, int desired_length) -> bool {
    switch (mode) {
        case PruModes::DMX:{
            current_mode_size = static_cast<int>(PruModeSize::DMX );
            current_mode = PruModes::DMX ;
            length = (desired_length == 0 ? static_cast<int>(PruModeSize::DMX) : std::min(static_cast<int>(PruModeSize::DMX),desired_length));
            break;
        }
        case PruModes::SSD:{
            current_mode_size = static_cast<int>(PruModeSize::SSD );
            current_mode = PruModes::SSD ;
            length = (desired_length == 0 ? static_cast<int>(PruModeSize::SSD) : std::min(static_cast<int>(PruModeSize::SSD),desired_length));
            break;
        }
        case PruModes::WS2812:{
            current_mode_size = static_cast<int>(PruModeSize::WS2812 );
            current_mode = PruModes::WS2812 ;
            length = (desired_length == 0 ? static_cast<int>(PruModeSize::WS2812) : std::min(static_cast<int>(PruModeSize::WS2812),desired_length));
            break;
        }
        default:
            length = 0 ;
            current_mode_size = PRU_MAX_SPACE;
            current_mode = PruModes::UNKNOWN ;

            return false ;
    }
#if defined(BEAGLE)
    auto outmode = static_cast<int>(mode) ;
    auto buffer = std::vector<unsigned char>(PRU_MAX_SPACE, 0 );
    this->setDataReady(false);
    BeaglePru::setData(reinterpret_cast<unsigned char*>(&outmode), 4,INDEX_TYPE);
    BeaglePru::setData(reinterpret_cast<unsigned char*>(&length), 4,INDEX_OUTPUTCOUNT);
#endif
    return true ;
}

// =====================================================================================
auto BlinkPru::mode() const -> PruModes {
    return current_mode ;
}


// ======================================================================================
auto BlinkPru::setConfig(const PRUConfig &config)->bool {
    this->setMode(config.mode,config.length);
    return true ;
}

// ======================================================================================
auto BlinkPru::checkFirmware() -> bool {
#if defined(BEAGLE)
    //DBGMSG(std::cout, "Firmware is: '"s + this->firmware()+"' and we want: '"s + BLINK_FIRMWARE+"'"s);
    return this->firmware() == BLINK_FIRMWARE ;
#else
    return true ;
#endif
}
    
// ======================================================================================
auto BlinkPru::checkState() -> bool {
#if defined(BEAGLE)
    return this->state() == "running" ;
#else
    return true ;
#endif
}


// ======================================================================================
auto BlinkPru::clear() -> void {
#if defined(BEAGLE)
    setDataReady(false) ;
    BeaglePru::clear(INDEX_PRUOUTPUT, current_mode_size) ;
    setDataReady(true);
#endif
}

// ======================================================================================
auto BlinkPru::setData(const std::uint8_t *data,int data_length) -> bool {
#if defined(BEAGLE)
    if (data_length > length) {
        data_length = length;
    }
    setDataReady(false);
    BeaglePru::setData(data, data_length, INDEX_PRUOUTPUT);
    setDataReady(true);
#endif
    return true;
}

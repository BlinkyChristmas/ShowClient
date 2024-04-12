// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BlinkPru.hpp"
#include <algorithm>
#include <vector>
#include "utility/dbgutil.hpp"

using namespace std::string_literals;

const std::string BlinkPru::BLINK_FIRMWARE = "blinkylight-fw" ;


// =====================================================================================
BlinkPru::BlinkPru(PruNumber pruNumber):BeaglePru(pruNumber),offset(0),length(0){
#if defined(BEAGLE)
    setMode(PruModes::SSD) ;
#endif
}

// =====================================================================================
auto BlinkPru::setMode(PruModes mode) -> bool {
#if !defined(BEAGLE)
    return true ;
#else
    auto size = (mode == PruModes::DMX? 512: 3072);
    auto outmode = static_cast<int>(mode) ;
    auto buffer = std::vector<unsigned char>(3072, 0 );
    this->length = size ;
    std::copy(reinterpret_cast<unsigned char*>(&outmode),reinterpret_cast<unsigned char*>(&outmode)+4, mapped_address+ INDEX_TYPE) ;
    std::copy(reinterpret_cast<unsigned char*>(&size),reinterpret_cast<unsigned char*>(&size)+4, mapped_address + INDEX_OUTPUTCOUNT) ;
    std::copy(reinterpret_cast<const unsigned char*>(&zero),reinterpret_cast<const unsigned char*>(&zero)+4, mapped_address + INDEX_DATAREADY);
    std::copy(buffer.begin(),buffer.end(),mapped_address + INDEX_PRUOUTPUT) ;
    std::copy(reinterpret_cast<const char*>(&one),reinterpret_cast<const char*>(&one)+4,mapped_address + INDEX_DATAREADY) ;
    return true ;
#endif
}

// =====================================================================================
auto BlinkPru::mode() const -> PruModes {
#if !defined(BEAGLE)
    return PruModes::SSD;
#else
    if ((pru_number != PruNumber::zero &&  pru_number != PruNumber::one) ) {
        return  PruModes::UNKNOWN ;
    }
    auto mode = std::int32_t(0) ;
    std::copy(mapped_address+ INDEX_TYPE,mapped_address+ INDEX_TYPE+4,reinterpret_cast<unsigned char*>(&mode)) ;
    if (mode == static_cast<int>(PruModes::SSD)){
        return PruModes::SSD ;
    }
    else if (mode == static_cast<int>(PruModes::DMX)) {
        return PruModes::DMX ;
    }
    else if (mode == static_cast<int>(PruModes::WS2812)) {
        return PruModes::WS2812 ;
    }
    else {
        return PruModes::UNKNOWN;
    }
#endif
}

// =====================================================================================
auto BlinkPru::setData(const std::uint8_t *data, int length, int offset ) -> bool {
#if !defined (BEAGLE)
    return true ;
#else
    if ((pru_number != PruNumber::zero &&  pru_number != PruNumber::one) ) {
        return false ;
    }
    if (offset + length > this->length) {
        length = this->length - offset ;
    }
    std::copy(data,data+length,mapped_address + INDEX_PRUOUTPUT);
    std::copy(reinterpret_cast<const char*>(&one),reinterpret_cast<const char*>(&one)+4,mapped_address + INDEX_DATAREADY) ;
    return true ;
#endif 
}

// =====================================================================================
auto BlinkPru::setData(const std::uint8_t *data, int length ) -> bool {
    return this->setData(data, length, this->offset);
}

// ======================================================================================
auto BlinkPru::setConfig(const PRUConfig &config)->bool {
    auto prumode = PruModes::SSD ;
    if (config.mode == PRUConfig::MODE_DMX){
        prumode = PruModes::DMX ;
    }
    else if (config.mode == PRUConfig::MODE_WS2812) {
        prumode = PruModes::WS2812 ;
    }
    this->setMode(prumode);
    this->offset = config.offset ;
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
    //DBGMSG(std::cout, "State is: "s + this->state());
    //auto rvalue = this->state() == "running" ;
    //DBGMSG(std::cout, "checkState returning: "s + std::to_string(rvalue));
    return this->state() == "running" ;
#else
    return true ;
#endif

}

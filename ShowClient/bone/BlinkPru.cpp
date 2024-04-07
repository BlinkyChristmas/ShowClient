// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BlinkPru.hpp"
#include <algorithm>
#include <vector>
const std::string BlinkPru::BLINK_FIRMWARE = "blinkylights.fw" ;

// =====================================================================================
BlinkPru::BlinkPru(PruNumber pruNumber):BeaglePru(pruNumber){
    this->load(BLINK_FIRMWARE);
    this->start() ;
    setMode(PruModes::SSD) ;
}

// =====================================================================================
auto BlinkPru::setMode(PruModes mode) -> bool {
    if ((pru_number != PruNumber::zero &&  pru_number != PruNumber::one) || !isValid()) {
        return  false ;
    }
    auto bit = std::int32_t((static_cast<int>(pru_number) == 0 ? 14 : 1)) ; // PRU 0 is on bit 14, pru 1 is on bit 1 ;
    auto size = (mode == PruModes::DMX? 512: 3072);
    auto outmode = static_cast<int>(mode) ;
    auto buffer = std::vector<unsigned char>(3072, 0 );
    std::copy(reinterpret_cast<unsigned char*>(&outmode),reinterpret_cast<unsigned char*>(&outmode)+4, mapped_address+ INDEX_TYPE) ;
    std::copy(reinterpret_cast<unsigned char*>(&bit),reinterpret_cast<unsigned char*>(&bit)+4, mapped_address + INDEX_BITREG) ;
    std::copy(reinterpret_cast<unsigned char*>(&size),reinterpret_cast<unsigned char*>(&size)+4, mapped_address + INDEX_OUTPUTCOUNT) ;
    std::copy(reinterpret_cast<const unsigned char*>(&zero),reinterpret_cast<const unsigned char*>(&zero)+4, mapped_address + INDEX_DATAREADY);
    std::copy(buffer.begin(),buffer.end(),mapped_address + INDEX_PRUOUTPUT) ;
    std::copy(reinterpret_cast<const char*>(&one),reinterpret_cast<const char*>(&one)+4,mapped_address + INDEX_DATAREADY) ;
    return true ;
}

// =====================================================================================
auto BlinkPru::mode() const -> PruModes {
    if ((pru_number != PruNumber::zero &&  pru_number != PruNumber::one) || !isValid()) {
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
}

// =====================================================================================
auto BlinkPru::setData(const std::uint8_t *data, int length, int offset ) -> bool {
    if ((pru_number != PruNumber::zero &&  pru_number != PruNumber::one) || !isValid()) {
        return false ;
    }
    if (offset + length > 3072) {
        length = 3072 - offset ;
    }
    std::copy(data,data+length,mapped_address + INDEX_PRUOUTPUT);
    std::copy(reinterpret_cast<const char*>(&one),reinterpret_cast<const char*>(&one)+4,mapped_address + INDEX_DATAREADY) ;
    return true ;
}

// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BlinkPru_hpp
#define BlinkPru_hpp

#include "BeaglePru.hpp"

enum class PruModes {
    SSD,DMX,WS2812,UNKNOWN
};

class BlinkPru : public BeaglePru {
    static const std::string BLINK_FIRMWARE ;
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;
    static constexpr auto zero = std::int32_t(0) ;
    static constexpr auto one = std::int32_t(1) ;


public:
    BlinkPru(PruNumber pruNumber) ;
    auto setMode(PruModes mode) -> bool ;
    auto mode() const -> PruModes ;
    auto setData(const std::uint8_t *data, int length, int offset = 0 ) -> bool ;
};
#endif /* BlinkPru_hpp */
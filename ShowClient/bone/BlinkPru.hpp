// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BlinkPru_hpp
#define BlinkPru_hpp

#include "BeaglePru.hpp"
#include "PRUConfig.hpp"
#include <utility>
#include <cstdint> 

enum class PruModes {
    SSD,DMX,WS2812,UNKNOWN
};

class BlinkPru : public BeaglePru {
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;
    static constexpr auto zero = std::int32_t(0) ;
    static constexpr auto one = std::int32_t(1) ;

    int offset ;
    int length ;
 public:
    static const std::string BLINK_FIRMWARE ;

    BlinkPru(PruNumber pruNumber) ;
    ~BlinkPru();
    
    auto setMode(PruModes mode) -> bool ;
    auto mode() const -> PruModes ;
    auto setData(const std::uint8_t *data, int length, int offset  ) -> bool ;
    auto setData(const std::uint8_t *data, int length ) -> bool ;
    auto setConfig(const PRUConfig &config)->bool ;
    auto destination() const -> std::pair<std::uint8_t*,int> ;
    auto checkFirmware() -> bool ;
    auto checkState() -> bool ;
    auto clear() -> void ;
    
};
#endif /* BlinkPru_hpp */

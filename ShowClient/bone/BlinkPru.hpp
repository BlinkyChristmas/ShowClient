// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BlinkPru_hpp
#define BlinkPru_hpp

#include "BeaglePru.hpp"
#include "PRUConfig.hpp"
#include <utility>
#include <cstdint> 

#include "PruModes.hpp"
#include "PruConstants.hpp"

class BlinkPru : public BeaglePru {
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;
    static constexpr auto PRU_MAX_SPACE = PRUMAPSIZE - INDEX_PRUOUTPUT ;
    
    static constexpr auto zero = std::int32_t(0) ;
    static constexpr auto one = std::int32_t(1) ;

    int length ;    
    PruModes current_mode ;
    int current_mode_size ;
    
    auto setDataReady(bool state) -> void ;
 public:
    static const std::string BLINK_FIRMWARE ;

    BlinkPru(PruNumber pruNumber) ;
    ~BlinkPru();
    
    auto setMode(PruModes mode,int desired_length = 0) -> bool ;
    auto mode() const -> PruModes ;
    auto setConfig(const PRUConfig &config)->bool ;
    auto checkFirmware() -> bool ;
    auto checkState() -> bool ;
    auto clear() -> void ;
    auto setData(const std::uint8_t *data,int data_length) -> bool ;
};
#endif /* BlinkPru_hpp */

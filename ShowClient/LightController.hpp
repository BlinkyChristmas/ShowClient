//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef LightController_hpp
#define LightController_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <array>

#include "lightfile/lightfile.hpp"
#include "PRUConfig.hpp"
//======================================================================
class LightController {
    static constexpr size_t PRUMAPSIZE = 8192 ;
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;

    auto mapPRU(int number) -> void ;
    auto freePRU(int prunumber) -> void ;
    auto initPRU(int number) -> void ;

    LightFile lightFile ;
    std::array<PRUConfig,2> pruConfiguration ;
    std::array<volatile std::uint8_t *,2> pruBuffers ;


public:
    LightController() ;
    ~LightController() ;
    
    auto start(std::int32_t frame = 0) -> bool ;
    auto stop(bool close = true) -> void ;
    auto close() -> void ;
    auto configurePRU(PRUConfig pru0,PRUConfig pru1) -> void ;

    auto load(const std::filesystem::path &path) -> bool ;
    auto frameCount() const -> std::int32_t ;
    auto sync(std::int32_t frame) -> void ;
};

#endif /* LightController_hpp */

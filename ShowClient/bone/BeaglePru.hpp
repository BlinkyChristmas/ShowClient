// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BeaglePru_hpp
#define BeaglePru_hpp

#include <string>
#include <iostream>

enum class PruNumber {
  zero=0,one=1,invalid=2
};



class BeaglePru {
protected:
    static const std::string firmware_location ;
    static const std::string firmware_state ;
    static const std::string runing_state ;
    static const std::string offline_state ;
    static const std::string halt_state ;
    
    static constexpr size_t PRUMAPSIZE = 8192 ;
    static constexpr auto INDEX_TYPE = 0 ;
    static constexpr auto INDEX_BITREG = 4 ;
    static constexpr auto INDEX_DATAREADY = 8 ;
    static constexpr auto INDEX_OUTPUTCOUNT = 12 ;
    static constexpr auto INDEX_PRUOUTPUT = 16 ;
 
    PruNumber pru_number ;
    
    std::uint8_t *mapped_address;
    
    auto mapPRU() -> bool ;
    auto unmapPRU() -> void ;
    
    auto isState(const std::string &state) const -> bool ;
    auto setState(const std::string &state) -> bool ;
    
public:
    BeaglePru(PruNumber pruNumber);
    virtual ~BeaglePru() ;
    auto open() -> bool ;
    auto close() -> void ;

    auto isValid() const -> bool ;
    auto address() -> std::uint8_t* ;
    
    auto isOffline() const -> bool ;
    auto isRunning() const -> bool ;
    auto load(const std::string &firmware) -> bool ;
    auto firmware() const -> std::string ;
    auto start() -> bool ;
    auto stop() -> bool ;
};

#endif /* BeaglePru_hpp */

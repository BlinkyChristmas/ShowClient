// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BeaglePru_hpp
#define BeaglePru_hpp

#include <string>
#include <iostream>

#include "PruModes.hpp"
#include "PruConstants.hpp"
class BeaglePru {
public:
    
protected:
    
    
    PruNumber pru_number ;
    
    std::uint8_t *mapped_address;
    
    int output_size ;
    
    auto mapPRU() -> bool ;
    auto unmapPRU() -> void ;
    
    auto loadData(const std::uint8_t *ptrToData, int length) const -> bool ;
public:
    BeaglePru(PruNumber pruNumber);
    virtual ~BeaglePru() ;

    auto address() -> std::uint8_t* ;
    auto firmware() const -> std::string ;
    auto state() const -> std::string ;
    auto setData(const std::uint8_t *ptrToData, int length, int offsetInPruMemory) -> bool ;
    auto clear(int offsetInPruMemory,int length) -> bool ;
};

#endif /* BeaglePru_hpp */

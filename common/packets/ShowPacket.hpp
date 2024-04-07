//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ShowPacket_hpp
#define ShowPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"
//======================================================================

/* *****************************************************************************
 ShowPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 state                          std::int32_t                            8 (0/1 value)
 ******************************************************************************* */
class ShowPacket : public Packet {
public:
    static constexpr auto STATEOFFSET = Packet::PACKETHEADERSIZE ;
    
public:
    static constexpr auto PACKETSIZE = STATEOFFSET + 4 ;
    
    ShowPacket() ;
    ShowPacket(bool state) ;
    
    auto state() const -> bool ;
    auto setState(bool value) -> void ;
    
};

#endif /* ShowPacket_hpp */

//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PlayPacket_hpp
#define PlayPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"
//======================================================================

/* *****************************************************************************
 PlayPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 state                          std::uint32_t                           8
 frame                          std::int32_t                            12
 ******************************************************************************* */
class PlayPacket : public Packet {
    
    static constexpr auto STATEOFFSET = Packet::PACKETHEADERSIZE ;
    static constexpr auto FRAMEOFFSET = STATEOFFSET + 4 ;
    
public:

    static constexpr auto PACKETSIZE = FRAMEOFFSET + 4 ;
    
    PlayPacket() ;
    PlayPacket(bool state, std::int32_t frame = 0);
    
    auto state() const -> bool ;
    auto setState(bool value) -> void ;
    
    auto frame() const -> std::int32_t ;
    auto setFrame(std::int32_t value) -> void ;
};

#endif /* PlayPacket_hpp */

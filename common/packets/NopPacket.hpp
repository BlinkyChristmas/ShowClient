//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef NopPacket_hpp
#define NopPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include "Packet.hpp"

//======================================================================
/* *****************************************************************************
 NopPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 respond                        std::uint32_t                           8
 ******************************************************************************* */

//======================================================================
class NopPacket : public Packet {
    
    static constexpr auto RESPONDOFFSET = Packet::PACKETHEADERSIZE ;
    
public:
    static constexpr auto PACKETSIZE = RESPONDOFFSET + 4 ;
    
    NopPacket() ;
    NopPacket(bool respond) ;
    
    auto respond() const -> bool ;
    auto setRespond(bool value) -> void ;
};

#endif /* NopPacket_hpp */

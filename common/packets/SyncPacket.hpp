//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef SyncPacket_hpp
#define SyncPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"

//======================================================================
/* *****************************************************************************
 SyncPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 syncFrame                      std::int32_t                            8
 ******************************************************************************* */

class SyncPacket : public Packet {
    static constexpr auto SYNCOFFSET = Packet::PACKETHEADERSIZE ;
    
public:
    static constexpr auto PACKETSIZE = SYNCOFFSET + 4 ;
    
    SyncPacket() ;
    SyncPacket(std::int32_t sync);
    auto syncFrame() const -> std::int32_t ;
    auto setSyncFrame(std::int32_t value) -> void ;
};

#endif /* SyncPacket_hpp */

// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BufferPacket_hpp
#define BufferPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"

//======================================================================
/* *****************************************************************************
 BufferPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 Buffer data                    [unsigned char]                         8
 ******************************************************************************* */

class BufferPacket : public Packet {
    static constexpr auto BUFFER = Packet::PACKETHEADERSIZE ;
    
public:
    
    
    BufferPacket() ;
    BufferPacket(const std::vector<std::uint8_t> &data);
    auto setPacketData(const std::vector<std::uint8_t> &data) -> void ;
    auto packetData() const  -> std::vector<std::uint8_t> ;
};

#endif /* BufferPacket_hpp */

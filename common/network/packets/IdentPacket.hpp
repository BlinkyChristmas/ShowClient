//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef IdentPacket_hpp
#define IdentPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"
//======================================================================

/* *****************************************************************************
 IdentPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 handle                         char[30]                                8
 ******************************************************************************* */
class IdentPacket : public Packet {
    static constexpr auto HANDLEOFFSET = Packet::PACKETHEADERSIZE ;
    static constexpr auto HANDLESIZE = 30 ;
public:
    static constexpr auto PACKETSIZE = HANDLEOFFSET + HANDLESIZE ;

    IdentPacket() ;
    IdentPacket(const std::string &handle);
    auto handle() const -> std::string ;
    auto setHandle(const std::string &value) -> void ;
};

#endif /* IdentPacket_hpp */

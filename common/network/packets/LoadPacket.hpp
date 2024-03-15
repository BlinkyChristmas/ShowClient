//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef LoadPacket_hpp
#define LoadPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "Packet.hpp"

//======================================================================
/* *****************************************************************************
 LoadPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 musicName                      char[30]                                8
 lightName                      char[30]                                38
 ******************************************************************************* */

class LoadPacket : public Packet {
    static constexpr auto NAMESIZE = 30 ;
    static constexpr auto MUSICOFFSET = Packet::PACKETHEADERSIZE ;
    static constexpr auto LIGHTOFFSET = MUSICOFFSET + NAMESIZE ;
    
public:
    static constexpr auto PACKETSIZE = LIGHTOFFSET + NAMESIZE ;
    
    LoadPacket() ;
    LoadPacket(const std::string &music, const std::string &light) ;
    
    auto musicName() const -> std::string ;
    auto setMusicName(const std::string &value) -> void ;
    
    auto lightName() const -> std::string ;
    auto setLightName(const std::string &value) -> void ;
};

#endif /* LoadPacket_hpp */

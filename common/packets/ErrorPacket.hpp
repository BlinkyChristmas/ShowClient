//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ErrorPacket_hpp
#define ErrorPacket_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Packet.hpp"

//======================================================================
/* *****************************************************************************
 ErrorPacket
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 statustype                     std::int32_t                            8
 musicName                      char[30]                                12
 ******************************************************************************* */

class ErrorPacket : public Packet {
    
public:
    enum CatType {
        AUDIO,LIGHT,UNKNOWN
    };
    static auto nameForCategory(CatType type) -> const std::string & ;
    static const std::unordered_map<CatType,std::string> CATNAME ;
private:
    
    static constexpr auto STATTYPEOFFSET = Packet::PACKETHEADERSIZE ;
    static constexpr auto NAMEOFFSET = STATTYPEOFFSET + 4 ;
    static constexpr auto NAMESIZE = 30 ;
    
public:
    static constexpr auto PACKETSIZE = NAMEOFFSET + NAMESIZE ;
    
    ErrorPacket() ;
    ErrorPacket(ErrorPacket::CatType cat, const std::string &name);
    auto category() const -> ErrorPacket::CatType ;
    auto setCategory(ErrorPacket::CatType cat) -> void ;
    auto name() const -> std::string ;
    auto setName(const std::string &value) -> void ;
};


#endif /* ErrorPacket_hpp */

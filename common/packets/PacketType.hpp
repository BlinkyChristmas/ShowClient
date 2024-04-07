//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PacketType_hpp
#define PacketType_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

//======================================================================
struct PacketType {
    static const std::vector<std::string> PACKETNAME ;
    enum PacketID : std::uint32_t {
        UNKNOWN = 0, IDENT, SYNC, LOAD, NOP,SHOW,PLAY,MYERROR
    };
    
    static auto nameForPacket(PacketID packID) -> const std::string& ;
    static auto packetForName(const std::string &name) -> PacketID ;
};

#endif /* PacketType_hpp */

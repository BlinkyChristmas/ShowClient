//Copyright © 2024 Charles Kerr. All rights reserved.

#include "PacketType.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <iterator>

#include "utility/strutil.hpp"

using namespace std::string_literals ;

// ========================================================================
const std::vector<std::string> PacketType::PACKETNAME{
    "UNKNOWN"s,"IDENT"s,"SYNC"s, "LOAD"s, "NOP"s,"SHOW"s,"PLAY"s,"ERROR"s
};

// ========================================================================
auto PacketType::nameForPacket(PacketID packID) -> const std::string& {
    
    // Check the size, ensure it is within range
    if (static_cast<size_t>(packID) >= PACKETNAME.size()) {
        return PACKETNAME.at(0);
    }
    return PACKETNAME.at(static_cast<size_t>(packID)) ;
}

// ========================================================================
auto PacketType::packetForName(const std::string &name) -> PacketID {
    auto iter = std::find_if( PACKETNAME.begin(),PACKETNAME.end(),[name](const std::string &entry ){
        return name == entry ;
    });
    if (iter == PACKETNAME.end()){
        // Couldn't find it!
        return PacketType::UNKNOWN ;
    }
    // We just need to find the distance this is from
    auto id = std::distance( PACKETNAME.begin(), iter ) ;
    return static_cast<PacketID>( id );
}


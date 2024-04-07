//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Packet_hpp
#define Packet_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "utility/buffer.hpp"
#include "utility/timeutil.hpp"

#include "PacketType.hpp"

/* *****************************************************************************
 All packets have the same header
 
 Name                               Type                                Offset
 packetID                       std::uint32_t                           0
 length                         std::uint32_t                           4
 
 ******************************************************************************* */

//======================================================================
class Packet : public util::Buffer {
    static constexpr auto PACKETLENGTHOFFSET = 4 ;
    util::ourclock::time_point timeStamp;
public:
    static constexpr auto PACKETHEADERSIZE = std::int32_t(8) ;

    Packet() ;
    Packet(const std::vector<std::uint8_t> &data);
    Packet(PacketType::PacketID packID, std::int64_t size) ;
    
    Packet(const Packet &valueToCopy) ;
    auto operator=(const Packet &valueToCopy) -> Packet& ;
    
    auto operator==(PacketType::PacketID packID) const -> bool ;

    // This return the internal packet data buffer size
    auto size() const -> std::uint32_t ;

    auto packetID() const -> PacketType::PacketID ;
    auto setPacketID(PacketType::PacketID packID) -> void ;
    auto type() const -> const std::string & ;
    
    // This returns the "length" of the packet, according to data in the packet
    auto length() const -> std::uint32_t ;
    auto setLength(std::uint32_t length) -> void ;
    
    // An ability to timestamp packets
    auto stamp() -> void ;
    auto millisecondsSince(const util::ourclock::time_point &now) -> size_t ;
    
};

#endif /* Packet_hpp */

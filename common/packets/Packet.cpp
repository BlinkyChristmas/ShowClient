//Copyright © 2024 Charles Kerr. All rights reserved.

#include "Packet.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

// ==========================================================================================
Packet::Packet() : util::Buffer() {
    this->setExtend(true);
    this->setOffset(0);
    this->timeStamp = util::ourclock::now() ;
}

// ==========================================================================================
Packet::Packet(const std::vector<std::uint8_t> &data):Packet() {
    this->data = data ;
}

// ==========================================================================================
Packet::Packet(PacketType::PacketID packID, std::int64_t size) : Packet() {
    this->resize(size);
    this->setPacketID(packID);
    this->setLength(static_cast<std::uint32_t>(size)) ;
}

// ==========================================================================================
Packet::Packet(const Packet &valueToCopy):Packet() {
    this->data = valueToCopy.data ;
}

// ==========================================================================================
auto Packet::operator=(const Packet &valueToCopy) -> Packet& {
    this->data = valueToCopy.data ;
    return *this ;
}

// ==========================================================================================
auto Packet::operator==(PacketType::PacketID packID) const -> bool  {
    return this->packetID() == packID ;
}

// This return the internal packet data buffer size
// ==========================================================================================
auto Packet::size() const -> std::uint32_t {
    return static_cast<std::uint32_t>(data.size()) ;
}

// ==========================================================================================
auto Packet::packetID() const -> PacketType::PacketID {
    auto idnum = this->read<std::uint32_t>(0) ;
    // We should check, to ensure this isnt beyond the larget we have
    if ( ( idnum >= PacketType::PACKETNAME.size() ) || ( this->size() <= 4) ){
        // Well, we can "throw", or we can just say, this is an unknown packet
        return PacketType::UNKNOWN ;
    }
    return static_cast<PacketType::PacketID>(idnum) ;
}
// ==========================================================================================
auto Packet::setPacketID(PacketType::PacketID packID) -> void {
    auto idnum = static_cast<std::uint32_t>(packID) ;
    this->write(idnum, 0) ;
}
// ==========================================================================================
auto Packet::type() const -> const std::string & {
    return PacketType::nameForPacket(this->packetID()) ;
}

// This returns the "length" of the packet, according to data in the packet
// ==========================================================================================
auto Packet::length() const -> std::uint32_t {
    if (PACKETLENGTHOFFSET + 4 > this->size()) {
        return 0 ;
    }
    return this->read<std::uint32_t>(PACKETLENGTHOFFSET) ;
}
// ==========================================================================================
auto Packet::setLength(std::uint32_t length) -> void {
    this->write(length,PACKETLENGTHOFFSET);
}

// ==========================================================================================
auto Packet::stamp() -> void {
    timeStamp = util::ourclock::now() ;
}
// ==========================================================================================
auto Packet::millisecondsSince(const util::ourclock::time_point &now) -> size_t{
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - timeStamp).count() ;
}


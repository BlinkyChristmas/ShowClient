// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BufferPacket.hpp"
//======================================================================
BufferPacket::BufferPacket() : Packet(PacketType::BUFFER,Packet::PACKETHEADERSIZE){
    
}

//======================================================================
BufferPacket::BufferPacket(const std::vector<std::uint8_t> &data) : BufferPacket() {
    // We need to copy the data into the buffer, and then set the length
    setPacketData(data) ;
}
//======================================================================
auto BufferPacket::setPacketData(const std::vector<std::uint8_t> &packetdata) -> void {
    this->resize(packetdata.size() + Packet::PACKETHEADERSIZE);
    this->setLength(static_cast<std::uint32_t>(packetdata.size()) + Packet::PACKETHEADERSIZE);
    std::copy(packetdata.begin(), packetdata.end(),this->bufferData().begin()+Packet::PACKETHEADERSIZE) ;

}

//======================================================================
auto BufferPacket::packetData() const   -> std::vector<std::uint8_t>  {
    auto packetdata = std::vector<std::uint8_t>(this->length() - Packet::PACKETHEADERSIZE,0) ;
    std::copy(this->bufferData().begin() + Packet::PACKETHEADERSIZE, this->bufferData().end(), packetdata.begin() );
    return packetdata;
}


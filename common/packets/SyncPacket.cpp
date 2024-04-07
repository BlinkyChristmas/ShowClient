//Copyright © 2024 Charles Kerr. All rights reserved.

#include "SyncPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
SyncPacket::SyncPacket() : Packet(PacketType::SYNC,SyncPacket::PACKETSIZE){
    
}

//======================================================================
SyncPacket::SyncPacket(std::int32_t sync) : SyncPacket() {
    this->setSyncFrame(sync) ;
}
//======================================================================
auto SyncPacket::syncFrame() const -> std::int32_t {
    return this->read<std::int32_t>(SYNCOFFSET) ;
}

//======================================================================
auto SyncPacket::setSyncFrame(std::int32_t value) -> void {
    return this->write(value,SYNCOFFSET) ;
}


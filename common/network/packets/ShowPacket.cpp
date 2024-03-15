//Copyright © 2024 Charles Kerr. All rights reserved.

#include "ShowPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
ShowPacket::ShowPacket() : Packet(PacketType::SHOW,PACKETSIZE){
}

//======================================================================
ShowPacket::ShowPacket(bool state) :ShowPacket() {
    this->setState(state);
}

//======================================================================
auto ShowPacket::state() const -> bool {
    return this->read<std::uint32_t>(STATEOFFSET) != 0 ;
}

//======================================================================
auto ShowPacket::setState(bool value) -> void {
    this->write( static_cast<std::int32_t>(value), STATEOFFSET) ;
}

//Copyright © 2024 Charles Kerr. All rights reserved.

#include "NopPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
NopPacket::NopPacket() :Packet(PacketType::NOP, PACKETSIZE){
    
}

//======================================================================
NopPacket::NopPacket(bool respond):NopPacket() {
    this->setRespond(respond);
}

//======================================================================
auto NopPacket::respond() const -> bool {
    return this->read<std::uint32_t>(RESPONDOFFSET) != 0 ;
}
//======================================================================
auto NopPacket::setRespond(bool value) -> void {
    this->write( (value ? std::uint32_t(1) : std::uint32_t(0)) ,RESPONDOFFSET);
}

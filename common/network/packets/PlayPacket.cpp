//Copyright © 2024 Charles Kerr. All rights reserved.

#include "PlayPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
PlayPacket::PlayPacket() : Packet(PacketType::PLAY, PACKETSIZE){
    
}

//======================================================================
PlayPacket::PlayPacket(bool state, std::int32_t frame):PlayPacket() {
    this->setState(state) ;
    this->setFrame(frame) ;
}


//======================================================================
auto PlayPacket::state() const -> bool {
    return this->read<std::int32_t>(STATEOFFSET) != 0 ;
}

//======================================================================
auto PlayPacket::setState(bool value) -> void {
    this->write( static_cast<std::int32_t>( value ), STATEOFFSET);
}


//======================================================================
auto PlayPacket::frame() const -> std::int32_t {
    return this->read<std::int32_t>(FRAMEOFFSET)  ;

}

//======================================================================
auto PlayPacket::setFrame(std::int32_t value) -> void {
    this->write( value , FRAMEOFFSET);

}


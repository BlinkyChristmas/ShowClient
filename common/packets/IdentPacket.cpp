//Copyright © 2024 Charles Kerr. All rights reserved.

#include "IdentPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;


//======================================================================
IdentPacket::IdentPacket():Packet(PacketType::IDENT, IdentPacket::PACKETSIZE) {
    
}

//======================================================================
IdentPacket::IdentPacket(const std::string &handle):IdentPacket() {
    this->setHandle(handle);
}

//======================================================================
auto IdentPacket::handle() const -> std::string {
    return this->read<std::string>(HANDLESIZE,HANDLEOFFSET) ;
}

//======================================================================
auto IdentPacket::setHandle(const std::string &value) -> void {
    this->write(value,HANDLESIZE,HANDLEOFFSET);
}

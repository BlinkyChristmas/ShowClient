//Copyright © 2024 Charles Kerr. All rights reserved.

#include "LoadPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
LoadPacket::LoadPacket() :Packet(PacketType::LOAD, PACKETSIZE){
    
}
//======================================================================
LoadPacket::LoadPacket(const std::string &music, const std::string &light) :LoadPacket(){
    this->setMusicName(music) ;
    this->setLightName(light);
}

//======================================================================
auto LoadPacket::musicName() const -> std::string {
    return this->read<std::string>(NAMESIZE,MUSICOFFSET) ;
}

//======================================================================
auto LoadPacket::setMusicName(const std::string &value) -> void {
    this->write(value,NAMESIZE,MUSICOFFSET);
}

//======================================================================
auto LoadPacket::lightName() const -> std::string {
    
    return this->read<std::string>(NAMESIZE,LIGHTOFFSET) ;
}

//======================================================================
auto LoadPacket::setLightName(const std::string &value) -> void {
    this->write(value,NAMESIZE,LIGHTOFFSET);

}


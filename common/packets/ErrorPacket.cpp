//Copyright © 2024 Charles Kerr. All rights reserved.

#include "ErrorPacket.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

//======================================================================
const std::unordered_map<ErrorPacket::CatType,std::string> ErrorPacket::CATNAME {
    {CatType::AUDIO,"AUDIO"s},{CatType::LIGHT,"LIGHT"s},{CatType::UNKNOWN,"UNKNOWN"s}
};

//======================================================================
auto  ErrorPacket::nameForCategory( ErrorPacket::CatType type) -> const std::string & {
    auto iter = std::find_if(CATNAME.begin(),CATNAME.end(),[type](const std::pair<ErrorPacket::CatType,std::string> &value){
        return type == value.first;
    });
    if (iter == CATNAME.end()) {
        return CATNAME.at(CatType::UNKNOWN) ;
    }
    return iter->second ;
}

//======================================================================
ErrorPacket::ErrorPacket() : Packet(PacketType::MYERROR,PACKETSIZE) {
    
}
//======================================================================
ErrorPacket::ErrorPacket(ErrorPacket::CatType cat, const std::string &name):ErrorPacket(){
    this->setCategory(cat) ;
    this->setName(name) ;
}
//======================================================================
auto ErrorPacket::category() const -> ErrorPacket::CatType {
    return static_cast<CatType>(this->read<int>(STATTYPEOFFSET) ) ;
    
}
//======================================================================
auto ErrorPacket::setCategory(ErrorPacket::CatType cat) -> void {
    this->write(static_cast<int>(cat),STATTYPEOFFSET) ;
}
//======================================================================
auto ErrorPacket::name() const -> std::string {
    return this->read<std::string>(NAMESIZE,NAMEOFFSET) ;

}
//======================================================================
auto ErrorPacket::setName(const std::string &value) -> void {
    this->write(value,NAMESIZE,NAMEOFFSET);
}

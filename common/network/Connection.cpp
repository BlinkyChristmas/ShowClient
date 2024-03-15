//Copyright © 2024 Charles Kerr. All rights reserved.

#include "Connection.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto Connection::open() -> bool {
    
    asio::error_code ec ;
    
    if (netSocket.is_open()) {
        netSocket.close() ;
    }
    
    netSocket.open(asio::ip::tcp::v4(),ec) ;
    if (ec) {
        DBGMSG(std::cerr,"Open socket failed: "s + ec.message());
        return false ;
    }
    // Set any options we have?
    try {
        netSocket.set_option(asio::socket_base::keep_alive(true)) ;
    }
    catch(...) {
        // we dont need to do anything I guess
    }

    return true ;
}


//======================================================================
auto Connection::read(int amount, int offset) -> void {
    if (netSocket.is_open()){
        incomingAmount = amount ;
        asio::async_read(this->netSocket,asio::buffer(incomingPacket.data.data()+offset,amount),std::bind(&Connection::readHandler,this,std::placeholders::_1,std::placeholders::_2));
    }
}

//======================================================================
auto Connection::readHandler(const asio::error_code& ec, size_t bytes_transferred) -> void {
    if (ec) {
        // We had an error on read
        //DBGMSG(std::cout, "Error on connection read: "s + ec.message());
        // I thinks we should just close?
        try {
            
            if (netSocket.is_open()) {
                netSocket.close() ;
                if (closeCallback != nullptr) {
                     closeCallback(this) ;
                 }
           }
        }
        catch(...){}
        return ;

    }
    if (bytes_transferred != incomingAmount) {
        // We should have gotten more?
        auto amount = incomingAmount - static_cast<int>(bytes_transferred) ;
        this->read( amount , incomingPacket.size() - amount ) ;
        return ;
    }
    if (incomingPacket.length() != incomingPacket.size()) {
        // We need more data!
        auto amount = incomingPacket.length() - incomingPacket.size() ;
        incomingPacket.resize(incomingPacket.length()) ;
        this->read(amount,incomingPacket.size() - amount) ;
        return ;
    }
    // We got what we need
    lastRead = util::ourclock::now() ;
    
    //
    auto status = true ;
    if (processingCallback != nullptr) {
        status = processingCallback(incomingPacket,this) ;
    }
    if (status) {
        this->read() ;
    }
}


//======================================================================
auto Connection::resolve(const std::string &ipaddress, std::uint16_t port) -> asio::ip::tcp::endpoint {
    try {
        asio::io_context io_context ;
        asio::ip::tcp::resolver resolver(io_context) ;
        asio::ip::tcp::resolver::query query(ipaddress.c_str(),"") ;
        auto iter = resolver.resolve(query) ;
        if (iter != asio::ip::tcp::resolver::iterator()) {
            auto serverEndpoint = iter->endpoint() ;
            // Now we need to add the port
            return asio::ip::tcp::endpoint(serverEndpoint.address(),port) ;
        }
        return  asio::ip::tcp::endpoint() ;
    }
    catch(...) {
        return  asio::ip::tcp::endpoint() ;
    }
    
}

//======================================================================
Connection::Connection(asio::io_context &io_context):netSocket(io_context),processingCallback(nullptr),closeCallback(nullptr) {
    
}

//======================================================================
Connection::~Connection(){
    
    if (netSocket.is_open()) {
        try {
            netSocket.cancel();
        }
        catch(...){}
        netSocket.close() ;
    }
}
//======================================================================
auto Connection::socket() -> asio::ip::tcp::socket& {
    return netSocket ;
}

//======================================================================
auto Connection::is_open() const -> bool{
    return netSocket.is_open() ;
}

//======================================================================
auto Connection::close() -> void {
    try {
        if (netSocket.is_open()) {
            netSocket.close() ;
        }
    }
    catch(...){
        // we do nothing, just ignore it if it happens (should not?)
    }
}

//=======================================================================
auto Connection::open(std::uint16_t port) -> bool {
    asio::error_code ec ;
    try {
        if (this->is_open()) {
            this->close() ;
        }
        this->open() ;
        if (port != 0) {
            // we are going to try to bind this socket
            auto endpoint = asio::ip::tcp::endpoint(asio::ip::address_v4::any(),port) ;
            asio::socket_base::reuse_address option(true) ;
            this->netSocket.set_option(option) ;
            netSocket.bind(endpoint,ec) ;
            if (ec) {
                DBGMSG(std::cerr, "Error on binding socket: "s + ec.message());
                netSocket.close() ;
                return false ;
            }
        }
        return true ;
    }
    catch(...) {
        this->close() ;
        return false ;
    }
}

//======================================================================
auto Connection::connect(asio::ip::tcp::endpoint &endpoint) -> bool {
    asio::error_code ec ;
    if (!netSocket.is_open()){
        DBGMSG(std::cerr, "Can not connect, socket was not open.");
        return false ;
    }
    netSocket.connect(endpoint,ec) ;
    if (ec) {
        // we got an error
        DBGMSG(std::cerr, "Unable to connect: "s + ec.message()) ;
        return false ;
    }
    // We need to get our peer information ;
    this->setPeer() ;
    this->timestamp() ;
    
    return true ;
}

//======================================================================
auto Connection::setPeer() -> void {
    if (netSocket.is_open()) {
        try {
            peer_port = std::to_string(netSocket.remote_endpoint().port()) ;
            peer_address = netSocket.remote_endpoint().address().to_string() ;
        }
        catch(...) {
            // Unable to do it, most likey not connected
        }
    }
}

//======================================================================
auto Connection::peer() const -> std::string {
    return peer_address + ":"s + peer_port ;
}

//======================================================================
auto Connection::timestamp() -> void {
    connectTime = util::ourclock::now() ;
}

//======================================================================
auto Connection::stampedTime() const -> std::string {
    static const auto FORMATSTRING = "%b %d %H:%M:%S"s ;
    return util::sysTimeToString(connectTime,FORMATSTRING) ;
}

//======================================================================
auto Connection::send(const Packet &packet) -> bool {
    asio::error_code ec ;
    if (!netSocket.is_open()) {
        return false ;
    }
    try {
        auto amount = asio::write(this->netSocket,asio::buffer(packet.data.data(),packet.size()),ec)  ;
        if (ec) {
            DBGMSG(std::cerr, "Write failed: "s + ec.message()) ;
            return false ;
        }
        auto status = amount == packet.size() ;
        if (status) {
            lastWrite = util::ourclock::now() ;
        }
        return  status ;
        
    }
    catch(...) {
        return false ;
    }
}

//======================================================================
auto Connection::read() -> void {
    if (netSocket.is_open()) {
        incomingPacket = Packet(PacketType::UNKNOWN,Packet::PACKETHEADERSIZE) ;
        this->read(Packet::PACKETHEADERSIZE,0) ;
    }
}

//======================================================================
auto Connection::setPacketRoutine(PacketProcessing function) -> void {
    processingCallback = function ;
}

//======================================================================
auto  Connection::setCloseCallback(CloseCallback function) -> void {
    closeCallback = function ;
}

//======================================================================
auto Connection::readExpired(int seconds) -> bool {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now()-lastRead).count() ;
    return elapsed > seconds ;
}

//======================================================================
auto Connection::writeExpired(int seconds) -> bool {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now()-lastWrite).count() ;
    return elapsed > seconds ;
}


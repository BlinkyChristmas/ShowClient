// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Connection.hpp"

#include <fstream>

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto Connection::read(int amount, int offset) -> void {
    if (netSocket.is_open()) {
        incomingAmount = amount ;
        asio::async_read(this->netSocket,asio::buffer(incomingPacket->data.data()+offset,amount),std::bind(&Connection::readHandler,this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
}

//======================================================================
auto Connection::readHandler(const asio::error_code& ec, size_t bytes_transferred) -> void {
    if (ec) {
        //DBGMSG(std::cerr, "Error on read: "s + ec.message());
        try {
            this->timestamp() ; // mark the event
            // We got an error, so we wont requeue our read
            if (closeCallback != nullptr) {
                 closeCallback(this->shared_from_this()) ;
            }
            if (netSocket.is_open()) {
                netSocket.close();
            }
        }
        catch(...){}
        
        return ;
        
    }
    if (bytes_transferred != incomingAmount) {
        // We should have gotten more?
        auto amount = incomingAmount - static_cast<int>(bytes_transferred) ;
        this->read( amount , incomingPacket->size() - amount ) ;
        return ;
    }
    if (incomingPacket->length() != incomingPacket->size()) {
        // We need more data!
        auto amount = incomingPacket->length() - incomingPacket->size() ;
        incomingPacket->resize(incomingPacket->length()) ;
        this->read(amount,incomingPacket->size() - amount) ;
        return ;
    }
    // We got what we need
    lastRead = util::ourclock::now() ;
    
    //
    auto status = true ;
    if (processingCallback != nullptr) {
        status = processingCallback(incomingPacket,this->shared_from_this()) ;
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

// =====================================================================
Connection::Connection(asio::io_context &context):netSocket(context), incomingAmount(0), processingCallback(nullptr), closeCallback(nullptr), connectTime(util::ourclock::now()), lastRead(util::ourclock::now()), lastWrite(util::ourclock::now()) {
    
}

// =====================================================================
Connection::~Connection(){
    this->close();
}

// ===========================================================================================
auto Connection::socket() -> asio::ip::tcp::socket& {
    return netSocket ;
}

// ===========================================================================================
auto Connection::close( ) -> void {
    try {
        if (netSocket.is_open()) {
            netSocket.close() ;
        }
    }
    catch(...) {}
}

//======================================================================
auto Connection::is_open() const -> bool{
    return netSocket.is_open() ;
}

//======================================================================
auto Connection::read() -> void {
    if (netSocket.is_open()) {
        incomingPacket = std::make_shared<Packet>(PacketType::UNKNOWN,Packet::PACKETHEADERSIZE) ;
        this->read(Packet::PACKETHEADERSIZE,0) ;
    }
}

//======================================================================
auto Connection::log(std::ostream &output, bool state) -> void {
    // the format is: name = timestamp , state(Connected/Disconnected) , ipaddress
    static auto const format = "%s = %s , %s , %s"s ;
    auto time = this->connectTime ;
    if (!state && netSocket.is_open()) {
        time = util::ourclock::now() ;
    }
    output << util::format(format, this->handle.c_str() , util::sysTimeToString(time).c_str() ,  (state?"Connected":"Disconnected") , this->peer().c_str()) << std::endl;
    DBGMSG(std::cout, util::format(format, this->handle.c_str() , util::sysTimeToString(time).c_str() ,  (state?"Connected":"Disconnected") , this->peer().c_str()));
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
auto Connection::clearPeer() -> void {
    peer_port = "";
    peer_address = "";
}

//======================================================================
auto Connection::timestamp() -> void {
    connectTime = util::ourclock::now() ;
}

//======================================================================
auto Connection::time() const -> util::ourclock::time_point {
    return connectTime ;
}
//======================================================================
auto Connection::stampedTime() const -> std::string {
    static const auto FORMATSTRING = "%b %d %H:%M:%S"s ;
    return util::sysTimeToString(connectTime,FORMATSTRING) ;
}

//======================================================================
auto Connection::connect(asio::ip::tcp::endpoint &endpoint) -> bool {
    asio::error_code ec ;
    if (!netSocket.is_open()) {
        DBGMSG(std::cerr, "Can not connect, socket was not open.");
        return false ;
    }
    netSocket.connect(endpoint,ec) ;
    if (ec) {
        // we got an error
        DBGMSG(std::cerr, "Unable to connect: "s + ec.message()) ;
        if (netSocket.is_open()) {
            netSocket.close() ;
        }
        return false ;
    }
    // We need to get our peer information ;
    this->setPeer() ;
    this->timestamp() ;
    return true ;
}

//======================================================================
auto Connection::open(std::uint16_t port) -> bool {
    asio::error_code ec ;
    try {
        if (!this->open() ) {
            return false ;
        }
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
        if (netSocket.is_open()) {
            netSocket.close();
        }
        DBGMSG(std::cerr, "Exception in opening socket with port: "s + std::to_string(port));
        return false ;
    }
}
//======================================================================
auto Connection::open() -> bool {
    asio::error_code ec ;
    if (netSocket.is_open()) {
        netSocket.close();
    }
    try {
        netSocket.open(asio::ip::tcp::v4(),ec) ;
        if (ec) {
            DBGMSG(std::cerr,"Open socket failed: "s + ec.message());
            return false ;
        }
        netSocket.set_option(asio::socket_base::keep_alive(true)) ;
        return true ;
    }
    catch(...) {
        if (netSocket.is_open()) {
            netSocket.close() ;
        }
        return false ;
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
auto Connection::clearReadTime() -> void {
    lastRead = util::ourclock::now() ;
}

//======================================================================
auto Connection::writeExpired(int seconds) -> bool {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now()-lastWrite).count() ;
    return elapsed > seconds ;
}

//======================================================================
auto Connection::clearWriteTime() -> void {
    lastWrite = util::ourclock::now() ;
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
            //DBGMSG(std::cerr, util::sysTimeToString(util::ourclock::now())+": "s + "Write failed: "s + ec.message()) ;
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

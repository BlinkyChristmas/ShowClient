// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Client.hpp"

#include <algorithm>

#include "utility/dbgutil.hpp"
#include "packets/allpackets.hpp"

using namespace std::string_literals;

// =======================================================================
auto Client::runConnection() -> void {
    client_context.run();
}

// =======================================================================
auto Client::closeCallback(ConnectionPointer conn) -> void {
    // anything we need to do ?
    DBGMSG(std::cout, "Disconnected from: "s + this->ip()) ;
    try {
        if (stopCallback!=nullptr){
            stopCallback(this->shared_from_this());
        }
        connection->shutdown() ;
    }
    catch(...){}
}
// =======================================================================
auto Client::processCallback(std::shared_ptr<Packet> packet , ConnectionPointer conn) -> bool {
    auto id = packet->packetID() ;
    auto iter = std::find_if(packetRoutines.begin(),packetRoutines.end(),[id](const std::pair<PacketType::PacketID, PacketFunction> &entry){
        return id == entry.first ;
    });
    if (iter != packetRoutines.end()) {
        // We found one!
        return iter->second(this->shared_from_this(),packet);
    }
    return true ;
}


// =======================================================================
Client::Client(const std::string & name, const PacketRoutines &routines):stopCallback(nullptr),connectBeforeRead(nullptr) {
    connection = std::make_shared<Connection>(client_context) ;
    connection->setCloseCallback(std::bind(&Client::closeCallback,this,std::placeholders::_1));
    connection->setPacketRoutine(std::bind(&Client::processCallback,this,std::placeholders::_1,std::placeholders::_2));
    connection->handle = name ;
    packetRoutines = routines ;
    connectThread = std::thread(&Client::runConnection,this) ;
}

// =======================================================================
Client::~Client() {
    if (connection->is_open()) {
        // clear out the callbacks
        connection->setCloseCallback(nullptr);
        connection->setPacketRoutine(nullptr);
        connection->close() ;
    }
    if (!client_context.stopped()) {
        client_context.stop() ;
    }
    if (connectThread.joinable()){
        connectThread.join();
    }
}

// =======================================================================
auto Client::send(const Packet &packet) -> bool {
    if (connection == nullptr || !connection->is_open()){
        return false ;
    }
    return connection->send(packet) ;
}

// =======================================================================
auto Client::is_open() const -> bool {
    if (connection == nullptr) {
        return false ;
    }
    return connection->is_open() ;
}

// =======================================================================
auto Client::close() -> void {
    if (connection != nullptr) {
        connection->shutdown() ;
        connection->close() ;
    }
    
   
}

// =======================================================================
auto Client::connect(const std::string &ip, std::uint16_t port, std::uint16_t bindport) -> bool {
    
    // First, can we resolve the server
    auto endpoint = Connection::resolve(ip, port) ;
    if (endpoint == asio::ip::tcp::endpoint()) {
        DBGMSG(std::cerr, "Unable to resolve: "s + ip + " : "s + std::to_string(port)) ;
        return false ;
    }
    if (connection == nullptr) {
        return false ;
    }
    if (!connection->is_open()) {
        // Why are we here?  we should not be open
        connection->close() ;
    }
    if (!connection->open(bindport) ) {
        return false ;
    }
    if (connection->connect(endpoint) ) {
        // We connected, what should we do?
        connection->clearReadTime();
        connection->clearWriteTime() ;

        auto packet = IdentPacket(connection->handle) ;
        connection->send(packet) ;
        if (connectBeforeRead != nullptr){
            connectBeforeRead(this->shared_from_this());
        }
        connection->read() ;
        return true ;
    }
    return false ;
}
// =======================================================================
auto Client::ip() const -> std::string {
    return  connection->peer() ;
}
// =======================================================================
auto Client::timeStamp() -> util::ourclock::time_point {
    return connection->time() ;
}

// =======================================================================
auto Client::handle() const -> const std::string& {
    return connection->handle ;
}

// =======================================================================
auto Client::expire(int seconds) -> bool {
    return connection->readExpired(seconds);
}

// =======================================================================
auto Client::clearReadTime() -> void {
    connection->clearReadTime() ;
}

// =======================================================================
auto Client::clearWriteTime() -> void {
    connection->clearWriteTime() ;
}
// =======================================================================
auto Client::setStopCallback(ClientStop function) -> void {
    stopCallback = function ;
}

// =======================================================================
auto Client::setConnectdBeforeRead(ConnectBeforeRead function) -> void {
    connectBeforeRead = function ;
}


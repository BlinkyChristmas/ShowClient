//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Connection_hpp
#define Connection_hpp

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#include "utility/timeutil.hpp"
#include "network/packets/Packet.hpp"

#include "asio/asio.hpp"

//======================================================================
class Connection {
    
public:
    // Define a few callback types
    using PacketProcessing = std::function<bool(Packet,Connection *)> ;
    using CloseCallback = std::function<void(Connection *)> ;

private:
    asio::ip::tcp::socket netSocket ;
    
    util::ourclock::time_point connectTime ;
    std::string peer_address ;
    std::string peer_port ;
    
    util::ourclock::time_point lastRead ;
    util::ourclock::time_point lastWrite ;
    
    Packet incomingPacket ;
    int incomingAmount ;
 
    PacketProcessing processingCallback ;
    CloseCallback closeCallback ;

    auto open() -> bool ;
    
    auto read(int amount, int offset) -> void ;
    auto readHandler(const asio::error_code& ec, size_t bytes_transferred) -> void ;
    
public:
    
    static auto resolve(const std::string &ipaddress, std::uint16_t port) -> asio::ip::tcp::endpoint ;
    
    Connection(asio::io_context &io_context) ;
    ~Connection() ;
    
    auto socket() -> asio::ip::tcp::socket& ;
    
    auto is_open() const -> bool ;
    auto close() -> void ;
    
    auto open(std::uint16_t port) -> bool ;
    auto connect(asio::ip::tcp::endpoint &endpoint) -> bool ;
    
    auto setPeer() -> void ;
    auto peer() const -> std::string ;
    
    auto timestamp() -> void ;
    auto stampedTime() const -> std::string ;

    auto send(const Packet &packet) -> bool ;

    auto read() -> void ;
    
    auto setPacketRoutine(PacketProcessing function) -> void ;
    auto setCloseCallback(CloseCallback function) -> void ;
    
    auto readExpired(int seconds) -> bool ;
    auto writeExpired(int seconds) -> bool ;
};

#endif /* Connection_hpp */

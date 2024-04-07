// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Connection_hpp
#define Connection_hpp

#include <functional>
#include <memory>
#include <string>
#include <ostream>
#include <mutex>

#include "asio.hpp"

#include "packets/Packet.hpp"
#include "utility/timeutil.hpp"

class Connection;

using ConnectionPointer = std::shared_ptr<Connection> ;
using PacketPointer = std::shared_ptr<Packet> ;

class Connection: public std::enable_shared_from_this<Connection> {
public:
    using PacketProcessing = std::function<bool(PacketPointer,ConnectionPointer)> ;
    using CloseCallback = std::function<void(ConnectionPointer)>;
private:
    asio::ip::tcp::socket netSocket ;
    
    util::ourclock::time_point connectTime ;
    util::ourclock::time_point lastRead ;
    util::ourclock::time_point lastWrite ;

    std::string peer_address ;
    std::string peer_port ;
    
    PacketPointer incomingPacket ;
    int incomingAmount ;

    auto read(int amount, int offset) -> void ;
    auto readHandler(const asio::error_code& ec, size_t bytes_transferred) -> void ;
    //mutable std::recursive_mutex read_access;

    PacketProcessing processingCallback ;
    CloseCallback closeCallback ;

    
public:
    static auto resolve(const std::string &ipaddress, std::uint16_t port) -> asio::ip::tcp::endpoint ;

    std::string handle ;
    Connection(asio::io_context &context);
    ~Connection() ;
    
    auto socket() -> asio::ip::tcp::socket& ;
    auto close() -> void ;
    auto is_open() const -> bool;
    auto read() -> void ;
    
    auto log(std::ostream &output, bool state) -> void ;
    
    auto setPeer() -> void ;
    auto peer() const -> std::string;
    auto clearPeer() -> void ;
    auto timestamp() -> void  ;
    auto time() const -> util::ourclock::time_point;
    auto stampedTime() const -> std::string ;
    
    auto connect(asio::ip::tcp::endpoint &endpoint) -> bool ;
    auto open(std::uint16_t port) -> bool ;
    auto open() -> bool ;
    
    auto setPacketRoutine(PacketProcessing function) -> void ;
    auto setCloseCallback(CloseCallback function) -> void;
    auto readExpired(int seconds) -> bool ;
    auto writeExpired(int seconds) -> bool ;
    auto clearReadTime() -> void ;
    auto clearWriteTime() -> void ;

    auto send(const Packet &packet) -> bool ;
    
    auto shutdown() -> void ;
};



#endif /* Connection_hpp */

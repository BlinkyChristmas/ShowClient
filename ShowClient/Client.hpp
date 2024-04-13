// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Client_hpp
#define Client_hpp

#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>

#include "network/Connection.hpp"
#include "packets/Packet.hpp"

#include "asio.hpp"

class Client ;

using ClientPointer = std::shared_ptr<Client> ;
using ClientStop = std::function<void(ClientPointer)>;
using PacketFunction = std::function<bool(ClientPointer,PacketPointer)> ;
using PacketRoutines = std::unordered_map<PacketType::PacketID, PacketFunction> ;
using ConnectBeforeRead = std::function<void(ClientPointer)> ;
class Client : public std::enable_shared_from_this<Client> {
    friend class Connection ;
    
    asio::io_context client_context ;
    asio::executor_work_guard<asio::io_context::executor_type> clientguard{asio::make_work_guard(client_context)} ;
    
    ConnectionPointer connection ;
    
    std::thread connectThread ;
    auto runConnection() -> void ;
    PacketRoutines packetRoutines ;
    ClientStop stopCallback ;
    ConnectBeforeRead connectBeforeRead;
    auto closeCallback(ConnectionPointer conn) -> void ;
    auto processCallback(PacketPointer packet , ConnectionPointer conn) -> bool ;
    
public:
    Client(const std::string &name, const PacketRoutines &routines ) ;
    ~Client() ;
    
    auto send(const Packet &packet) -> bool ;
    auto is_open() const -> bool ;
    auto close() -> void ;

    auto connect(const std::string &ip, std::uint16_t port, std::uint16_t bindport) -> bool ;
    auto ip() const -> std::string ;
    auto timeStamp() -> util::ourclock::time_point ;
    auto handle() const -> const std::string& ;
    auto expire(int seconds) -> bool ;
    auto clearReadTime() -> void ;
    auto clearWriteTime() -> void ;
    auto setStopCallback(ClientStop function) -> void ;
    auto setConnectdBeforeRead(ConnectBeforeRead function) -> void ;
    auto shutdown() ->void ;
};
#endif /* Client_hpp */

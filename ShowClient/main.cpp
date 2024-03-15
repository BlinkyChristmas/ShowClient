//Copyright © 2023 Charles Kerr. All rights reserved.

#include <atomic>
#include <cstdlib>
#include <chrono>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>

#include "asio/asio.hpp"

#include "network/Connection.hpp"
#include "network/Timer.hpp"
#include "network/packets/allpackets.hpp"
#include "utility/dbgutil.hpp"

#include "Configuration.hpp"
#include "StatusController.hpp"
#include "LightController.hpp"
#include "MusicController.hpp"

using namespace std::string_literals ;

// Our configuration
Configuration config ;

// All our connection related items
asio::io_context client_context ;
asio::executor_work_guard<asio::io_context::executor_type> clientguard{asio::make_work_guard(client_context)} ;
std::unique_ptr<Connection> connection = nullptr ;
std::thread clientThread ;

std::string musicName ;
std::string lightName ;

// Our timer
std::unique_ptr<Timer> timer  = nullptr ;
asio::io_context timer_context ;
asio::executor_work_guard<asio::io_context::executor_type> timerguard{asio::make_work_guard(timer_context)} ;
std::thread timerThread ;

// Our "client" type items
LightController lightController ;
MusicController musicController ;
StatusController statusController ;

std::atomic<int> currentFrame ;
std::atomic<int> totalFrame ;

std::atomic<bool> timerThreadRunning = false ;
std::atomic<bool> clientThreadRunning = false ;

std::mutex lightAccess ;
// packet related items
// our two states
LEDInfo::ledstate audioState = LEDInfo::ledstate::OFF;
LEDInfo::ledstate playState = LEDInfo::ledstate::OFF;

// =================================================================================
// Forward Declares
// =================================================================================
auto timerUpdate(Timer *timer) -> void ;
auto processPacket(Packet packet, Connection *conn) -> bool ;
auto processingLoop( const Configuration &config) -> void ;
auto play(bool state,int frame,bool sendstatus =true) -> void ;
auto closeCallback(Connection *conn) -> void ;
//====================================================================
// Context routines
// ==================================================================

// ==================================================================
auto runConnection() -> void {
    //std::cout <<"Rseting client_context"<<std::endl;
    client_context.reset() ;
    clientThreadRunning = true ;
    //std::cout <<"run client_context"<<std::endl;
    client_context.run() ;
    //std::cout <<"exit client_context"<<std::endl;
    clientThreadRunning = false ;
}
// ==================================================================
auto runTimer() -> void {
    //std::cout <<"Rseting timer_context"<<std::endl;
    timer_context.reset() ;
    //std::cout <<"run timer_context"<<std::endl;
    timerThreadRunning = true ;
    timer_context.run() ;
    timerThreadRunning = false ;
    //std::cout <<"exit timer_context"<<std::endl;
}

// ================================================================
auto stopContext() -> void {
    if (timer != nullptr) {
        try {
            timer->stop() ;
        }
        catch(...){}
    }
    if (connection != nullptr) {
        // First we want to reset all callbacks
        connection->setCloseCallback(nullptr) ;
        connection->setPacketRoutine(nullptr) ;
        if (connection->is_open()) {
            connection->close() ;
        }
    }
    
    // Ok, now we want to stop our context
    if (!timer_context.stopped()) {
        timer_context.stop() ;
    }
    if (!client_context.stopped()) {
        client_context.stop() ;
    }
    // Now we want to wait on our threads
    if (timerThread.joinable()) {
        if (timerThreadRunning) {
            // This should not be!!
            if (!timer_context.stopped()) {
                timer_context.stop();
            }
        }
        timerThread.join() ;
        timerThread = std::thread() ;
    }
    if (clientThread.joinable()) {
        if (clientThreadRunning) {
            // This should not be!!
            if (!client_context.stopped()) {
                client_context.stop();
            }
        }
        clientThread.join() ;
        clientThread = std::thread() ;
    }
    // Now, clear out our timer/connection
    timer = nullptr ;
    connection = nullptr ;
    //std::cout << "Leaving stop context" << std::endl;
}
/*
 This should be removed, it can creae a race condition!
 */
/*
// ==================================================================
auto stopContext() -> void {
    //std::cout <<"Stop Timer" << std::endl;
    if (timer != nullptr) {
        timer->stop() ;
        timer = nullptr ;
    }
    //std::cout <<"Closing  Connection" << std::endl;
    if (connection != nullptr) {
        if (connection->is_open()) {
            try {
                connection->close() ;
            }
            catch(...){}
        }
        connection = nullptr ;
    }
   //std::cout <<"Stopping timer  Context" << std::endl;
    if (!timer_context.stopped()){
        //std::cout <<"Stopping timer  Context for real" << std::endl;
        timer_context.stop() ;
    }
    
    //std::cout <<"Stopping client  Context" << std::endl;
    if (!client_context.stopped()){
        //std::cout <<"Stopping client  Context for real" << std::endl;
       
        client_context.stop() ;
    }
    //std::cout << "Waiting on timer in case " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));  // Do we need this?
    //std::cout <<"Waiting on timer thread " << std::endl;
    if (timerThread.joinable()) {
        
        //std::cout <<"Waiting on timer thread for real, and thread is running: " << std::to_string(timerThreadRunning) << std::endl;
        if (timerThreadRunning) {
            // Why is our context still runing?
            try {
                //std::cout << "stopping timer context again! 1 " << std::endl;
                timer_context.stop() ;
            }
            catch(...) {}
        }
        //std::cout <<"Waiting on timer thread for real (after force attemp), and thread is running: " << std::to_string(timerThreadRunning) << std::endl;
        timerThread.join() ;
        timerThread = std::thread() ;
    }
    //std::cout <<"Waiting on client thread " << std::endl;
    if (clientThread.joinable()) {
        //std::cout <<"Waiting on client thread for real, and thread is running: " <<  std::to_string(clientThreadRunning) << std::endl;
        if (clientThreadRunning) {
            // Why is our context still runing?
            try {
                //std::cout << "stopping client context again! 2 " << std::endl;
                client_context.stop() ;
            }
            catch(...) {}
        }
        //std::cout <<"Waiting on client thread (after force attempt) for real, and thread is running: " <<  std::to_string(clientThreadRunning) << std::endl;
        clientThread.join() ;
        clientThread = std::thread() ;
    }
    //std::cout <<"Leaving stop context " << std::endl;
    
}
*/
// =======================================================================
auto startContext() -> void {
    timerThread = std::thread(&runTimer) ;
    timer = std::make_unique<Timer>(timer_context) ;
    timer->setUpdateCallback(&timerUpdate) ;
    
    clientThread = std::thread(&runConnection) ;
    connection = std::make_unique<Connection>(client_context) ;
    connection->setPacketRoutine(&processPacket) ;
    //connection->setCloseCallback(&closeCallback) ;
    
}


// ==================================================================
auto timerUpdate(Timer *timer) -> void {
    currentFrame += 1 ;
    if (currentFrame >= totalFrame) {
        // We do something
        play(false,0) ;
    }
    else {
        if (audioState == LEDInfo::ON) {
            musicController.sync(currentFrame);
        }
        if (config.useLight) {
            //DBGMSG(std::cout,  "Sync lights with: "s + std::to_string(currentFrame)) ;
            auto lock = std::lock_guard(lightAccess) ;
            lightController.sync(currentFrame);
        }
    }
}


//=================================================================
// Action routines
//=================================================================
constexpr auto IGNORESPAN = 3 ;
constexpr auto NUDGESPAN = 8 ;
//=================================================================
auto sync(int frame) -> void {
    // WE just modify currentFrame
    if (std::abs(frame - currentFrame) >= IGNORESPAN) {
        if (std::abs(frame - currentFrame) >= NUDGESPAN) {
            currentFrame = frame ;
        }
        else {
            
            if (frame > currentFrame) {
                currentFrame += 1 ;
            }
            else {
                currentFrame -= 1 ;
            }
        }
    }
    
}
//=================================================================
auto play(bool state,int frame,bool sendstatus  ) -> void {
    if (state) {
        //DBGMSG(std::cout, "Play: ON Frame: "s+std::to_string(frame));
        auto status = LEDInfo::ON ;
        // We need to do a few things
        if ( (musicController.frameCount()>0) || (lightController.frameCount()>0)){
            // We actually need to do something
            currentFrame = frame ;
            if (config.useLight && lightController.frameCount()>0) {
                auto lock = std::lock_guard(lightAccess) ;
                if(!lightController.start(frame)) {
                    DBGMSG(std::cout, "Start failed on lightController for frame: "s + std::to_string(frame)) ;
                    status = LEDInfo::FLASH ;
                }
            }
            if (audioState == LEDInfo::ON && musicController.frameCount() > 0) {
                if (!musicController.start(frame,true)) {
                    status = LEDInfo::FLASH ;
                }
            }
            timer->start() ;
        }
        
        if (status == LEDInfo::FLASH && sendstatus) {
            auto packet = ErrorPacket() ;
            packet.setCategory(ErrorPacket::PLAY);
            packet.setName(musicName);
            connection->send(packet) ;
        }
        statusController.setState(LEDInfo::PLAY, status,false) ;
    }
    else {
        
        timer->stop() ;
        if (audioState == LEDInfo::ON) {
            musicController.stop() ;
            
        }
        if (config.useLight) {
            auto lock = std::lock_guard(lightAccess) ;
            lightController.stop() ;
        }
        if (audioState != LEDInfo::FLASH) {
            statusController.setState(LEDInfo::PLAY, LEDInfo::OFF,false) ;
        }
        else {
            statusController.setState(LEDInfo::PLAY, LEDInfo::FLASH,false) ;
        }
        currentFrame = 0 ;
        totalFrame = 0 ;
    }
}
//=================================================================
auto load(const std::string &music, const std::string light) -> void {
    if (audioState == LEDInfo::ON) {
        //DBGMSG(std::cout, "Load: "s + music);
        musicName = music ;
        if (!musicController.load( config.musicPath / std::filesystem::path(music + config.musicExtension))){
            // Should we do something?  Play will pick it up
        }
    }
    if (config.useLight) {
        lightName = light ;
        auto lock = std::lock_guard(lightAccess) ;
        if (!lightController.load( config.lightPath / std::filesystem::path(light + config.lightExtension))){
            // Should we do something?  Play will pick it up
        }
    }
    currentFrame = 0 ;
    totalFrame = std::max(musicController.frameCount() , lightController.frameCount()) ;
}

//=================================================================
auto closeCallback(Connection *conn) -> void {
    play(false,0,false) ;
    statusController.setState(LEDInfo::SHOW, LEDInfo::OFF, false) ;
    statusController.setState(LEDInfo::CONNECT, LEDInfo::FLASH, false) ;
    DBGMSG(std::cout, "Connection was closed") ;
    
    
}
//=================================================================
auto processPacket(Packet packet,Connection *conn) -> bool {
    switch (packet.packetID()) {
        case PacketType::SYNC:{
            auto ptr = static_cast<const SyncPacket*>(&packet) ;
            sync(ptr->syncFrame()) ;
            break;
        }
            
        case PacketType::LOAD: {
            auto ptr = static_cast<const LoadPacket*>(&packet) ;
            load(ptr->musicName(),ptr->lightName());
            break;
        }
            
        case PacketType::NOP: {
            auto ptr = static_cast<const NopPacket*>(&packet) ;
            if ( ptr->respond()) {
                auto nop = NopPacket() ;
                connection->send(nop) ;
            }
            break;
        }
            
        case PacketType::PLAY: {
            auto ptr = static_cast<const PlayPacket*>(&packet) ;
            play(ptr->state(),ptr->frame()) ;
            break;
        }
        case PacketType::SHOW: {
            auto ptr = static_cast<const ShowPacket*>(&packet) ;
            if (ptr->state()) {
                statusController.setState(LEDInfo::SHOW, LEDInfo::ON, false);
            }
            else {
                statusController.setState(LEDInfo::SHOW, LEDInfo::OFF, false);
            }
            break;
        }
        default:
            break ;
    }
    return true ;
}



// Our logic loop

//=================================================================================
int main(int argc, const char * argv[]) {
    auto exitcode = EXIT_SUCCESS ;
    try {
        
        if (argc < 2) {
            throw std::runtime_error("Configuration file not specified");
        }
        if (!config.load( std::filesystem::path(argv[1]) ) ) {
            throw std::runtime_error("Unable to load: "s + std::string(argv[1]));
        }
        if (config.useAudio) {
            audioState = LEDInfo::ON ;
        }
       // DBGMSG(std::cout, "Light setting is: "s + std::to_string(config.useLight)) ;
        if (config.useLight) {
            //DBGMSG(std::cout, "Configuring PRU") ;
            auto lock = std::lock_guard(lightAccess) ;
            lightController.configurePRU(config.pruSetting[0], config.pruSetting[1]) ;
        }
        statusController.setAll(LEDInfo::OFF, true);
        
        processingLoop(config) ;
        
        
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    catch (...) {
        std::cerr << "Unknown error!" << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    try {
        if (!client_context.stopped()) {
            client_context.stop() ;
        }
        if (clientThread.joinable()) {
            clientThread.join() ;
        }
    }
    catch(...) {
        exitcode = EXIT_FAILURE;
    }
    if (exitcode == EXIT_FAILURE) {
        statusController.setAll(LEDInfo::FLASH, true);
    }
    stopContext() ;
    
    return exitcode;
}

//=================================================================================
auto processingLoop( const Configuration &config) -> void {
    
    while(config.runSpan.inRange()) {
        statusController.setState(LEDInfo::RUN, LEDInfo::ON,false) ;
        if (config.connectTime.inRange()) {
            if (connection == nullptr) {
                startContext() ;
                statusController.setState(LEDInfo::CONNECT, LEDInfo::FLASH,false) ;
            }
            else {
                if (!connection->is_open()) {
                    // First, check our states?
                    play(false,0,false) ;;
                    statusController.setState(LEDInfo::SHOW, LEDInfo::OFF,false) ;
                    statusController.setState(LEDInfo::CONNECT, LEDInfo::FLASH,false) ;
                    if (connection->open(config.clientPort)) {
                        auto endpoint = Connection::resolve(config.serverIP,config.serverPort) ;
                        if (endpoint != asio::ip::tcp::endpoint()) {
                            DBGMSG(std::cout,"Attempting to connect");
                            if (connection->connect(endpoint)) {
                                DBGMSG(std::cout, "Connect SUCCESS");
                                statusController.setState(LEDInfo::CONNECT, LEDInfo::ON, false) ;
                                connection->setPeer() ;
                                connection->timestamp() ;
                                auto packet = IdentPacket() ;
                                packet.setHandle(config.name) ;
                                connection->send(packet) ;

                                auto audiopacket = ErrorPacket() ;
                                auto senderror = false ;
                                audioState = (config.useAudio ? LEDInfo::ON : LEDInfo::OFF) ;
                                if (config.useAudio && !musicController.initialize(config.audioDevice, 441000)) {
                                    
                                    audiopacket.setCategory(ErrorPacket::AUDIO) ;
                                    senderror = true ;
                                }
                                musicController.close() ;
                                if (senderror) {
                                    DBGMSG(std::cout, "Sending error for music not present");
                                    connection->send(audiopacket) ;
                                    senderror = false ;
                                }
                                connection->read() ;
                                
                            }
                            else {
                                DBGMSG(std::cout, "Connect unsuccessful");
                                stopContext() ;
                                std::this_thread::sleep_for(std::chrono::seconds(20)) ;
                            }
                        }
                    }
                    
                }
                else {
                    if (connection->readExpired(80) && connection->writeExpired(80)){
                        try {
                            connection->socket().cancel() ;
                        }
                        catch(...) {
                            
                        }
                        DBGMSG(std::cout, "We havent received anything, stopping context");
                        statusController.setState(LEDInfo::CONNECT, LEDInfo::FLASH,false) ;
                        stopContext() ;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(20)) ;
        }
        else {
            statusController.setState(LEDInfo::CONNECT, LEDInfo::OFF,false) ;
            if (connection != nullptr) {
                stopContext() ;
            }
            std::this_thread::sleep_for(std::chrono::minutes(1)) ;
        }
    }
}


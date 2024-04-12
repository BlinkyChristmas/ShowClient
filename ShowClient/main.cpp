//
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <thread>
#include <functional>

#include "packets/allpackets.hpp"
#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"

#include "ClientConfiguration.hpp"
#include "StatusController.hpp"
#include "MusicController.hpp"
#include "LightController.hpp"
#include "Client.hpp"

using namespace std::string_literals ;

auto runLoop(ClientConfiguration &config) -> bool ;

StatusController ledController ;

int main(int argc, const char * argv[]) {
    ClientConfiguration configuration ;
    auto exitcode = EXIT_SUCCESS ;
    try {
        ledController.clear() ;
        ledController.setState(StatusLed::RUN, LedState::FLASH);
        if (argc < 2) { throw std::runtime_error("Missing configuration file!");}
        if (!configuration.load(std::filesystem::path(argv[1]))){
            throw std::runtime_error("Unable to process: "s + argv[1]);
        }
        // We are now have our configuration file, lets start our run loop
        ledController.setState(StatusLed::RUN, LedState::OFF);
        if(!runLoop(configuration)) {
            throw std::runtime_error("Error occurred while running!") ;
        }
        
    }
    catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    catch(...) {
        std::cerr << "Unknown error!" << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    if (exitcode == EXIT_FAILURE) {
        ledController.flash();
        
    }
    else {
        ledController.clear();
    }
    return exitcode;
}


// =============================================================================================================
// Our main runloop
// ============================================================================================================

auto initialConnect(ClientPointer client) -> void ;

auto musicError(MusicPointer music) -> void ;

auto processLoad(ClientPointer connection,PacketPointer packet) -> bool;
auto processSync(ClientPointer connection,PacketPointer packet) -> bool;
auto processPlay(ClientPointer connection,PacketPointer packet) -> bool;
auto processShow(ClientPointer connection,PacketPointer packet) -> bool;
auto processNop(ClientPointer connection,PacketPointer packet) -> bool;
auto processBuffer(ClientPointer connection,PacketPointer packet) -> bool;
auto stopCallback(ClientPointer client) -> void ;

MusicController musicController ;
LightController lightController ;

std::shared_ptr<Client> client  = nullptr ;
// ====================================================================
auto runLoop(ClientConfiguration &config) -> bool {
    ledController.clear() ;
    PacketRoutines routines ;
    routines.insert_or_assign(PacketType::LOAD,std::bind(&processLoad,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::SYNC,std::bind(&processSync,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::PLAY,std::bind(&processPlay,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::SHOW,std::bind(&processShow,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::NOP,std::bind(&processNop,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::BUFFER,std::bind(&processBuffer,std::placeholders::_1,std::placeholders::_2)) ;

    client = std::make_shared<Client>(config.name,routines) ;
    client->setStopCallback(std::bind(&stopCallback,std::placeholders::_1));
    client->setConnectdBeforeRead(std::bind(&initialConnect,std::placeholders::_1));
    musicController.setEnabled(config.useAudio) ;
    musicController.setDevice(config.audioDevice);
    musicController.setMusicInformation(config.musicPath, config.musicExtension);
    musicController.setMusicErrorCallback(std::bind(&musicError,std::placeholders::_1));
    lightController.setPRUInfo(config.pruSetting[0], config.pruSetting[1]) ;
    lightController.setEnabled(config.useLight) ;
    lightController.setLightInfo(config.lightPath, config.lightExtension);
    while (config.runSpan.inRange()) {
        ledController.setState(StatusLed::RUN, LedState::ON) ;
        try {
            if (config.refresh()) {
                // We shoud set anything we need to because the config file changed
                musicController.setEnabled(config.useAudio) ;
                musicController.setDevice(config.audioDevice);
                musicController.setMusicInformation(config.musicPath, config.musicExtension);
                lightController.setEnabled(config.useLight) ;
                lightController.setLightInfo(config.lightPath, config.lightExtension);

            }
        }
        catch(...) {
            // We had an error processing the config file, but we aren't going to worry about it, we did it initially, we will assume we can continue
        }
        if (config.connectTime.inRange()) {
            // We should be connected!
            if (!client->is_open()) {
                ledController.setState(StatusLed::CONNECT, LedState::FLASH) ;
                
                // we are not open!
                if (client->connect(config.serverIP, config.serverPort, config.clientPort)) {
                    // We connected!
                    DBGMSG(std::cout,"Connected to "s + config.serverIP+":"s+std::to_string(config.serverPort));
                    ledController.setState(StatusLed::CONNECT, LedState::ON) ;
                    // We need to setup everything!
                }
                else {
                    // We didn't connect, could because we are still holding onto the port we binded to,
                    // that takes about 40 to 60 seconds to release
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
            }
            if (client->is_open()){
                
                // this is where our action is
                
                if (client->expire(180)) {
                    // It has been three minutes since we got something, we probably should just disconnect
                    client->close() ;
                }
            }
        }
        else {
            // We should not be closed down
            if (client->is_open()){
                client->close() ;
                DBGMSG(std::cout,"Disconnecting from: "s + client->ip());
                
                ledController.setState(StatusLed::CONNECT, LedState::OFF) ;
            }
        }
        
        
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ledController.setState(StatusLed::RUN, LedState::OFF) ;
    
    if (client->is_open()){
        client->close();
    }
    client = nullptr ;
    return true ;
}

// ==============================================================================================
// Packet routines
// ==============================================================================================

// ==============================================================================================
auto processLoad(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<LoadPacket*>(packet.get()) ;
    
    auto music = payload->musicName() ;
    auto light = payload->lightName() ;
    DBGMSG(std::cout, util::format("Load: %s, %s",music.c_str(),light.c_str()));
    if (musicController.isEnabled()){
        musicController.load(music);
    }
    if (lightController.isEnabled()) {
        lightController.loadLight(light) ;
    }
    return true ;
}

// ==============================================================================================
auto processSync(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<SyncPacket*>(packet.get()) ;
    
    auto frame = payload->syncFrame() ;
    musicController.setSync(frame);
    lightController.setSync(frame);
    return true ;
}

// ==============================================================================================
auto processPlay(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<PlayPacket*>(packet.get()) ;
    auto state = payload->state() ;
    auto frame = payload->frame() ;
    
    ledController.setState(StatusLed::PLAY, (state?LedState::ON: LedState::OFF)) ;
    if (state) {
        if (musicController.isEnabled()){
            if (musicController.hasError()){
                auto packet = ErrorPacket(ErrorPacket::CatType::PLAY, musicController.currentLoaded());
                DBGMSG(std::cout, "Error on "s + musicController.currentLoaded());
                client->send(packet);
                ledController.setState(StatusLed::PLAY, LedState::FLASH) ;

            }
            else if (!musicController.start(frame)) {
                DBGMSG(std::cout, "Error on "s + musicController.currentLoaded());
                auto packet = ErrorPacket(ErrorPacket::CatType::PLAY, musicController.currentLoaded());
                client->send(packet);
                ledController.setState(StatusLed::PLAY, LedState::FLASH) ;

            }
        }
        if (lightController.isEnabled()){
            if (lightController.hasError()){
                auto packet = ErrorPacket(ErrorPacket::CatType::PLAY, lightController.currentLoaded());
                DBGMSG(std::cout, "Error on "s + lightController.currentLoaded());
                client->send(packet);
                ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
            }
            else if (!lightController.start(frame)) {
                
                
            }
        }
        
    }
    else {
        musicController.stop() ;
        lightController.stop();
    }
            
    return true ;
}

// ==============================================================================================
auto processShow(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<ShowPacket*>(packet.get()) ;
    auto state = payload->state() ;
    ledController.setState(StatusLed::SHOW, (state?LedState::ON: LedState::OFF)) ;
    return true ;
}

// ==============================================================================================
auto processNop(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<NopPacket*>(packet.get()) ;
    auto respond = payload->respond() ;
    if (respond) {
        connection->send(NopPacket()) ;
    }
    return true ;
}
// ==============================================================================================
auto processBuffer(ClientPointer connection,PacketPointer packet) -> bool{
    auto payload = static_cast<BufferPacket*>(packet.get()) ;
    auto length = payload->length()  - 8 ;
    auto data = std::vector<std::uint8_t>(length,0) ;
    std::copy(payload+8,payload+8+length,data.data()) ;
    lightController.loadBuffer(data);

}
// ================================================================================================
auto stopCallback(ClientPointer client) -> void {
    // We stopped, so we have some cleanup, but lets do a few things
    // We should turn of playing
    ledController.setState(StatusLed::PLAY, LedState::OFF) ;
    musicController.stop() ;
    // We should turn off show
    ledController.setState(StatusLed::SHOW, LedState::OFF) ;
}

// ================================================================================================
auto musicError(MusicPointer music) -> void {
    DBGMSG(std::cout, "Error on "s + musicController.currentLoaded());
    auto packet = ErrorPacket(ErrorPacket::CatType::PLAY, musicController.currentLoaded());
    client->send(packet) ;
    musicController.stop() ;
    ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
}

// ================================================================================================
auto initialConnect(ClientPointer client) -> void {
    if (!musicController.initialize(musicController.device()) ) {
        auto errorPacket = ErrorPacket(ErrorPacket::CatType::AUDIO,"") ;
        client->send(errorPacket);
        ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
    }
}

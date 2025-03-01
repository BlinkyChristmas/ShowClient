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
    musicController.setDataInformation(config.musicPath, config.musicExtension);
    musicController.setMusicErrorCallback(std::bind(&musicError,std::placeholders::_1));
    lightController.setPRUInfo(config.pruSetting[0], config.pruSetting[1]) ;
    lightController.setEnabled(config.useLight) ;
    lightController.clear() ;
    lightController.setDataInformation(config.lightPath, config.lightExtension);
    while (config.runSpan.inRange()) {
        ledController.setState(StatusLed::RUN, LedState::ON) ;
        try {
            if (config.refresh()) {
                // We shoud set anything we need to because the config file changed
                musicController.setEnabled(config.useAudio) ;
                musicController.setDevice(config.audioDevice);
                musicController.setDataInformation(config.musicPath, config.musicExtension);
                lightController.setEnabled(config.useLight) ;
                lightController.setDataInformation(config.lightPath, config.lightExtension);
                
            }
        }
        catch(...) {
            // We had an error processing the config file, but we aren't going to worry about it, we did it initially, we will assume we can continue
        }
        if (config.connectTime.inRange()) {
            // We should be connected!
            if (!client->is_open()) {
                
                ledController.setState(StatusLed::CONNECT, LedState::FLASH) ;
                musicController.stop() ;
                lightController.stop() ;
                lightController.clear() ;
                ledController.setState(StatusLed::PLAY, LedState::OFF) ;
                ledController.setState(StatusLed::SHOW, LedState::OFF) ;

            
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
                    
                    if (client->is_open()) {
                        client->shutdown() ;
                        
                        client->close() ;
                        musicController.stop();
                        lightController.stop();
                    }
                }
            }
        }
        else {
            // We should  be closed down
            if (client->is_open()){
                client->close() ;
                musicController.stop();
                lightController.stop() ;
                lightController.clear() ;
                ledController.setState(StatusLed::SHOW, LedState::OFF);
                ledController.setState(StatusLed::PLAY, LedState::OFF);
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

bool load_error = false ;

// ==============================================================================================
auto processLoad(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<LoadPacket*>(packet.get()) ;
    load_error = false ;
    ledController.setState(StatusLed::PLAY, LedState::OFF) ;
    
    auto music = payload->musicName() ;
    auto light = payload->lightName() ;
    //DBGMSG(std::cout, util::format("Load: %s, %s",music.c_str(),light.c_str()));
    if (musicController.isEnabled()){
        if (!musicController.load(music)) {
            load_error = true ;
            ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
            auto packet = ErrorPacket(ErrorPacket::CatType::AUDIO, music);
            client->send(packet);
            ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
        }
    }
    if (lightController.isEnabled()) {
        if (!lightController.load(light)) {
            load_error = true ;
            ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
            auto packet = ErrorPacket(ErrorPacket::CatType::LIGHT, light);
            client->send(packet);
            ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
        }
    }
    return true ;
}

// ==============================================================================================
auto processSync(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<SyncPacket*>(packet.get()) ;
    
    auto frame = payload->syncFrame() ;
    musicController.syncFrame(frame);
    lightController.syncFrame(frame);
    return true ;
}

// ==============================================================================================
auto processPlay(ClientPointer connection,PacketPointer packet) -> bool {
    auto payload = static_cast<PlayPacket*>(packet.get()) ;
    auto state = payload->state() ;
    auto frame = payload->frame() ;
    
    if (state) {
        auto got_play_error = false ;
        if (musicController.isEnabled()){
            if (musicController.isLoaded()){
                if (!musicController.start(frame)) {
                    DBGMSG(std::cout, "Error on "s + musicController.name());
                    auto packet = ErrorPacket(ErrorPacket::CatType::AUDIO, musicController.name());
                    client->send(packet);
                    ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
                    got_play_error = true ;
                }
            }
            else if (!load_error && musicController.hasError()) {
                DBGMSG(std::cout, "Error on "s + musicController.name());
                auto packet = ErrorPacket(ErrorPacket::CatType::AUDIO, musicController.name());
                client->send(packet);
                ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
                got_play_error = true ;

            }
        }
        if (lightController.isEnabled()){
            if (lightController.isLoaded()) {
                if (!lightController.start(frame)) {
                    auto packet = ErrorPacket(ErrorPacket::CatType::LIGHT, lightController.name());
                    client->send(packet);
                    ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
                    got_play_error = true ;
                }
            }
            else if (!load_error && lightController.hasError()) {
                DBGMSG(std::cout, "Error on "s + lightController.name());
                auto packet = ErrorPacket(ErrorPacket::CatType::LIGHT, lightController.name());
                client->send(packet);
                ledController.setState(StatusLed::PLAY, LedState::FLASH) ;
                got_play_error = true ;

            }
        }
        if (!got_play_error && !load_error) {
            ledController.setState(StatusLed::PLAY, LedState::ON) ;
        }
    }
    else {
        musicController.stop() ;
        lightController.stop();
        ledController.setState(StatusLed::PLAY, LedState::OFF) ;
        load_error = false;
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
    //DBGMSG(std::cout, "We think the buffer to load is: "s + std::to_string(length));
    lightController.loadBuffer(payload->packetData());
    musicController.clear() ;
    return true ;
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
    DBGMSG(std::cout, "Error on "s + musicController.name());
    auto packet = ErrorPacket(ErrorPacket::CatType::AUDIO, musicController.name());
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

// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ClientConfiguration.hpp"

#include "utility/strutil.hpp"

using namespace std::string_literals ;
// ==============================================================================================
ClientConfiguration::ClientConfiguration(): BaseConfiguration() {
    clientPort = 50001 ;
    serverIP = "windsorway.org" ;
    serverPort = 50000 ;
    
    name = "Blinky Show Client" ;
    
    
    musicPath = std::filesystem::path("/media/Music");
    lightPath = std::filesystem::path("/media/Lights");
    musicExtension  = ".wav" ;
    lightExtension = ".light" ;
    
    useAudio = false ;
    useLight = false ;
    
    audioDevice = 0 ;
 
}

// ===============================================================================================
ClientConfiguration::ClientConfiguration(const std::filesystem::path &path): ClientConfiguration() {
    if (!this->load(path)) {
        throw std::runtime_error("Unable to process: "s + path.string()) ;
    }
}

// ===============================================================================================
auto ClientConfiguration::processKeyValue(const std::string &key, const std::string &value) ->void {
    auto ukey = util::upper(key) ;
    try {
        if (ukey == "CLIENTPORT") {
            clientPort = static_cast<std::uint16_t>(std::stoul(value,nullptr,0));
        }
        else if (ukey == "SERVER") {
            // we need to spit it up
            auto [ip,port] = util::split(value,",") ;
            if (ip.empty() || port.empty()) {
                throw std::runtime_error("Error processing ip/port in client configuration");
            }
            serverPort = static_cast<std::uint16_t>(std::stoul(port,nullptr,0));
            serverIP = ip ;
        }
        else if (ukey == "NAME") {
            name = value ;
        }
        else if (ukey == "MUSICPATH") {
            musicPath = std::filesystem::path(value) ;
        }
        else if (ukey == "MUSICEXTENSION") {
            musicExtension = value  ;
        }
        else if (ukey == "LIGHTPATH") {
            lightPath = std::filesystem::path(value) ;
        }
        else if (ukey == "LIGHTEXTENSION") {
            lightExtension = value  ;
        }
        else if (ukey == "CONNECTHOURS") {
            connectTime = util::HourRange(value) ;
        }
        else if (ukey == "RUNSPAN") {
            runSpan = util::MonthRange(value) ;
        }
        else if (ukey == "AUDIO") {
            useAudio = std::stoi(value,nullptr,0) != 0 ;
        }
        else if (ukey == "AUDIODEVICE") {
            audioDevice = std::stoi(value,nullptr,0) != 0 ;
        }
        else if (ukey == "LIGHTS") {
            useLight = std::stoi(value,nullptr,0) != 0 ;
        }
        else if (ukey == "PRU") {
            auto pru = PRUConfig(value)  ;
            if (pru.pru >=0 && pru.pru <= 1) {
                pruSetting.at(pru.pru) = pru ;
            }
        }
    }
    catch(...) {
        throw std::runtime_error("Errors processing client configuration");
    }

}

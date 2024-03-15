//Copyright © 2024 Charles Kerr. All rights reserved.

#include "Configuration.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "utility/strutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
Configuration::Configuration():clientPort(0),serverPort(50000),useAudio(false),useLight(false),musicExtension(".wav"),audioDevice(0),lightExtension(".light"),name("NoName") {
    
}

//======================================================================
Configuration::Configuration(const std::filesystem::path &path):Configuration() {
    if (!this->load(path)){
        throw std::runtime_error("Error processing: "s + path.string()) ;
    }
}

//======================================================================
auto Configuration::load(const std::filesystem::path &path) -> bool {
    auto buffer = std::vector<char>(1024,0) ;
    auto input = std::ifstream(path.string()) ;
    if (!input.is_open()){
        return false ;
    }
    while(input.good() && !input.eof()){
        input.getline(buffer.data(),buffer.size()-1) ;
        if (input.gcount() > 0) {
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data() ;
            line = util::trim(util::strip(line,"#")) ;
            if (!line.empty()) {
                auto [key,value] = util::split(line,"=") ;
                if (!value.empty()) {
                    if (!processKeyValue(key, value)){
                        DBGMSG(std::cout, "Error processing line: "s + line);
                    }
                }
            }
        }
    }
    return true ;
}

//======================================================================
auto Configuration::processKeyValue(const std::string &key, const std::string &value) -> bool {
    auto ukey = util::upper(key) ;
    try {
        if (ukey == "CLIENTPORT") {
            clientPort = static_cast<std::uint16_t>(std::stoul(value,nullptr,0));
        }
        else if (ukey == "SERVER") {
            // we need to spit it up
            auto [ip,port] = util::split(value,",") ;
            if (ip.empty() || port.empty()) {
                return false ;
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
        return true ;
    }
    catch(...) {
        return false ;
    }
}

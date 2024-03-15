//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightheader.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include "utility/dbgutil.hpp"
using namespace std::string_literals ;

//======================================================================
auto LightHeader::clear() -> void {
    sampleRate = 37 ;
    frameCount = 0 ;
    frameLength = 0 ;
    sourceName = "" ;
    offsetToData = OFFSETTODATA ;
    version = 0 ;
 }
//======================================================================
LightHeader::LightHeader():sampleRate(37),frameCount(0),frameLength(0),offsetToData(OFFSETTODATA),version(0){
    
}
//======================================================================
LightHeader::LightHeader( const char *ptr):LightHeader() {
    load(ptr) ;
}
//======================================================================
LightHeader::LightHeader( std::istream &input):LightHeader() {
    load(input) ;
}

//======================================================================
auto LightHeader::load(const char *ptr) -> void {
    //DBGMSG(std::cout,  "Loading mapped header");
    std::copy(ptr,ptr+4,reinterpret_cast<char*>(&signature)) ;
    // Lets check our signature
    if ((signature &0xff) == 0) {
        DBGMSG(std::cout,  "Assuming old format for header");
        // we are going to "assume old format"?
        float check = 0.0 ;
        std::copy(ptr+1,ptr+5,reinterpret_cast<char*>(&check));
        if ( int(check*1000) != 37) {
            DBGMSG(std::cout,  "Bad sample rate in older format");
            throw std::runtime_error("Unsupported light format");
        }
        // we need the frame acount and frame length
        sampleRate = 37 ;
        
        std::copy(ptr+5,ptr+9,reinterpret_cast<char*>(&frameCount)) ;
        //DBGMSG(std::cout, "Frame count is : "s + std::to_string(frameCount));
        
        std::copy(ptr+9,ptr+13,reinterpret_cast<char*>(&frameLength)) ;
        //DBGMSG(std::cout, "Frame Length is : "s + std::to_string(frameLength)) ;
        offsetToData = 13 ;
        return ;
    }
    else if (signature == SIGNATURE) {
        DBGMSG(std::cout,  "Assuming new format for header");
        std::copy(ptr+4,ptr+8,reinterpret_cast<char*>(&version)) ;
        std::copy(ptr+8,ptr+12,reinterpret_cast<char*>(&offsetToData)) ;
        std::copy(ptr+12,ptr+16,reinterpret_cast<char*>(&sampleRate)) ;
        std::copy(ptr+16,ptr+20,reinterpret_cast<char*>(&frameCount)) ;
        //DBGMSG(std::cout, "Frame count is : "s + std::to_string(frameCount));
        std::copy(ptr+20,ptr+24,reinterpret_cast<char*>(&frameLength)) ;
        //DBGMSG(std::cout, "Frame Length is : "s + std::to_string(frameLength));
        auto buffer = std::vector<char>(NAMESIZE+1,0) ;
        std::copy(ptr+24,ptr+24+NAMESIZE,buffer.data()) ;
        sourceName = buffer.data() ;
        offsetToData = OFFSETTODATA ;
        return ;
    }
    DBGMSG(std::cout,  "Invalid format for header");

    std::runtime_error("Unsupported light format");
}
//======================================================================
auto LightHeader::load(std::istream &input) -> void {
    input.read(reinterpret_cast<char*>(&signature),4);
    // Lets check our signature
    if ((signature &0xff) == 0) {
        DBGMSG(std::cout,  "Assuming old format for header");
        // we are going to "assume old format"?
        float check = 0.0 ;
        input.seekg(1,std::ios::beg) ;
        
        input.read(reinterpret_cast<char*>(&check),4) ;
        if ( int(1000.0 * check) != 37) {
            DBGMSG(std::cout,  "Bad sample rate in older format");
            throw std::runtime_error("Unsupported light format");
        }
        // we need the frame acount and frame length
        sampleRate = 37 ;
        input.read(reinterpret_cast<char*>(&frameCount),4) ;
        input.read(reinterpret_cast<char*>(&frameLength),4) ;
        offsetToData = 13 ;
        return ;
    }
    else if (signature == SIGNATURE){
        DBGMSG(std::cout,  "Assuming new format for header");

        input.read(reinterpret_cast<char*>(&version),4);
        input.read(reinterpret_cast<char*>(&offsetToData),4);
        input.read(reinterpret_cast<char*>(&sampleRate),4);
        input.read(reinterpret_cast<char*>(&frameCount),4);
        input.read(reinterpret_cast<char*>(&frameLength),4);
        auto buffer = std::vector<char>(NAMESIZE+1,0) ;
        input.read(buffer.data(),buffer.size()-1);
        sourceName = buffer.data() ;
        offsetToData = OFFSETTODATA ;
        return ;
    }
    DBGMSG(std::cout,  "Invalid format for header");

    throw std::runtime_error("Unsupported light format");
}
//============================================================================
auto LightHeader::write(std::ostream &output) -> void {
    output.write(reinterpret_cast<const char*>(&signature),4) ;
    output.write(reinterpret_cast<const char*>(&version),4);
    output.write(reinterpret_cast<const char*>(&offsetToData),4);
    output.write(reinterpret_cast<const char*>(&sampleRate),4);
    output.write(reinterpret_cast<const char*>(&frameCount),4);
    output.write(reinterpret_cast<const char*>(&frameLength),4);
    auto buffer = std::vector<char>(NAMESIZE+1,0) ;
    std::copy(sourceName.data(),sourceName.data()+ std::min(static_cast<int>(sourceName.size()),NAMESIZE),buffer.data());
    output.write(buffer.data(),buffer.size()-1);
}

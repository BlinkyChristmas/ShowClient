//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <cstring>

#include "utility/mapfile.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;


//======================================================================
LightFile::LightFile() {
    
}
//======================================================================
LightFile::LightFile(const std::filesystem::path &lightfile) {
    
}
//======================================================================
auto LightFile::loadFile(const std::filesystem::path &lightfile) -> bool {
    try {
        if (lightData.ptr  != nullptr) {
            clear() ;
        }
        //DBGMSG(std::cout, "Mapping: "s + lightfile.string()) ;
        
        auto ptr = lightData.map(lightfile) ;
        // we should check right here if the right type of file
        if (ptr == nullptr) {
            DBGMSG(std::cout, "Failed to map: "s + lightfile.string()) ;
            return false ;
        }
        try {
            DBGMSG(std::cout, "Loading light header: "s + lightfile.string()) ;
            lightHeader.load(reinterpret_cast<const char *>(lightData.ptr)) ;
            //std::cout << "Mapped file: " << reinterpret_cast<std::uint64_t>(ptr) << " size: " << lightData.size << std::endl;
        }
        catch (...) {
            lightHeader.clear();
            lightData.unmap() ;
            DBGMSG(std::cout, "Error Loading light header: "s + lightfile.string()) ;

            return false ;

        }
        return true ;
    }
    catch (const std::exception &e){
        // we had some type of error
        DBGMSG(std::cerr, "Error loading: "s + lightfile.string()+"\n"s+e.what());
        return false ;
    }
}

//======================================================================
auto LightFile::isLoaded() const -> bool {
    return lightData.ptr != nullptr ;
}

//======================================================================
auto LightFile::frameCount() const -> std::int32_t {
    return static_cast<std::int32_t>(lightHeader.frameCount) ;
}
//======================================================================
auto LightFile::frameLength() const -> std::int32_t {
    return static_cast<std::int32_t>(lightHeader.frameLength) ;
}

//======================================================================
auto LightFile::copy(std::uint32_t frame,  unsigned char *buffer, int offset, int length )  -> int {
    //std::cout <<"Before copy, length is: "s << std::to_string(length) << std::endl;
    if (lightData.ptr == nullptr) {
        DBGMSG(std::cout, "Light file thought l;ight data ptr was null"s);
        return 0 ;
    }
    if (buffer == nullptr) {
        DBGMSG(std::cout, "Light file thought buffer ptr was null"s);
        return 0 ;
    }
    if (frame >= lightHeader.frameCount) {
        DBGMSG(std::cout, "Light file thought frame ( "s + std::to_string(frame) + " ) was larger then light file ( "s + std::to_string(lightHeader.frameCount)+ ")"s );
        return 0 ;
    }
    if (length == 0) {
        DBGMSG(std::cout, "Light file thought length requested was 0, so setting to frame length"s);
        length = lightHeader.frameLength ;
    }
    if ( (offset + length) > lightHeader.frameLength) {
       
        std::cout << "length: " << length << " offset: " << offset  << " combined exceeds framelength, resetting to : "<< lightHeader.frameLength - offset << std::endl;
        length = lightHeader.frameLength - offset ;
    }
    
    auto dataoffset = lightHeader.offsetToData  + (frame * lightHeader.frameLength) + offset;
    if (dataoffset < lightData.size) {
        if (dataoffset + length >= this->lightData.size) {
            length = static_cast<int>(lightData.size) - dataoffset ;
        }
        if (length > 0) {
            DBGMSG(std::cout, "Copying "s + std::to_string(length) + " from light file from offset: "s + std::to_string(dataoffset) + " (frame: "s + std::to_string(frame) +" FrameLength: "s + std::to_string(lightHeader.frameLength) + " cfg offset: "s + std::to_string(offset));
            std::memcpy(buffer, lightData.ptr + dataoffset, length);
            
            DBGMSG(std::cout, "Copy complete") ;
        }
        else {
            length = 0 ;
        }
        
    }
    else {
        DBGMSG(std::cout, "Avoided corruption, frame: "s+std::to_string(frame));
        return 0 ;
    }
    return length ;
}

//======================================================================
auto LightFile::clear(bool nothrow) -> void  {
    if (lightData.ptr != nullptr) {
        //std::cout << "Clearing : " << reinterpret_cast<std::uint64_t>(lightData.ptr)<< " with Size: " << lightData.size << std::endl;
        lightHeader.clear() ;
        lightData.unmap() ;
        lightData.ptr = nullptr ;
        lightData.size = 0 ;
    }
    
}

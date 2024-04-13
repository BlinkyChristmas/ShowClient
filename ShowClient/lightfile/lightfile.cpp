//Copyright © 2023 Charles Kerr. All rights reserved.

#include "lightfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <cstring>

#include "utility/mapfile.hpp"
#include "utility/dbgutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/strutil.hpp"

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
        
        auto ptr = lightData.map(lightfile) ;
        // we should check right here if the right type of file
        if (ptr == nullptr) {
            DBGMSG(std::cout, util::sysTimeToString(util::ourclock::now())+": "s + "Failed to map: "s + lightfile.string()) ;
            return false ;
        }
        try {
//            DBGMSG(std::cout,"Loading light header: "s + lightfile.string()) ;
            lightHeader.load(reinterpret_cast<const char *>(lightData.ptr)) ;
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
auto LightFile::dataForFrame(std::int32_t frame) const -> const std::uint8_t* {
    const std::uint8_t *rvalue = nullptr ;
    if (lightData.ptr != nullptr ){
        if (frame >= lightHeader.frameCount) {
            frame = lightHeader.frameCount - 1 ;
        }
        if (frame >= 0){
            rvalue  = lightData.ptr + lightHeader.offsetToData  + (frame * this->frameLength()) ;
            
        }
    }
    return rvalue ;
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

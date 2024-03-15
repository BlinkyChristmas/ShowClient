//Copyright © 2024 Charles Kerr. All rights reserved.

#include "LightController.hpp"

#include <algorithm>
#include <stdexcept>

#if defined(BEAGLE)
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"
using namespace std::string_literals ;


//======================================================================
auto LightController::mapPRU(int number) -> void {
    auto pru = pruBuffers[number] ;
    //DBGMSG(std::cout, "mapping pru: " + std::to_string(number)) ;
    if (pru != nullptr){
        this->freePRU(number) ;
    }
    pru = nullptr ;
#if defined(BEAGLE)
    off_t target = (number == 0 ? 0x4a300000 : 0x4a302000) ;
   // DBGMSG(std::cout,"Offset is: "s +util::ntos(target,16,true,8) );
    auto fd = ::open("/dev/mem", O_RDWR | O_SYNC) ;
    if (fd <0) {
        // we couldn't open it
        std::cerr << "unable to open PRU " << std::to_string(number)<<"\n"s;
        return  ;
    }
    auto temp = mmap(0,PRUMAPSIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd, target) ;
    ::close(fd) ;
    if (temp == MAP_FAILED ) {
        DBGMSG(std::cerr,"Mapped failed!" );

        return  ;
    }
    //DBGMSG(std::cout,"Mapped success!" );
    pru = reinterpret_cast<std::uint8_t*>(temp) ;
    
    
#endif
    pruBuffers[number] = pru ;

}

//======================================================================
auto LightController::freePRU(int prunumber) -> void {
    [[maybe_unused]] auto pru = pruBuffers[prunumber] ;
#if defined(BEAGLE)
    if (pru != nullptr) {
        auto ptr = reinterpret_cast<void*>(const_cast<unsigned char*>(pru)) ; // the const cast gets rids of volatile
        munmap(ptr, PRUMAPSIZE) ;
    }
#endif
    pruBuffers[prunumber] = nullptr ;

}

//======================================================================
auto LightController::initPRU(int number) -> void {
    DBGMSG(std::cout,"Initialize pru: "s+std::to_string(number) );
    mapPRU(number) ;
    auto pru = pruBuffers[number] ;
    auto bit = std::int32_t((number == 0 ? 14 : 1)) ; // PRU 0 is on bit 14, pru 1 is on bit 1 ;
    DBGMSG(std::cout,"Bit is: "s+std::to_string(bit) );

    auto mode =  std::int32_t(pruConfiguration[number].mode );
    auto zero = std::int32_t(0) ;
    auto one = std::int32_t(1) ;
    auto size = std::int32_t((mode == PRUConfig::MODE_SSD ? 3072 : 512)) ;
    if (pru != nullptr) {
        auto buffer = std::vector<unsigned char>(3072, 0 ) ;
        //DBGMSG(std::cout,"Setting up pru memory header" );
        DBGMSG(std::cout, "Setting mode for PRU: "s + std::to_string(number) + " to "s + std::to_string(mode));
        std::copy(reinterpret_cast<unsigned char*>(&mode),reinterpret_cast<unsigned char*>(&mode)+4,pru + INDEX_TYPE) ;
        DBGMSG(std::cout, "Setting bit for PRU: "s + std::to_string(number) + " to "s + std::to_string(bit));
        std::copy(reinterpret_cast<unsigned char*>(&bit),reinterpret_cast<unsigned char*>(&bit)+4,pru + INDEX_BITREG) ;
        DBGMSG(std::cout, "Setting count for PRU: "s + std::to_string(number) + " to "s + std::to_string(size));
        std::copy(reinterpret_cast<unsigned char*>(&size),reinterpret_cast<unsigned char*>(&size)+4,pru + INDEX_OUTPUTCOUNT) ;
        std::copy(reinterpret_cast<unsigned char*>(&zero),reinterpret_cast<unsigned char*>(&zero)+4,pru + INDEX_DATAREADY) ;
        DBGMSG(std::cout, "CLEARING data for PRU: "s + std::to_string(number) + " at "s + std::to_string(reinterpret_cast<std::uint64_t>(pru) ) + " offset: "s + std::to_string(INDEX_PRUOUTPUT));
        std::copy(buffer.begin(),buffer.end(),pru + INDEX_PRUOUTPUT) ;
        std::copy(reinterpret_cast<char*>(&one),reinterpret_cast<char*>(&one)+4,pru + INDEX_DATAREADY) ;

    }

}

//======================================================================
LightController::LightController() {
    for (size_t index = 0 ; index < pruBuffers.size();index++){
        pruBuffers[index] = nullptr ;
    }
}

//======================================================================
LightController::~LightController() {
    freePRU(0);
    freePRU(1) ;
    lightFile.clear();
}

//======================================================================
auto LightController::start(std::int32_t frame ) -> bool {
    if (!lightFile.isLoaded()) {
        DBGMSG(std::cerr, "Start attempted with nothing loaded") ;
        return false ;
    }
    auto one = std::int32_t(1) ;
    // Do something here?
    for ( auto pru = 0 ; pru < 2; pru++ ){
        auto length = pruConfiguration.at(pru).length ;
        auto maxlength = (lightFile.frameLength() - pruConfiguration.at(pru).offset) ;
        if (length > maxlength) {
            length  = maxlength ;
        }
        //DBGMSG(std::cout, "Copy start lights for pru: "s + std::to_string(pru) + " for frame: "s + std::to_string(frame));
        lightFile.copy(frame, reinterpret_cast<unsigned char*>(const_cast<std::uint8_t*>(pruBuffers.at(pru)+INDEX_PRUOUTPUT)),pruConfiguration.at(pru).offset,length) ;
        //DBGMSG(std::cout, "Setting start data ready for pru: "s + std::to_string(pru));
        std::copy(reinterpret_cast<std::uint8_t*>(&one), reinterpret_cast<std::uint8_t*>(&one) + 4, pruBuffers[pru] + INDEX_DATAREADY) ;
    }
    return true ;
}

//======================================================================
auto LightController::stop(bool close) -> void {
    if (close) {
        //DBGMSG(std::cout, "Clearing light file") ;
        lightFile.clear() ;
    }
}

//======================================================================
auto LightController::close() -> void {
    lightFile.clear() ;
}
//======================================================================
auto LightController::configurePRU(PRUConfig pru0,PRUConfig pru1) -> void {
    pruConfiguration[0] = pru0 ;
    //DBGMSG(std::cout, "Initializing pru 0");
    initPRU(0);
    pruConfiguration[1] = pru1 ;
   // DBGMSG(std::cout, "Initializing pru 1");
    initPRU(1) ;

}

//======================================================================
auto LightController::load(const std::filesystem::path &path) -> bool {
    try {
        if (path.empty()) {
            return false ;
        }
        DBGMSG(std::cout, "Loading light file: "s + path.string()) ;
        if (!lightFile.loadFile(path)) {
            DBGMSG(std::cout, "Loading unsuccessful for light file: "s + path.string()) ;
            return false ;
        }
        return lightFile.frameCount()!= 0 ;
    }
    catch(...) {
        return false ;
    }
   
}

//======================================================================
auto LightController::frameCount() const-> std::int32_t  {
    if (lightFile.isLoaded()) {
        return lightFile.frameCount() ;
    }
    return 0 ;
    
}
//======================================================================
auto LightController::sync(std::int32_t frame) -> void {
    auto one = std::int32_t(1) ;
    if (  frame < lightFile.frameCount()) {
        for (auto p=0 ; p < 2; p++) {
            auto index = pruConfiguration.at(p).offset ;
            auto length = static_cast<unsigned int>(pruConfiguration.at(p).length) ;
            auto maxlength = (lightFile.frameLength() - index) ;
            if (length > maxlength) {
                length  = maxlength ;
            }
            //DBGMSG(std::cout, "Copy lights for pru: "s + std::to_string(p) + " for frame: "s + std::to_string(frame));
            lightFile.copy(frame, reinterpret_cast<unsigned char*>(const_cast<std::uint8_t*>(pruBuffers.at(p)+INDEX_PRUOUTPUT)),index,length) ;
           // DBGMSG(std::cout, "Setting data ready for pru: "s + std::to_string(p));
            std::copy(reinterpret_cast<std::uint8_t*>(&one), reinterpret_cast<std::uint8_t*>(&one) + 4, pruBuffers[p] + INDEX_DATAREADY) ;
            //DBGMSG(std::cout, "Leaving data setup for pru: "s + std::to_string(p));
        }

    }
    else {
        //DBGMSG(std::cout , "Did nothing for lights, frame: "s + std::to_string(frame) +" file had a total of: "s + std::to_string(lightFile.frameCount()));
    }
}

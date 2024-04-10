// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BeaglePru.hpp"

#include <stdexcept>
#include <fstream>
#include <vector>

#if defined(BEAGLE)
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"

using namespace std::string_literals ;

// ==============================================================================
const std::string BeaglePru::firmware_location = "/sys/class/remoteproc/remoteproc%i/firmware"s ;
const std::string BeaglePru::firmware_state = "/sys/class/remoteproc/remoteproc%i/state"s ;;
const std::string BeaglePru::runing_state = "running"s;
const std::string BeaglePru::offline_state = "offline"s;
const std::string BeaglePru::halt_state = "stop"s;

// ==============================================================================
auto BeaglePru::unmapPRU() -> void {
    if (mapped_address != nullptr) {
#if defined(BEAGLE)
        munmap(mapped_address, PRUMAPSIZE) ;
#endif
        mapped_address = nullptr ;
    }
}

// ==============================================================================
auto BeaglePru::mapPRU() -> bool {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return false ;
    }
#if defined(BEAGLE)
    if (mapped_address != nullptr) {
        // we have to unmap it
        unmapPRU() ;
    }
    
    // Now we get to map the memory of the PRU (the 4K memory region)
    off_t prumem = (pru_number == PruNumber::zero ? 0x4a300000 : 0x4a302000) ; // These are the two region address
    auto fd = ::open("/dev/mem", O_RDWR | O_SYNC) ;
    if (fd <0) {
        // we couldn't open it
        DBGMSG(std::cerr, "Unable to open memory for pru: "s + std::to_string(static_cast<int>(pru_number)));
        return false ;
    }
    auto temp = mmap(0,PRUMAPSIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd, prumem) ;
    ::close(fd) ;
    if (temp == MAP_FAILED ) {
        DBGMSG(std::cerr,"Mapping failed for pru: "s + std::to_string(static_cast<int>(pru_number)));

        return false ;
    }
    mapped_address = reinterpret_cast<std::uint8_t*>(temp) ;
#endif
    return true ;
}

// =============================================================================
auto BeaglePru::isState(const std::string &state) const -> bool {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return false ;
    }
#if !defined(BEAGLE)
    return true ;
#else
    auto path = util::format(firmware_state,static_cast<int>(pru_number)+1) ;
    auto input = std::ifstream(path) ;
    if (!input.is_open()) {
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1) ;
    if (input.gcount() == 0) {
        return false ;
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string prustate = buffer.data() ;
    DBGMSG(std::cout, "State is '"s+prustate+"'");
    return prustate == state ;
#endif
}

// =============================================================================
auto BeaglePru::setState(const std::string &state) -> bool {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return  false ;
    }
#if !defined(BEAGLE)
    return true ;
#else
    auto path = util::format(firmware_state,static_cast<int>(pru_number)+1) ;
    auto output = std::ofstream(path) ;
    if (!output.is_open()){
        return false ;
    }
    output << state ;
    output.close();
    return true ;
#endif
}

// ==============================================================================
BeaglePru::BeaglePru(PruNumber pruNumber):pru_number(pruNumber),mapped_address(nullptr){
#if defined(BEAGLE)
    if (pru_number == PruNumber::zero || pru_number == PruNumber::one) {

        if (!mapPRU()) {
            throw std::runtime_error("Unable to map pru: "s + std::to_string(static_cast<int>(pru_number))) ;
        }
    }
#endif
}

// =============================================================================
BeaglePru::~BeaglePru() {
#if defined(BEAGLE)
    unmapPRU();
#endif
}

// =============================================================================
auto BeaglePru::open() -> bool {
#if !defined(BEAGLE)
    return true ;
#else
    return mapPRU();
#endif
}
// =============================================================================
auto BeaglePru::close() -> void {
#if defined(BEAGLE)
    return unmapPRU() ;
#endif
}

// =========================================================================================================
auto BeaglePru::isValid() const -> bool {
#if !defined(BEAGLE)
    return true ;
#else
    return mapped_address != nullptr ;
#endif
}

// =========================================================================================================
auto BeaglePru::address() -> std::uint8_t* {
    
    return mapped_address ;
}

// =========================================================================================================
auto BeaglePru::isOffline() const -> bool {
    return isState(offline_state);
}
// =========================================================================================================
auto BeaglePru::isRunning() const -> bool {
    return isState(runing_state);
    
}
// =========================================================================================================
auto BeaglePru::load(const std::string &firmware) -> bool {
    
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return  false ;
    }
#if !defined(BEAGLE)
    return true ;
#else
    if (this->firmware() == firmware){
        return true ;
    }
    if (this->isRunning()){
        this->stop();
    }
    
    auto firmpath = util::format(firmware_location, static_cast<int>(pru_number)+1) ;
    auto output = std::ofstream(firmpath) ;
    if (!output.is_open()) {
        return false ;
    }
    output << firmware ;
    output.close();
    return true ;
#endif
}
// =========================================================================================================
auto BeaglePru::firmware() const -> std::string {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return  ""s ;
    }
#if !defined(BEAGLE)
    return "blinkylight-fw" ;
#else
    auto firmpath = util::format(firmware_location, static_cast<int>(pru_number)+1) ;
    auto input = std::ifstream(firmpath) ;
    if (!input.is_open()) {
        return ""s ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1) ;
    if (input.gcount() == 0) {
        return ""s ;
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string firm = buffer.data() ;
    return firm ;
#endif
}

// =========================================================================================================
auto BeaglePru::start() -> bool {
#if !defined(BEAGLE)
    return true ;
#else
    if (!this->isRunning()) {
        return setState("start");
    }
    return true ;
#endif
}

// =========================================================================================================
auto BeaglePru::stop() -> bool {
#if !defined(BEAGLE)
    return true ;
#else
    if (!this->isOffline()) {
        return setState(halt_state) ;
    }
    return true ;
#endif
}

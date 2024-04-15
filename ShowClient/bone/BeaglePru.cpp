// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BeaglePru.hpp"

#include <stdexcept>
#include <fstream>
#include <vector>
#include <algorithm>
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
    //DBGMSG(std::cout, "Determine address");
    // Now we get to map the memory of the PRU (the 4K memory region)
    off_t prumem = (pru_number == PruNumber::zero ? PRU0_MEMORYSPACE : PRU1_MEMORYSPACE) ; // These are the two region address
    auto fd = ::open("/dev/mem", O_RDWR | O_SYNC) ;
    if (fd <0) {
        // we couldn't open it
        DBGMSG(std::cerr, "Unable to open memory for pru: "s + std::to_string(static_cast<int>(pru_number)));
        return false ;
    }
    //DBGMSG(std::cout, "MMAP pru");
    auto temp = mmap(0,PRUMAPSIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd, prumem) ;
    ::close(fd) ;
    if (temp == MAP_FAILED ) {
        DBGMSG(std::cerr,"Mapping failed for pru: "s + std::to_string(static_cast<int>(pru_number)));

        return false ;
    }
    mapped_address = reinterpret_cast<std::uint8_t*>(temp) ;
    //DBGMSG(std::cout, "Determine bit for pru");
    auto bit = std::int32_t((static_cast<int>(pru_number) == 0 ? 14 : 1)) ; // PRU 0 is on bit 14, pru 1 is on bit 1 ;
    auto size = 3072;
    auto zero = 0 ;
    auto outmode = 0 ;
    this->clear(16,PRUMAPSIZE-16) ;
    //DBGMSG(std::cout, "Setting mode");
    std::copy(reinterpret_cast<unsigned char*>(&outmode),reinterpret_cast<unsigned char*>(&outmode)+4, mapped_address) ;
    std::copy(reinterpret_cast<unsigned char*>(&bit),reinterpret_cast<unsigned char*>(&bit)+4, mapped_address + 4) ;
    std::copy(reinterpret_cast<unsigned char*>(&size),reinterpret_cast<unsigned char*>(&size)+4, mapped_address + 12) ;
    std::copy(reinterpret_cast<const unsigned char*>(&zero),reinterpret_cast<const unsigned char*>(&zero)+4, mapped_address + 8);
    

#endif
    return true ;
}

// =============================================================================
auto BeaglePru::state() const -> std::string  {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        throw std::runtime_error("Invalid pru number to obain state information");
    }
#if !defined(BEAGLE)
    return "" ;
#else
    auto path = util::format(PRU_FIRMWARE_STATE,static_cast<int>(pru_number)+1) ;
    auto input = std::ifstream(path) ;
    if (!input.is_open()) {
        throw std::runtime_error("Unable to obtain state information: "s+path);
 ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1) ;
    if (input.gcount() == 0) {
        throw std::runtime_error("Unable to read state information from: "s+path);
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string prustate = buffer.data() ;
    //DBGMSG(std::cout, "State is '"s+prustate+"'");
    return prustate ;
#endif
}


// ==============================================================================
BeaglePru::BeaglePru(PruNumber pruNumber):pru_number(pruNumber),mapped_address(nullptr){
#if defined(BEAGLE)
    if (pru_number == PruNumber::zero || pru_number == PruNumber::one) {
        //DBGMSG(std::cout, "Mapping PRU");
        if (!mapPRU()) {
            DBGMSG(std::cerr, "Error Mapping PRU");

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


// =========================================================================================================
auto BeaglePru::address() -> std::uint8_t* {
    
    return mapped_address ;
}

// =========================================================================================================
auto BeaglePru::firmware() const -> std::string {
    if (pru_number != PruNumber::zero &&  pru_number != PruNumber::one) {
        return  ""s ;
    }
#if !defined(BEAGLE)
    return "blinkylight-fw" ;
#else
    auto firmpath = util::format(PRU_FIRMWARE_LOCATION, static_cast<int>(pru_number)+1) ;
    auto input = std::ifstream(firmpath) ;
    if (!input.is_open()) {
        throw std::runtime_error("Unable to obtain firmware from: "s + firmpath);
        return ""s ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    input.getline(buffer.data(), buffer.size()-1) ;
    if (input.gcount() == 0) {
        throw std::runtime_error("Unable to read firmware from: "s + firmpath);
    }
    buffer[input.gcount()] = 0 ;
    input.close();
    std::string firm = buffer.data() ;
    return firm ;
#endif
}

// =========================================================================================================
auto BeaglePru::setData(const std::uint8_t *ptrToData, int length, int offsetInPruMemory) -> bool {
#if defined(BEAGLE)
    if (this->mapped_address == nullptr || (length + offsetInPruMemory > PRUMAPSIZE)) {
        return false ;
    }
    DBGMSG(std::cout,"Asked to copy: "s + std::to_string(length) + " to offset: "s + std::to_string(offsetInPruMemory));
    std::copy(ptrToData,ptrToData+length,mapped_address + offsetInPruMemory) ;
#endif
    return true ;
}
// =========================================================================================================
auto BeaglePru::clear(int offsetInPruMemory,int length) -> bool {
#if defined(BEAGLE)
    if (this->mapped_address == nullptr || (length + offsetInPruMemory > PRUMAPSIZE)) {
        return false ;
    }
    auto buffer = std::vector<std::uint8_t>(length,0) ;
    return setData(buffer.data(), length, offsetInPruMemory);
#else
    return true ;
#endif
}

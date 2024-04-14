// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PruConstants_hpp
#define PruConstants_hpp



// Generic PRU stuff 
static constexpr size_t PRUMAPSIZE = 8192 ;
static constexpr size_t PRU0_MEMORYSPACE  = 0x4a300000 ;
static constexpr size_t PRU1_MEMORYSPACE  = 0x4a302000 ;

const std::string PRU_FIRMWARE_LOCATION  = "/sys/class/remoteproc/remoteproc%i/firmware" ;
const std::string PRU_FIRMWARE_STATE = "/sys/class/remoteproc/remoteproc%i/state" ;;
const std::string PRU_RUNNING = "running";
const std::string PRU_OFFLINE = "offline";
const std::string PRU_HALT = "stop";



#endif /* PruConstants_hpp */

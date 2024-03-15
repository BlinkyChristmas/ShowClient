//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef MusicController_hpp
#define MusicController_hpp

#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "rtaudio/RtAudio.h"
#include "wavfile/mwavfile.hpp"

//======================================================================
class MusicController{
    
    MWAVFile musicfile ;
    
    RtAudio soundDac ;
    RtAudio::StreamParameters rtParameters ;

    bool use_sync ;
    
    std::int32_t current_frame ;
    std::int32_t sync_frame ;

    int my_device ;
    
    std::uint32_t bufferFrames ;
    
    static auto rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int ;
 

public:
    static auto getSoundDevices() -> std::vector<std::pair<int,std::string>> ;
    static auto getDefaultDevice() -> int ;
    static auto deviceExists(int device) -> bool ;
    
     
    MusicController() ;
    ~MusicController() ;
    
    auto load(const std::filesystem::path &path) -> bool ;
    auto close() -> void ;
    auto frameCount() const -> std::int32_t ;
    
    auto initialize(int device, std::uint32_t sampeRate) -> bool ;

    auto setDevice(int dev = 0) -> void ;
    
    auto isPlaying() const -> bool ;
    
    auto isEnabled() const -> bool ;
    auto setEnabled(bool state) -> void ;
    
    auto start(std::int32_t frame = 0,bool useSync = false) -> bool ;
    auto stop(bool close = true) -> void ;
    
    auto sync(std::int32_t frame) -> void ;
    
    auto requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int ;
    auto errorCallback(RtAudioErrorType type, const std::string &errorText) -> void ;
};

#endif /* MusicController_hpp */

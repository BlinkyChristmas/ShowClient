// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef MusicController_hpp
#define MusicController_hpp

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <mutex>
#include "rtaudio-6.0.1/RtAudio.h"
#include "wavfile/mwavfile.hpp"
#include "IOController.hpp"
class MusicController;
using MusicPointer = MusicController* ;
using MusicError = std::function<void(MusicPointer)> ;
class MusicController: public IOController  {
    
    RtAudio soundDac ;
    RtAudio::StreamParameters rtParameters ;
    
    MWAVFile musicFile ;
    
    static auto rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int ;

    int my_device ;
    std::uint32_t bufferFrames ; // This we will use to setup our buffer size to match a frame period
    
    MusicError musicErrorCallback ;
    
    auto clearLoaded() -> void ;
    auto load(const std::filesystem::path &path) -> bool ;
    auto userSetSync(int syncframe) -> void ;

 public:
    static auto getSoundDevices() -> std::vector<std::pair<int,std::string>> ;
    static auto getDefaultDevice() -> int ;
    static auto deviceExists(int device) -> bool ;

    MusicController() ;
    ~MusicController() ;
    auto initialize(int device, std::uint32_t sampeRate = 44100) -> bool ;
    auto isPlaying() const -> bool final ;
    auto setDevice(int device) -> void ;
    auto device() const -> int;
 
    auto load(const std::string &dataname) -> bool final ;
    auto stop() -> void final ;
    auto start(std::int32_t frame = 0,int period = IOController::FRAMEPERIOD) -> bool final  ;
    auto clear() -> void final ;
    
    // The callbacks we will use
    auto requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int ;
    auto errorCallback(RtAudioErrorType type, const std::string &errorText) -> void ;
    auto setMusicErrorCallback(MusicError function) -> void ;

};

#endif /* MusicController_hpp */

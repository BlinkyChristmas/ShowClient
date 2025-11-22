// Copyright © 2024 Charles Kerr. All rights reserved.

#include "MusicController.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/strutil.hpp"
#include "MixerControl.hpp"
using namespace std::string_literals;

/* ************************************************************************************************************************************
 This is our global callback, that then relays it to the instance version
 ************************************************************************************************************************************ */
// ======================================================================
auto MusicController::rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int {
    // We pass the address to our instance in ptr!
    return reinterpret_cast<MusicController*>(ptr)->requestData(reinterpret_cast<std::uint8_t*>(outputBuffer),nFrames,StreamTime,status) ;
}

// ======================================================================
auto MusicController::clearLoaded() -> void {
    if (isPlaying() ) {
        this->stop() ;
    }
    is_loaded = false ;
    if (musicFile.isLoaded()){
        musicFile.close();
    }
    has_error = false ;
    data_name = "" ;
}

// ======================================================================
auto MusicController::load(const std::filesystem::path &path) -> bool {
    if (!std::filesystem::exists(path)){
        DBGMSG(std::cout, "File does not exist: "s + path.string()) ;
        has_error = true ;
        return false ;
    }
    is_loaded  = musicFile.load(path) ;
    has_error = !is_loaded;
    return is_loaded ;
}

// =====================================================================
auto MusicController::userSetSync(int syncframe) -> void {
    musicFile.setFrame(syncframe);
}

// ======================================================================
auto MusicController::getSoundDevices() -> std::vector<std::pair<int,std::string>> {
    RtAudio soundDac ;
    auto devices = soundDac.getDeviceIds() ;
    auto names = soundDac.getDeviceNames() ;
    auto rvalue = std::vector<std::pair<int,std::string>>() ;
    for (size_t index = 0 ; index < devices.size();index++) {
        if (index >= names.size()) {
            break ;
        }
        rvalue.push_back(std::make_pair(devices.at(index), names.at(index))) ;
    }
    return rvalue ;
}

// ======================================================================
auto MusicController::getDefaultDevice() -> int {
    RtAudio soundDac ;
    return soundDac.getDefaultOutputDevice() ;
}

// ======================================================================
auto MusicController::deviceExists(int device) -> bool {
    RtAudio soundDac ;
    auto devices = soundDac.getDeviceIds() ;
    auto iter = std::find_if(devices.begin(),devices.end(),[device](int dev) {
        return dev == device ;
    });
    return iter != devices.end() ;
}

// ==========================================================================================
MusicController::MusicController():IOController(),bufferFrames(1632),my_device(0),musicErrorCallback(nullptr){
    soundDac.showWarnings(false);
    soundDac.setErrorCallback( std::bind( &MusicController::errorCallback, this, std::placeholders::_1, std::placeholders::_2) );
}
// ======================================================================
MusicController::~MusicController() {
    if (isPlaying()) {
        this->stop();
    }
    if (is_loaded) {
        this->clear() ;
    }
}

// ======================================================================
auto MusicController::initialize(int device, std::uint32_t sampleRate )-> bool {
    if (soundDac.isStreamOpen()){
        soundDac.closeStream();
    }
    //DBGMSG(std::cout, "We think our device is "s + std::to_string(device)) ;
    if (!is_enabled){
        return true ;
    }
    has_error = false ;
    if (device == 0) {
        // get the default device
        device = MusicController::getDefaultDevice() ;
        //DBGMSG(std::cout, "Default device is: "s + std::to_string(device)) ;
    }
    if (device == 0) {
        //DBGMSG(std::cout, "Return false because: Could not find a default device"s);
        my_device = 0 ;
        has_error = true ;
        return false ;
    }
    // Make sure this device exists ?
    if (!MusicController::deviceExists(device)) {
        //DBGMSG(std::cout, "Return false because: Does not exist device: "s + std::to_string(device));

        has_error = true ;
        return false ;
    }
    my_device = device ;
    // Device exists, we are set and ready
    if (soundDac.isStreamOpen()) {
        this->stop() ;
    }
    rtParameters.deviceId = device ;
    rtParameters.nChannels = 2 ;
    auto status = soundDac.openStream(&rtParameters, NULL, RTAUDIO_SINT16, sampleRate, &bufferFrames, &MusicController::rtCallback,this) ;
    if (status == RTAUDIO_SYSTEM_ERROR ) {
        has_error = true ;
        return false ;
    }
    if (status == RTAUDIO_INVALID_USE) {
        //DBGMSG(std::cout, "Return false because: RTAUDIO_INVALID_USE");

        has_error = true ;
        return false ;
    }
    return true ;
}
// ======================================================================================================================
auto MusicController::isPlaying() const -> bool {
    return soundDac.isStreamRunning() ;
}


// ======================================================================
auto MusicController::setDevice(int device) -> void {
    my_device = device ;
}

// ======================================================================
auto MusicController::device() const -> int {
    return my_device ;
}

// ======================================================================
auto MusicController::load(const std::string &musicname) -> bool {
    this->clearLoaded() ;
    if (!is_enabled || musicname.empty()) {
        return true ;
    }
    
    this->data_name = musicname ;
    //setVolume(86) ;
    auto path = data_location / std::filesystem::path(this->data_name+data_extension) ;
    return load(path);
}

// ======================================================================
auto MusicController::stop() -> void {
    if (soundDac.isStreamOpen()) {
        if (soundDac.isStreamRunning()) {
            soundDac.abortStream() ;
        }
        soundDac.closeStream() ;
    }
    is_playing = false ;
}



// ======================================================================
auto MusicController::start(std::int32_t frame ,int period ) -> bool {
    if (has_error) {
        return false ;
    }
    if (!is_enabled || !is_loaded){
        
        return true ;
    }
    
    {
        auto lock = std::lock_guard(frame_access);
        current_frame = frame ;
        musicFile.setFrame(frame) ;
    }
    // Now, start the playing
    if (!initialize(my_device, musicFile.sampleRate())){
        has_error = true ;
        return false ;
    }
    soundDac.startStream() ;
    
    return soundDac.isStreamRunning() ;
}

// ======================================================================
auto MusicController::clear() -> void {
    this->clearLoaded();
}

// Our two callbacks
// ======================================================================
auto MusicController::requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int {
    if (!soundDac.isStreamRunning() || !soundDac.isStreamOpen() ) {
        return 2 ;
    }
    auto lock = std::lock_guard(frame_access);
    auto amount = musicFile.loadBuffer(data, frameCount) ;
    current_frame += 1 ;
    if (amount < frameCount) {
        return 1 ;
    }
    else if (amount == 0 ){
        return 2 ;
    }
    return 0 ;
}

//=============================================================
auto MusicController::errorCallback(RtAudioErrorType type, const std::string &errorText) -> void {
    //DBGMSG(std::cout, "Error received on sound dac: "s + std::string(soundDac.getErrorText()));
    if (soundDac.isStreamOpen()) {
        soundDac.abortStream() ;
    }
    if (musicErrorCallback != nullptr) {
        musicErrorCallback(this) ;
    }
}

//=============================================================
auto MusicController::setMusicErrorCallback(MusicError function) -> void {
    musicErrorCallback = function ;
}


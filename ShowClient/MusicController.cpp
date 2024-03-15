//Copyright © 2024 Charles Kerr. All rights reserved.

#include "MusicController.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

/* ************************************************************************************************************************************
 This is our global callback, that then relays it to the instance version
 ************************************************************************************************************************************ */
// ======================================================================
auto MusicController::rtCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames,double StreamTime, RtAudioStreamStatus status , void *ptr) -> int {
    // We pass the address to our instance in ptr!
    return reinterpret_cast<MusicController*>(ptr)->requestData(reinterpret_cast<std::uint8_t*>(outputBuffer),nFrames,StreamTime,status) ;
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

// ======================================================================
auto MusicController::initialize(int device, std::uint32_t sampleRate )-> bool {
    if (device == 0) {
        // get the default device
        device = MusicController::getDefaultDevice() ;
    }
    if (device == 0) {
        return false ;
    }
    // Make sure this device exists ?
    if (!MusicController::deviceExists(device)) {
        return false ;
    }
    soundDac.showWarnings(false) ;
    soundDac.setErrorCallback(std::bind(&MusicController::errorCallback,this,std::placeholders::_1, std::placeholders::_2)) ;
    // Device exists, we are set and ready
    if (soundDac.isStreamOpen()) {
        this->stop(false) ;
    }
    rtParameters.deviceId = device ;
    rtParameters.nChannels = 2 ;
    auto status = soundDac.openStream(&rtParameters, NULL, RTAUDIO_SINT16, sampleRate, &bufferFrames, &MusicController::rtCallback,this) ;
    if (status == RTAUDIO_SYSTEM_ERROR ) {
        return false ;
    }
    if (status == RTAUDIO_INVALID_USE) {
        return false ;
    }
    
    return true ;
}
// ======================================================================
MusicController::MusicController():current_frame(0),use_sync(false),my_device(0),bufferFrames(1632) {
    
}

// ======================================================================
MusicController::~MusicController() {
    
    if (musicfile.isLoaded()) {
        musicfile.close() ;
    }
}


// ======================================================================
auto MusicController::load(const std::filesystem::path &path) -> bool {
    if (this->isPlaying()) {
        this->stop(true) ;
    }
    return musicfile.load(path) ;
}

// ======================================================================
auto MusicController::close() -> void {
    this->stop(true) ;
}

// ======================================================================
auto MusicController::frameCount() const -> std::int32_t {
    return musicfile.frameCount() ;
}

// ======================================================================
auto MusicController::setDevice(int dev  ) -> void  {
    my_device = dev ;
}

// ======================================================================
auto MusicController::isPlaying() const -> bool {
    return soundDac.isStreamRunning() ;
}

// ======================================================================
auto MusicController::start(std::int32_t frame , bool useSync ) -> bool {
    use_sync = useSync ;
    current_frame = frame ;
    sync_frame = frame  ;
    musicfile.setFrame(frame) ;
    // Now, start the playing
    if (!initialize(my_device, musicfile.sampleRate())){
        return false ;
    }
    soundDac.startStream() ;
    return soundDac.isStreamRunning() ;
}

// ======================================================================
auto MusicController::stop(bool close ) -> void {
    if (soundDac.isStreamOpen()) {
        if (soundDac.isStreamRunning()) {
            soundDac.abortStream() ;
        }
        soundDac.closeStream() ;
    }
    if (close) {
        if (musicfile.isLoaded()) {
            musicfile.close();
        }
    }
}

// ======================================================================
auto MusicController::requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int {
    if (!soundDac.isStreamRunning() || !soundDac.isStreamOpen() ) {
        return 2 ;
    }
    if (use_sync) {
        //DBGMSG(std::cout,"Player frame: "s + std::to_string(current_frame) + " Player Sync: "s + std::to_string(sync_frame));
        auto delta = 0 ;
        if (current_frame > sync_frame) {
            delta = current_frame - sync_frame ;
        }
        else {
            delta = sync_frame - current_frame ;
        }
        if (delta >= 2) {
            musicfile.setFrame(sync_frame) ;
            current_frame = static_cast<std::uint32_t>(sync_frame) ;
        }
    }
    //DBGMSG(std::cout,"Player frame: "s + std::to_string(current_frame));
    auto amount = musicfile.loadBuffer(data, frameCount) ;
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
auto MusicController::sync(std::int32_t frame) -> void {
    sync_frame = frame ;
}

//=============================================================
auto MusicController::errorCallback(RtAudioErrorType type, const std::string &errorText) -> void {
    DBGMSG(std::cout, "Error received on sound dac: "s + std::string(soundDac.getErrorText()));
    if (soundDac.isStreamOpen()) {
        soundDac.abortStream() ;
    }
}

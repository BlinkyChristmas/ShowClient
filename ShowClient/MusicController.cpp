// Copyright © 2024 Charles Kerr. All rights reserved.

#include "MusicController.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/strutil.hpp"

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
auto MusicController::shouldPlay() const ->bool {
    return is_enabled && !errorState && musicFile.isLoaded() ;
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

// ======================================================================================================================
MusicController::MusicController():errorState(false),is_enabled(false),bufferFrames(1632),mydevice(0),current_frame(0),musicErrorCallback(nullptr){
    soundDac.showWarnings(false);
    soundDac.setErrorCallback(std::bind(&MusicController::errorCallback,this,std::placeholders::_1,std::placeholders::_2));
}
// ======================================================================
MusicController::~MusicController() {
    
    if (musicFile.isLoaded()) {
        musicFile.close() ;
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
    errorState = false ;
    if (device == 0) {
        // get the default device
        device = MusicController::getDefaultDevice() ;
        //DBGMSG(std::cout, "Default device is: "s + std::to_string(device)) ;
    }
    if (device == 0) {
        //DBGMSG(std::cout, "Return false because: Could not find a default device"s);
        mydevice = 0 ;
        errorState = true ;
        return false ;
    }
    // Make sure this device exists ?
    if (!MusicController::deviceExists(device)) {
        //DBGMSG(std::cout, "Return false because: Does not exist device: "s + std::to_string(device));

        errorState = true ;
        return false ;
    }
    mydevice = device ;
    // Device exists, we are set and ready
    if (soundDac.isStreamOpen()) {
        this->stop(false) ;
    }
    rtParameters.deviceId = device ;
    rtParameters.nChannels = 2 ;
    auto status = soundDac.openStream(&rtParameters, NULL, RTAUDIO_SINT16, sampleRate, &bufferFrames, &MusicController::rtCallback,this) ;
    if (status == RTAUDIO_SYSTEM_ERROR ) {
        errorState = true ;
        return false ;
    }
    if (status == RTAUDIO_INVALID_USE) {
        //DBGMSG(std::cout, "Return false because: RTAUDIO_INVALID_USE");

        errorState = true ;
        return false ;
    }
    return true ;
}
// ======================================================================================================================
auto MusicController::isPlaying() const -> bool {
    return soundDac.isStreamRunning() ;
}

// ======================================================================================================================
auto MusicController::hasError() const -> bool {
    return errorState ;
}

// ======================================================================
auto MusicController::setDevice(int device) -> void {
    mydevice = device ;
}

// ======================================================================
auto MusicController::device() const -> int {
    return mydevice ;
}

// ======================================================================
auto MusicController::setEnabled(bool state) -> void {
    is_enabled = state ;
}

// ======================================================================
auto MusicController::isEnabled() const -> bool {
    return is_enabled ;
}

// ======================================================================
auto MusicController::currentLoaded() const -> std::string {
    return musicname  ;
}

// ======================================================================
auto MusicController::setMusicInformation(const std::filesystem::path &path, const std::string &musicExtension) -> void {
    this->musicExtension = musicExtension ;
    this->musicPath = path ;
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
        if (musicFile.isLoaded()) {
            musicFile.close();
        }
    }
    
}

// ======================================================================
auto MusicController::load(const std::filesystem::path &path) -> bool {
    errorState = false ;
    musicname = path.stem().string();
    if (musicFile.isLoaded()){
        musicFile.close() ;
    }
    if (isEnabled()){
        if (this->isPlaying()) {
            this->stop(true) ;
        }
        errorState = !musicFile.load(path) ;
    }
    return shouldPlay() ;
}

// ======================================================================
auto MusicController::load(const std::string &musicname) -> bool {
    this->musicname = musicname ;
    if (musicFile.isLoaded()){
        musicFile.close() ;
    }

    auto path = musicPath / std::filesystem::path(musicname+musicExtension) ;
    if (!std::filesystem::exists(path)){
        DBGMSG(std::cout, "File does not exist: "s + path.string()) ;
        errorState = true ;
        return false ;
    }
    return load(path);
}

// ======================================================================
auto MusicController::start(std::int32_t frame  ) -> bool {
    errorState = false ;
    if (shouldPlay()) {
        {
            auto lock = std::lock_guard(frameAccess);
            current_frame = frame ;
            musicFile.setFrame(frame) ;
        }
        // Now, start the playing
        if (!initialize(mydevice, musicFile.sampleRate())){
            errorState = true ;
            return false ;
        }
        soundDac.startStream() ;
    }
    
    return soundDac.isStreamRunning() ;
}



// Our two callbacks
// ======================================================================
auto MusicController::requestData(std::uint8_t *data,std::uint32_t frameCount, double time, RtAudioStreamFlags status ) -> int {
    if (!soundDac.isStreamRunning() || !soundDac.isStreamOpen() ) {
        return 2 ;
    }
    auto lock = std::lock_guard(frameAccess);
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

// =================================================================
auto MusicController::setSync(int frame) ->void {
    auto lock = std::lock_guard(frameAccess);
    auto delta = current_frame - frame ;
    if (std::abs(delta) < 3) {
        return ;
    }
    if (std::abs(delta) < 6) {
        if (delta > 0) {
            current_frame -= 1 ;
            musicFile.setFrame(current_frame);
        }
        else {
            current_frame += 1 ;
            musicFile.setFrame(current_frame);
        }
    }
    else {
        musicFile.setFrame(frame) ;
        current_frame = frame ;
    }
}

// Copyright © 2024 Charles Kerr. All rights reserved.

#include "LightController.hpp"

#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"

using namespace std::string_literals ;

// ===============================================================================
auto LightController::runThread() -> void {
    io_context.run() ;
}

// ==================================================================================================
auto LightController::tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void {
    if (ec != asio::error::operation_aborted) {
        auto frame = 0 ;
        {
            auto lock = std::lock_guard(frame_access);
            current_frame += 1 ;
            frame = current_frame ;
        }
        updateLight(frame);
        auto time = timer->expiry() ;
        timer->expires_at(time + std::chrono::milliseconds(framePeriod)) ;
        timer->async_wait(std::bind(&LightController::tick,this,std::placeholders::_1,timer) );
    }

}

// ===============================================================================
auto LightController::userSetEnabled(bool value) -> void {
    if (value) {
        // Lets check our firmware
#if defined (BEAGLE)
        //DBGMSG(std::cout, "checking firmware for pru 0");
        if(!pru0.checkFirmware()) {
            throw std::runtime_error("Incorrect firmware in pru 0: "s + pru0.firmware());
        }
        //DBGMSG(std::cout, "checking state for pru 0");
        if (!pru0.checkState()) {
            //DBGMSG(std::cout, "Throwing check state exception for pru 0");
            throw std::runtime_error("Incorrect state in pru 0: "s + pru0.state());
        }
        //DBGMSG(std::cout, "checking firmware for pru 1");
        if(!pru1.checkFirmware()) {
            throw std::runtime_error("Incorrect firmware in pru 1: "s + pru1.firmware());
        }
        //DBGMSG(std::cout, "checking state for pru 1");
        if (!pru1.checkState()) {
            //DBGMSG(std::cout, "Throwing check state exception for pru 1");
            throw std::runtime_error("Incorrect state in pru 1: "s + pru1.state());
        }
#endif
    }
}

// =============================================================================
auto LightController::clearLoaded() -> void {
    if (is_playing){
        this->stop() ;
    }
    if (lightFile.isLoaded()){
        lightFile.clear();
    }
    data_buffer = std::vector<std::uint8_t>() ;

    data_name = "" ;
    is_loaded = false ;
    has_error = false ;
#if defined(BEAGLE)
    pru0.clear();
    pru1.clear();
#endif
}

// ===============================================================================
auto LightController::updateLight(int frame ) -> void {
#if defined(BEAGLE)
    auto [data,length] = this->dataForFrame(frame);
    if (data != nullptr && length != 0 ){
        DBGMSG(std::cout, "We are telling pru to write: "s + std::to_string(length));
        pru0.setData(data, length);
        pru1.setData(data, length);
    }
#endif
}

// ===============================================================================
auto LightController::dataForFrame(int frame) -> std::pair<const std::uint8_t*,int> {
    const std::uint8_t *ptr = nullptr ;
    auto length = 0 ;
    if (is_loaded){
        length = lightFile.frameLength() ;
        ptr = lightFile.dataForFrame(frame);
    }
    return std::make_pair(ptr, length);
}
// ===============================================================================
LightController::LightController():IOController(),timer(io_context), pru0(PruNumber::zero), pru1(PruNumber::one), framePeriod(FRAMEPERIOD){
    
    timerThread = std::thread(&LightController::runThread,this) ;
}

// ===============================================================================
LightController::~LightController(){
    try {
        timer.cancel();
    }
    catch(...){}
    if (!io_context.stopped()) {
        io_context.stop() ;
    }
    if (timerThread.joinable()){
        timerThread.join();
    }
}

// ===============================================================================
auto LightController::setPRUInfo(const PRUConfig &config0,const PRUConfig &config1)-> void {
    pru0.setConfig(config0);
    pru1.setConfig(config1);
}

// =============================================================================
auto LightController::loadBuffer(const std::vector<std::uint8_t> &data) -> bool {
    if (!is_enabled){
        return true;
    }
    if (isPlaying()){
        return false ;
    }
    pru0.setData(data.data(), static_cast<int>(data.size()));
    pru1.setData(data.data(), static_cast<int>(data.size()));
    return true ;
}

// =============================================================================
auto LightController::load(const std::string &name) -> bool {
    clearLoaded() ;
    if (!is_enabled){
        return true ;
    }
    if (name.empty()) {
        // This is ok, we just are going to do nothing
        return true ;
    }
    auto path = data_location / std::filesystem::path(name + data_extension) ;
    if (!std::filesystem::exists(path)){
        has_error = true ;
        return false ;
    }
    is_loaded = lightFile.loadFile(path) ;
    has_error = !is_loaded ;
    return is_loaded;
}
// ===============================================================================
auto LightController::start(int frame, int period ) -> bool {
    framePeriod = period ;
    is_playing = false ;
    if ( !is_enabled ) {
        // We are going to respond yes, because we swallow this if we are not enabled
        is_playing = true ;
    }
    
    else if (has_error){
        return false ;
    }
    try{ timer.cancel();}catch(...){} ;
    {
        auto lock = std::lock_guard(frame_access) ;
        current_frame = frame ;
        
    }
    
    timer.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(framePeriod));
    timer.async_wait(std::bind(&LightController::tick,this,std::placeholders::_1,&timer) );
    is_playing = true ;
    return is_playing ;
}
// ===============================================================================
auto LightController::stop() -> void {
    if (is_playing){
        try {timer.cancel();} catch(...){}
    }
    is_playing = false ;
    
#if defined(BEAGLE)
    pru0.clear();
    pru1.clear();
#endif

}

// ===============================================================================
auto LightController::clear() -> void {
    clearLoaded();
}

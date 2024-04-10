// Copyright © 2024 Charles Kerr. All rights reserved.

#include "LightController.hpp"

// ===============================================================================
auto LightController::runThread() -> void {
    io_context.run() ;
}

// ===============================================================================
LightController::LightController():timer(io_context), pru0(PruNumber::zero), pru1(PruNumber::one), currentFrame(0),file_mode(true), is_enabled(false), has_error(false), current_frame(0), framePeriod(FRAMEPERIOD) {
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
auto LightController::setLightInfo(const std::filesystem::path &location, const std::string &extension) -> void {
    this->location = location ;
    this->extension = extension ;
}

// ===============================================================================
auto LightController::setPRUInfo(const PRUConfig &config0,const PRUConfig &config1)-> void {
    this->config0 = config0 ;
    this->config1 = config1 ;
    
}

// ===============================================================================
auto LightController::setEnabled(bool value) -> void {
    is_enabled = value ;
}
// ===============================================================================
auto LightController::isEnabled() const -> bool {
    return is_enabled ;
}

// =============================================================================
auto LightController::loadLight(const std::string &name) -> bool {
    current_loaded = name ;
    if (!is_enabled) {
        return true ;
    }
    try { timer.cancel() ;} catch(...){}
    {
        auto lock = std::lock_guard(frameAccess) ;
        currentFrame = 0 ;
    }
    has_error = false ;
    file_mode = true ;
    auto path = location / std::filesystem::path(name + extension) ;
    if (!std::filesystem::exists(path)){
        has_error = true ;
        return false ;
    }
    if (lightFile.isLoaded()){
        lightFile.clear();
    }
    has_error = lightFile.loadFile(path) ;
    return has_error ;
    
}

// ===============================================================================
auto LightController::currentLoaded() const -> const std::string& {
    return current_loaded ;
}
// ===============================================================================
auto LightController::start(int frame, int period ) -> bool {
    framePeriod = period ;
    
    if ((lightFile.isLoaded() || !file_mode) && is_enabled){
        try{ timer.cancel();}catch(...){} ;
        {
            auto lock = std::lock_guard(frameAccess) ;
            currentFrame = frame ;
           
        }
        timer.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(framePeriod));
        timer.async_wait(std::bind(&LightController::tick,this,std::placeholders::_1,&timer) );

    }
    return false ;
}
// ===============================================================================
auto LightController::stop() -> void {
    timer.cancel() ;
    {
        auto lock = std::lock_guard(frameAccess) ;
        currentFrame = 0 ;
    }

}
// ===============================================================================
auto LightController::setSync(int syncFrame) -> void {
    auto lock = std::lock_guard(frameAccess);
    auto delta = current_frame - syncFrame ;
    if (std::abs(delta) < 3) {
        return ;
    }
    if (std::abs(delta) < 6) {
        if (delta > 0) {
            current_frame -= 1 ;
            
        }
        else {
            current_frame += 1 ;
            
        }
    }
    else {

        current_frame = syncFrame ;
    }

}
// ===============================================================================
auto LightController::hasError() const -> bool {
    return has_error ;
}
// ==================================================================================================
auto LightController::tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void {
    if (ec != asio::error::operation_aborted) {
        // reschedule another
        {
            auto lock = std::lock_guard(frameAccess);
            current_frame += 1 ;
        }
        auto time = timer->expiry() ;
        timer->expires_at(time + std::chrono::milliseconds(framePeriod)) ;
        timer->async_wait(std::bind(&LightController::tick,this,std::placeholders::_1,timer) );
    }

}
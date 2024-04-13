// Copyright © 2024 Charles Kerr. All rights reserved.

#include "IOController.hpp"

#include <algorithm>
#include "utility/dbgutil.hpp"

using namespace std::string_literals;

// =======================================================================
IOController::IOController():is_loaded(false),has_error(false),is_enabled(false),current_frame(0),is_playing(false){
    
}

// =======================================================================
IOController::~IOController() {
    if(is_playing) {
        this->stop() ;
    }
    if (is_loaded) {
        this->clear();
    }
}

// =======================================================================
auto IOController::name() const -> const std::string& {
    return data_name ;
}

// ======================================================================
auto IOController::hasError() const -> bool {
    return has_error ;
}

// ======================================================================
auto IOController::isLoaded() const -> bool {
    return is_loaded ;
}

// ======================================================================
auto IOController::isEnabled() const -> bool {
    return is_enabled ;
}

// ======================================================================
auto IOController::isPlaying() const -> bool {
    return is_playing ;
}

// ======================================================================
auto IOController::setEnabled(bool state) -> void {
    is_enabled = state ;
    this->userSetEnabled(state);
}

// ======================================================================
auto IOController::setDataInformation(const std::filesystem::path &location, const std::string &extension) -> void {
    data_location = location ;
    data_extension = extension ;
}

// ======================================================================
auto IOController::stop() -> void {
    is_playing = false ;
}

// ======================================================================
auto IOController::clear() -> void {
    is_loaded = false ;
    data_name = "" ;
}

// ======================================================================
auto IOController::syncFrame(int sync_frame) -> void {
    auto lock = std::lock_guard(frame_access);
    auto delta = current_frame - sync_frame ;
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
        //DBGMSG(std::cout, "Resetting from to sync: "s + std::to_string(syncFrame));
        current_frame = sync_frame ;
    }
    userSetSync(current_frame);
}


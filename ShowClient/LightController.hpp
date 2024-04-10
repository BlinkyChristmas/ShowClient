// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef LightController_hpp
#define LightController_hpp

#include <string>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <filesystem>

#include "asio.hpp"

#include "bone/BlinkPru.hpp"
#include "lightfile/lightfile.hpp"
#include "PRUConfig.hpp"

class LightController {
    static constexpr int FRAMEPERIOD = 37 ;
    BlinkPru pru0 ;
    BlinkPru pru1 ;
    
    asio::io_context io_context;
    asio::executor_work_guard<asio::io_context::executor_type> timerguard{asio::make_work_guard(io_context)} ;
    asio::steady_timer timer ;
    auto tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void ;
    
    std::thread timerThread ;
    auto runThread() -> void ;
    
    int currentFrame ;
    mutable std::mutex frameAccess ;
    int framePeriod ;
    
    bool file_mode ;
    bool is_enabled ;
    
    bool has_error ;
    int current_frame ;
    
    std::filesystem::path location ;
    std::string extension ;
    
    LightFile lightFile ;
    PRUConfig config0 ;
    PRUConfig config1 ;
    std::string current_loaded ;
public:
    LightController() ;
    ~LightController() ;
    
    auto copyToPRU(PruNumber number, const std::uint8_t *data, int length, int offset = 0) -> bool ;
    auto start(int frame,int period = FRAMEPERIOD) -> bool ;
    auto stop() -> void ;
    auto setSync(int syncFrame) -> void ;
    auto loadLight(const std::string &name) -> bool ;
    auto setLightInfo(const std::filesystem::path &location, const std::string &extension) -> void ;
    auto setPRUInfo(const PRUConfig &config0,const PRUConfig &config1)-> void ;
    auto setEnabled(bool value) -> void ;
    auto isEnabled() const -> bool ;
    auto hasError() const -> bool ;
    auto currentLoaded() const -> const std::string& ;
};

#endif /* LightController_hpp */

// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef LightController_hpp
#define LightController_hpp

#include <string>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <filesystem>
#include <utility>

#include "asio.hpp"

#include "bone/BlinkPru.hpp"
#include "lightfile/lightfile.hpp"
#include "PRUConfig.hpp"
#include "IOController.hpp"

class LightController : public IOController {
    BlinkPru pru0 ;
    BlinkPru pru1 ;
    
    std::thread timerThread ;
    auto runThread() -> void ;

    asio::io_context io_context;
    asio::executor_work_guard<asio::io_context::executor_type> timerguard{asio::make_work_guard(io_context)} ;
    asio::steady_timer timer ;
    auto tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void ;
    
    int framePeriod ;
    
    bool file_mode ;
    
    LightFile lightFile ;
    PRUConfig config0 ;
    PRUConfig config1 ;
    std::vector<std::uint8_t> data_buffer ;
    
    auto userSetEnabled(bool state) -> void final;

    auto clearLoaded() -> void ;
    auto updateLight(int frame) -> void ;
    auto dataForFrame(int frame) -> std::pair<const std::uint8_t*,int> ;
 public:
    LightController() ;
    ~LightController() ;

    // Unique onese
    
    auto setPRUInfo(const PRUConfig &config0,const PRUConfig &config1)-> void ;
    auto loadBuffer(const std::vector<std::uint8_t> &data) -> bool ;
    

    auto load(const std::string &name) -> bool final ;
    auto start(int frame,int period = IOController::FRAMEPERIOD) -> bool  final;
    auto stop() -> void final ;
    auto clear() -> void final ;
};

#endif /* LightController_hpp */

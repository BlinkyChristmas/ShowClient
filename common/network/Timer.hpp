//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Timer_hpp
#define Timer_hpp

#include <cstdint>
#include <iostream>
#include <string>

#include "asio/asio.hpp"

//======================================================================
class Timer {
  
public:
    using UpdateCallback = std::function<void(Timer*)> ;
    
private:
    UpdateCallback callbackFunction ;
    int rate ;
    asio::steady_timer timer;
    
    auto tick(const asio::error_code &ec) -> void ;
    
public:
    
    Timer(asio::io_context &timer_context);
    ~Timer() ;
    auto setUpdateCallback( UpdateCallback function) -> void ;
    auto start(int update = 37) -> void ;
    auto stop() -> void ;
};

#endif /* Timer_hpp */

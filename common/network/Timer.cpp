//Copyright © 2024 Charles Kerr. All rights reserved.

#include "Timer.hpp"

#include <algorithm>
#include <stdexcept>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto Timer::tick(const asio::error_code &ec) -> void {
    if (ec) {
        // We got an error?
        // Do we need to cancel?
        //DBGMSG(std::cerr, "Timer error: "s + ec.message()) ;
        return ;
    }
    timer.expires_at( timer.expiry() + asio::chrono::milliseconds(rate) ) ;
    timer.async_wait(std::bind(&Timer::tick,this,std::placeholders::_1) ) ;
    if (callbackFunction != nullptr) {
        callbackFunction(this) ;
    }
}

//======================================================================
Timer::Timer(asio::io_context &timer_context):timer(timer_context),rate(37),callbackFunction(nullptr){
    
}

//======================================================================
Timer::~Timer() {
    try {
        timer.cancel() ;
    }
    catch(...){
        
    }
}

//======================================================================
auto Timer::stop() -> void {
    try {
        
        timer.cancel() ;
    }
    catch(...){
    }
}

//======================================================================
auto Timer::start(int update) -> void {
    rate = update ;
    timer.expires_after(asio::chrono::milliseconds( update )) ;
    timer.async_wait(std::bind(&Timer::tick,this,std::placeholders::_1) ) ;
}
//======================================================================
auto Timer::setUpdateCallback( UpdateCallback function) -> void {
    callbackFunction = function ;
}

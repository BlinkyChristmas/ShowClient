// Copyright © 2024 Charles Kerr. All rights reserved.

#include "FrameTimer.hpp"

#include <functional>
#include <algorithm>
#include <chrono>
#include <limits>

// ==================================================================================================
FrameTimer::FrameTimer(int frame_period):syncTimer(io_context),framePeriod(frame_period),updateCallBack(nullptr),stopCallBack(nullptr),settleFrame(7){}

// ==================================================================================================
FrameTimer::~FrameTimer() {
    try {
        syncTimer.cancel() ;
    }
    catch(...) {}
    if (io_thread.joinable()) {
        io_thread.join();
    }
}

// ==================================================================================================
auto FrameTimer::runIO() -> void {
    io_context.run() ;
}
// ==================================================================================================
auto FrameTimer::tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void {
    if (ec != asio::error::operation_aborted) {
        // reschedule another
        counter += 1 ;
        if (counter >= stopAt) {
            if (stopCallBack != nullptr) {
                stopCallBack(FrameValue(counter));
            }
            return ; // We need to return  here, so we dont requeue ourselves
        }
        else if (updateCallBack != nullptr  && ( ( counter % frequency == 0 )  || ( settleFrame == counter ) )  )  {
            updateCallBack(FrameValue(counter)) ;
        }
        auto time = timer->expiry() ;
        timer->expires_at(time + std::chrono::milliseconds(framePeriod)) ;
        timer->async_wait(std::bind(&FrameTimer::tick,this,std::placeholders::_1,timer) );
    }

}

// ==================================================================================================
auto FrameTimer::start(int frequency, int stop_frame ) -> void {
    counter = 0 ;
    this->frequency = std::min(frequency,1) ;
    stopAt = stop_frame ;
    io_context.restart();
    syncTimer.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(framePeriod));
    syncTimer.async_wait(std::bind(&FrameTimer::tick,this,std::placeholders::_1,&syncTimer) );
    io_thread = std::thread(&FrameTimer::runIO,this);
}

// ==================================================================================================
auto FrameTimer::stop() ->void {
    syncTimer.cancel() ;
    if (io_thread.joinable()) {
        io_thread.join();
        io_thread = std::thread() ;
    }
}

// ==================================================================================================
auto FrameTimer::setUpdateCallback(FrameTimerUpdateCallback routine) -> void {
    updateCallBack = routine;
}

// ==================================================================================================
auto FrameTimer::setStopCallback(FrameTimerUpdateCallback routine) -> void {
    stopCallBack = routine ;
}

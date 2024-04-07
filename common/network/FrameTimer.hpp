// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef FrameTimer_hpp
#define FrameTimer_hpp

#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "utility/timeutil.hpp"

#include "asio.hpp"

#include "FrameValue.hpp"
// ==================================================================================================
// A Frame timer
// ==================================================================================================

class FrameTimer;
using FrameTimerUpdateCallback = std::function<void(FrameValue)> ;
using FrameTimerStopCallback = std::function<void(FrameValue)> ;
class FrameTimer {
    asio::io_context io_context ;
    asio::steady_timer syncTimer ;
    int framePeriod;
    int counter ;
    int frequency ;
    int stopAt ;
    std::thread io_thread ;
    FrameTimerUpdateCallback updateCallBack ;
    FrameTimerStopCallback stopCallBack;
    auto tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void;
    auto runIO() -> void ;
public:
    int settleFrame ;

    FrameTimer(int frame_period = 37);
    ~FrameTimer() ;
    auto start(int frequency, int stop_frame = std::numeric_limits<int>::max()) -> void ;
    auto stop() ->void ;
    auto setUpdateCallback(FrameTimerUpdateCallback routine) -> void ;
    auto setStopCallback(FrameTimerUpdateCallback routine) -> void ;

};
#endif /* FrameTimer_hpp */

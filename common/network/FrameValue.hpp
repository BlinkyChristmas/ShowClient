// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef FrameValue_hpp
#define FrameValue_hpp

#include <mutex>
#include <queue>

#include "utility/timeutil.hpp"
// ==================================================================================================
// This represents the "sync value" (frame number) that the client should be on.
// It time stamps itself on creation, and allows for an adjustment when the value is called
// ==================================================================================================
class FrameValue {
    util::ourclock::time_point timestamp;
    int frame;
    bool noadjust ;
public:
    FrameValue(int frame_value = 0,bool no_adjust = false) ;
    auto value() const -> int ;
    auto rawValue() -> int ;
};

// ==================================================================================================
// A container for the syncs that occurr
// ==================================================================================================

class FrameContainer {
    mutable std::mutex access ;
    std::queue<FrameValue> dataHolder ;
public:
    auto empty() const -> bool ;
    auto clear() -> void ;
    auto push(const FrameValue &value) -> void ;
    auto pop() -> FrameValue ;
};

#endif /* FrameValue_hpp */

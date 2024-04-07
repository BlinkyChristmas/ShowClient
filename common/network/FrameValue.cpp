// Copyright © 2024 Charles Kerr. All rights reserved.

#include "FrameValue.hpp"

#include <algorithm>

// ==================================================================================================
// This represents the "sync value" (frame number) that the client should be on.
// It time stamps itself on creation, and allows for an adjustment when the value is called
// ==================================================================================================

// ==================================================================================================
FrameValue::FrameValue(int frame_value,bool no_adjust):frame(frame_value),noadjust(no_adjust),timestamp(util::ourclock::now()) {}

// ==================================================================================================
auto FrameValue::value() const -> int {
    if (noadjust) {
        return frame;
    }
    auto adjustment = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(util::ourclock::now() - timestamp).count()/37) ;
    return frame + adjustment ;
}

// ==================================================================================================
auto FrameValue::rawValue() -> int {
    return frame ;
}

// ==================================================================================================
// A container for the syncs that occurr
// ==================================================================================================
auto FrameContainer::empty() const -> bool {
    auto lock = std::lock_guard(access);
    return dataHolder.empty();
}

// ==================================================================================================
auto FrameContainer::clear() -> void {
    std::queue<FrameValue> empty;
    auto lock = std::lock_guard(access);

    std::swap(empty,dataHolder);
}

// ==================================================================================================
auto FrameContainer::push(const FrameValue &value) -> void {
    auto lock = std::lock_guard(access);
    dataHolder.push(value) ;
}

// ==================================================================================================
auto FrameContainer::pop() -> FrameValue {
    auto lock = std::lock_guard(access);
    if (dataHolder.empty()) {
        throw std::runtime_error("Attempt to retrieve FrameValue from an empty container");
    }
    auto value = dataHolder.front();
    dataHolder.pop();
    return value;
}

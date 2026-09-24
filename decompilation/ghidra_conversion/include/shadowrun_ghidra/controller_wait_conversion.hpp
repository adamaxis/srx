#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ControllerWaitHooks {
    std::function<void()> wait_for_frame;
    std::function<void()> latch_events;
    std::function<void()> sample_input;
    std::function<void()> update;
    std::function<std::uint16_t()> event_word;
};

// ROM 0xE00A and 0xF312: service one frame at a time until the controller
// event word has a high-byte event, then take one final input sample. The two
// source entries differ only in the update routine placed in this boundary;
// the host loop accepts that work as an explicit callback.
std::uint32_t wait_for_controller_event(const ControllerWaitHooks& hooks);

} // namespace shadowrun::ghidra_conversion

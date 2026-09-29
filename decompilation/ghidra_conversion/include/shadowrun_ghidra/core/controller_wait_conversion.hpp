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

struct ControllerLatchState {
    std::uint16_t events = 0; // FFDD22
    std::uint16_t extra_events = 0; // FFDD26
    std::int16_t confirm_latch = 0; // FFDDC0
    std::int16_t start_latch = 0; // FFDDC2
};

// ROM 0x0BF2: if (FFDD22|FFDD26) high byte is set, FFDDC0=-1. Bit 15 of
// FFDD22 then sets FFDDC2=-1. Returns the masked high-byte word.
std::uint16_t latch_controller_events(ControllerLatchState& state);

} // namespace shadowrun::ghidra_conversion

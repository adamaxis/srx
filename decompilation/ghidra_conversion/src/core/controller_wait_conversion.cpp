#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint32_t wait_for_controller_event(
    const ControllerWaitHooks& hooks) {
    std::uint32_t serviced_frames = 0;
    do {
        hooks.wait_for_frame();
        hooks.latch_events();
        hooks.sample_input();
        hooks.update();
        ++serviced_frames;
    } while ((hooks.event_word() & 0xFF00U) == 0U);

    hooks.sample_input();
    return serviced_frames;
}

std::uint16_t latch_controller_events(ControllerLatchState& state) {
    const auto masked = static_cast<std::uint16_t>(
        (state.events | state.extra_events) & 0xFF00U);
    if (masked == 0U) {
        return 0U;
    }
    state.confirm_latch = -1;
    if ((state.events & 0x8000U) != 0U) {
        state.start_latch = -1;
    }
    return masked;
}

void sample_controller_input(ControllerSampleState& state,
                             const std::uint8_t port_byte) {
    const auto current = static_cast<std::uint16_t>(port_byte);
    const auto newly = static_cast<std::uint16_t>(
        (current ^ state.held) & current);
    state.held = current;
    state.events = static_cast<std::uint16_t>(current | (newly << 8U));
    state.heading = kControllerHeadingTable[current & 0x0FU];
}

} // namespace shadowrun::ghidra_conversion

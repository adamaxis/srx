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

} // namespace shadowrun::ghidra_conversion

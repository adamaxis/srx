#include "shadowrun_ghidra/timing_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void wait_for_vdp_ready(const VdpReadyCallbacks& callbacks) {
    if (callbacks.warmup_delay) {
        callbacks.warmup_delay(11U * 4096U);
    }
    if (callbacks.wait_until_ready) {
        callbacks.wait_until_ready();
    }
    if (callbacks.settle_delay) {
        callbacks.settle_delay(256U);
    }
}

bool wait_for_vblank(VblankFrameState& state,
                     const NextVblankBoundary& next_vblank) {
    ++state.frame_count;
    if (!next_vblank) {
        return false;
    }
    state.observed_sample = next_vblank(state.observed_sample);
    return true;
}

std::uint32_t wait_for_palette_frame(
    VblankFrameState& state,
    const std::uint16_t source_counter,
    const NextVblankBoundary& next_vblank,
    const ControllerLatchBoundary& latch_controller_events) {
    const auto iterations = static_cast<std::uint32_t>(source_counter) + 1U;
    for (std::uint32_t iteration = 0; iteration < iterations; ++iteration) {
        wait_for_vblank(state, next_vblank);
        if (latch_controller_events) {
            latch_controller_events();
        }
    }
    return iterations;
}

} // namespace shadowrun::ghidra_conversion

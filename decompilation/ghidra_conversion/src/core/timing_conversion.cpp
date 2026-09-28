#include "shadowrun_ghidra/core/timing_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool wait_for_even_frame(
    EvenFrameState& state,
    const NextEvenFrameBoundary& next_even_frame) {
    if (!next_even_frame) {
        return false;
    }
    state.observed_even_frame = next_even_frame(state.observed_even_frame) &
                                ~1U;
    return true;
}

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

std::uint32_t wait_for_vblank_frames(
    VblankFrameState& state,
    const std::uint16_t source_counter,
    const NextVblankBoundary& next_vblank) {
    const auto iterations = static_cast<std::uint32_t>(source_counter) + 1U;
    for (std::uint32_t iteration = 0; iteration < iterations; ++iteration) {
        wait_for_vblank(state, next_vblank);
    }
    return iterations;
}

std::uint32_t wait_for_controller_event(
    VblankFrameState& state,
    const ControllerWaitCallbacks& callbacks) {
    if (!callbacks.next_vblank || !callbacks.latch_controller_events ||
        !callbacks.sample_controller_input) {
        return 0U;
    }

    std::uint32_t iterations = 0U;
    std::uint16_t event_word = 0U;
    do {
        wait_for_vblank(state, callbacks.next_vblank);
        callbacks.latch_controller_events();
        event_word = callbacks.sample_controller_input();
        ++iterations;
    } while ((event_word & 0xFF00U) == 0U);

    callbacks.sample_controller_input();
    return iterations;
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

int consume_host_frames(std::int64_t& deadline_ns,
                        const std::int64_t elapsed_ns) {
    int frames = 0;
    while (deadline_ns < elapsed_ns && frames < kHostFrameCatchUpLimit) {
        deadline_ns += kHostFrameNanoseconds;
        ++frames;
    }
    return frames;
}

} // namespace shadowrun::ghidra_conversion

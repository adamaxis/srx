#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct VdpReadyCallbacks {
    std::function<void(std::uint32_t source_delay)> warmup_delay;
    std::function<void()> wait_until_ready;
    std::function<void(std::uint32_t source_delay)> settle_delay;
};

// ROM 0x073A: collapse the 11 * 4096-iteration startup delay and the
// 256-iteration post-ready delay into host timing boundaries. The Genesis
// status-bit poll is represented by wait_until_ready; it is not a runtime VDP
// loop.
void wait_for_vdp_ready(const VdpReadyCallbacks& callbacks);

struct VblankFrameState {
    std::uint32_t frame_count = 0;
    std::uint16_t observed_sample = 0;
};

using NextVblankBoundary =
    std::function<std::uint16_t(std::uint16_t previous_sample)>;

// ROM 0x0BD2: count a frame and obtain the next interrupt-side sample from a
// host frame boundary. The source's equality spin is intentionally absent.
bool wait_for_vblank(VblankFrameState& state,
                     const NextVblankBoundary& next_vblank);

using ControllerLatchBoundary = std::function<void()>;

// ROM 0x1342: the source uses DBF, so a starting counter N performs N + 1
// vblank/latch iterations. Returns the number of iterations performed.
std::uint32_t wait_for_palette_frame(
    VblankFrameState& state,
    std::uint16_t source_counter,
    const NextVblankBoundary& next_vblank,
    const ControllerLatchBoundary& latch_controller_events);

} // namespace shadowrun::ghidra_conversion

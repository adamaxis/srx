#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeDisplayResetState {
    std::uint32_t primary_transfer_marker = 0;
    std::uint32_t secondary_transfer_marker = 0;
};

struct ModeDisplayResetCallbacks {
    std::function<void()> clear_plane_a;
    std::function<void()> clear_plane_b;
    std::function<void()> wait_for_vblank;
    std::function<void()> reset_upload_state;
    std::function<void()> write_transfer_header;
    std::function<void()> write_fixed_transfer_pair;
};

// ROM 0x10952: clear both display planes, wait for the frame boundary, reset
// upload state and markers, then restore the fixed transfer setup.
void reset_mode_display_buffers(
    ModeDisplayResetState& state,
    const ModeDisplayResetCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

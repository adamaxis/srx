#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class DisplayTransitionPhase {
    initial,
    sweep,
    settle,
    clear_one,
    clear_two,
    clear_three,
    final_wait,
};

enum class DisplayTransitionTransferStyle {
    masked,
    masked_with_display_control,
};

enum class DisplayTransitionTarget {
    initial_background,
    initial_display_panel,
    initial_overlay,
    sweep_primary,
    sweep_secondary,
    sweep_display_primary,
    sweep_display_secondary,
    settle_primary,
    settle_secondary,
    settle_overlay,
    clear_one_overlay,
    clear_two_overlay,
    clear_three_overlay,
};

struct DisplayTransitionTransferRequest {
    DisplayTransitionPhase phase = DisplayTransitionPhase::initial;
    DisplayTransitionTransferStyle style =
        DisplayTransitionTransferStyle::masked;
    DisplayTransitionTarget target =
        DisplayTransitionTarget::initial_background;
    std::uint16_t iteration = 0;
    std::uint16_t word_count = 1;
};

struct DisplayTransitionState {
    std::int8_t audio_marker = -1; // FFF0B1
    bool display_transfer_dirty = false; // FFEF14 bit 0
    DisplayTransitionPhase phase = DisplayTransitionPhase::initial;
    bool completed = false;
};

struct DisplayTransitionCallbacks {
    std::function<void(std::uint8_t)> play_sfx_if_enabled;
    std::function<void()> upload_initial_asset;
    std::function<void()> reset_mode_display_buffers;
    std::function<void()> copy_transition_asset;
    std::function<void(const DisplayTransitionTransferRequest&)>
        write_masked_transfer;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
};

// ROM 0x26A00: run the fixed display-transition choreography while exposing
// assets, transfer targets, timing, and input sampling as native boundaries.
void run_display_transition(
    DisplayTransitionState& state,
    const DisplayTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/rendering/display_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void tick(const DisplayTransitionCallbacks& callbacks) {
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.latch_controller_events) {
        callbacks.latch_controller_events();
    }
    if (callbacks.sample_controller_input) {
        callbacks.sample_controller_input();
    }
}

void wait_ticks(const DisplayTransitionCallbacks& callbacks,
                const std::uint16_t count) {
    for (std::uint16_t index = 0; index < count; ++index) {
        tick(callbacks);
    }
}

void transfer(const DisplayTransitionCallbacks& callbacks,
              const DisplayTransitionPhase phase,
              const DisplayTransitionTransferStyle style,
              const DisplayTransitionTarget target,
              const std::uint16_t iteration,
              const std::uint16_t word_count) {
    if (callbacks.write_masked_transfer) {
        callbacks.write_masked_transfer(
            {phase, style, target, iteration, word_count});
    }
}

} // namespace

void run_display_transition(
    DisplayTransitionState& state,
    const DisplayTransitionCallbacks& callbacks) {
    state.audio_marker = -1;
    if (callbacks.service_audio_if_enabled) {
        callbacks.service_audio_if_enabled(0x29U);
    }
    if (callbacks.upload_initial_asset) {
        callbacks.upload_initial_asset();
    }
    if (callbacks.reset_mode_display_buffers) {
        callbacks.reset_mode_display_buffers();
    }
    if (callbacks.copy_transition_asset) {
        callbacks.copy_transition_asset();
    }
    state.display_transfer_dirty = true;

    transfer(callbacks, DisplayTransitionPhase::initial,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::initial_background, 0U, 40U);
    transfer(callbacks, DisplayTransitionPhase::initial,
             DisplayTransitionTransferStyle::masked_with_display_control,
             DisplayTransitionTarget::initial_display_panel, 0U, 40U);
    transfer(callbacks, DisplayTransitionPhase::initial,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::initial_overlay, 0U, 1U);

    state.phase = DisplayTransitionPhase::sweep;
    for (std::uint16_t iteration = 0U; iteration < 14U; ++iteration) {
        tick(callbacks);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked,
                 DisplayTransitionTarget::sweep_primary, iteration, 1U);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked,
                 DisplayTransitionTarget::sweep_secondary, iteration, 1U);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked_with_display_control,
                 DisplayTransitionTarget::sweep_display_primary, iteration, 1U);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked_with_display_control,
                 DisplayTransitionTarget::sweep_display_secondary, iteration,
                 1U);
    }

    state.phase = DisplayTransitionPhase::settle;
    for (std::uint16_t iteration = 0U; iteration < 6U; ++iteration) {
        tick(callbacks);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked,
                 DisplayTransitionTarget::settle_primary, iteration, 1U);
        transfer(callbacks, state.phase,
                 DisplayTransitionTransferStyle::masked,
                 DisplayTransitionTarget::settle_secondary, iteration, 1U);
    }
    transfer(callbacks, state.phase,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::settle_overlay, 0U, 1U);

    state.phase = DisplayTransitionPhase::clear_one;
    wait_ticks(callbacks, 11U);
    transfer(callbacks, state.phase,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::clear_one_overlay, 0U, 1U);
    state.phase = DisplayTransitionPhase::clear_two;
    wait_ticks(callbacks, 6U);
    transfer(callbacks, state.phase,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::clear_two_overlay, 0U, 1U);
    state.phase = DisplayTransitionPhase::clear_three;
    wait_ticks(callbacks, 6U);
    transfer(callbacks, state.phase,
             DisplayTransitionTransferStyle::masked,
             DisplayTransitionTarget::clear_three_overlay, 0U, 1U);
    state.phase = DisplayTransitionPhase::final_wait;
    wait_ticks(callbacks, 11U);
    state.completed = true;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/matrix_menu_service_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

bool clear_matrix_program_slot(
    MatrixProgramSlotCleanupState& state,
    const std::size_t slot,
    const MatrixProgramSlotCleanupCallbacks& callbacks) {
    if (slot >= state.program_codes.size() ||
        slot >= state.visual_states.size()) {
        return false;
    }

    switch (state.program_codes[slot]) {
    case 3U:
        state.menu_flags = static_cast<std::uint8_t>(state.menu_flags & 0xDFU);
        state.pending_status = 0;
        break;
    case 5U:
        state.secondary_status = 0;
        break;
    case 6U:
        state.menu_flags = static_cast<std::uint8_t>(state.menu_flags & 0xFBU);
        break;
    case 7U:
        state.menu_flags = static_cast<std::uint8_t>(state.menu_flags & 0xFDU);
        state.effect_marker = 0;
        break;
    default:
        break;
    }

    state.program_codes[slot] = 0xFFU;
    state.visual_states[slot] = 6U;
    call_once(callbacks.write_repeated_grid);
    call_once(callbacks.write_grid);
    call_once(callbacks.render_rows);
    return true;
}

MatrixProgramLoadingEvent advance_matrix_program_loading(
    MatrixProgramLoadingState& state,
    const MatrixProgramLoadingCallbacks& callbacks) {
    state.overlay_countdown =
        static_cast<std::uint8_t>(state.overlay_countdown - 1U);
    MatrixProgramLoadingEvent event = MatrixProgramLoadingEvent::none;
    if ((state.overlay_countdown & 0x80U) != 0U) {
        state.overlay_countdown = 0x28U;
        call_once(callbacks.render_rows);
        event = MatrixProgramLoadingEvent::overlay_refreshed;
    }

    call_once(callbacks.advance_overlay_timer);
    if (!state.loading_active) {
        return event;
    }

    --state.progress_delay;
    if (state.progress_delay >= 0) {
        return event;
    }

    for (std::size_t slot = 0; slot < state.visual_states.size(); ++slot) {
        if (state.visual_states[slot] == 6U) {
            continue;
        }

        state.selected_slot = static_cast<std::uint8_t>(slot);
        ++state.visual_states[slot];
        if (callbacks.write_slot_grid) {
            callbacks.write_slot_grid(slot);
        }
        state.progress_delay = state.slot_delays[slot];
        if (state.visual_states[slot] != 6U) {
            return MatrixProgramLoadingEvent::slot_advanced;
        }

        if (callbacks.publish_completed_selection) {
            callbacks.publish_completed_selection(slot);
        }
        call_once(callbacks.render_selection_text);
        return MatrixProgramLoadingEvent::slot_completed;
    }

    return event;
}

HudModeTransitionEvent update_hud_mode_transition(
    HudModeTransitionState& state,
    const std::int8_t delta,
    const HudModeTransitionCallbacks& callbacks) {
    const auto summed = static_cast<int>(state.mode_variant) +
                        static_cast<int>(delta);
    bool drove_upward = false;
    if (summed != 0 && summed != 1) {
        if (summed < 2) {
            state.mode_variant = -1;
        } else {
            state.mode_variant = 1;
            drove_upward = true;
        }
    }

    if (drove_upward && state.transition_gate != 0) {
        call_once(callbacks.advance_fade);
        call_once(callbacks.run_world_effects);
        call_once(callbacks.render_mode_text);
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.wait_for_controller);
        return HudModeTransitionEvent::completed_transition;
    }

    if (state.transition_gate < 0) {
        return HudModeTransitionEvent::unchanged;
    }

    call_once(callbacks.render_mode_text);
    if (state.published_variant == state.mode_variant) {
        return HudModeTransitionEvent::rendered;
    }

    state.published_variant = state.mode_variant;
    if (callbacks.publish_audio_variant) {
        callbacks.publish_audio_variant(state.mode_variant);
    }
    return HudModeTransitionEvent::audio_published;
}

} // namespace shadowrun::ghidra_conversion

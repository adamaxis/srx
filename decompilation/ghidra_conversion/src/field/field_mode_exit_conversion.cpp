#include "shadowrun_ghidra/field/field_mode_exit_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldModeExitAction prepare_field_mode_exit(
    FieldModeExitState& state,
    const FieldModeExitCallbacks& callbacks) {
    const bool scene_zero = state.scene_id == 0U && state.submode == 0U;
    const bool scene_two = state.scene_id == 2U && state.submode == 1U;
    const bool scene_one = state.scene_id == 1U && state.submode == 6U;
    const bool scene_four = state.scene_id == 4U && state.submode == 4U;
    if (!scene_zero && !scene_two && !scene_one && !scene_four) {
        return FieldModeExitAction::no_matching_route;
    }

    if (scene_zero) {
        state.menu_mode = 0x0BU;
        const bool guarded_world_state =
            state.alternate_result == 0 &&
            (state.mode_flags & (1UL << 3U)) == 0U &&
            (state.world_flags & (1UL << 23U)) != 0U &&
            (state.world_flags & (1UL << 24U)) != 0U;
        if (guarded_world_state &&
            (state.world_flags & (1UL << 25U)) == 0U) {
            state.effect_flags =
                (state.effect_flags & ~(1UL << 18U)) | (1UL << 19U);
            state.effect_marker = 0x19U;
            if (callbacks.mark_effect_bit) {
                callbacks.mark_effect_bit(state.effect_marker);
            }
        } else if (guarded_world_state &&
                   (state.world_flags & (1UL << 25U)) != 0U &&
                   (state.world_flags & (1UL << 29U)) == 0U &&
                   (state.service_flags & 0x01U) != 0U) {
            state.service_flags &= ~0x01U;
            state.service_options |= 0x0200U;
            state.effect_flags &= ~(1UL << 20U);
            state.render_flags &=
                ~((1UL << 2U) | (1UL << 3U) | (1UL << 11U));
            state.world_flags |= (1UL << 29U);
        }
    } else if (scene_two) {
        state.menu_mode = 0x08U;
    } else if (scene_one) {
        state.menu_mode = 0x09U;
    } else {
        state.menu_mode = 0x0AU;
    }

    state.alternate_result = 0;
    if (callbacks.run_mode_exit_services) {
        callbacks.run_mode_exit_services(state);
    }
    if (state.alternate_result != 0) {
        state.alternate_result = 0;
        if (!callbacks.run_hospital_options_menu) {
            return FieldModeExitAction::hospital_options_menu_unresolved;
        }
        callbacks.run_hospital_options_menu();
        return FieldModeExitAction::hospital_options_menu_entered;
    }
    state.caller_exit_status = -1;
    return FieldModeExitAction::services_completed;
}

} // namespace shadowrun::ghidra_conversion

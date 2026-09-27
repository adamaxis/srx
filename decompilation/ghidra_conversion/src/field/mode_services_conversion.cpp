#include "shadowrun_ghidra/field/mode_services_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_scene_mode_state(
    SceneModeInitializationState& state,
    const SceneModeInitializationCallbacks& callbacks) {
    state.current_mode = callbacks.prepare_mode_entry
        ? callbacks.prepare_mode_entry()
        : 0U;
    state.scene_transition_value = 0U;
    state.scene_mode_marker = 0xFFU;
    state.camera_state = 0xFFFFU;
    state.special_mode_flag = 0U;
    state.special_mode_indicator =
        static_cast<std::uint8_t>(state.current_mode == 6U);
}

void finalize_mode_exit(ModeExitState& state,
                        const ModeExitCallbacks& callbacks) {
    state.exit_service_marker = 0U;
    if (callbacks.service_mode_exit) {
        callbacks.service_mode_exit();
    }
}

void service_mode_world_state(
    ModeWorldRefreshState& state,
    const ModeWorldRefreshCallbacks& callbacks) {
    state.random_samples = 0U;
    const auto sample = [&](const std::uint16_t bound) {
        ++state.random_samples;
        if (!callbacks.sample_random || bound == 0U) {
            return static_cast<std::uint16_t>(0U);
        }
        const std::uint16_t value = callbacks.sample_random(bound);
        return value < bound ? value : static_cast<std::uint16_t>(bound - 1U);
    };

    const std::uint16_t first = sample(2U);
    const std::uint16_t second = sample(2U);
    state.plan = {};
    state.plan.first_selection_nonzero = first != 0U;
    state.plan.second_selection_nonzero = second != 0U;
    if (first == 0U) {
        state.plan.variant_source =
            ModeWorldVariantSource::fixed_three_variant_resources;
        state.plan.primary_entry_index =
            static_cast<std::uint8_t>(sample(3U));
        state.plan.variant_entry_index =
            static_cast<std::uint8_t>(sample(3U));
        if (second == 0U) {
            state.plan.primary_source =
                ModeWorldPrimarySource::first_zero_second_zero_table;
            state.plan.variant_byte_offset = 4U;
        } else {
            state.plan.primary_source =
                ModeWorldPrimarySource::first_zero_second_nonzero_table;
        }
    } else {
        state.plan.variant_source =
            ModeWorldVariantSource::indexed_four_variant_resources;
        state.plan.primary_entry_index =
            static_cast<std::uint8_t>(sample(3U));
        state.plan.variant_entry_index =
            static_cast<std::uint8_t>(sample(4U));
        state.plan.grid_row_count = 1U;
        if (second == 0U) {
            state.plan.primary_source =
                ModeWorldPrimarySource::first_nonzero_second_zero_table;
        } else {
            state.plan.primary_source =
                ModeWorldPrimarySource::first_nonzero_second_nonzero_table;
            state.plan.variant_byte_offset = 2U;
        }
    }

    state.grid_write_requested = true;
    if (callbacks.write_world_grid) {
        callbacks.write_world_grid(state.plan);
    }
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_field_mode(
    FieldModeInitializationState& state,
    const FieldModeProfile profile,
    const FieldModeInitializationCallbacks& callbacks) {
    state.profile = profile;
    if (callbacks.select_profile_data) {
        callbacks.select_profile_data(profile);
    }
    if (callbacks.prepare_mode_entry) {
        callbacks.prepare_mode_entry();
    }
    state.entry_transient = {};
    state.world_effects_active = false;
    if (callbacks.initialize_scene_mode_state) {
        callbacks.initialize_scene_mode_state();
    }
}

FieldModeTransitionPlan advance_field_mode_transition(
    const FieldModeTransitionInput& input,
    const FieldModeTransitionCallbacks& callbacks) {
    const auto column = static_cast<std::int16_t>(
        static_cast<std::int8_t>(
            static_cast<std::uint8_t>(input.field_mode - 8U)));
    const auto row = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(input.mode_index) << 2));
    const auto table_offset =
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(row) +
                                   static_cast<std::uint16_t>(column));
    FieldModeTransitionPlan plan{};
    if (table_offset < kFieldModeTransitionModes.size()) {
        plan.next_mode_index = static_cast<std::int16_t>(
            static_cast<std::int8_t>(kFieldModeTransitionModes[table_offset]));
    }
    if (plan.next_mode_index == 0x13 &&
        (input.world_activity_lock & 0x100U) == 0U) {
        plan.start_world_activity = true;
        plan.world_activity_descriptor = kFieldWorldActivityDescriptor;
        plan.world_activity_handler = kFieldWorldActivityHandler;
        if (callbacks.begin_world_activity) {
            callbacks.begin_world_activity();
        }
        if (callbacks.schedule_timeout) {
            callbacks.schedule_timeout();
        }
    }
    const auto scene_index =
        static_cast<std::uint16_t>(plan.next_mode_index);
    if (scene_index < kFieldModeTransitionScenes.size()) {
        plan.scene_id = kFieldModeTransitionScenes[scene_index];
    }
    std::uint8_t select = 0U;
    if (table_offset < kFieldModeTransitionSelects.size()) {
        select = kFieldModeTransitionSelects[table_offset];
    }
    std::int16_t group = static_cast<std::int16_t>(plan.scene_id);
    if (plan.scene_id < 0x2CU) {
        group = static_cast<std::int16_t>(group - 6);
    } else {
        group = static_cast<std::int16_t>(group - 0x28);
    }
    const auto word_index =
        static_cast<std::size_t>(static_cast<std::uint16_t>(group) * 8U) +
        static_cast<std::size_t>(select) * 2U;
    if (word_index + 1U < kFieldModeTransitionOverrides.size()) {
        plan.override_x = kFieldModeTransitionOverrides[word_index];
        plan.override_y = kFieldModeTransitionOverrides[word_index + 1U];
    }
    return plan;
}

} // namespace shadowrun::ghidra_conversion

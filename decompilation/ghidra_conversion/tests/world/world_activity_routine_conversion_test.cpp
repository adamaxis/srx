#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

shadowrun::ghidra_conversion::WorldActivityRoutineCallbacks
fixed_resource_random_callbacks(const std::uint16_t sampled_index,
                                const std::uint16_t marker_random_word) {
    shadowrun::ghidra_conversion::WorldActivityRoutineCallbacks callbacks{};
    callbacks.sample_source_random_bound =
        [sampled_index](const std::uint16_t) {
            return std::optional<std::uint16_t>{sampled_index};
        };
    callbacks.advance_source_prng = [marker_random_word] {
        return std::optional<std::uint16_t>{marker_random_word};
    };
    return callbacks;
}

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 32> event_mode_table{};
    event_mode_table[0] = 0x15U;
    event_mode_table[0x1AU] = 0x14U;
    if (!expect(resolve_world_activity_scene_id(0U, event_mode_table) ==
                    std::optional<std::uint16_t>(0x15U) &&
                resolve_world_activity_scene_id(0x1AU, event_mode_table) ==
                    std::optional<std::uint16_t>(0x14U) &&
                !resolve_world_activity_scene_id(32U, event_mode_table)
                     .has_value())) {
        return 7;
    }

    WorldActivityRoutineInput resource_input{};
    resource_input.initial_scene_id = 0x20U;
    resource_input.default_resource_word = 0x1234U;
    resource_input.resource_random_index_bound = 6U;
    resource_input.resource_cursor_source = {-2, 0x4567U};
    resource_input.resource_overrides.force_index_two_marker_three = true;
    WorldActivityRoutineState resource_state{};
    std::array<std::uint8_t, 3> resource_callback_order{};
    std::size_t resource_callback_count = 0U;
    std::uint16_t observed_resource_bound = 0U;
    WorldActivityRoutineCallbacks resource_callbacks{};
    resource_callbacks.sample_source_random_bound =
        [&](const std::uint16_t bound) {
            observed_resource_bound = bound;
            resource_callback_order[resource_callback_count++] = 1U;
            return std::optional<std::uint16_t>{4U};
        };
    resource_callbacks.advance_source_prng = [&] {
        resource_callback_order[resource_callback_count++] = 2U;
        return std::optional<std::uint16_t>{0x000BU};
    };
    int resource_uploads = 0;
    resource_callbacks.upload_mode_word_block = [&] {
        resource_callback_order[resource_callback_count++] = 3U;
        ++resource_uploads;
    };
    const auto resource_result = run_world_activity_routine(
        resource_input, resource_state, resource_callbacks);
    if (!expect(resource_result == WorldActivityResult::resource_activity_ready &&
                resource_state.mode == WorldActivityMode::resource_activity &&
                resource_state.resource_random_index_bound == 6U &&
                resource_state.selected_resource_random_index == 2U &&
                resource_state.resource_marker_ordinal == 3U &&
                resource_state.resource_cursor.cursor == -2 &&
                resource_state.resource_cursor.resource_index == 0x4567U &&
                resource_state.world_activity_effect_words_cleared &&
                resource_uploads == 1 &&
                observed_resource_bound == 6U &&
                resource_callback_count == 3U &&
                resource_callback_order ==
                    std::array<std::uint8_t, 3>{1U, 2U, 3U})) {
        return 1;
    }

    WorldActivityRoutineInput overridden_resource_input{};
    overridden_resource_input.initial_scene_id = 0x15U;
    overridden_resource_input.resource_random_index_bound = 5U;
    overridden_resource_input.resource_overrides.force_index_five_marker_zero =
        true;
    overridden_resource_input.resource_overrides.force_index_two_marker_three =
        true;
    overridden_resource_input.resource_overrides.force_index_four = true;
    WorldActivityRoutineState overridden_resource_state{};
    const auto overridden_resource_result = run_world_activity_routine(
        overridden_resource_input, overridden_resource_state,
        fixed_resource_random_callbacks(4U, 0x000AU));
    if (!expect(overridden_resource_result ==
                    WorldActivityResult::resource_activity_ready &&
                overridden_resource_state.resource_random_index_bound == 5U &&
                overridden_resource_state.selected_resource_random_index == 4U &&
                overridden_resource_state.resource_marker_ordinal == 3U)) {
        return 16;
    }

    overridden_resource_input.resource_overrides = {};
    overridden_resource_input.resource_overrides
        .force_index_five_marker_zero = true;
    overridden_resource_state = WorldActivityRoutineState{};
    if (!expect(run_world_activity_routine(
                    overridden_resource_input,
                    overridden_resource_state,
                    fixed_resource_random_callbacks(4U, 0x000AU)) ==
                    WorldActivityResult::resource_activity_ready &&
                overridden_resource_state.selected_resource_random_index ==
                    5U &&
                overridden_resource_state.resource_marker_ordinal == 0U)) {
        return 19;
    }

    WorldActivityRoutineInput unavailable_resource_input{};
    unavailable_resource_input.initial_scene_id = 0x15U;
    WorldActivityRoutineState unavailable_resource_state{};
    if (!expect(run_world_activity_routine(
                    unavailable_resource_input,
                    unavailable_resource_state) ==
                    WorldActivityResult::
                        resource_activity_bound_unavailable &&
                unavailable_resource_state.mode ==
                    WorldActivityMode::resource_activity)) {
        return 17;
    }

    unavailable_resource_input.resource_random_index_bound = 5U;
    if (!expect(run_world_activity_routine(
                    unavailable_resource_input,
                    unavailable_resource_state) ==
                    WorldActivityResult::
                        resource_activity_random_input_unavailable)) {
        return 20;
    }

    if (!expect(run_world_activity_routine(
                    unavailable_resource_input,
                    unavailable_resource_state,
                    fixed_resource_random_callbacks(5U, 0x000AU)) ==
                    WorldActivityResult::
                        resource_activity_sample_out_of_range)) {
        return 18;
    }

    WorldActivityRoutineInput gated_input{};
    gated_input.initial_scene_id = 0x0DU;
    WorldActivityRoutineState gated_state{};
    if (!expect(run_world_activity_routine(gated_input, gated_state) ==
                    WorldActivityResult::blocked_by_scene_gate &&
                gated_state.mode == WorldActivityMode::field &&
                gated_state.world_activity_effect_words_cleared)) {
        return 2;
    }

    WorldActivityRoutineInput script_input{};
    script_input.initial_scene_id = 0x0CU;
    script_input.primary_runner_marker = 0xFFU;
    script_input.runner_slot_by_scene = {4, 2, 5};
    script_input.world_script_available = {false, false, true};
    WorldActivityRoutineState script_state{};
    const auto script_result = run_world_activity_routine(
        script_input, script_state);
    if (!expect(script_result == WorldActivityResult::world_script_selected &&
                script_state.selected_world_script.has_value() &&
                *script_state.selected_world_script == 2U)) {
        return 3;
    }

    WorldActivityRoutineInput selected_input{};
    selected_input.initial_scene_id = 0x0CU;
    selected_input.primary_runner_marker = 3U;
    selected_input.runner_slot_by_scene = {4, 2, 2};
    selected_input.world_script_available = {true, true, true};
    WorldActivityRoutineState selected_state{};
    if (!expect(run_world_activity_routine(selected_input, selected_state) ==
                    WorldActivityResult::field_ready &&
                !selected_state.selected_world_script.has_value())) {
        return 4;
    }

    WorldActivityRoutineInput timeout_input{};
    timeout_input.initial_scene_id = 0x10U;
    timeout_input.timeout_override = 1U;
    timeout_input.descriptors_pending = true;
    WorldActivityRoutineState timeout_state{};
    int timeouts = 0;
    int published = 0;
    const auto timeout_result = run_world_activity_routine(
        timeout_input, timeout_state,
        {[&] { ++timeouts; }, [&] { ++published; }, {}, {}, {}});
    if (!expect(timeout_result == WorldActivityResult::blocked_by_scene_gate &&
                timeout_state.scene_id == 0x0FU && timeouts == 1 &&
                published == 1 && timeout_state.descriptors_published)) {
        return 5;
    }

    WorldActivityRoutineInput halt_input{};
    halt_input.initial_scene_id = 0x38U;
    WorldActivityRoutineState halt_state{};
    if (!expect(run_world_activity_routine(halt_input, halt_state) ==
                    WorldActivityResult::halted &&
                halt_state.mode == WorldActivityMode::halted)) {
        return 6;
    }

    WorldActivityStartState started{};
    begin_world_activity(started, 0x00053EE2U);
    if (!expect(started.timer_frames == 0x3CU &&
                started.activity_pointer == 0x00053EE2U)) {
        return 8;
    }

    WorldActivityTimeoutState skipped{0x36U, 0U, 0, 0U, 0U};
    if (!expect(!schedule_world_activity_timeout(skipped) &&
                skipped.timeout_marker == 0 && skipped.timeout_frames == 0U)) {
        return 9;
    }

    WorldActivityTimeoutState armed{2U, 0U, 0, 0U, 0U};
    if (!expect(schedule_world_activity_timeout(armed) &&
                armed.timeout_marker == -1 && armed.timeout_frames == 0x78U)) {
        return 10;
    }

    WorldActivityTimeoutState keep{2U, 0U, 5, 0U, 0U};
    if (!expect(schedule_world_activity_timeout(keep) &&
                keep.timeout_marker == 5 && keep.timeout_frames == 0x78U)) {
        return 11;
    }

    WorldActivityTimeoutState extended{0x36U, 1U, 0, 0U, 1U};
    if (!expect(schedule_world_activity_timeout(extended) &&
                extended.timeout_marker == -1 &&
                extended.timeout_frames == 0x0258U)) {
        return 12;
    }

    WorldActivityRequestState idle{0U, 0x11U, 1U, 2U, 3U, 4U};
    WorldActivityStartState idle_activity{1U, 2U};
    std::array<WorldActivityRequestSlot, kWorldActivityRequestSlotCount> slots{};
    slots[0] = {1U, 0x80U};
    if (!expect(!handle_world_activity_request(idle, slots, idle_activity) &&
                idle.clear_80 == 0U && idle.clear_82 == 0U &&
                idle.clear_84 == 0U && idle.clear_87 == 0U &&
                idle.found_marker == 0x11U &&
                idle_activity.timer_frames == 1U)) {
        return 13;
    }

    WorldActivityRequestState missing{1U, 0U, 9U, 9U, 9U, 9U};
    WorldActivityStartState missing_activity{};
    slots = {};
    slots[2] = {1U, 0U};
    slots[3] = {0U, 0x80U};
    if (!expect(handle_world_activity_request(missing, slots, missing_activity) &&
                missing.request_flag == 0U && missing.found_marker == 0xFFU &&
                missing_activity.timer_frames == 0x3CU &&
                missing_activity.activity_pointer ==
                    kWorldActivityRequestDescriptor)) {
        return 14;
    }

    WorldActivityRequestState found{1U, 0xFFU, 1U, 1U, 1U, 1U};
    WorldActivityStartState found_activity{};
    slots = {};
    slots[5] = {0x03U, 0x80U};
    if (!expect(handle_world_activity_request(found, slots, found_activity) &&
                found.found_marker == 0U &&
                found_activity.activity_pointer ==
                    kWorldActivityRequestDescriptor)) {
        return 15;
    }

    return 0;
}

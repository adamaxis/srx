#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

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
    resource_input.sampled_random_bound = 5U;
    resource_input.advanced_prng_value = 0x0BU;
    resource_input.resource_cursor_source = {-2, 0x4567U};
    resource_input.force_random_pair_two_three = true;
    WorldActivityRoutineState resource_state{};
    int resource_uploads = 0;
    const auto resource_result = run_world_activity_routine(
        resource_input, resource_state,
        { {}, {}, {}, {}, [&] { ++resource_uploads; } });
    if (!expect(resource_result == WorldActivityResult::resource_activity_ready &&
                resource_state.mode == WorldActivityMode::resource_activity &&
                resource_state.random_bound == 2U &&
                resource_state.random_slot == 3U &&
                resource_state.resource_cursor.cursor == -2 &&
                resource_state.resource_cursor.resource_index == 0x4567U &&
                resource_state.world_activity_effect_words_cleared &&
                resource_uploads == 1)) {
        return 1;
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

    return 0;
}

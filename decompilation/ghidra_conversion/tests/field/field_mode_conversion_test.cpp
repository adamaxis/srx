#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeInitializationState state{};
    state.profile = FieldModeProfile::profile_b;
    state.entry_transient.pending_world_activity_handler = 0x12345678U;
    state.entry_transient.world_effect_active = 1U;
    state.entry_transient.pending_world_effect = 2U;
    state.entry_transient.effect_context_active = 3U;
    state.entry_transient.alternate_effect_context = 4U;
    state.entry_transient.mode_five_effect_marker = 5U;
    state.entry_transient.world_activity_request = 6U;
    state.world_effects_active = true;
    std::vector<int> order;
    FieldModeProfile selected = FieldModeProfile::profile_b;
    initialize_field_mode(
        state, FieldModeProfile::profile_a,
        {
            [&order, &selected](const FieldModeProfile profile) {
                order.push_back(1);
                selected = profile;
            },
            [&order] { order.push_back(2); },
            [&order] { order.push_back(3); },
        });
    if (!expect(state.profile == FieldModeProfile::profile_a &&
                selected == FieldModeProfile::profile_a &&
                state.entry_transient.pending_world_activity_handler == 0U &&
                state.entry_transient.world_effect_active == 0U &&
                state.entry_transient.pending_world_effect == 0U &&
                state.entry_transient.effect_context_active == 0U &&
                state.entry_transient.alternate_effect_context == 0U &&
                state.entry_transient.mode_five_effect_marker == 0U &&
                state.entry_transient.world_activity_request == 0U &&
                !state.world_effects_active &&
                order == std::vector<int>{1, 2, 3})) {
        return 1;
    }

    initialize_field_mode(state, FieldModeProfile::profile_b, {});
    if (!expect(state.profile == FieldModeProfile::profile_b &&
                state.entry_transient.pending_world_activity_handler == 0U &&
                !state.world_effects_active)) {
        return 2;
    }

    // Exits are Lua-resolved (world_links.cave_exit_flat); vectors mirror
    // world_links_tests.lua. FFFBF9=8, FFFC10=0 → mode 5, scene 0x2D, select 1, pair 0x5E00/0x2280.
    const auto street = advance_field_mode_transition(
        {8U, 0, 0U, FieldModeTransitionExit{5, 0x2DU, 0x5E00U, 0x2280U}}, {});
    if (!expect(street.resolved && street.next_mode_index == 5 &&
                street.scene_id == 0x2DU &&
                street.override_x == 0x5E00U &&
                street.override_y == 0x2280U &&
                !street.start_world_activity)) {
        return 3;
    }

    // FFFBF9=11, FFFC10=0 → mode 0, scene 6, select 0, pair 0x0500/0x5580.
    const auto first_scene = advance_field_mode_transition(
        {11U, 0, 0U, FieldModeTransitionExit{0, 0x06U, 0x0500U, 0x5580U}}, {});
    if (!expect(first_scene.resolved && first_scene.next_mode_index == 0 &&
                first_scene.scene_id == 0x0006U &&
                first_scene.override_x == 0x0500U &&
                first_scene.override_y == 0x5580U)) {
        return 4;
    }

    // FFFBF9=8, FFFC10=2 → mode 4, scene 0x2C, select 1, pair 0x2E00/0x3880.
    const auto alley = advance_field_mode_transition(
        {8U, 2, 0U, FieldModeTransitionExit{4, 0x2CU, 0x2E00U, 0x3880U}}, {});
    if (!expect(alley.resolved && alley.next_mode_index == 4 &&
                alley.scene_id == 0x2CU &&
                alley.override_x == 0x2E00U && alley.override_y == 0x3880U)) {
        return 5;
    }

    int started = 0;
    int timed = 0;
    // FFFC10=0x14, FFFBF9=8 → mode 0x13. Bit 8 clear fires 0x1E2EC/0x1DBAA.
    const auto world = advance_field_mode_transition(
        {8U, 0x14, 0U, FieldModeTransitionExit{0x13, 0x2FU, 0x5E80U, 0x1300U}},
        {[&started] { ++started; }, [&timed] { ++timed; }});
    if (!expect(world.resolved && world.next_mode_index == 0x13 &&
                world.scene_id == 0x2FU &&
                world.override_x == 0x5E80U && world.override_y == 0x1300U &&
                world.spawn_position_available &&
                world.start_world_activity && started == 1 && timed == 1 &&
                world.pending_actor_list ==
                    FieldModePendingActorList::council_island_spirit_eyes &&
                field_mode_pending_actor_list_id(
                    *world.pending_actor_list) == "mode_13_entry")) {
        return 6;
    }

    started = 0;
    timed = 0;
    const auto locked = advance_field_mode_transition(
        {8U, 0x14, 0x100U,
         FieldModeTransitionExit{0x13, 0x2FU, 0x5E80U, 0x1300U}},
        {[&started] { ++started; }, [&timed] { ++timed; }});
    if (!expect(locked.resolved && locked.next_mode_index == 0x13 &&
                locked.scene_id == 0x2FU &&
                !locked.start_world_activity && started == 0 && timed == 0 &&
                !locked.pending_actor_list.has_value())) {
        return 7;
    }

    started = 0;
    timed = 0;
    const auto out_of_table = advance_field_mode_transition(
        {8U, 25, 0U, std::nullopt},
        {[&started] { ++started; }, [&timed] { ++timed; }});
    if (!expect(!out_of_table.resolved && started == 0 && timed == 0)) {
        return 8;
    }

    return 0;
}

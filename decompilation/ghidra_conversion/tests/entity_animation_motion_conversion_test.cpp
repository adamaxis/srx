#include "shadowrun_ghidra/entity_animation_motion_conversion.hpp"

#include <cassert>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> calls;
    EntityAnimationMotionCallbacks finish_callbacks{
        {}, {}, {}, {}, {}, [&] { calls.push_back(1); }, {}};

    EntityAnimationMotionState early{};
    early.global_phase_active = true;
    early.entity_type = 0x28U;
    early.frame_counter = 0x003CU;
    finish_callbacks.finish_animation = [&] {
        calls.push_back(2);
        return true;
    };
    const auto early_result = run_entity_animation_motion(
        early, finish_callbacks);
    assert(early_result.routed_to_finish && early_result.handler_installed &&
           calls == std::vector<int>{2});

    EntityAnimationMotionState blocked{};
    blocked.animation_mode = 1U;
    assert(run_entity_animation_motion(blocked, {}).blocked_by_animation_mode);

    EntityAnimationMotionState mode_two{};
    mode_two.animation_mode = 2U;
    mode_two.linked_position = EntityAnimationPoint{0x1234U, 0x5678U};
    int continued = 0;
    const auto mode_two_result = run_entity_animation_motion(
        mode_two, {{}, {}, {}, {}, {}, [&] { ++continued; }, {}});
    assert(mode_two_result.linked_position_copied && mode_two_result.routed_to_target_update &&
           mode_two.target_position.x == 0x1234U &&
           mode_two.target_position.y == 0x5678U && continued == 1);

    EntityAnimationMotionState normal{};
    normal.current_position = {0x0100U, 0x0200U};
    normal.linked_position = EntityAnimationPoint{0x0120U, 0x0210U};
    normal.entity_flags = 0x08U;
    normal.route_flags = 0x10U;
    normal.distance_type = 3U;
    normal.frame_counter = 0x0030U;
    EntityAnimationMotionCallbacks normal_callbacks{
        [](const EntityAnimationPoint&, const EntityAnimationPoint&) {
            return std::optional<EntityAnimationHeadingDistance>(
                EntityAnimationHeadingDistance{0x1000U, 0x0020U});
        },
        [](const EntityAnimationDistanceTable table, const std::uint8_t index) {
            assert(table == EntityAnimationDistanceTable::by_entity_type && index == 3U);
            return std::optional<EntityAnimationDistanceBounds>(
                EntityAnimationDistanceBounds{0x0030U, 0x0040U});
        },
        {},
        [](const std::uint16_t direction, const std::uint16_t magnitude) {
            assert(direction == 0x5000U && magnitude == 0x0040U);
            return std::optional<EntityAnimationVectorDelta>(
                EntityAnimationVectorDelta{0x0010U, 0xFFF0U});
        },
        [] { return std::optional<bool>(false); },
        [&] { ++continued; },
        {}};
    const auto normal_result = run_entity_animation_motion(
        normal, normal_callbacks);
    assert(normal_result.distance_table_used && normal_result.target_position_built &&
           normal_result.routed_to_target_update &&
           normal.target_position.x == 0x0110U &&
           normal.target_position.y == 0x01F0U &&
           (normal.route_flags & 0x10U) != 0U);

    EntityAnimationMotionState in_range = normal;
    in_range.delay = 3U;
    in_range.frame_counter = 0x003CU;
    in_range.linked_position = EntityAnimationPoint{0x0120U, 0x0210U};
    EntityAnimationMotionCallbacks in_range_callbacks = normal_callbacks;
    in_range_callbacks.measure_heading_distance =
        [](const EntityAnimationPoint&, const EntityAnimationPoint&) {
            return std::optional<EntityAnimationHeadingDistance>(
                EntityAnimationHeadingDistance{0x2000U, 0x0038U});
        };
    in_range_callbacks.transform_vector =
        [](const std::uint16_t, const std::uint16_t) {
            return std::optional<EntityAnimationVectorDelta>(
                EntityAnimationVectorDelta{0U, 0U});
        };
    in_range_callbacks.finish_animation = finish_callbacks.finish_animation;
    in_range_callbacks.evaluate_position_gate = [] {
        return std::optional<bool>(true);
    };
    const auto in_range_result = run_entity_animation_motion(
        in_range, in_range_callbacks);
    assert(in_range_result.target_position_built &&
           in_range_result.position_gate_carry && in_range_result.routed_to_finish &&
           in_range.delay == 0U && in_range_result.handler_installed);

    EntityAnimationMotionState phase_table{};
    phase_table.current_position = {1U, 2U};
    phase_table.linked_position = EntityAnimationPoint{4U, 6U};
    phase_table.delay = 2U;
    phase_table.animation_frame = -1;
    phase_table.animation_phase = 3U;
    phase_table.frame_counter = 0x0040U;
    bool phase_selector_called = false;
    const auto phase_result = run_entity_animation_motion(
        phase_table,
        {[](const EntityAnimationPoint&, const EntityAnimationPoint&) {
             return std::optional<EntityAnimationHeadingDistance>(
                 EntityAnimationHeadingDistance{0U, 1U});
         },
         [](const EntityAnimationDistanceTable table, const std::uint8_t index) {
             assert(table == EntityAnimationDistanceTable::by_animation_phase && index == 5U);
             return std::optional<EntityAnimationDistanceBounds>(
                 EntityAnimationDistanceBounds{2U, 3U});
         },
         [&](const bool negative, const std::uint8_t phase)
             -> std::optional<EntityAnimationPhaseDistanceSelection> {
             phase_selector_called = true;
             assert(negative && phase == 3U);
             return EntityAnimationPhaseDistanceSelection{5U, 7U};
         },
         [](const std::uint16_t, const std::uint16_t) {
             return std::optional<EntityAnimationVectorDelta>(
                 EntityAnimationVectorDelta{0U, 0U});
         },
         [] { return std::optional<bool>(false); },
         {},
         {}});
    assert(phase_selector_called && phase_result.distance_table_used &&
           phase_result.phase_source_row == 7U && phase_table.delay == 1U);

    EntityAnimationMotionState terrain = normal;
    terrain.terrain_gate = 1U;
    terrain.frame_counter = 0x0040U;
    const auto terrain_result = run_entity_animation_motion(
        terrain, normal_callbacks);
    assert(terrain_result.routed_to_finish && !terrain_result.target_position_built &&
           !terrain_result.handler_installed);
    return 0;
}

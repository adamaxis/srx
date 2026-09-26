#include "shadowrun_ghidra/entities/entity_animation_route_conversion.hpp"

#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationRouteState expired{};
    expired.frame_counter = 0x003DU;
    const auto expired_result =
        advance_entity_animation_target_route(expired);
    if (!expect(expired_result.action ==
                    EntityAnimationRouteAction::timer_expired &&
                expired.continuation == 0U)) {
        return 1;
    }

    EntityAnimationRouteState direct{};
    direct.current_x = 0x0200U;
    direct.current_y = 0x0100U;
    direct.target_x = 0x0400U;
    direct.target_y = 0x0100U;
    direct.frame_counter = 0x003CU;
    direct.movement_mode = 0x1000;
    const auto direct_result =
        advance_entity_animation_target_route(direct);
    const auto direct_measurement = measure_relative_heading_and_distance(
        direct.current_x,
        direct.current_y,
        direct.target_x,
        direct.target_y);
    if (!expect(direct_result.action ==
                    EntityAnimationRouteAction::direct_target_metric_published &&
                direct_result.continuation_installed &&
                direct.heading_metric == direct_measurement.heading &&
                direct.distance_metric == direct_measurement.distance &&
                direct.continuation == 0xB574U)) {
        return 2;
    }

    EntityAnimationRouteState marker{};
    marker.current_x = 0x0200U;
    marker.current_y = 0x0100U;
    marker.target_x = 0x0200U;
    marker.target_y = 0x0200U;
    marker.movement_flags = 0x04U;
    marker.axis_flags = 0x0FU;
    const auto marker_result =
        advance_entity_animation_target_route(marker);
    if (!expect(marker_result.axis_flags_derived &&
                (marker.movement_flags & 0x04U) == 0U &&
                marker.axis_flags == 0x03U &&
                marker_result.action ==
                    EntityAnimationRouteAction::direct_target_metric_published)) {
        return 3;
    }

    EntityAnimationRouteState queued{};
    queued.current_x = 0U;
    queued.current_y = 0U;
    queued.target_x = 0x0400U;
    queued.target_y = 0U;
    queued.movement_flags = 0x02U;
    queued.movement_mode = 0;
    queued.route.tiles = {0U, 1U, 0xFFFFU};
    queued.obstruction_map.assign(128U, 0U);
    const auto queued_result =
        advance_entity_animation_target_route(queued);
    if (!expect(queued_result.route_queue_entered &&
                queued_result.route_cursor_advanced &&
                queued.route_cursor_bytes == 2U &&
                queued_result.action ==
                    EntityAnimationRouteAction::route_target_metric_published &&
                queued.heading_metric == 0x4000U &&
                queued.continuation == 0xB574U)) {
        return 4;
    }

    EntityAnimationRouteState selected{};
    selected.current_x = 0x0200U;
    selected.current_y = 0U;
    selected.target_x = 0x0400U;
    selected.target_y = 0x0100U;
    selected.movement_mode = 0;
    selected.obstruction_map.assign(256U, 0U);
    selected.obstruction_map[130U] = 1U;
    const auto selected_result =
        advance_entity_animation_target_route(selected);
    if (!expect(selected_result.route_selection_called &&
                selected_result.route_selection_action ==
                    ObstructionRouteAction::direct_step &&
                selected.route.tiles[0] == 66U &&
                selected.route.tiles[1] == 0xFFFFU &&
                selected_result.route_queue_entered &&
                selected_result.continuation_installed)) {
        return 5;
    }

    EntityAnimationRouteState blocked_target{};
    blocked_target.current_x = 0x0200U;
    blocked_target.current_y = 0U;
    blocked_target.target_x = 0x0400U;
    blocked_target.target_y = 0x0100U;
    blocked_target.movement_mode = 0;
    blocked_target.obstruction_map.assign(128U, 0U);
    blocked_target.obstruction_map[2U] = 1U;
    const auto blocked_target_result =
        advance_entity_animation_target_route(blocked_target);
    if (!expect(blocked_target_result.route_selection_called &&
                blocked_target_result.route_selection_action ==
                    ObstructionRouteAction::target_blocked &&
                blocked_target.movement_flags == 0U &&
                blocked_target_result.continuation_installed)) {
        return 6;
    }

    EntityAnimationRouteState blocked{};
    blocked.current_x = 0x0400U;
    blocked.current_y = 0x0100U;
    blocked.target_x = 0x0600U;
    blocked.target_y = 0x0100U;
    blocked.movement_flags = 0x0AU;
    blocked.route.tiles = {0U, 0xFFFFU};
    blocked.obstruction_map.assign(128U, 0U);
    blocked.obstruction_map[compute_obstruction_grid_index(
        blocked.target_x, blocked.target_y)] = 1U;
    const auto blocked_result =
        advance_entity_animation_target_route(blocked);
    if (!expect(blocked_result.action ==
                    EntityAnimationRouteAction::route_path_cleared &&
                blocked.movement_flags == 0U &&
                blocked.continuation == 0U)) {
        return 7;
    }

    EntityAnimationRouteState completed{};
    completed.current_x = 0U;
    completed.current_y = 0U;
    completed.target_x = 0x0400U;
    completed.target_y = 0U;
    completed.movement_flags = 0x02U;
    completed.route.tiles = {0U, 0xFFFFU};
    completed.obstruction_map.assign(128U, 0U);
    const auto completed_result =
        advance_entity_animation_target_route(completed);
    if (!expect(completed_result.action ==
                    EntityAnimationRouteAction::route_completed_to_target &&
                completed.movement_flags == 0U &&
                completed.axis_flags == 0x03U &&
                completed_result.continuation_installed)) {
        return 8;
    }

    EntityAnimationRouteState replanned{};
    replanned.current_x = 0x1400U;
    replanned.current_y = 0U;
    replanned.target_x = 0U;
    replanned.target_y = 0U;
    replanned.movement_flags = 0x0AU;
    replanned.heading_metric = 0x4000U;
    replanned.route.tiles = {10U, 0xFFFFU};
    replanned.obstruction_map.assign(128U, 0U);
    replanned.obstruction_map[11U] = 1U;
    replanned.obstruction_max_row = 1U;
    const auto replanned_result =
        advance_entity_animation_target_route(replanned);
    if (!expect(replanned_result.action ==
                    EntityAnimationRouteAction::route_replanned &&
                replanned_result.route_replan_attempted &&
                replanned.route_cursor_bytes == 0U &&
                replanned.route.tiles[0] == 74U &&
                replanned.route.tiles[1] == 0xFFFFU)) {
        return 9;
    }

    EntityAnimationRouteState exhausted{};
    exhausted.current_x = 0x1400U;
    exhausted.current_y = 0U;
    exhausted.target_x = 0x1400U;
    exhausted.target_y = 0U;
    exhausted.movement_flags = 0x0AU;
    exhausted.heading_metric = 0x4000U;
    exhausted.route.tiles = {10U, 0xFFFFU};
    exhausted.obstruction_map.assign(128U, 1U);
    exhausted.obstruction_map[10U] = 0U;
    exhausted.obstruction_max_row = 1U;
    const auto exhausted_result =
        advance_entity_animation_target_route(exhausted);
    if (!expect(exhausted_result.action ==
                    EntityAnimationRouteAction::route_replan_exhausted &&
                exhausted.movement_flags == 0U)) {
        return 10;
    }

    return 0;
}

#include "shadowrun_ghidra/entity_animation_route_conversion.hpp"

#include "shadowrun_ghidra/movement_metric_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint32_t kEntityTargetContinuation = 0x0000B574U;

std::int32_t signed_word_delta(const std::uint16_t target,
                               const std::uint16_t origin) {
    return static_cast<std::int16_t>(
        static_cast<std::uint16_t>(target - origin));
}

void publish_direct_metric(EntityAnimationRouteState& state,
                           EntityAnimationRouteResult& result) {
    const auto measurement = measure_relative_heading_and_distance(
        state.current_x,
        state.current_y,
        state.target_x,
        state.target_y);
    state.heading_metric = measurement.heading;
    state.distance_metric = static_cast<std::uint16_t>(measurement.distance);
    state.continuation = kEntityTargetContinuation;
    result.continuation_installed = true;
}

std::uint8_t heading_direction_slot(const std::uint16_t heading) {
    const auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) |
        static_cast<std::uint16_t>(heading >> 13U));
    return static_cast<std::uint8_t>(rotated & 0x0006U);
}

std::int16_t direction_step(const std::uint8_t direction_slot) {
    static constexpr std::array<std::int16_t, 8> kSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    if (direction_slot > 6U || (direction_slot & 1U) != 0U) {
        return 0;
    }
    return kSteps[direction_slot];
}

bool map_tile_open(const std::vector<std::uint8_t>& obstruction_map,
                   const std::int32_t tile) {
    return tile >= 0 &&
        tile < static_cast<std::int32_t>(obstruction_map.size()) &&
        obstruction_map[static_cast<std::size_t>(tile)] == 0U;
}

std::optional<std::int32_t> choose_replanned_tile(
    const EntityAnimationRouteState& state,
    const std::int32_t current_tile) {
    auto primary = heading_direction_slot(state.heading_metric);
    auto secondary = static_cast<std::uint8_t>((primary + 2U) & 0x06U);
    const auto primary_step = direction_step(primary);
    const auto opposite_step = direction_step(
        static_cast<std::uint8_t>((primary + 4U) & 0x06U));
    const auto secondary_step = direction_step(secondary);

    // 0x14A54 first accepts a clear primary/opposite-side pair and then asks
    // 0x148AA for the first occupied tile in the current heading direction.
    if (map_tile_open(state.obstruction_map,
                      current_tile + primary_step) &&
        map_tile_open(state.obstruction_map,
                      current_tile + opposite_step + secondary_step)) {
        const auto occupied = find_first_occupied_obstruction_tile(
            current_tile, primary, state.obstruction_map);
        if (occupied.has_value()) {
            return occupied->tile_index;
        }
    }

    // The source initializes D0 to four and uses DBF, giving five attempts.
    for (int attempt = 0; attempt < 5; ++attempt) {
        const auto candidate = find_alternate_route_tile(
            current_tile,
            primary,
            secondary,
            state.obstruction_max_column,
            state.obstruction_max_row,
            state.obstruction_map);
        if (candidate != current_tile) {
            return candidate;
        }
        primary = static_cast<std::uint8_t>((primary + 6U) & 0x06U);
        secondary = static_cast<std::uint8_t>((secondary + 6U) & 0x06U);
    }
    return std::nullopt;
}

EntityAnimationRouteResult replan_obstruction_route(
    EntityAnimationRouteState& state,
    EntityAnimationRouteResult result) {
    result.route_replan_attempted = true;
    const auto current_tile = static_cast<std::int32_t>(
        compute_obstruction_grid_index(state.current_x, state.current_y));
    const auto replanned = choose_replanned_tile(state, current_tile);
    if (!replanned.has_value()) {
        state.movement_flags = clear_entity_route_flags(
            state.movement_flags);
        result.action = EntityAnimationRouteAction::route_replan_exhausted;
        return result;
    }

    if (state.route.tiles.size() < 2U) {
        state.route.tiles.resize(2U, 0xFFFFU);
    }
    state.route_cursor_bytes = 0U;
    state.route.cursor = 0U;
    state.route.tiles[0] = static_cast<std::uint16_t>(*replanned);
    state.route.tiles[1] = 0xFFFFU;
    result.action = EntityAnimationRouteAction::route_replanned;
    return result;
}

EntityAnimationRouteResult service_route_queue(
    EntityAnimationRouteState& state,
    EntityAnimationRouteResult result) {
    result.route_queue_entered = true;
    state.route.cursor = state.route_cursor_bytes / 2U;
    const auto cursor_before = state.route.cursor;
    const auto queue_result = advance_obstruction_route_queue(
        state.current_x,
        state.current_y,
        state.target_x,
        state.target_y,
        state.movement_flags,
        state.route,
        state.obstruction_map);
    state.route_cursor_bytes = static_cast<std::uint16_t>(
        state.route.cursor * 2U);
    result.route_cursor_advanced = state.route.cursor != cursor_before;

    switch (queue_result.action) {
    case RouteQueueAction::obstruction_path_blocked:
        state.movement_flags = queue_result.movement_flags;
        result.action = EntityAnimationRouteAction::route_path_cleared;
        return result;

    case RouteQueueAction::advance_to_tile:
        state.heading_metric = queue_result.heading;
        state.distance_metric = static_cast<std::uint16_t>(
            queue_result.distance);
        state.continuation = kEntityTargetContinuation;
        result.continuation_installed = true;
        result.action =
            EntityAnimationRouteAction::route_target_metric_published;
        return result;

    case RouteQueueAction::target_reached:
        // 0x14A36 clears bits 1 and 3, rebuilds +0xB1, then falls through to
        // the same 0x145E8/0x15D3C metric publication as the direct path.
        state.movement_flags = queue_result.movement_flags;
        state.axis_flags = derive_entity_axis_relation_flags(
            state.current_x,
            state.current_y,
            state.target_x,
            state.target_y);
        result.axis_flags_derived = true;
        publish_direct_metric(state, result);
        result.action = EntityAnimationRouteAction::route_completed_to_target;
        return result;

    case RouteQueueAction::obstruction_replan_required:
        return replan_obstruction_route(state, result);

    case RouteQueueAction::no_route_entry:
        result.action = EntityAnimationRouteAction::route_queue_unresolved;
        return result;
    }

    result.action = EntityAnimationRouteAction::route_queue_unresolved;
    return result;
}

} // namespace

EntityAnimationRouteResult advance_entity_animation_target_route(
    EntityAnimationRouteState& state) {
    EntityAnimationRouteResult result{};
    if (state.frame_counter > 0x003CU) {
        result.action = EntityAnimationRouteAction::timer_expired;
        return result;
    }

    if ((state.movement_flags & 0x02U) != 0U) {
        return service_route_queue(state, result);
    }

    const bool had_axis_update_marker =
        (state.movement_flags & 0x04U) != 0U;
    state.movement_flags = static_cast<std::uint8_t>(
        state.movement_flags & static_cast<std::uint8_t>(~0x04U));
    if (had_axis_update_marker) {
        state.axis_flags = derive_entity_axis_relation_flags(
            state.current_x,
            state.current_y,
            state.target_x,
            state.target_y);
        result.axis_flags_derived = true;
    } else if (state.movement_mode != 0x1000) {
        const auto axis_update = update_entity_axis_relation_state(
            0,
            0,
            signed_word_delta(state.target_x, state.current_x),
            signed_word_delta(state.target_y, state.current_y),
            state.axis_flags,
            state.movement_mode,
            state.movement_flags);
        state.axis_flags = axis_update.axis_flags;
        state.movement_flags = axis_update.movement_flags;
        result.axis_state_updated = true;
        if ((state.movement_flags & 0x02U) != 0U) {
            return service_route_queue(state, result);
        }
        state.axis_flags = derive_entity_axis_relation_flags(
            state.current_x,
            state.current_y,
            state.target_x,
            state.target_y);
        result.axis_flags_derived = true;
    }

    publish_direct_metric(state, result);
    result.action = EntityAnimationRouteAction::direct_target_metric_published;
    return result;
}

} // namespace shadowrun::ghidra_conversion

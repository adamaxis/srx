#pragma once

#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationRouteState {
    std::uint16_t current_x = 0; // record +0x00
    std::uint16_t current_y = 0; // record +0x04
    std::uint16_t target_x = 0; // record +0x96
    std::uint16_t target_y = 0; // record +0x98
    std::uint16_t frame_counter = 0; // record +0x16
    std::uint8_t movement_flags = 0; // record +0xB0
    std::uint8_t axis_flags = 0; // record +0xB1
    std::int16_t movement_mode = 0; // record +0xB8
    std::uint16_t heading_metric = 0; // record +0x34
    std::uint16_t distance_metric = 0; // record +0xC8
    std::uint32_t continuation = 0; // record +0x3A
    std::uint16_t route_cursor_bytes = 0; // record +0xBA
    RouteQueueState route; // packed words beginning at record +0xBC

    // FF1700 and the two source map limits used by 0x14764. These are
    // extracted world data, not a runtime ROM or VDP address dependency.
    std::vector<std::uint8_t> obstruction_map;
    std::uint16_t obstruction_max_column = 0x003FU;
    std::uint16_t obstruction_max_row = 0x001FU;
};

enum class EntityAnimationRouteAction {
    timer_expired,
    direct_target_metric_published,
    route_target_metric_published,
    route_completed_to_target,
    route_path_cleared,
    route_replanned,
    route_replan_exhausted,
    route_queue_unresolved,
};

struct EntityAnimationRouteResult {
    EntityAnimationRouteAction action =
        EntityAnimationRouteAction::route_queue_unresolved;
    bool route_queue_entered = false;
    bool axis_state_updated = false;
    bool axis_flags_derived = false;
    bool route_cursor_advanced = false;
    bool continuation_installed = false;
    bool route_replan_attempted = false;
};

// ROM 0x145A2: finish the entity movement continuation after the animation
// position gate. The host model keeps the record's movement/axis fields,
// packed route cursor, extracted obstruction map, and B574 continuation as
// typed state. It does not emulate a 68000 record or execute a ROM address.
EntityAnimationRouteResult advance_entity_animation_target_route(
    EntityAnimationRouteState& state);

} // namespace shadowrun::ghidra_conversion

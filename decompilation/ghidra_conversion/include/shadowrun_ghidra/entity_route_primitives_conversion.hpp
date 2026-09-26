#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct HeadingProjection {
    std::int32_t world_x = 0;
    std::int32_t world_y = 0;
};

struct ObstructionSearchResult {
    std::int32_t last_open_tile = 0;
    std::int16_t step = 0;
};

struct ObstructionStripProbe {
    std::int32_t starting_tile = 0;
    std::int32_t last_open_tile = 0;
    bool left_map = false;
};

struct ObstructionTileProjection {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::int8_t direction = 0;
};

struct EntityAxisUpdate {
    std::uint8_t axis_flags = 0;
    std::uint8_t movement_flags = 0;
    bool route_advance_requested = false;
};

enum class ObstructionRouteAction {
    already_retrying,
    target_blocked,
    direct_step,
    bypass_available,
    retry_around_obstruction,
    no_obstruction_found,
};

struct ObstructionRouteSelection {
    std::uint8_t movement_flags = 0;
    ObstructionRouteAction action =
        ObstructionRouteAction::no_obstruction_found;
    std::int32_t route_tile = 0;
    std::int16_t route_step = 0;
    std::int32_t secondary_route_tile = 0;
    std::int16_t secondary_route_step = 0;
    bool has_route_tile = false;
    bool has_secondary_route_tile = false;
    bool route_cursor_reset = false;
    bool route_queue_requested = false;
};

struct ObstructionBypassRoute {
    std::int32_t primary_tile = 0;
    std::int32_t secondary_tile = 0;
};

struct RouteQueueState {
    std::vector<std::uint16_t> tiles;
    std::size_t cursor = 0;
};

enum class RouteQueueAction {
    no_route_entry,
    advance_to_tile,
    target_reached,
    obstruction_path_blocked,
    obstruction_replan_required,
};

struct RouteQueueAdvance {
    RouteQueueAction action = RouteQueueAction::no_route_entry;
    std::uint8_t movement_flags = 0;
    std::uint8_t axis_flags = 0;
    std::uint16_t heading = 0;
    std::uint32_t distance = 0;
    ObstructionTileProjection target{};
    bool has_target = false;
};

// ROM 0x14756: clear the pending-route and route-retry bits from an entity's
// movement flags after route selection fails or completes.
std::uint8_t clear_entity_route_flags(std::uint8_t movement_flags);

// ROM 0x148AA: walk in one of the four cardinal obstruction-map directions
// until a nonzero tile is found. The source returns the last open tile in D1
// and the step in D0; it does not return the occupied tile. The bounded host
// form returns no result at a map edge instead of allowing the source pointer
// walk to escape its table.
std::optional<ObstructionSearchResult> find_first_occupied_obstruction_tile(
    std::int32_t starting_tile,
    std::uint8_t direction_slot,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14800: advance through open tiles along a primary direction while
// the adjacent secondary tile remains occupied. The source returns the last
// open candidate in D1, including the starting tile when the first step is
// blocked; the host reports that candidate and makes a map-edge exit explicit.
ObstructionStripProbe probe_obstruction_strip(
    std::int32_t starting_tile,
    std::uint8_t primary_direction_slot,
    std::uint8_t secondary_direction_slot,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14854: return the two open route tiles used by the source's
// alternate-side obstruction path. The first tile is on the primary side and
// the second is its open secondary-side companion.
std::optional<ObstructionBypassRoute> find_bypass_route(
    std::int32_t starting_tile,
    std::uint8_t primary_direction_slot,
    std::uint8_t secondary_direction_slot,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14854: test the source's alternate-side obstruction path. A true
// result means the primary and opposite-primary probes are both blocked;
// false means a clear bypass exists.
bool can_bypass_obstruction(
    std::int32_t starting_tile,
    std::uint8_t primary_direction_slot,
    std::uint8_t secondary_direction_slot,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14764: accumulate a primary grid step until the primary tile is
// occupied or its adjacent side becomes open, returning the last candidate
// selected by the source route search.
std::int32_t find_alternate_route_tile(
    std::int32_t starting_tile,
    std::uint8_t primary_direction_slot,
    std::uint8_t secondary_direction_slot,
    std::int32_t maximum_column,
    std::int32_t maximum_row,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14AF4: try the source strip probe in five rotated cardinal
// directions and return the first non-current route tile.
std::optional<std::int32_t> find_rotating_route_strip_target(
    std::uint16_t world_x,
    std::uint16_t world_y,
    std::uint16_t heading,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14682: set the pending-route bit, reject an occupied target, write a
// one- or two-tile route for a clear/bypass path, or request the five-way
// 0x14AF4 retry followed by 0x149C0. The heading is the record +0x34 metric
// used by that retry branch.
ObstructionRouteSelection select_obstruction_route(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y,
    std::uint8_t movement_flags,
    const std::vector<std::uint8_t>& obstruction_map,
    std::uint16_t heading = 0U);

// ROM 0x149C0: consume a packed route queue entry, decode its next target,
// and expose the source's path-blocked/replan exits as typed actions. The
// source tests record +0xB0 bits 3/1 directly; the host therefore accepts the
// record's low movement-flags byte rather than the Ghidra C export's
// endian-confused 0x0800 word mask.
RouteQueueAdvance advance_obstruction_route_queue(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y,
    std::uint8_t movement_flags,
    RouteQueueState& route,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x1494C: walk the source's horizontal or vertical grid line between
// two packed obstruction indices and report whether an occupied tile blocks
// the walk. The packed grid uses six low bits for the column and 0x40 rows.
bool is_obstruction_path_blocked(
    std::int32_t start_tile,
    std::int32_t target_tile,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14914: convert entity and target positions to packed grid indices,
// then check the obstruction line in both source directions.
bool is_entity_path_blocked(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y,
    const std::vector<std::uint8_t>& obstruction_map);

// ROM 0x14902: map world coordinates into the source obstruction-grid index.
// The source grid uses 0x200 horizontal units and a 0x40-byte row stride.
std::uint16_t compute_obstruction_grid_index(
    std::uint16_t world_x,
    std::uint16_t world_y);

// ROM 0x14B4E: decode the packed tile coordinate and select the direction
// from that tile toward the reference tile. Horizontal slots are 2/6 and
// vertical slots are 4/0 for negative/positive relative movement.
ObstructionTileProjection decode_obstruction_tile_projection(
    std::uint16_t packed_tile,
    std::uint16_t reference_tile);

// ROM 0x14C56: project the current position by the extracted eight-entry
// heading vector table. The source heading is encoded in the word's top
// three bits, so the host keeps the result as ordinary signed coordinates.
HeadingProjection project_heading_position(
    std::int32_t world_x,
    std::int32_t world_y,
    std::uint16_t heading);

// ROM 0x14C2E: set bit 0 when X is no greater than its target and bit 1 when
// Y is no greater than its target.
std::uint8_t derive_entity_axis_relation_flags(
    std::uint16_t entity_x,
    std::uint16_t entity_y,
    std::uint16_t target_x,
    std::uint16_t target_y);

// ROM 0x14BA6: update signed X/Y relation bits and report whether the source
// would request its unresolved route-advance helper at 0x14682.
EntityAxisUpdate update_entity_axis_relation_state(
    std::int32_t entity_x,
    std::int32_t entity_y,
    std::int32_t target_x,
    std::int32_t target_y,
    std::uint8_t axis_flags,
    std::int16_t movement_mode,
    std::uint8_t movement_flags);

// ROM 0x15A8C: return the first available source slot in the six-slot range
// 9..14, or the one-past-last value 15 when every slot is occupied.
std::uint8_t find_first_available_entity_slot(
    const std::array<std::uint8_t, 6>& slot_flags);

} // namespace shadowrun::ghidra_conversion

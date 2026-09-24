#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct HeadingProjection {
    std::int32_t world_x = 0;
    std::int32_t world_y = 0;
};

struct ObstructionTileProjection {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::int8_t direction = 0;
};

// ROM 0x14756: clear the pending-route and route-retry bits from an entity's
// movement flags after route selection fails or completes.
std::uint8_t clear_entity_route_flags(std::uint8_t movement_flags);

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

// ROM 0x15A8C: return the first available source slot in the six-slot range
// 9..14, or the one-past-last value 15 when every slot is occupied.
std::uint8_t find_first_available_entity_slot(
    const std::array<std::uint8_t, 6>& slot_flags);

} // namespace shadowrun::ghidra_conversion

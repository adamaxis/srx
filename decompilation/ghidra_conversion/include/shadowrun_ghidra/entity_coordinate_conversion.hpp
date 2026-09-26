#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityCoordinateTarget {
    std::uint16_t current = 0;
    std::uint16_t target = 0;
};

// ROM 0xB530: move a fixed-point coordinate toward its target in 0x800-unit
// steps, snapping to the target when the remaining distance is one step or
// less.
void approach_entity_fixed_point_target(EntityCoordinateTarget& state);

struct FieldBoundaryMotionState {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

// ROM 0xBCAE: apply the mode-5 vertical boundary adjustment used at the two
// field edges. All source comparisons are unsigned 16-bit coordinate tests.
void apply_mode_five_boundary_motion(FieldBoundaryMotionState& state,
                                     std::uint8_t mode);

struct EntityFixedPointPosition {
    std::int32_t world_x = 0;
    std::int32_t world_y = 0;
    std::int16_t tile_offset_x = 0;
    std::int16_t tile_offset_y = 0;
};

struct EntityWorldProbeResult {
    std::int32_t world_x = 0;
    std::int32_t world_y = 0;
    std::uint16_t terrain_result = 0;
};

using EntityWorldProbe =
    std::function<std::uint16_t(std::int32_t world_x,
                                std::int32_t world_y)>;

// ROM 0x14F26: add signed 8-bit-shifted tile offsets to an entity's fixed
// point position and pass the resulting host coordinates to the terrain
// probe. The source preserves the coordinate registers across that call.
EntityWorldProbeResult probe_entity_world_position(
    const EntityFixedPointPosition& position,
    const EntityWorldProbe& probe);

struct EntityHeadingOffsets {
    std::int16_t horizontal = 0;
    std::int16_t vertical = 0;
};

// ROM 0x150E8: select the two source direction tables from the heading's
// high byte, scale both signed samples by 0xC/0x10 with an arithmetic shift,
// and return the offsets stored at record fields 0x4C and 0x4E.
EntityHeadingOffsets derive_entity_heading_offsets(std::uint16_t heading);

// ROM 0x15646/0x15830: the effect/projectile constructors use the same two
// heading tables with source-specific signed multipliers and arithmetic
// shifts. Keeping the scale explicit avoids pretending those records use the
// ordinary 0xC/0x10 movement offset.
EntityHeadingOffsets derive_scaled_entity_heading_offsets(
    std::uint16_t heading,
    std::int16_t multiplier,
    std::uint8_t arithmetic_shift);

struct EntityHeadingVector {
    std::int16_t horizontal = 0;
    std::int16_t vertical = 0;
};

// ROM 0x15DCC: use the heading table as signed Q15 factors, multiply the
// signed word magnitude, double the products, and return their high words.
// A zero magnitude follows the source's explicit zero-vector branch.
EntityHeadingVector transform_entity_heading_vector(
    std::uint16_t heading,
    std::int16_t magnitude);

} // namespace shadowrun::ghidra_conversion

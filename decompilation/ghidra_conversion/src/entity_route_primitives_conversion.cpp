#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t clear_entity_route_flags(const std::uint8_t movement_flags) {
    return static_cast<std::uint8_t>(movement_flags & ~0x0AU);
}

std::uint16_t compute_obstruction_grid_index(
    const std::uint16_t world_x,
    const std::uint16_t world_y) {
    const auto row_offset = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(world_y >> 4U) & 0x7FE0U) * 2U);
    return static_cast<std::uint16_t>(row_offset + (world_x >> 9U));
}

ObstructionTileProjection decode_obstruction_tile_projection(
    const std::uint16_t packed_tile,
    const std::uint16_t reference_tile) {
    const auto world_x = static_cast<std::uint16_t>(
        ((packed_tile & 0x003FU) << 9U) + 0x0100U);
    const auto world_y = static_cast<std::uint16_t>(
        (((packed_tile >> 6U) << 9U) | (packed_tile >> 13U)) + 0x0100U);

    std::int8_t direction = 0;
    if ((packed_tile & 0xFFC0U) != (reference_tile & 0xFFC0U)) {
        direction = (reference_tile & 0xFFC0U) <
                (packed_tile & 0xFFC0U)
            ? 4
            : 0;
    } else {
        direction = (reference_tile & 0x003FU) <
                (packed_tile & 0x003FU)
            ? 2
            : 6;
    }

    return ObstructionTileProjection{world_x, world_y, direction};
}

HeadingProjection project_heading_position(
    const std::int32_t world_x,
    const std::int32_t world_y,
    const std::uint16_t heading) {
    struct HeadingDelta {
        std::int16_t x;
        std::int16_t y;
    };

    // ROM 0xB2F6, indexed by the top three bits of the heading word.
    static constexpr HeadingDelta kHeadingDeltas[] = {
        {0, -128}, {90, -90}, {128, 0}, {90, 90},
        {0, 128}, {-90, 90}, {-128, 0}, {-90, -90},
    };
    const auto heading_index = static_cast<std::size_t>(heading >> 13U);
    const auto& delta = kHeadingDeltas[heading_index];
    return HeadingProjection{
        world_x + delta.x,
        world_y + delta.y,
    };
}

std::uint8_t derive_entity_axis_relation_flags(
    const std::uint16_t entity_x,
    const std::uint16_t entity_y,
    const std::uint16_t target_x,
    const std::uint16_t target_y) {
    auto flags = static_cast<std::uint8_t>(0);
    if (entity_x <= target_x) {
        flags = static_cast<std::uint8_t>(flags | 0x01U);
    }
    if (entity_y <= target_y) {
        flags = static_cast<std::uint8_t>(flags | 0x02U);
    }
    return flags;
}

std::uint8_t find_first_available_entity_slot(
    const std::array<std::uint8_t, 6>& slot_flags) {
    for (std::size_t index = 0; index < slot_flags.size(); ++index) {
        if ((slot_flags[index] & 0x01U) == 0U) {
            return static_cast<std::uint8_t>(9U + index);
        }
    }
    return 15U;
}

} // namespace shadowrun::ghidra_conversion

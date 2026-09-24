#include "shadowrun_ghidra/entity_route_primitives_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint8_t clear_entity_route_flags(const std::uint8_t movement_flags) {
    return static_cast<std::uint8_t>(movement_flags & ~0x0AU);
}

std::optional<ObstructionSearchResult> find_first_occupied_obstruction_tile(
    const std::int32_t starting_tile,
    const std::uint8_t direction_slot,
    const std::vector<std::uint8_t>& obstruction_map) {
    static constexpr std::array<std::int16_t, 8> kDirectionSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    if (direction_slot > 6U || (direction_slot & 1U) != 0U) {
        return std::nullopt;
    }

    const auto step = kDirectionSteps[direction_slot];
    auto tile = starting_tile;
    while (true) {
        tile += step;
        if (tile < 0 ||
            tile >= static_cast<std::int32_t>(obstruction_map.size())) {
            return std::nullopt;
        }
        if (obstruction_map[static_cast<std::size_t>(tile)] != 0U) {
            return ObstructionSearchResult{tile, step};
        }
    }
}

ObstructionStripProbe probe_obstruction_strip(
    const std::int32_t starting_tile,
    const std::uint8_t primary_direction_slot,
    const std::uint8_t secondary_direction_slot,
    const std::vector<std::uint8_t>& obstruction_map) {
    static constexpr std::array<std::int16_t, 8> kDirectionSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    const auto valid_direction = [](const std::uint8_t direction) {
        return direction <= 6U && (direction & 1U) == 0U;
    };
    if (!valid_direction(primary_direction_slot) ||
        !valid_direction(secondary_direction_slot)) {
        return ObstructionStripProbe{starting_tile, starting_tile, true};
    }

    const auto primary_step = kDirectionSteps[primary_direction_slot];
    const auto secondary_step = kDirectionSteps[secondary_direction_slot];
    auto tile = starting_tile;
    while (true) {
        tile += primary_step;
        if (tile < 0 ||
            tile >= static_cast<std::int32_t>(obstruction_map.size())) {
            return ObstructionStripProbe{starting_tile, tile, true};
        }
        if (obstruction_map[static_cast<std::size_t>(tile)] != 0U) {
            return ObstructionStripProbe{starting_tile, tile, false};
        }

        const auto side_tile = tile + secondary_step;
        if (side_tile < 0 ||
            side_tile >= static_cast<std::int32_t>(obstruction_map.size()) ||
            obstruction_map[static_cast<std::size_t>(side_tile)] == 0U) {
            return ObstructionStripProbe{starting_tile, tile, false};
        }
    }
}

bool can_bypass_obstruction(
    const std::int32_t starting_tile,
    const std::uint8_t primary_direction_slot,
    const std::uint8_t secondary_direction_slot,
    const std::vector<std::uint8_t>& obstruction_map) {
    static constexpr std::array<std::int16_t, 8> kDirectionSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    const auto valid_direction = [](const std::uint8_t direction) {
        return direction <= 6U && (direction & 1U) == 0U;
    };
    if (!valid_direction(primary_direction_slot) ||
        !valid_direction(secondary_direction_slot)) {
        return true;
    }

    const auto primary_step = kDirectionSteps[primary_direction_slot];
    const auto secondary_step = kDirectionSteps[secondary_direction_slot];
    const auto opposite_step = kDirectionSteps[primary_direction_slot ^ 4U];
    const auto occupied = [&obstruction_map](const std::int32_t tile) {
        return tile < 0 ||
            tile >= static_cast<std::int32_t>(obstruction_map.size()) ||
            obstruction_map[static_cast<std::size_t>(tile)] != 0U;
    };

    auto candidate = starting_tile + primary_step;
    if (!occupied(candidate)) {
        const auto side = candidate + secondary_step;
        if (!occupied(side)) {
            return false;
        }
    }

    candidate = starting_tile;
    while (true) {
        candidate += opposite_step;
        if (occupied(candidate)) {
            return true;
        }
        if (!occupied(candidate + secondary_step)) {
            return false;
        }
    }
}

std::int32_t find_alternate_route_tile(
    const std::int32_t starting_tile,
    const std::uint8_t primary_direction_slot,
    const std::uint8_t secondary_direction_slot,
    const std::int32_t maximum_column,
    const std::int32_t maximum_row,
    const std::vector<std::uint8_t>& obstruction_map) {
    static constexpr std::array<std::int16_t, 8> kDirectionSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    const auto valid_direction = [](const std::uint8_t direction) {
        return direction <= 6U && (direction & 1U) == 0U;
    };
    if (!valid_direction(primary_direction_slot) ||
        !valid_direction(secondary_direction_slot)) {
        return starting_tile;
    }

    const auto in_grid = [maximum_column, maximum_row](
                             const std::int32_t tile) {
        return tile >= 0 &&
            (tile & 0x3FU) <= maximum_column &&
            (tile >> 6) <= maximum_row;
    };
    const auto open = [&obstruction_map](const std::int32_t tile) {
        return tile >= 0 &&
            tile < static_cast<std::int32_t>(obstruction_map.size()) &&
            obstruction_map[static_cast<std::size_t>(tile)] == 0U;
    };

    auto primary = primary_direction_slot;
    auto secondary = secondary_direction_slot;
    auto primary_step = static_cast<std::int32_t>(kDirectionSteps[primary]);
    auto secondary_step = static_cast<std::int32_t>(kDirectionSteps[secondary]);

    const auto initial_side = starting_tile + secondary_step;
    if (in_grid(initial_side) && open(initial_side)) {
        primary = static_cast<std::uint8_t>((primary + 2U) & 0x06U);
        secondary = static_cast<std::uint8_t>((secondary + 2U) & 0x06U);
        primary_step = kDirectionSteps[primary];
        secondary_step = kDirectionSteps[secondary];
    }

    auto candidate = starting_tile + primary_step;
    while (in_grid(candidate) && open(candidate)) {
        if (!open(candidate + secondary_step)) {
            candidate += primary_step;
            continue;
        }
        return candidate;
    }
    return candidate - primary_step;
}

std::optional<std::int32_t> find_rotating_route_strip_target(
    const std::uint16_t world_x,
    const std::uint16_t world_y,
    const std::uint16_t heading,
    const std::vector<std::uint8_t>& obstruction_map) {
    static constexpr std::array<std::int16_t, 8> kDirectionSteps{
        -64, 0, 1, 0, 64, 0, -1, 0};
    const auto starting_tile = static_cast<std::int32_t>(
        compute_obstruction_grid_index(world_x, world_y));
    auto primary_direction = static_cast<std::uint8_t>(
        ((static_cast<std::uint32_t>(heading) << 3U) |
         (static_cast<std::uint32_t>(heading) >> 13U)) &
        0x06U);
    auto secondary_direction = static_cast<std::uint8_t>(
        (primary_direction + 2U) & 0x06U);

    const auto open = [&obstruction_map](const std::int32_t tile) {
        return tile >= 0 &&
            tile < static_cast<std::int32_t>(obstruction_map.size()) &&
            obstruction_map[static_cast<std::size_t>(tile)] == 0U;
    };
    const auto probe = [&](const std::uint8_t primary,
                           const std::uint8_t secondary) {
        const auto primary_step = static_cast<std::int32_t>(
            kDirectionSteps[primary]);
        const auto secondary_step = static_cast<std::int32_t>(
            kDirectionSteps[secondary]);
        auto candidate = starting_tile + primary_step;
        while (open(candidate)) {
            if (open(candidate + secondary_step)) {
                return candidate;
            }
            candidate += primary_step;
        }
        return candidate - primary_step;
    };

    for (int attempt = 0; attempt < 5; ++attempt) {
        const auto candidate = probe(primary_direction, secondary_direction);
        if (candidate != starting_tile) {
            return candidate;
        }
        primary_direction = static_cast<std::uint8_t>(
            (primary_direction + 6U) & 0x06U);
        secondary_direction = static_cast<std::uint8_t>(
            (secondary_direction + 6U) & 0x06U);
    }
    return std::nullopt;
}

bool is_obstruction_path_blocked(
    const std::int32_t start_tile,
    const std::int32_t target_tile,
    const std::vector<std::uint8_t>& obstruction_map) {
    const auto occupied = [&obstruction_map](const std::int32_t tile) {
        return tile < 0 ||
            tile >= static_cast<std::int32_t>(obstruction_map.size()) ||
            obstruction_map[static_cast<std::size_t>(tile)] != 0U;
    };

    if ((target_tile & 0x003FU) == (start_tile & 0x003FU)) {
        auto tile = target_tile;
        const auto target_row = target_tile & 0xFFC0;
        const auto start_row = start_tile & 0xFFC0;
        const auto row_step = start_row < target_row ? -0x40 : 0x40;
        while ((tile & 0xFFC0) != start_row) {
            tile += row_step;
            if (occupied(tile)) {
                return true;
            }
        }
        return false;
    }

    auto tile = start_tile;
    const auto target_column = target_tile & 0x003FU;
    const auto column_step =
        (start_tile & 0x003FU) < target_column ? 1 : -1;
    while ((tile & 0x003FU) != target_column) {
        tile += column_step;
        if (occupied(tile)) {
            return true;
        }
    }
    return false;
}

bool is_entity_path_blocked(
    const std::uint16_t entity_x,
    const std::uint16_t entity_y,
    const std::uint16_t target_x,
    const std::uint16_t target_y,
    const std::vector<std::uint8_t>& obstruction_map) {
    const auto target_tile = compute_obstruction_grid_index(target_x, target_y);
    const auto entity_tile = compute_obstruction_grid_index(entity_x, entity_y);

    return is_obstruction_path_blocked(
               target_tile, entity_tile, obstruction_map) ||
        is_obstruction_path_blocked(
            entity_tile, target_tile, obstruction_map);
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

EntityAxisUpdate update_entity_axis_relation_state(
    const std::int32_t entity_x,
    const std::int32_t entity_y,
    const std::int32_t target_x,
    const std::int32_t target_y,
    std::uint8_t axis_flags,
    const std::int16_t movement_mode,
    std::uint8_t movement_flags) {
    const auto delta_x = target_x - entity_x;
    const auto delta_y = target_y - entity_y;

    // Bits 0 and 1 reverse the source's X/Y comparisons. Bits 2 and 3 are
    // only set by this routine; unrelated flags remain intact.
    const auto x_points_positive = (axis_flags & 0x01U) == 0U;
    if ((x_points_positive && delta_x >= 0) ||
        (!x_points_positive && delta_x < 0)) {
        axis_flags = static_cast<std::uint8_t>(axis_flags | 0x04U);
    }

    const auto y_points_positive = (axis_flags & 0x02U) == 0U;
    if ((y_points_positive && delta_y >= 0) ||
        (!y_points_positive && delta_y < 0)) {
        axis_flags = static_cast<std::uint8_t>(axis_flags | 0x08U);
    }

    if (movement_mode >= 0) {
        const auto x_relation = (axis_flags & 0x04U) != 0U;
        const auto y_relation = (axis_flags & 0x08U) != 0U;
        const auto diagonal_mode = (movement_mode & 0x0002) != 0;
        if ((!x_relation && (!y_relation || !diagonal_mode)) ||
            (x_relation && diagonal_mode)) {
            return EntityAxisUpdate{axis_flags, movement_flags, false};
        }
    }

    movement_flags = static_cast<std::uint8_t>(movement_flags | 0x02U);
    return EntityAxisUpdate{axis_flags, movement_flags, true};
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

#include "shadowrun_ghidra/field/terrain_map_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

bool replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    const TerrainMarkerReplacementState& state) {
    if (state.field_mode != 2U ||
        (state.effect_context_active == 0U &&
         state.alternate_effect_context == 0U &&
         state.effect_handler_index != 4U) ||
        state.selected_effect_index != state.active_effect_index ||
        state.world_effect_active) {
        return false;
    }

    auto remaining = static_cast<std::int16_t>(
        static_cast<std::int8_t>(state.marker_ordinal));
    for (auto& value : map) {
        if (value != 0xDAU) {
            continue;
        }
        --remaining;
        if (remaining < 0) {
            value = 0x9AU;
            return true;
        }
    }
    return false;
}

void replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    const std::uint8_t marker,
    const std::uint8_t replacement,
    std::uint16_t ordinal) {
    for (auto& value : map) {
        if (value != marker) {
            continue;
        }
        if (ordinal == 0U) {
            value = replacement;
            return;
        }
        --ordinal;
    }
}

void rebuild_terrain_collision_map(
    std::array<std::uint8_t, 0x1000>& map,
    bool& rebuild_requested,
    const std::vector<TerrainCollisionRectangle>& rectangles,
    const TerrainMapPostProcess& post_process) {
    if (!rebuild_requested) {
        return;
    }

    rebuild_requested = false;
    map.fill(0);
    for (const auto& rectangle : rectangles) {
        for (std::uint32_t y = rectangle.top;
             y <= rectangle.bottom; y += 0x20U) {
            for (std::uint32_t x = rectangle.left;
                 x <= rectangle.right; x += 0x20U) {
                const auto map_index = static_cast<std::size_t>(
                    ((((y & 0xFFE0U) << 1U) + (x >> 5U)) & 0x0FFFU));
                map[map_index] = rectangle.value;
            }
        }
    }
    post_process(map);
}

void copy_terrain_layer(
    std::vector<std::uint16_t>& destination,
    const std::vector<std::uint16_t>& source,
    const std::size_t word_count) {
    const auto count = word_count < source.size() ? word_count : source.size();
    const auto bounded_count =
        count < destination.size() ? count : destination.size();
    for (std::size_t index = 0; index < bounded_count; ++index) {
        destination[index] = source[index];
    }
}

} // namespace shadowrun::ghidra_conversion

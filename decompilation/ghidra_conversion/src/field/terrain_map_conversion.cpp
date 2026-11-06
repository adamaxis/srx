#include "shadowrun_ghidra/field/terrain_map_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

bool marker_replacement_enabled(
    const TerrainMarkerReplacementState& state) {
    return state.field_mode == 2U &&
           (state.effect_context_active ||
            state.alternate_effect_context ||
            state.package_recovery_active) &&
           state.selected_effect_index == state.active_effect_index &&
           !state.world_effect_active;
}

template <typename TerrainRange>
std::optional<std::size_t> select_effect_terrain_marker_cell(
    const TerrainRange& terrain,
    const TerrainMarkerReplacementState& state) {
    if (!marker_replacement_enabled(state)) {
        return std::nullopt;
    }

    auto remaining = static_cast<std::int16_t>(
        static_cast<std::int8_t>(state.marker_ordinal));
    for (std::size_t index = 0U; index < terrain.size(); ++index) {
        if (terrain[index] != 0xDAU) {
            continue;
        }
        --remaining;
        if (remaining < 0) {
            return index;
        }
    }
    return std::nullopt;
}

} // namespace

std::optional<std::size_t> select_effect_terrain_marker_cell(
    const std::vector<std::uint8_t>& terrain,
    const TerrainMarkerReplacementState& state) {
    return select_effect_terrain_marker_cell<std::vector<std::uint8_t>>(
        terrain, state);
}

std::optional<std::size_t> select_effect_terrain_marker_cell(
    const std::array<std::uint8_t, 0x1000>& terrain,
    const TerrainMarkerReplacementState& state) {
    return select_effect_terrain_marker_cell<std::array<std::uint8_t, 0x1000>>(
        terrain, state);
}

bool replace_nth_terrain_marker(
    std::array<std::uint8_t, 0x1000>& map,
    const TerrainMarkerReplacementState& state) {
    const auto selected = select_effect_terrain_marker_cell(map, state);
    if (!selected.has_value()) {
        return false;
    }
    map[*selected] = 0x9AU;
    return true;
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

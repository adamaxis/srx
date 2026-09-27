#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t tile_index(
    const std::uint16_t x,
    const std::uint16_t y) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(x >> 5U) +
        static_cast<std::uint16_t>((y & 0x7FE0U) << 1U));
}

std::uint16_t add_word(
    const std::uint16_t value,
    const std::int16_t offset) {
    return static_cast<std::uint16_t>(
        static_cast<std::int32_t>(value) + offset);
}

} // namespace

TerrainTileSample sample_terrain_collision_tiles(
    const std::uint16_t x,
    const std::uint16_t y,
    const bool expanded_probe,
    const TerrainTileLookup& tile_lookup) {
    TerrainTileSample sample{};
    if (!expanded_probe) {
        sample.selected_tile_index = tile_index(x, y);
        sample.values[0] = tile_lookup(sample.selected_tile_index);
        sample.selected_value = sample.values[0];
        sample.contains_blocked_tile = sample.selected_value == 0x7FU;
        return sample;
    }

    const std::array<std::pair<std::int16_t, std::int16_t>, 4> offsets{{
        {-12, -7}, {12, -7}, {-12, 7}, {12, 7}}};
    std::uint8_t highest = 0;
    for (std::size_t index = 0; index < offsets.size(); ++index) {
        const auto probe_x = add_word(x, offsets[index].first);
        const auto probe_y = add_word(y, offsets[index].second);
        const auto index_value = tile_index(probe_x, probe_y);
        sample.values[index] = tile_lookup(index_value);
        if (sample.values[index] == 0x7FU) {
            sample.contains_blocked_tile = true;
        }
        if (index == 0 || sample.values[index] >= highest) {
            highest = sample.values[index];
            sample.selected_tile_index = index_value;
        }
    }

    sample.selected_value =
        sample.contains_blocked_tile ? 0x7FU : highest;
    return sample;
}

bool evaluate_entity_terrain_collision_directions(
    const TerrainTileSample& sample,
    const std::uint16_t x,
    const std::uint16_t y) {
    const auto window = decode_terrain_tile_window(sample.selected_tile_index);
    const std::array<std::pair<std::int16_t, std::int16_t>, 4> offsets{{
        {-12, -7}, {12, -7}, {-12, 7}, {12, 7}}};
    for (std::size_t index = 0; index < sample.values.size(); ++index) {
        if ((sample.values[index] & 0x20U) == 0U) {
            continue;
        }
        const auto probe_x = add_word(x, offsets[index].first);
        const auto probe_y = add_word(y, offsets[index].second);
        if (evaluate_terrain_direction_boundary(
                sample.values[index], window, probe_x, probe_y)) {
            return true;
        }
    }
    return false;
}

TerrainCollisionResult evaluate_terrain_collision(
    const TerrainCollisionInput& input,
    const TerrainCollisionCallbacks& callbacks) {
    TerrainCollisionResult result{};
    const auto normalized_x = static_cast<std::uint16_t>(input.source_x >> 4U);
    const auto normalized_y = static_cast<std::uint16_t>(input.source_y >> 4U);
    const auto& bounds = input.bounds;
    if (normalized_x < bounds.minimum_x_tile ||
        normalized_x >= bounds.exclusive_maximum_x_tile ||
        normalized_y < bounds.minimum_y_tile ||
        normalized_y >= bounds.exclusive_maximum_y_tile) {
        result.state.outside_bounds_marker = true;
        result.state.sample.selected_value = 0x7FU;
        result.state.sample.values[0] = 0x7FU;
        result.state.sample.values[2] = 0x7FU;
        result.code = 0x7F;
        return result;
    }

    const bool expanded_probe = (input.entity_flags & 0x04U) != 0U;
    if (callbacks.sample_tiles) {
        result.state.sample =
            callbacks.sample_tiles(normalized_x, normalized_y, expanded_probe);
    }
    const auto selected_value = result.state.sample.selected_value;
    result.state.raw_terrain_code = static_cast<std::int16_t>(
        static_cast<std::int8_t>(selected_value));
    result.code = result.state.raw_terrain_code;
    if (selected_value == 0U || selected_value == 0x7FU ||
        result.state.raw_terrain_code < 0) {
        return result;
    }

    const auto directional_collision = [&] {
        if (callbacks.evaluate_directions) {
            return callbacks.evaluate_directions(
                result.state.sample, normalized_x, normalized_y);
        }
        return evaluate_entity_terrain_collision_directions(
            result.state.sample, normalized_x, normalized_y);
    };
    if ((selected_value & 0x40U) != 0U) {
        result.state.directional_collision_marker = true;
        if (selected_value == 0x7EU || (selected_value & 0x20U) == 0U ||
            directional_collision()) {
            result.code = 0x7F;
        } else {
            result.code = 0;
        }
        return result;
    }

    if ((selected_value & 0x20U) == 0U) {
        const bool contains_object =
            callbacks.contains_object &&
            callbacks.contains_object(
                selected_value,
                static_cast<std::int16_t>(normalized_x),
                static_cast<std::int16_t>(normalized_y));
        result.code = contains_object ? 0x7F : 0;
        return result;
    }

    result.code = directional_collision() ? 0x7F : 0;
    return result;
}

bool contains_terrain_object_bounds(
    const std::uint16_t object_id,
    const std::int16_t x,
    const std::int16_t y,
    const TerrainObjectLookup& object_lookup) {
    const auto object = object_lookup(object_id);
    return x >= object.center_x - 12 && x <= object.center_x + 12 &&
           y >= object.center_y - 7 && y <= object.center_y + 7;
}

TerrainTileWindow decode_terrain_tile_window(
    const std::uint16_t tile_index_value) {
    const auto row_origin = static_cast<std::uint16_t>(
        (tile_index_value & static_cast<std::uint16_t>(~0x003FU)) >> 1U);
    return TerrainTileWindow{
        static_cast<std::uint16_t>((tile_index_value & 0x003FU) << 5U),
        row_origin,
        static_cast<std::uint16_t>(row_origin + 0x001FU)};
}

bool evaluate_terrain_direction_boundary(
    const std::uint8_t terrain_code,
    const TerrainTileWindow& window,
    const std::uint16_t x,
    const std::uint16_t y) {
    const auto x_delta = static_cast<std::uint16_t>(x - window.origin_x);
    const auto y_delta = static_cast<std::uint16_t>(y - window.origin_y);
    const auto end_delta = static_cast<std::uint16_t>(window.end_y - y);

    switch (terrain_code & 0x38U) {
    case 0x20U:
        return x_delta < end_delta;
    case 0x28U:
        return x_delta >= y_delta;
    case 0x38U:
        return x_delta >= end_delta;
    default:
        return x_delta < y_delta;
    }
}

} // namespace shadowrun::ghidra_conversion

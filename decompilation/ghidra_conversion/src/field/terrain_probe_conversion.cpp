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

std::uint32_t pack_sample_state(
    const std::array<std::uint8_t, 4>& values) {
    return (static_cast<std::uint32_t>(values[0]) << 24U) |
           (static_cast<std::uint32_t>(values[1]) << 16U) |
           (static_cast<std::uint32_t>(values[2]) << 8U) |
           static_cast<std::uint32_t>(values[3]);
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
    result.packed_sample_state = pack_sample_state(input.prior_sample_state);
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
        // MOVE.W #$7F,(FFEF10) and MOVE.W #$7F,(FFEF12) publish
        // 00 7F 00 7F before the 0xB48C wrapper copies the state.
        result.packed_sample_state = 0x007F007FU;
        result.code = 0x7F;
        return result;
    }

    const bool expanded_probe = (input.entity_flags & 0x04U) != 0U;
    if (callbacks.sample_tiles) {
        result.state.sample =
            callbacks.sample_tiles(normalized_x, normalized_y, expanded_probe);
    }
    auto sample_state = input.prior_sample_state;
    if (expanded_probe) {
        sample_state = result.state.sample.values;
    } else {
        sample_state[0] = result.state.sample.values[0];
    }
    result.packed_sample_state = pack_sample_state(sample_state);
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
    const auto expand_word = [](const std::int16_t value,
                                const std::int16_t amount) {
        return static_cast<std::int16_t>(
            static_cast<std::int32_t>(value) + amount);
    };
    return x >= expand_word(object.left, -12) &&
           x <= expand_word(object.right, 12) &&
           y >= expand_word(object.top, -7) &&
           y <= expand_word(object.bottom, 7);
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

FieldTileSampleOrigin compose_field_tile_sample_origin(
    const std::uint16_t camera_x,
    const std::uint16_t camera_y,
    const std::int16_t entity_offset_x,
    const std::int16_t entity_offset_y,
    const std::int16_t stamp_x,
    const std::int16_t stamp_y) {
    const auto x = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(stamp_x) +
        static_cast<std::uint16_t>(entity_offset_x) - 0x0080U +
        (camera_x >> 4U));
    const auto y = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(stamp_y) +
        static_cast<std::uint16_t>(entity_offset_y) - 0x0080U +
        (camera_y >> 4U));
    return {x, y};
}

std::uint8_t field_tile_sample_nibble_mask(const std::uint8_t packed) {
    if (packed == 0U) {
        return 0U;
    }
    if ((packed & 0xF0U) == 0U) {
        return 0x0FU;
    }
    if ((packed & 0x0FU) == 0U) {
        return 0xF0U;
    }
    return 0xFFU;
}

std::size_t decode_field_tile_sample(
    const FieldTileSampleOrigin origin,
    const std::uint16_t size_word,
    const FieldTileSampleDecodeCallbacks& callbacks) {
    if (!callbacks.seek_tile || !callbacks.read_pattern ||
        !callbacks.read_sample || !callbacks.write_masked) {
        return 0;
    }
    auto x = origin.x;
    const auto saved_y = origin.y;
    auto rows = static_cast<std::int16_t>(size_word >> 2U);
    const auto saved_columns = static_cast<std::int16_t>(size_word & 3U);
    std::size_t written = 0;
    do {
        auto columns = saved_columns;
        auto y = saved_y;
        do {
            callbacks.seek_tile(x, y);
            auto remaining = static_cast<std::int16_t>(0x001F);
            do {
                const auto packed = callbacks.read_pattern();
                const auto sample = callbacks.read_sample();
                callbacks.write_masked(static_cast<std::uint8_t>(
                    field_tile_sample_nibble_mask(packed) & sample));
                ++written;
                remaining = static_cast<std::int16_t>(remaining - 1);
            } while (remaining != -1);
            y = static_cast<std::uint16_t>(y + 8U);
            columns = static_cast<std::int16_t>(columns - 1);
        } while (columns != -1);
        x = static_cast<std::uint16_t>(x + 8U);
        rows = static_cast<std::int16_t>(rows - 1);
    } while (rows != -1);
    return written;
}

} // namespace shadowrun::ghidra_conversion

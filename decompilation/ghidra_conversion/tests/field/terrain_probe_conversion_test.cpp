#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"

#include <cstdint>
#include <map>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::map<std::uint16_t, std::uint8_t> tiles{
        {static_cast<std::uint16_t>((0x1FF0U >> 5U) +
                                    ((0x2FF9U & 0x7FE0U) << 1U)),
         3},
        {static_cast<std::uint16_t>((0x2008U >> 5U) +
                                    ((0x2FF9U & 0x7FE0U) << 1U)),
         8},
        {static_cast<std::uint16_t>((0x1FF0U >> 5U) +
                                    ((0x3007U & 0x7FE0U) << 1U)),
         5},
        {static_cast<std::uint16_t>((0x2008U >> 5U) +
                                    ((0x3007U & 0x7FE0U) << 1U)),
         8}};
    const auto lookup = [&tiles](const std::uint16_t index) {
        const auto found = tiles.find(index);
        return found == tiles.end() ? static_cast<std::uint8_t>(0)
                                    : found->second;
    };

    const auto expanded = sample_terrain_collision_tiles(
        0x2000, 0x3000, true, lookup);
    if (!expect(expanded.values[0] == 3 && expanded.values[1] == 8 &&
                expanded.values[2] == 5 && expanded.values[3] == 8) ||
        !expect(expanded.selected_value == 8) ||
        !expect(expanded.selected_tile_index ==
                static_cast<std::uint16_t>((0x2008U >> 5U) +
                                            ((0x3007U & 0x7FE0U) << 1U))) ||
        !expect(!expanded.contains_blocked_tile)) {
        return 1;
    }

    const auto blocked = sample_terrain_collision_tiles(
        0x2000, 0x3000, true,
        [](const std::uint16_t index) {
            return index == static_cast<std::uint16_t>(
                                 (0x1FF0U >> 5U) +
                                 ((0x2FF9U & 0x7FE0U) << 1U))
                       ? static_cast<std::uint8_t>(0x7F)
                       : static_cast<std::uint8_t>(1);
        });
    if (!expect(blocked.selected_value == 0x7F) ||
        !expect(blocked.contains_blocked_tile)) {
        return 2;
    }

    const auto center = sample_terrain_collision_tiles(
        0x2000, 0x3000, false,
        [](const std::uint16_t index) {
            return index == static_cast<std::uint16_t>(
                                 (0x2000U >> 5U) +
                                 ((0x3000U & 0x7FE0U) << 1U))
                       ? static_cast<std::uint8_t>(6)
                       : static_cast<std::uint8_t>(0);
        });
    if (!expect(center.selected_value == 6 &&
                center.values[0] == 6 &&
                !center.contains_blocked_tile)) {
        return 3;
    }

    TerrainTileSample directional_sample{};
    directional_sample.selected_tile_index = 0x0845U;
    directional_sample.values[0] = 0x20U;
    if (!expect(evaluate_entity_terrain_collision_directions(
            directional_sample, 0x00B0U, 0x0430U))) {
        return 4;
    }

    const TerrainCollisionBounds collision_bounds{0x0100U, 0x0300U,
                                                   0x0100U, 0x0400U};
    int sample_calls = 0;
    bool saw_expanded_probe = false;
    const auto collision_callbacks = TerrainCollisionCallbacks{
        [&](const std::uint16_t x, const std::uint16_t y,
            const bool expanded_probe) {
            ++sample_calls;
            saw_expanded_probe = expanded_probe && x == 0x0200U && y == 0x0300U;
            TerrainTileSample result{};
            result.selected_value = 0x10U;
            return result;
        },
        [](const std::uint8_t object_id, const std::int16_t x,
           const std::int16_t y) {
            return object_id == 0x10U && x == 0x0200 && y == 0x0300;
        },
        {}};
    const auto object_collision = evaluate_terrain_collision(
        {0x2000U, 0x3000U, 0x04U, collision_bounds}, collision_callbacks);
    if (!expect(object_collision.code == 0x7F && sample_calls == 1 &&
                saw_expanded_probe && object_collision.state.raw_terrain_code ==
                    0x10)) {
        return 5;
    }

    const auto directional_collision = evaluate_terrain_collision(
        {0x2000U, 0x3000U, 0x04U, collision_bounds},
        {[](const std::uint16_t, const std::uint16_t, const bool) {
             TerrainTileSample result{};
             result.selected_value = 0x60U;
             return result;
         },
         {},
         [](const TerrainTileSample&, const std::uint16_t, const std::uint16_t) {
             return true;
         }});
    if (!expect(directional_collision.code == 0x7F &&
                directional_collision.state.directional_collision_marker)) {
        return 6;
    }

    const auto outside_collision = evaluate_terrain_collision(
        {0x0FFFU, 0x3000U, 0x00U, collision_bounds}, {});
    if (!expect(outside_collision.code == 0x7F &&
                outside_collision.state.outside_bounds_marker)) {
        return 7;
    }

    const auto object_lookup = [](const std::uint16_t object_id) {
        return TerrainObjectBounds{
            static_cast<std::int16_t>(0x1000 + object_id),
            static_cast<std::int16_t>(0x2000 + object_id)};
    };
    if (!expect(contains_terrain_object_bounds(
            4, 0x1000 + 4 + 12, 0x2000 + 4 - 7, object_lookup)) ||
        !expect(!contains_terrain_object_bounds(
            4, 0x1000 + 4 + 13, 0x2000 + 4, object_lookup))) {
        return 8;
    }

    const auto window = decode_terrain_tile_window(0x0845);
    if (!expect(window.origin_x == 0x00A0 &&
                window.origin_y == 0x0420 &&
                window.end_y == 0x043F)) {
        return 9;
    }
    if (!expect(evaluate_terrain_direction_boundary(
                    0x20, window, 0x00A1, 0x0425)) ||
        !expect(!evaluate_terrain_direction_boundary(
                    0x20, window, 0x00A1, 0x043F)) ||
        !expect(evaluate_terrain_direction_boundary(
                    0x28, window, 0x00B0, 0x0420)) ||
        !expect(!evaluate_terrain_direction_boundary(
                    0x38, window, 0x00A1, 0x0425))) {
        return 10;
    }

    return 0;
}

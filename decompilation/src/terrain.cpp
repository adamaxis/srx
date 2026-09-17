#include "shadowrun/terrain.hpp"

#include <algorithm>

namespace shadowrun::terrain {
namespace {

std::uint16_t wrap_word(std::int32_t value) {
    return static_cast<std::uint16_t>(value);
}

std::int16_t signed_word(std::uint16_t value) {
    return static_cast<std::int16_t>(value);
}

std::uint8_t cell_status(const Map& map, std::uint16_t x,
                         std::uint16_t y, std::size_t& selected_cell) {
    const auto column = static_cast<std::size_t>(x >> 5U);
    const auto row = static_cast<std::size_t>((y & 0xFFE0U) >> 5U);
    if (column >= Map::kWidth || row >= Map::kHeight ||
        map.statuses.size() != Map::kWidth * Map::kHeight) {
        selected_cell = 0;
        return 0x7FU;
    }
    selected_cell = row * Map::kWidth + column;
    return map.statuses[selected_cell];
}

struct SelectedCell {
    std::uint16_t offset = 0;
    std::uint8_t status = 0;
};

SelectedCell select_cell(const Map& map, std::uint16_t x, std::uint16_t y,
                         bool sample_four_corners) {
    auto selected_cell = std::size_t{0};
    auto best = cell_status(map, x, y, selected_cell);
    SelectedCell result{
        static_cast<std::uint16_t>(selected_cell), best};
    if (!sample_four_corners) {
        return result;
    }

    bool found_seven_f = best == 0x7FU;
    const auto consider = [&](std::int16_t x_delta,
                              std::int16_t y_delta,
                              SelectedCell& current) {
        std::size_t candidate_cell = 0;
        const auto candidate = cell_status(
            map, wrap_word(static_cast<std::int32_t>(x) + x_delta),
            wrap_word(static_cast<std::int32_t>(y) + y_delta),
            candidate_cell);
        if (candidate == 0x7FU) {
            found_seven_f = true;
        }
        if (candidate >= current.status) {
            current = {static_cast<std::uint16_t>(candidate_cell), candidate};
        }
    };
    // ROM 0xE8A0 considers (-12,-7), (+12,-7), (-12,+7), (+12,+7).
    result = {static_cast<std::uint16_t>(selected_cell), best};
    consider(-0x000C, -0x0007, result);
    consider(0x000C, -0x0007, result);
    consider(-0x000C, 0x0007, result);
    consider(0x000C, 0x0007, result);
    if (found_seven_f) {
        result.status = 0x7FU;
    }
    return result;
}

struct Geometry {
    std::uint16_t left = 0;
    std::uint16_t top = 0;
    std::uint16_t right = 0;
};

Geometry geometry_for_cell(std::uint16_t offset) {
    const auto left = static_cast<std::uint16_t>(
        (offset & 0x003FU) << 5U);
    const auto top = static_cast<std::uint16_t>(
        (offset & 0xFFC0U) >> 1U);
    return {left, top, static_cast<std::uint16_t>(top + 0x001FU)};
}

std::uint16_t negated_difference(std::uint16_t left, std::uint16_t right) {
    return wrap_word(-static_cast<std::int32_t>(
        wrap_word(static_cast<std::int32_t>(left) - right)));
}

bool directional_test(const SelectedCell& cell, std::uint16_t x,
                      std::uint16_t y) {
    const auto geometry = geometry_for_cell(cell.offset);
    const auto kind = static_cast<std::uint16_t>(cell.status & 0x0038U);
    if (kind == 0x0020U) {
        return negated_difference(geometry.left, x) >=
               static_cast<std::uint16_t>(geometry.right - y);
    }
    if (kind == 0x0028U) {
        const auto left = negated_difference(geometry.top, y);
        const auto right = negated_difference(geometry.right, x);
        return right < left;
    }
    if (kind == 0x0038U) {
        return negated_difference(geometry.left, x) <
               static_cast<std::uint16_t>(geometry.right - y);
    }
    return negated_difference(geometry.left, x) >=
           negated_difference(geometry.top, y);
}

bool rectangle_test(const Map& map, std::uint8_t status,
                    std::uint16_t x, std::uint16_t y) {
    const auto index = static_cast<std::size_t>(status);
    if (index >= map.regions.size()) {
        // A status outside the reviewed rectangle table has no safe native
        // interpretation. ROM paths treat negative statuses as terminal;
        // fail closed for a native caller instead of indexing arbitrary data.
        return true;
    }
    const auto& rectangle = map.regions[index];
    const auto x_min = wrap_word(static_cast<std::int32_t>(rectangle.x_start) -
                                 0x000C);
    const auto x_max = wrap_word(static_cast<std::int32_t>(rectangle.x_end) +
                                 0x000C);
    const auto y_min = wrap_word(static_cast<std::int32_t>(rectangle.y_start) -
                                 0x0007);
    const auto y_max = wrap_word(static_cast<std::int32_t>(rectangle.y_end) +
                                 0x0007);
    return signed_word(x) >= signed_word(x_min) &&
           signed_word(x) <= signed_word(x_max) &&
           signed_word(y) >= signed_word(y_min) &&
           signed_word(y) <= signed_word(y_max);
}

} // namespace

ProbeResult probe(const Map& map, std::uint16_t world_x_fixed,
                  std::uint16_t world_y_fixed, bool sample_four_corners) {
    const auto x = static_cast<std::uint16_t>(world_x_fixed >> 4U);
    const auto y = static_cast<std::uint16_t>(world_y_fixed >> 4U);
    if (x >= map.x_exclusive_upper || x < map.x_inclusive_lower ||
        y >= map.y_exclusive_upper || y < map.y_inclusive_lower) {
        return {0x7FU, 0x7FU, true, false, true, 0};
    }

    const auto selected = select_cell(map, x, y, sample_four_corners);
    if (selected.status == 0 || selected.status == 0x7FU ||
        signed_word(selected.status) < 0) {
        return {selected.status, selected.status, false, false,
                selected.status != 0,
                selected.offset};
    }

    const auto directional = (selected.status & 0x40U) != 0 ||
                             (selected.status & 0x20U) != 0;
    if ((selected.status & 0x40U) != 0) {
        if (selected.status == 0x7EU ||
            (selected.status & 0x20U) == 0) {
        return {selected.status, 0x7FU, false, true, true,
                    selected.offset};
        }
    }
    const auto blocked = directional
        ? directional_test(selected, x, y)
        : rectangle_test(map, selected.status, x, y);
    return {selected.status, static_cast<std::uint8_t>(blocked ? 0x7FU : 0),
            false, (selected.status & 0x40U) != 0, blocked,
            selected.offset};
}

} // namespace shadowrun::terrain

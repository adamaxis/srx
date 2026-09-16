#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::terrain {

struct Rectangle {
    std::uint16_t x_start = 0;
    std::uint16_t x_end = 0;
    std::uint16_t y_start = 0;
    std::uint16_t y_end = 0;
    std::uint8_t status = 0;
};

// Host representation of the byte table rebuilt by ROM 0xEE10. Coordinates
// are the post-0xE7C8-shift world-pixel units; the grid itself is one cell per
// 32 pixels. Rectangles are retained because positive status bytes can select
// their source record in ROM 0xE9EE.
struct Map {
    static constexpr std::size_t kWidth = 64;
    static constexpr std::size_t kHeight = 64;

    std::uint16_t x_exclusive_upper = 0;
    std::uint16_t y_exclusive_upper = 0;
    std::uint16_t x_inclusive_lower = 0;
    std::uint16_t y_inclusive_lower = 0;
    std::size_t region_count = 0;
    std::vector<Rectangle> regions;
    std::vector<std::uint8_t> statuses;
};

struct ProbeResult {
    std::uint8_t status = 0;
    bool out_of_bounds = false;
    bool collision_gate = false;
    bool blocked = false;
    std::size_t selected_cell = 0;
};

// Renderer-neutral translation of ROM 0xE7C8, including its 0xE8A0 grid
// lookup and the directional/rectangle tests at 0xEA36/0xEAD2/0xE9EE.
// Inputs retain the record's 1/16-pixel fixed-point representation.
ProbeResult probe(const Map& map,
                  std::uint16_t world_x_fixed,
                  std::uint16_t world_y_fixed,
                  bool sample_four_corners = false);

} // namespace shadowrun::terrain

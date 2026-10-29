#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// A game-level direction shared by field input, scripted movement, and the
// actor presentation boundary. It carries no controller or ROM encoding.
enum class MovementDirection : std::uint8_t {
    north,
    north_east,
    east,
    south_east,
    south,
    south_west,
    west,
    north_west,
};

} // namespace shadowrun::ghidra_conversion

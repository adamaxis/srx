#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WorldEffectEdgeCandidateState {
    // Source field words FFDDC6/FFDDC8 are the current world origin and
    // FFDDCE/FFDDD0 are the map dimensions used to choose an edge.
    std::uint16_t map_width = 0;
    std::uint16_t origin_x = 0;
    std::uint16_t origin_y = 0;
    std::uint16_t map_height = 0;
    std::vector<std::pair<std::uint16_t, std::uint16_t>> candidates;
};

struct WorldEffectEdgeCandidateCallbacks {
    // The source accepts only coordinates whose compact world tile lookup is
    // zero. An optional result keeps an absent host table entry distinct from
    // a real zero-valued tile.
    std::function<std::optional<std::uint8_t>(
        std::uint16_t x, std::uint16_t y)> lookup_tile;
};

// ROM 0x1E302: choose the map edge opposite the current origin, align a
// five-point Y ring on x_anchor+x_step and a five-point X ring on x_anchor
// at y_anchor+y_step, and keep coordinates whose world tile lookup is zero.
void build_world_effect_edge_candidates(
    WorldEffectEdgeCandidateState& state,
    const WorldEffectEdgeCandidateCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

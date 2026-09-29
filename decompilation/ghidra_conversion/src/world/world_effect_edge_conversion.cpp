#include "shadowrun_ghidra/world/world_effect_edge_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint16_t alignment_mask = 0xFE00U;

std::uint16_t add_word(const std::uint16_t value, const std::int32_t delta) {
    return static_cast<std::uint16_t>(static_cast<std::int32_t>(value) + delta);
}

std::uint16_t align_coordinate(const std::uint16_t value) {
    return add_word(static_cast<std::uint16_t>(value & alignment_mask), 0x100);
}

bool is_open_tile(const WorldEffectEdgeCandidateCallbacks& callbacks,
                  const std::uint16_t x,
                  const std::uint16_t y) {
    if (!callbacks.lookup_tile) {
        return false;
    }
    const auto tile = callbacks.lookup_tile(x, y);
    return tile.has_value() && *tile == 0U;
}

} // namespace

void build_world_effect_edge_candidates(
    WorldEffectEdgeCandidateState& state,
    const WorldEffectEdgeCandidateCallbacks& callbacks) {
    state.candidates.clear();

    std::int32_t x_step = 0x0BE0;
    const auto x_anchor = add_word(state.origin_x, 0x0A00);
    if ((state.map_width >> 1U) < x_anchor) {
        x_step = -0x0BE0;
    }

    std::int32_t y_step = 0x08C0;
    const auto y_anchor = add_word(state.origin_y, 0x0700);
    if ((state.map_height >> 1U) < y_anchor) {
        y_step = -0x08C0;
    }

    const auto edge_x = align_coordinate(add_word(x_anchor, x_step));
    const auto origin_x = align_coordinate(x_anchor);
    const auto first_y = align_coordinate(y_anchor);
    const auto second_y = align_coordinate(add_word(y_anchor, y_step));

    constexpr std::array<std::int32_t, 5> ring_offsets{
        0, 0x0200, 0x0400, -0x0200, -0x0400};
    const auto append_ring = [&](const std::uint16_t fixed_x,
                                 const std::uint16_t fixed_y,
                                 const bool vary_x) {
        for (const auto offset : ring_offsets) {
            const auto candidate_x = vary_x ? add_word(fixed_x, offset)
                                            : fixed_x;
            const auto candidate_y = vary_x ? fixed_y
                                            : add_word(fixed_y, offset);
            if (is_open_tile(callbacks, candidate_x, candidate_y)) {
                state.candidates.emplace_back(candidate_x, candidate_y);
            }
        }
    };

    // First ring: D5 = aligned x_anchor+x_step, vary D6 from aligned y_anchor.
    // Second ring: D5 = aligned x_anchor, D6 = aligned y_anchor+y_step, vary D5.
    append_ring(edge_x, first_y, false);
    append_ring(origin_x, second_y, true);
}

} // namespace shadowrun::ghidra_conversion

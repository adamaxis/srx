#include "shadowrun_ghidra/world_effect_edge_conversion.hpp"

#include <cstdint>
#include <utility>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldEffectEdgeCandidateState state{
        0x2000U, 0x1800U, 0x1800U, 0x2000U, {}};
    std::vector<std::pair<std::uint16_t, std::uint16_t>> lookups;
    build_world_effect_edge_candidates(
        state,
        WorldEffectEdgeCandidateCallbacks{
            [&](const std::uint16_t x, const std::uint16_t y)
                -> std::optional<std::uint8_t> {
                lookups.emplace_back(x, y);
                return 0U;
            }});

    if (lookups.size() != 10U || state.candidates != lookups ||
        lookups.front() != std::pair<std::uint16_t, std::uint16_t>{0x1700U,
                                                                    0x1F00U} ||
        lookups[4] != std::pair<std::uint16_t, std::uint16_t>{0x1700U,
                                                               0x1B00U} ||
        lookups[5] != std::pair<std::uint16_t, std::uint16_t>{0x1700U,
                                                               0x1700U} ||
        lookups.back() != std::pair<std::uint16_t, std::uint16_t>{0x1300U,
                                                                   0x1700U}) {
        return 1;
    }

    state.candidates.emplace_back(1U, 2U);
    std::size_t blocked_lookups = 0U;
    build_world_effect_edge_candidates(
        state,
        WorldEffectEdgeCandidateCallbacks{
            [&](const std::uint16_t, const std::uint16_t)
                -> std::optional<std::uint8_t> {
                ++blocked_lookups;
                return 1U;
            }});
    if (blocked_lookups != 10U || !state.candidates.empty()) {
        return 2;
    }

    build_world_effect_edge_candidates(state, {});
    return state.candidates.empty() ? 0 : 3;
}

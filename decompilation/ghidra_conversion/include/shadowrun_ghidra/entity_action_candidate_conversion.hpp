#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityActionCandidateInput {
    std::uint8_t status_flags = 0;
    std::uint16_t action_word = 0;
};

// ROM 0x14446: scan the three source entity slots and retain the indices that
// are active, not suppressed by status bits 6/4, and have a nonzero action
// word.
std::vector<std::size_t> collect_entity_action_candidates(
    const std::array<EntityActionCandidateInput, 3>& entities);

} // namespace shadowrun::ghidra_conversion

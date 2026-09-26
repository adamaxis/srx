#include "shadowrun_ghidra/entities/entity_action_candidate_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<EntityActionCandidateInput, 3> entities{
        EntityActionCandidateInput{0x01U, 0x1000U},
        EntityActionCandidateInput{0x41U, 0x1000U},
        EntityActionCandidateInput{0x11U, 0x2000U},
    };
    if (collect_entity_action_candidates(entities) !=
        std::vector<std::size_t>{0U}) {
        return 1;
    }

    const std::array<EntityActionCandidateInput, 3> second{
        EntityActionCandidateInput{0x01U, 0U},
        EntityActionCandidateInput{0x21U, 0x10U},
        EntityActionCandidateInput{0x81U, 0x20U},
    };
    if (collect_entity_action_candidates(second) !=
        std::vector<std::size_t>{1U, 2U}) {
        return 2;
    }

    return 0;
}

#include "shadowrun_ghidra/entities/entity_action_candidate_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::vector<std::size_t> collect_entity_action_candidates(
    const std::array<EntityActionCandidateInput, 3>& entities) {
    std::vector<std::size_t> result;
    for (std::size_t index = 0; index < entities.size(); ++index) {
        const auto& entity = entities[index];
        if ((entity.status_flags & 0x01U) != 0U &&
            (entity.status_flags & 0x40U) == 0U &&
            (entity.status_flags & 0x10U) == 0U &&
            entity.action_word != 0U) {
            result.push_back(index);
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/entity_sort_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::int16_t effective_depth(const EntityDepthEntry& entity) {
    if ((entity.flags & 0x50U) != 0U) {
        return 0;
    }
    return entity.depth;
}

} // namespace

void sort_entity_slots_by_depth(
    std::array<EntityDepthEntry, 16>& entities) {
    std::stable_sort(
        entities.begin(), entities.end(),
        [](const EntityDepthEntry& left, const EntityDepthEntry& right) {
            return effective_depth(left) > effective_depth(right);
        });
}

} // namespace shadowrun::ghidra_conversion

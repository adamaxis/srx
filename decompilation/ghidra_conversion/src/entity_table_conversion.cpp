#include "shadowrun_ghidra/entity_table_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool entity_table_has_free_slot(
    const std::array<std::uint16_t, 10>& occupancy_words) {
    for (const auto occupancy : occupancy_words) {
        if (occupancy == 0U) {
            return true;
        }
    }
    return false;
}

std::optional<std::size_t> find_ready_entity_slot(
    const std::array<EntityReadinessState, 13>& entities) {
    for (std::size_t index = 0; index < entities.size(); ++index) {
        if (entities[index].active && entities[index].ready) {
            return index;
        }
    }
    return std::nullopt;
}

} // namespace shadowrun::ghidra_conversion

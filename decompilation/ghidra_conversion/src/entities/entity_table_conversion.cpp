#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::uint16_t> find_free_entity_table_slot(
    const std::uint16_t start_cursor,
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words) {
    auto cursor = start_cursor;
    for (const auto occupancy : occupancy_words) {
        if (occupancy == 0U) {
            return cursor;
        }
        ++cursor;
    }
    return std::nullopt;
}

bool entity_table_has_free_slot(
    const std::array<std::uint16_t, kEntityTableSlotCount>& occupancy_words) {
    return find_free_entity_table_slot(0U, occupancy_words).has_value();
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

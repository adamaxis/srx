#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

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

void reset_entity_service_records(
    std::int16_t& timeout_marker,
    std::array<EntityServiceRecord, kEntityServiceRecordCount>& records) {
    timeout_marker = 0;
    for (auto& record : records) {
        record.pair_a2 = 0U;
        record.pair_ac = 0U;
        record.byte_43 = 0U;
        record.route_flags = static_cast<std::uint8_t>(record.route_flags & 0xBFU);
        record.animation_flags =
            static_cast<std::uint8_t>(record.animation_flags & 0xEFU);
    }
}

} // namespace shadowrun::ghidra_conversion

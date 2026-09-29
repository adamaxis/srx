#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint16_t, 10> occupancy{};
    occupancy.fill(1U);
    if (entity_table_has_free_slot(occupancy)) {
        return 1;
    }
    occupancy[7] = 0U;
    if (!entity_table_has_free_slot(occupancy)) {
        return 2;
    }
    std::array<EntityReadinessState, 13> entities{};
    entities[9] = {true, true};
    const auto ready = find_ready_entity_slot(entities);
    if (!ready.has_value() || *ready != 9U) {
        return 3;
    }
    entities[2] = {true, false};
    const auto still_ready = find_ready_entity_slot(entities);
    if (!still_ready.has_value() || *still_ready != 9U) {
        return 4;
    }

    std::int16_t timeout_marker = -1;
    std::array<EntityServiceRecord, kEntityServiceRecordCount> records{};
    records[0] = {0x11111111U, 0x22222222U, 0x33U, 0xFFU, 0xFFU};
    records[9] = {0xAAAAAAAAU, 0xBBBBBBBBU, 0x44U, 0x40U, 0x10U};
    reset_entity_service_records(timeout_marker, records);
    if (timeout_marker != 0 || records[0].pair_a2 != 0U ||
        records[0].pair_ac != 0U || records[0].byte_43 != 0U ||
        records[0].route_flags != 0xBFU ||
        records[0].animation_flags != 0xEFU || records[9].pair_a2 != 0U ||
        records[9].route_flags != 0U || records[9].animation_flags != 0U) {
        return 5;
    }
    return 0;
}

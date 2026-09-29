#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint16_t, 10> occupancy{};
    occupancy.fill(1U);
    if (entity_table_has_free_slot(occupancy) ||
        find_free_entity_table_slot(0x10U, occupancy).has_value()) {
        return 1;
    }
    occupancy[7] = 0U;
    const auto found = find_free_entity_table_slot(0x10U, occupancy);
    if (!entity_table_has_free_slot(occupancy) || !found.has_value() ||
        *found != 0x17U) {
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
    return 0;
}

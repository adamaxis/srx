#include "shadowrun_ghidra/entity_table_conversion.hpp"

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
    return 0;
}

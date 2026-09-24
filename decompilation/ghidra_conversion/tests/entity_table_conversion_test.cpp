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
    return 0;
}

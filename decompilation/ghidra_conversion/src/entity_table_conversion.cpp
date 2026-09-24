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

} // namespace shadowrun::ghidra_conversion

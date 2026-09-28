#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto& catalog = us_armor_catalog();
    if (catalog.size() != kUsArmorDefinitionCount ||
        catalog.front().armor_index != 1U ||
        catalog.front().primary_modifier != 2U ||
        catalog.front().secondary_modifier != 1U ||
        catalog.back().armor_index != 7U ||
        catalog.back().primary_modifier != 8U ||
        catalog.back().secondary_modifier != 6U ||
        subtract_us_armor_primary_modifier(0x120AU, 2U, catalog) != 0x1209U ||
        subtract_us_armor_secondary_modifier(0x120AU, 2U, catalog) !=
            0x1208U ||
        subtract_us_armor_primary_modifier(0x120AU, 0U, catalog) != 0x120AU) {
        return 1;
    }

    const auto* second = find_us_armor_definition(2U);
    if (second != &catalog[1] || second->primary_modifier != 1U ||
        second->secondary_modifier != 2U ||
        find_us_armor_definition(0U) != nullptr ||
        find_us_armor_definition(8U) != nullptr) {
        return 2;
    }

    return 0;
}

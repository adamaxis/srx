#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto& catalog = us_armor_catalog();
    // Raw US-ROM D4D50 bytes at each 0x30-byte record's +0x25/+0x26.
    constexpr std::array<std::array<std::uint8_t, 2U>, 7U> kSourceModifiers{{
        {{2U, 1U}}, {{1U, 2U}}, {{5U, 3U}}, {{4U, 3U}},
        {{3U, 2U}}, {{6U, 4U}}, {{8U, 6U}},
    }};
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

    for (std::size_t row = 0U; row < catalog.size(); ++row) {
        const auto item_code = static_cast<std::uint8_t>(0x2CU + row);
        const auto* selected = find_us_armor_definition_for_item_code(item_code);
        if (catalog[row].armor_index != row + 1U ||
            catalog[row].primary_modifier != kSourceModifiers[row][0] ||
            catalog[row].secondary_modifier != kSourceModifiers[row][1] ||
            selected != &catalog[row]) {
            return 3;
        }
    }
    if (find_us_armor_definition_for_item_code(0x2BU) != nullptr ||
        find_us_armor_definition_for_item_code(0x33U) != nullptr) {
        return 4;
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

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto& catalog = us_item_catalog();
    constexpr std::array<std::uint16_t, kUsItemDefinitionCount> kRomBaseCosts{{
        0x0064U, 0x015EU, 0x012CU, 0x01C2U, 0x01F4U, 0x01C2U, 0x01C2U,
        0x03E8U, 0x05DCU, 0x0320U, 0x0578U, 0x03E8U, 0x012CU, 0x00B4U,
        0x00B4U, 0x00C8U, 0x012CU, 0x012CU, 0x01F4U, 0x2710U, 0x03E8U,
        0x0BB8U, 0xC350U, 0x3A98U, 0x0FA0U, 0x0FA0U, 0x0FA0U, 0x0FA0U,
        0x0FA0U, 0x0FA0U, 0x0FA0U, 0x0FA0U, 0x0FA0U, 0x0FA0U, 0x0FA0U,
        0x0FA0U, 0x0FA0U, 0x0FA0U, 0x445CU, 0x445CU, 0x0002U, 0xFFFFU,
        0x0000U, 0x00C8U, 0x01F4U, 0x0384U, 0x0258U, 0x02BCU, 0x2710U,
        0x4E20U,
    }};
    for (std::size_t index = 0; index < kRomBaseCosts.size(); ++index) {
        if (catalog[index].base_cost != kRomBaseCosts[index]) {
            return 8;
        }
    }

    // US ROM D4540 item rows 1..15. Routine 0x12F50 uses +0x25 as
    // the CF84C Damage row; 0x13000 uses +0x26 to select the Power label.
    // These are source values, not a host-derived damage calculation.
    constexpr std::array<std::int8_t, 15> kRomWeaponDamageRows{{
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1}};
    constexpr std::array<std::uint8_t, 15> kRomWeaponPowerSelectors{{
        0x04U, 0x06U, 0x06U, 0x06U, 0x0AU, 0x09U, 0x09U, 0x06U,
        0x07U, 0x06U, 0x0AU, 0x09U, 0x08U, 0x06U, 0x0AU}};
    constexpr std::array<std::uint8_t, 15> kRomWeaponMagazineCapacities{{
        0x06U, 0x0CU, 0x0BU, 0x19U, 0x06U, 0x0AU, 0x0FU, 0x1EU,
        0x1CU, 0x18U, 0x05U, 0x08U, 0x06U, 0x06U, 0x06U}};
    for (std::size_t index = 0; index < kRomWeaponDamageRows.size(); ++index) {
        if (catalog[index].signed_detail_asset_index !=
                kRomWeaponDamageRows[index] ||
            catalog[index].detail_selector != kRomWeaponPowerSelectors[index] ||
            catalog[index].source_combat_factor !=
                kRomWeaponMagazineCapacities[index]) {
            return 9;
        }
    }

    if (catalog.size() != kUsItemDefinitionCount ||
        catalog.front().item_code != 1U ||
        !catalog.front().has_detail_art ||
        catalog.front().base_cost != 0x0064U ||
        catalog.front().source_combat_factor != 0x06U ||
        catalog.front().signed_detail_asset_index != 0 ||
        catalog.front().detail_selector != 0x04U) {
        return 1;
    }

    const auto& item_five = catalog[4];
    if (item_five.item_code != 5U || item_five.base_cost != 0x01F4U ||
        item_five.source_combat_factor != 0x06U ||
        item_five.signed_detail_asset_index != 1 ||
        item_five.detail_selector != 0x0AU) {
        return 2;
    }

    const auto& item_forty_one = catalog[40];
    if (item_forty_one.item_code != 41U ||
        item_forty_one.has_detail_art ||
        item_forty_one.base_cost != 0x0002U ||
        item_forty_one.source_combat_factor != 0xFFU ||
        item_forty_one.signed_detail_asset_index != -1 ||
        item_forty_one.detail_selector != 0U) {
        return 3;
    }

    // ROM D4540 row 43 (Bear Totem) +0x26/+0x27 = 00 02.
    if (catalog[42].has_detail_art || catalog[42].base_cost != 0U ||
        catalog[42].detail_selector != 0U ||
        !catalog[43].has_detail_art ||
        catalog.back().item_code != 50U || catalog.back().base_cost != 0x4E20U ||
        catalog.back().detail_selector != 6U) {
        return 4;
    }

    // 0x104F6 reads aligned words from the late special-item rows rather
    // than a generic modifier matrix: item 41 +0x22, item 42 +0x24, and
    // item 43 +0x26/+0x27 are each independently source-backed.
    if (catalog[40].source_action_modifier_word(0) != 0x0002U ||
        catalog[41].source_action_modifier_word(1) != 0x0002U ||
        catalog[42].source_action_modifier_word(2) != 0x0002U ||
        catalog[42].source_action_parameter_tail != 0x02U) {
        return 6;
    }

    const auto& selectors = us_special_item_modifier_selector_table();
    if (selectors.field_selectors.size() != 16U ||
        selectors.field_selectors[5] != 0 ||
        selectors.field_selectors[11] != 2 ||
        selectors.field_selectors[15] != 12) {
        return 7;
    }

    const auto* first = find_us_item_definition(1U);
    const auto* last = find_us_item_definition(50U);
    if (first != &catalog.front() || last != &catalog.back() ||
        find_us_item_definition(0U) != nullptr ||
        find_us_item_definition(51U) != nullptr) {
        return 5;
    }

    return 0;
}

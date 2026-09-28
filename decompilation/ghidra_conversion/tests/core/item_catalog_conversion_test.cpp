#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto& catalog = us_item_catalog();
    if (catalog.size() != kUsItemDefinitionCount ||
        catalog.front().item_code != 1U ||
        catalog.front().base_cost != 0x0064U ||
        catalog.front().source_combat_factor != 0x06U ||
        catalog.front().signed_detail_asset_index != 0 ||
        catalog.front().detail_selector != 0x04U) {
        return 1;
    }

    const auto& item_five = catalog[4];
    if (item_five.item_code != 5U || item_five.base_cost != 0x00F4U ||
        item_five.source_combat_factor != 0x06U ||
        item_five.signed_detail_asset_index != 1 ||
        item_five.detail_selector != 0x0AU) {
        return 2;
    }

    const auto& item_forty_one = catalog[40];
    if (item_forty_one.item_code != 41U ||
        item_forty_one.base_cost != 0x0002U ||
        item_forty_one.source_combat_factor != 0xFFU ||
        item_forty_one.signed_detail_asset_index != -1 ||
        item_forty_one.detail_selector != 0U) {
        return 3;
    }

    if (catalog[42].base_cost != 0U || catalog[42].detail_selector != 2U ||
        catalog.back().item_code != 50U || catalog.back().base_cost != 0x20U ||
        catalog.back().detail_selector != 6U) {
        return 4;
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

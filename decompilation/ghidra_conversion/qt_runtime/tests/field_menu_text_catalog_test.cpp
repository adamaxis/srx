#include "field_menu_text_catalog.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <iostream>

int main() {
    const auto loaded = shadowrun::ghidra_conversion::load_conversion_asset_catalog(
        SHADOWRUN_GHIDRA_ASSET_ROOT);
    if (!loaded.complete()) {
        std::cerr << "direct asset catalog is incomplete\n";
        return 1;
    }
    const auto text =
        shadowrun::ghidra_conversion::qt_runtime::load_field_menu_text_catalog(
            loaded.catalog);
    if (!text.complete()) {
        std::cerr << "field menu text catalog is incomplete\n";
        return 2;
    }
    using namespace shadowrun::ghidra_conversion::qt_runtime;
    if (text.catalog.command_labels[0] != "Inventory" ||
        text.catalog.command_labels[6] != "Pocket Secretary" ||
        text.catalog.archetype_names[2] != "Gator" ||
        text.catalog.archetype_names[3] != "Shaman" ||
        text.catalog.runner_names[0] != "Joshua" ||
        text.catalog.item_detail_labels[0] != "ammo" ||
        text.catalog.item_detail_labels[1] != "uses" ||
        text.catalog.item_detail_labels[2] != "rtng" ||
        text.catalog.physical_label != "Physical" ||
        text.catalog.mental_label != "Mental" ||
        text.catalog.attack_label != "Atk" ||
        text.catalog.defense_label != "Def" ||
        text.catalog.damage_label != "Damage" ||
        text.catalog.no_armor_label != "No Armor" ||
        text.catalog.weapons_label != "Wpns" ||
        text.catalog.guns_label != "Guns" ||
        text.catalog.power_label != "Power" ||
        text.catalog.action_item_equipped_prefix != "This item is" ||
        text.catalog.action_item_equipped_suffix != "  equipped" ||
        text.catalog.action_discard_prompt != "Discard item?" ||
        text.catalog.action_cannot_discard != "Cannot discard" ||
        text.catalog.action_cannot_trade != "Cannot trade magic items!" ||
        text.catalog.action_choice_no != "No" ||
        text.catalog.action_choice_yes != "Yes" ||
        text.catalog.armor_labels[1][0] != "   Armor    " ||
        text.catalog.armor_labels[1][1] != "  Clothing  " ||
        text.catalog.power_labels[0].empty() ||
        !text.catalog.runner_names[1].empty() ||
        !text.catalog.item_categories[15].empty()) {
        std::cerr << "field menu text catalog does not match source labels\n";
        return 3;
    }
    return 0;
}

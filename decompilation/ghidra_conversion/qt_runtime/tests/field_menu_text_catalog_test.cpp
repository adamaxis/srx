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
        for (const auto& error : text.errors) {
            std::cerr << error << '\n';
        }
        return 2;
    }
    using namespace shadowrun::ghidra_conversion::qt_runtime;
    const std::array<std::string, kCyberwareEntryCount>
        expected_cyberware_entries{{
            "    Datajack", "    CyberEyes", "   Hand Razors",
            "      Spurs", "    Smartlink", "Muscle Replace(1)",
            "Muscle Replace(2)", "Muscle Replace(3)",
            "Muscle Replace(4)", "Dermal Plating(1)",
            "Dermal Plating(2)", "Dermal Plating(3)",
            "Wired Reflexes(1)", "Wired Reflexes(2)",
            "Wired Reflexes(3)"}};
    if (text.catalog.command_labels[0] != "Inventory" ||
        text.catalog.command_labels[6] != "Pocket Secretary" ||
        text.catalog.archetype_names[2] != "Gator" ||
        text.catalog.archetype_names[3] != "Shaman" ||
        text.catalog.race_names !=
            std::array<std::string, kFieldMenuRaceNameCount>{
                "Human", "  Elf", "Dwarf", "  Orc", "Troll"} ||
        text.catalog.class_names !=
            std::array<std::string, kFieldMenuClassNameCount>{
                "Mage   ", "Shaman ", "Samurai", "Decker "} ||
        text.catalog.runner_names[0] != "Joshua" ||
        text.catalog.item_detail_labels[0] != "ammo" ||
        text.catalog.item_detail_labels[1] != "uses" ||
        text.catalog.item_detail_labels[2] != "rtng" ||
        text.catalog.cyberware_title_lines[0] != " CYBERWARE" ||
        text.catalog.cyberware_title_lines[1] !=
            std::string("[") + std::string(10U, '\\') ||
        text.catalog.cyberware_empty_message != "NONE" ||
        text.catalog.cyberware_entries != expected_cyberware_entries ||
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
        text.catalog.clips_commands[0] != "Reload" ||
        text.catalog.clips_commands[2] != "Distribute" ||
        text.catalog.clips_trade_commands[3] != "Change Recipient" ||
         text.catalog.clips_trade_prompt[0] != "Trade" ||
         text.catalog.clips_trade_prompt[1] != "clips to" ||
         text.catalog.clips_trade_prompt[2] != "  ????" ||
         text.catalog.clips_exchange_complete[0] != "Exchange" ||
         text.catalog.clips_exchange_complete[1] != "complete" ||
        text.catalog.clips_title != "    CLIPS" ||
        text.catalog.clips_separator !=
            std::string("   [") + std::string(6U, static_cast<char>(92)) ||
        text.catalog.clips_counter_label != "  Clips" ||
        text.catalog.clips_reload_prompt[2] != "   Reload?" ||
        text.catalog.clips_reload_success[1] != " chummers!!" ||
        text.catalog.clips_out_of_ammo[1] != " ammo clips!" ||
        text.catalog.clips_item_no_clips[1] != " does not" ||
        text.catalog.clips_no_party[0] != " You have no" ||
        text.catalog.clips_no_party[2] != "to trade with." ||
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

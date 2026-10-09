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
    if (text.catalog.field_service_titles[0] != "Weapons World" ||
        text.catalog.field_service_titles[1] != "Ares Weapon Emporium" ||
        text.catalog.field_service_title_destinations[1] != 0xC110U ||
        text.catalog.field_service_action_titles[1] !=
            " Available Weapons  " ||
        text.catalog.field_service_action_titles[4] !=
            " Display Available  " ||
        text.catalog.field_service_action_titles[5] !=
            "     Equipment      " ||
        text.catalog.field_service_action_titles[10] !=
            "   Sell Item From   " ||
        text.catalog.field_service_universal_clips_title !=
            "Universal\nGun Clips" ||
        text.catalog.field_service_universal_clips_description.find(
            "These clips may be used in\nany gun.") != 0U ||
        text.catalog.field_service_universal_clips_description.find(
            "Cost per Clip") == std::string::npos ||
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
        text.catalog.magic_defense_title != " Magical Defense" ||
        text.catalog.magic_force_posture_heading !=
            "Set Force & Posture." ||
        text.catalog.magic_success_label != "Success" ||
        text.catalog.magic_force_label != "Force" ||
        text.catalog.magic_availability_label != "Damage" ||
        text.catalog.dismiss_runner_labels !=
            std::array<std::string, kDismissRunnerLabelCount>{
                "   Dismiss", "    Who??", "    Abort"} ||
        text.catalog.dismiss_runner_separator !=
            std::string("[") + std::string(12U, static_cast<char>(92)) ||
        text.catalog.pocket_secretary_cyberdeck_unavailable_lines !=
            std::array<std::string, kCyberdeckUnavailableLineCount>{
                " You do not have", "  a cyberdeck!!", " Keep looking."} ||
        text.catalog.pocket_secretary_group_item_names[0] !=
            "Prototype Cyber-heart" ||
        text.catalog.pocket_secretary_group_item_names[14] !=
            "Ork Armband" ||
        text.catalog.pocket_secretary_group_items_empty_lines !=
            std::array<std::string,
                       kPocketSecretaryGroupItemsEmptyLineCount>{
                "You are not carrying", "  any group items."} ||
        text.catalog.notebook_labels !=
            std::array<std::string, kNotebookLabelCount>{
                "  Notebook", "Current Run", "Tips & Clues", "Contacts",
                "Mr. Johnsons", "Shadowrunners"} ||
        text.catalog.notebook_no_run_lines !=
            std::array<std::string, kNotebookNoRunLineCount>{
                " You are not currently", "engaged in a shadowrun.",
                "     Get a job!!"} ||
        text.catalog.notebook_mr_johnson_names !=
            std::array<std::string, kNotebookMrJohnsonCount>{
                "Mr. Gunderson", "Julius Strouther", "Mortimer Reed",
                "Vigore & Jarl", "Caleb Brightmore"} ||
        text.catalog.notebook_mr_johnson_details[0] !=
            "Mr. Gunderson is a\nsmall-time Johnson\nwho works out of the\n"
            "Jump House bar in\nRedmond Barrens." ||
        text.catalog.notebook_mr_johnson_details[4] !=
            "Caleb Brightmore is\nan upper-level Johnson\nwho specializes in\n"
            "Matrix Runs.\n\nCan be contacted at\nIcarus Descending in\n"
            "Downtown Seattle." ||
        text.catalog.notebook_tips_title != "Tips & Clues" ||
        text.catalog.notebook_tips_separator !=
            std::string("[") + std::string(15U, static_cast<char>(92)) ||
        text.catalog.notebook_tips_page_count_label != "Page    of" ||
        text.catalog.notebook_tips_page_number_clear != "  " ||
        text.catalog.notebook_tips[0].find(
            "Michael's last credstick\ntransaction:") != 0U ||
        text.catalog.notebook_tips[2].find(
            "<The letter addressed to\nMichael reads as follows:") != 0U ||
        text.catalog.notebook_tips[95].find(
            "You have deleted the") != 0U ||
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
        text.catalog.shop_transaction_prompt != "Complete Transaction?" ||
        text.catalog.shop_no_inventory_slot !=
            "Your inventory\n   is full!" ||
        text.catalog.shop_insufficient_funds !=
            " You do not have the\nnuyen for this item!!" ||
        text.catalog.shop_special_action_limit !=
            "  You are already\ncarrying the maximum\n  number " ||
        text.catalog.shop_protected_sale != "Totems cannot be sold!!" ||
        text.catalog.shop_sale_quote_heading != "Item sells for       $" ||
        text.catalog.shop_purchase_price_heading != "Price        $" ||
        text.catalog.field_action_detail_texts[16] !=
            "A slap patch that instantly\nincreases mental health\nby 20%." ||
        text.catalog.action_trade_recipient_label != "Give to" ||
        text.catalog.action_trade_full_inventory_message !=
            "  has a full\n  inventory.\n\nPress a button\n to continue." ||
        text.catalog.action_automatic_use_message !=
            "This item is\nautomatically\n    used." ||
        text.catalog.first_aid_prompt != " Heal who??" ||
        text.catalog.first_aid_label != "    First Aid    " ||
        text.catalog.first_aid_cancel != "CANCEL" ||
        text.catalog.first_aid_not_injured !=
            "This person is\n not injured!" ||
        text.catalog.first_aid_slap_patch_used != "Slap-patch used." ||
        text.catalog.first_aid_medkit_used != "Medkit used." ||
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

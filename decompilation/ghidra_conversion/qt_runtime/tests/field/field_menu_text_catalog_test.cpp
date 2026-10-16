#include "field_menu_text_catalog.hpp"

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main() {
    const auto loaded = shadowrun::ghidra_conversion::load_conversion_asset_catalog(
        SHADOWRUN_GHIDRA_ASSET_ROOT);
    if (!loaded.complete()) {
        std::cerr << "direct asset catalog is incomplete\n";
        return 1;
    }
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime scripts;
    if (!scripts.load_package(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) / "dialogue" /
                "script-text.lua")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    const auto text =
        shadowrun::ghidra_conversion::qt_runtime::load_field_menu_text_catalog(
            loaded.catalog, scripts);
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
    const std::array<std::string, kCyberwareTextCount>
        expected_cyberware_text_ids{{
            "title", "separator", "empty", "datajack", "cyber_eyes",
            "hand_razors", "spurs", "smartlink", "muscle_replace_1",
            "muscle_replace_2", "muscle_replace_3", "muscle_replace_4",
            "dermal_plating_1", "dermal_plating_2", "dermal_plating_3",
            "wired_reflexes_1", "wired_reflexes_2", "wired_reflexes_3"}};
    const std::array<std::string, kFieldMetricTextCount>
        expected_field_metric_text_ids{{
            "physical", "mental", "attack", "defense", "damage", "drain",
            "none", "no_armor", "weapons", "guns", "power"}};
    const std::array<std::string, kDismissRunnerLabelCount>
        expected_dismiss_runner_choice_ids{{
            "dismiss", "who_question", "abort"}};
    const std::array<std::string, kCyberdeckUnavailableLineCount>
        expected_cyberdeck_text_ids{{
            "no_deck_first", "no_deck_second", "look_for_deck"}};
    const std::array<std::string, kPocketSecretaryGroupItemCount>
        expected_group_item_text_ids{{
            "prototype_cyberheart", "indian_lands_visa", "feathered_scale",
            "hell_hound_pelt", "gargoyle_horn", "gift_lady_gillian",
            "gift_red_buffalo_woman", "landmark_map", "map_overlay",
            "current_run_package", "security_pass", "weapon_permit",
            "eye_fiver_ear_cuff", "halloweener_patch", "ork_armband"}};
    const std::array<std::string, kPocketSecretaryGroupItemCount>
        expected_group_item_names{{
            "Prototype Cyber-heart", "Indian Lands Visa", "Feathered Scale",
            "Hell Hound Pelt", "Gargoyle Horn", "Gift for Lady Gillian",
            "Gift for Red Buffalo Woman", "Landmark Map", "Map Overlay",
            "Package for current run", "Security Pass", "Weapon Permit",
            "Eye-Fiver Ear Cuff", "Halloweener Patch", "Ork Armband"}};
    const std::array<std::string, kPocketSecretaryGroupItemsEmptyLineCount>
        expected_group_items_empty_text_ids{{"first_line", "second_line"}};
    const std::array<std::string, kNotebookChoiceCount>
        expected_notebook_choice_ids{{
            "current_run", "tips_and_clues", "contacts", "mr_johnsons",
            "shadowrunners"}};
    const std::array<std::string, kNotebookNoRunLineCount>
        expected_notebook_no_run_text_ids{{
            "first_line", "second_line", "last_line"}};
    const std::array<std::string, kNotebookMrJohnsonCount>
        expected_johnson_text_ids{{
            "gunderson", "julius_strouther", "mortimer_reed",
            "vigore_and_jarl", "caleb_brightmore"}};
    const std::array<std::string, kNotebookMrJohnsonCount>
        expected_johnson_names{{
            "Mr. Gunderson", "Julius Strouther", "Mortimer Reed",
            "Vigore & Jarl", "Caleb Brightmore"}};
    const std::array<std::string, kNotebookMrJohnsonCount>
        expected_johnson_details{{
            "Mr. Gunderson is a\nsmall-time Johnson\nwho works out of the\n"
            "Jump House bar in\nRedmond Barrens.",
            "Julius Strouther is a\nmid-level Johnson who\noperates out of the\n"
            "Matchstick nightclub\nin Downtown Seattle.",
            "Mortimer Reed is a\nmid-level Johnson who\noperates out of The Big\n"
            "Rhino Resturant in\nthe Penumbra District.",
            "Vigore & Jarl offer\nmid-level runs and\nspecialize in\n"
            "acquisitions.\n\nThey operate from the\nWanderer in the\n"
            "Puyallup Barrens.",
            "Caleb Brightmore is\nan upper-level Johnson\nwho specializes in\n"
            "Matrix Runs.\n\nCan be contacted at\nIcarus Descending in\n"
            "Downtown Seattle."}};
    std::array<std::string, kNotebookContactCount> expected_contact_text_ids{};
    for (std::size_t index = 0; index < expected_contact_text_ids.size();
         ++index) {
        expected_contact_text_ids[index] =
            "record_" + std::to_string(index);
    }
    std::array<std::string, kNotebookShadowrunnerCount>
        expected_shadowrunner_text_ids{};
    for (std::size_t index = 0;
         index < expected_shadowrunner_text_ids.size(); ++index) {
        expected_shadowrunner_text_ids[index] =
            "record_" + std::to_string(index);
    }
    const std::array<std::string, kNotebookContactCount>
        expected_contact_names{{
            "Capt Quinton Jaymes", "Alesandro Hobbs", "Sharkey",
            "Kipp David", "Wilma Temmenhoff", "Sgt. Macklemann",
            "Gregory Wilns", "Mr. Faradouchi", "Agira Tetsumi",
            "Henry J. Culver", "Max", "Sgt. Chillicut",
            "Prof. Jefferson"}};
    const std::array<std::string, kNotebookShadowrunnerCount>
        expected_shadowrunner_names{{
            "Ricky", "Winston Marrs", "Trent Delisario", "Petr Uvehr",
            "Walking Bear", "Phantom", "Ilene Two Fists",
            "Freya Goldenhair", "Rianna Heartbane", "Stark"}};
    const std::array<std::string, kNotebookTipsMetadataCount>
        expected_notebook_tips_metadata_text_ids{{
            "title", "separator", "page_count", "page_number_clear"}};
    std::array<std::string, kNotebookTipCount> expected_notebook_tip_text_ids{};
    for (std::size_t index = 0; index < expected_notebook_tip_text_ids.size();
         ++index) {
        expected_notebook_tip_text_ids[index] =
            "tip_" + std::to_string(index + 1U);
    }
    const std::array<std::string, kFieldMenuRaceNameCount>
        expected_race_text_ids{{"human", "elf", "dwarf", "orc", "troll"}};
    const std::array<std::string, kFieldMenuClassNameCount>
        expected_class_text_ids{{"mage", "shaman", "samurai", "decker"}};
    const std::array<std::string, kFieldMenuStatusNameCount>
        expected_posture_text_ids{{"full_defense", "medium_defense", "neutral",
                                   "medium_offense", "full_offense"}};
    const std::array<std::string, kMagicSpellTextCount> expected_magic_spell_ids{{
        "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
        "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
        "rockskin", "heal_wounds", "confusion"}};
    const std::array<std::string, kMagicSpellTextCount> expected_magic_spell_names{{
        "Flame Dart", "Flame Bolt", "Hellblast", "Invisibility", "Sleep",
        "Stink", "Super Barrier", "Mana Zap", "Mana Blast", "Mana Storm",
        "Rockskin", "Heal Wounds", "Confusion"}};
    const std::array<std::string, kMagicSpellTextCount>
        expected_magic_spell_descriptions{{
            "A small burst of fire that\ndelivers a nasty burn.",
            "A large ball of fire which\ncauses a vicious burn.",
            "Creates fire to incinerate\nall in its area of effect!",
            "Renders a person invisible.",
            "An area attack on one's\nmental state, fatiguing them\n"
            "into unconciousness.",
            "An area effect spell which\nsickens all targets with\n"
            "a foul stench.",
            "Creates a magical wall which\ninhibits everything.",
            "A small zap of energy that\ndelivers a nasty jolt.",
            "A burst of magical energy\nthat causes a vicious jolt.",
            "A magical conflagration\nengulfing all within its\narea of effect!",
            "Hardens flesh to resist\nthe effects of damage.",
            "Cures damage to characters.",
            "An area effect spell which\ndistracts all enemies with\n"
            "sensory overload."}};
    const std::array<std::string, kMagicPanelTextCount> expected_magic_panel_text_ids{{
        "defense_title", "force_posture_heading", "success_label",
        "force_label", "availability_label", "panel_metric_labels",
        "panel_force_posture_labels"}};
    const std::array<std::string, kClipsTextCount> expected_clips_text_ids{{
        "header_title", "header_separator", "counter_label",
        "trade_prompt_action", "trade_prompt_clips_to",
        "trade_prompt_recipient", "exchange_complete_first",
        "exchange_complete_second", "reload_prompt_ammo_remaining",
        "reload_prompt_ammo_suffix", "reload_prompt_question",
        "reload_success_first", "reload_success_second",
        "out_of_ammo_name_suffix", "out_of_ammo_clips",
        "item_without_clips_first", "item_without_clips_second",
        "item_without_clips_third", "no_party_first", "no_party_second",
        "no_party_third", "function_aborted"}};
    const std::array<std::string, kItemUiTextCount> expected_item_ui_text_ids{{
        "equipped_prefix", "equipped_suffix", "discard_prompt",
        "cannot_discard", "cannot_trade", "trade_recipient",
        "trade_full_inventory", "automatic_use", "first_aid_prompt",
        "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
        "first_aid_slap_patch_used", "first_aid_medkit_used"}};
    const std::array<std::string, kAttributesSkillsTextCount>
        expected_attributes_skills_text_ids{{
            "body", "quickness", "strength", "charisma", "intelligence",
            "willpower", "essence", "magic", "sorcery", "firearms",
            "pistols", "smgs", "shotguns", "melee_combat", "throwing",
            "computer", "biotech", "electronics", "reputation",
            "negotiation", "upgrade_header", "maximum_level", "mage_only",
            "insufficient_karma", "upgrade_prompt"}};
    const std::array<std::string, kAttributeSkillLabelCount>
        expected_attribute_skill_labels{{
            "Body", "Quickness", "Strength", "Charisma", "Intelligence",
            "Willpower", "Essence      .", "Magic", "Sorcery", "Firearms",
            "Pistols", "SMGs", "Shotguns", "Melee Combat", "Throwing",
            "Computer", "Biotech", "Electronics", "Reputation",
            "Negotiation"}};
    const std::array<std::string, kItemDisplayTextCount>
        expected_item_display_text_ids{{
            "power_meager", "power_minor", "power_very_low", "power_low",
            "power_very_light", "power_light", "power_medium",
            "power_medium_high", "power_high", "power_brutal",
            "power_severe", "power_massive", "power_incredible",
            "armor_vest_top", "armor_vest_bottom", "armor_clothing_top",
            "armor_clothing_bottom", "armor_jacket_top", "armor_jacket_bottom",
            "armor_plates_top", "armor_plates_bottom", "lined_duster_top",
            "lined_duster_bottom", "light_combat_armor_top",
            "light_combat_armor_bottom", "heavy_combat_armor_top",
            "heavy_combat_armor_bottom"}};
    const std::array<std::string, kFieldMenuPowerLabelCount>
        expected_power_labels{{
            "Meager", "Minor", "Very Low", "Low", "Very Light", "Light",
            "Medium", "Medium High", "High", "Brutal", "Severe", "Massive",
            "Incredible!"}};
    const std::array<std::array<std::string, kFieldMenuArmorRowCount>,
                     kFieldMenuArmorCount>
        expected_armor_labels{{
            {"", " Armor Vest "},
            {"   Armor    ", "  Clothing  "},
            {"   Armor    ", "   Jacket   "},
            {"    Vest    ", "  w/Plates  "},
            {"   Lined    ", "   Duster   "},
            {"   Light    ", "Combat Armor"},
            {"   Heavy    ", "Combat Armor"}}};
    const std::array<std::string, kClipsTextCount> actual_clips_text{{
        text.catalog.clips_title, text.catalog.clips_separator,
        text.catalog.clips_counter_label,
        text.catalog.clips_trade_prompt[0],
        text.catalog.clips_trade_prompt[1],
        text.catalog.clips_trade_prompt[2],
        text.catalog.clips_exchange_complete[0],
        text.catalog.clips_exchange_complete[1],
        text.catalog.clips_reload_prompt[0],
        text.catalog.clips_reload_prompt[1],
        text.catalog.clips_reload_prompt[2],
        text.catalog.clips_reload_success[0],
        text.catalog.clips_reload_success[1],
        text.catalog.clips_out_of_ammo[0],
        text.catalog.clips_out_of_ammo[1],
        text.catalog.clips_item_no_clips[0],
        text.catalog.clips_item_no_clips[1],
        text.catalog.clips_item_no_clips[2],
        text.catalog.clips_no_party[0],
        text.catalog.clips_no_party[1],
        text.catalog.clips_no_party[2],
        text.catalog.clips_function_aborted}};
    const std::array<std::string, kClipsTextCount> expected_clips_text{{
        "    CLIPS", std::string("   [") + std::string(6U, '\\'),
        "  Clips", "Trade", "clips to", "  ????", "Exchange", "complete",
        "You still have", "  ammo left.", "   Reload?", "Rock & Roll,",
        " chummers!!", "  is out of", " ammo clips!", "This item",
        " does not", "use clips!!", " You have no", "party members",
        "to trade with.", "Function\nAborted!"}};
    if (!text.catalog.field_service_titles[0].empty() ||
        !text.catalog.field_service_titles[1].empty() ||
        text.catalog.field_service_title_destinations[1] != 0xC110U ||
        !text.catalog.field_service_action_titles[1].empty() ||
        !text.catalog.field_service_action_titles[31].empty() ||
        !text.catalog.field_service_universal_clips_title.empty() ||
        !text.catalog.field_service_universal_clips_description.empty() ||
        text.catalog.field_metric_text_ids !=
            expected_field_metric_text_ids ||
        text.catalog.dismiss_runner_choice_ids !=
            expected_dismiss_runner_choice_ids ||
        text.catalog.dismiss_runner_separator_text_id != "separator" ||
        text.catalog.pocket_secretary_cyberdeck_text_ids !=
            expected_cyberdeck_text_ids ||
        text.catalog.pocket_secretary_group_item_text_ids !=
            expected_group_item_text_ids ||
        text.catalog.pocket_secretary_group_items_empty_text_ids !=
            expected_group_items_empty_text_ids ||
        text.catalog.notebook_choice_ids != expected_notebook_choice_ids ||
        text.catalog.notebook_no_run_text_ids !=
            expected_notebook_no_run_text_ids ||
        text.catalog.notebook_mr_johnson_name_text_ids !=
            expected_johnson_text_ids ||
        text.catalog.notebook_mr_johnson_detail_text_ids !=
            expected_johnson_text_ids ||
        text.catalog.notebook_contact_name_text_ids !=
            expected_contact_text_ids ||
        text.catalog.notebook_contact_detail_text_ids !=
            expected_contact_text_ids ||
        text.catalog.notebook_shadowrunner_name_text_ids !=
            expected_shadowrunner_text_ids ||
        text.catalog.notebook_shadowrunner_detail_text_ids !=
            expected_shadowrunner_text_ids ||
        text.catalog.notebook_record_page_text_ids !=
            std::array<std::string, kNotebookRecordPageLabelCount>{
                "next", "prior"} ||
        text.catalog.notebook_tips_metadata_text_ids !=
            expected_notebook_tips_metadata_text_ids ||
        text.catalog.notebook_tip_text_ids !=
            expected_notebook_tip_text_ids ||
        text.catalog.race_text_ids != expected_race_text_ids ||
        text.catalog.class_text_ids != expected_class_text_ids ||
        text.catalog.status_text_ids != expected_posture_text_ids ||
        text.catalog.archetype_names[2] != "Gator" ||
        text.catalog.archetype_names[3] != "Shaman" ||
        text.catalog.attribute_skill_text_ids !=
            expected_attributes_skills_text_ids ||
        text.catalog.attribute_skill_labels !=
            expected_attribute_skill_labels ||
        text.catalog.attributes_upgrade_header != "Upgrade to       Cost" ||
        text.catalog.attributes_upgrade_maximum_level_message !=
            "You are at the maximum level." ||
        text.catalog.attributes_upgrade_mage_only_message !=
            "This skill is for mages" ||
        text.catalog.attributes_upgrade_insufficient_karma_message !=
            "You do not have enough" ||
        text.catalog.attributes_upgrade_prompt != "Upgrade?" ||
        text.catalog.item_display_text_ids != expected_item_display_text_ids ||
        text.catalog.power_labels != expected_power_labels ||
        text.catalog.armor_labels != expected_armor_labels ||
        text.catalog.race_names !=
            std::array<std::string, kFieldMenuRaceNameCount>{
                "Human", "  Elf", "Dwarf", "  Orc", "Troll"} ||
        text.catalog.class_names !=
            std::array<std::string, kFieldMenuClassNameCount>{
                "Mage   ", "Shaman ", "Samurai", "Decker "} ||
        text.catalog.status_names !=
            std::array<std::string, kFieldMenuStatusNameCount>{
                "Full Defense", "Med. Defense", "Neutral", "Med. Offense",
                "Full Offense"} ||
        text.catalog.runner_names[0] != "Joshua" ||
        !text.catalog.item_detail_labels[0].empty() ||
        !text.catalog.item_detail_labels[1].empty() ||
        !text.catalog.item_detail_labels[2].empty() ||
        text.catalog.cyberware_title_lines[0] != " CYBERWARE" ||
        text.catalog.cyberware_title_lines[1] !=
            std::string("[") + std::string(10U, '\\') ||
        text.catalog.cyberware_empty_message != "NONE" ||
        text.catalog.cyberware_text_ids != expected_cyberware_text_ids ||
        text.catalog.magic_defense_title != " Magical Defense" ||
        text.catalog.magic_force_posture_heading !=
            "Set Force & Posture." ||
        text.catalog.magic_success_label != "Success" ||
        text.catalog.magic_force_label != "Force" ||
        text.catalog.magic_availability_label != "Damage" ||
        text.catalog.magic_panel_text_ids != expected_magic_panel_text_ids ||
        text.catalog.magic_panel_metric_labels !=
            "Success\n\nDrain\n\nDefense\n  vs. Phys\n\n  vs. Mana" ||
        text.catalog.magic_force_posture_labels !=
            "Force\n\n\nPosture" ||
        text.catalog.magic_spell_ids != expected_magic_spell_ids ||
        text.catalog.magic_spell_names != expected_magic_spell_names ||
        text.catalog.magic_spell_descriptions !=
            expected_magic_spell_descriptions ||
        text.catalog.magic_spell_descriptions[0] !=
            "A small burst of fire that\ndelivers a nasty burn." ||
        text.catalog.magic_spell_descriptions[12] !=
            "An area effect spell which\ndistracts all enemies with\n"
            "sensory overload." ||
        text.catalog.dismiss_runner_labels !=
            std::array<std::string, kDismissRunnerLabelCount>{
                "   Dismiss", "    Who??", "    Abort"} ||
        text.catalog.dismiss_runner_separator !=
            std::string("[") + std::string(12U, static_cast<char>(92)) ||
        text.catalog.pocket_secretary_cyberdeck_unavailable_lines !=
            std::array<std::string, kCyberdeckUnavailableLineCount>{
                " You do not have", "  a cyberdeck!!", " Keep looking."} ||
        text.catalog.pocket_secretary_group_item_names !=
            expected_group_item_names ||
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
        text.catalog.notebook_mr_johnson_names != expected_johnson_names ||
        text.catalog.notebook_mr_johnson_details != expected_johnson_details ||
        text.catalog.notebook_contact_names != expected_contact_names ||
        text.catalog.notebook_shadowrunner_names !=
            expected_shadowrunner_names ||
        text.catalog.notebook_record_page_labels !=
            std::array<std::string, kNotebookRecordPageLabelCount>{
                "<Next Screen>", "<Prior Screen>"} ||
        text.catalog.notebook_contact_details[0].find(
            "Capt. Quinton Jaymes\n\nEx-Merc") != 0U ||
        text.catalog.notebook_shadowrunner_details[9].find(
            "Stark is the only\nsurviving member") != 0U ||
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
        text.catalog.drain_label != "Drain" ||
        text.catalog.none_label != "None" ||
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
        text.catalog.confirmation_choice_ids !=
            std::array<std::string, kConfirmationChoiceCount>{"yes", "no"} ||
        text.catalog.inventory_action_choice_ids !=
            std::array<std::string, kInventoryActionChoiceCount>{
                "equip_or_use", "discard", "trade", "cancel"} ||
        text.catalog.item_ui_text_ids != expected_item_ui_text_ids ||
        !text.catalog.shop_transaction_prompt.empty() ||
        !text.catalog.shop_no_inventory_slot.empty() ||
        !text.catalog.shop_insufficient_funds.empty() ||
        !text.catalog.shop_special_action_limit.empty() ||
        !text.catalog.shop_protected_sale.empty() ||
        !text.catalog.shop_sale_quote_heading.empty() ||
        !text.catalog.shop_purchase_price_heading.empty() ||
        !text.catalog.field_action_detail_texts[0].empty() ||
        !text.catalog.field_action_detail_texts[16].empty() ||
        !text.catalog.item_detail_labels[0].empty() ||
        !text.catalog.items[0].name.empty() ||
        !text.catalog.items[15].category.empty() ||
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
        text.catalog.clips_commands !=
            std::array<std::string, kFieldMenuClipsCommandCount>{
                "Reload", "Trade", "Distribute", "Cancel"} ||
        text.catalog.clips_trade_commands !=
            std::array<std::string, kFieldMenuClipsCommandCount>{
                "Increase Clips", "Decrease Clips", "Trade Clips",
                "Change Recipient"} ||
        text.catalog.clips_text_ids != expected_clips_text_ids ||
        actual_clips_text != expected_clips_text ||
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
        text.catalog.clips_function_aborted != "Function\nAborted!" ||
        text.catalog.armor_labels[1][0] != "   Armor    " ||
        text.catalog.armor_labels[1][1] != "  Clothing  " ||
        text.catalog.power_labels[0].empty() ||
        !text.catalog.runner_names[1].empty()) {
        std::cerr << "field menu text catalog does not match source labels\n";
        return 3;
    }
    return 0;
}

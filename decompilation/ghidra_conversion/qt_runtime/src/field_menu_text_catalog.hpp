#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// These counts are the selector/data sets used by the source command
// overview. The text file is generated offline from the ROM; the Qt runtime
// receives ordinary host strings and never follows a ROM pointer table.
inline constexpr std::size_t kCommandOverviewLabelCount = 7U;
inline constexpr std::size_t kFieldMenuArchetypeNameCount = 4U;
inline constexpr std::size_t kFieldMenuRaceNameCount = 5U;
inline constexpr std::size_t kFieldMenuClassNameCount = 4U;
inline constexpr std::size_t kFieldMenuRunnerNameCount = 3U;
inline constexpr std::size_t kFieldMenuStatusNameCount = 5U;
inline constexpr std::size_t kFieldMenuPowerLabelCount = 13U;
inline constexpr std::size_t kFieldMenuArmorCount = 7U;
inline constexpr std::size_t kFieldMenuArmorRowCount = 2U;
inline constexpr std::size_t kFieldMenuItemDetailLabelCount = 3U;
inline constexpr std::size_t kFieldActionDetailTextCount = 32U;
inline constexpr std::size_t kFieldMenuItemNameCount = 50U;
inline constexpr std::size_t kFieldMenuClipsCommandCount = 4U;
inline constexpr std::size_t kAttributeSkillLabelCount = 20U;
inline constexpr std::size_t kCyberwareTitleLineCount = 2U;
inline constexpr std::size_t kCyberwareEntryCount = 15U;
inline constexpr std::size_t kCyberdeckUnavailableLineCount = 3U;
inline constexpr std::size_t kDismissRunnerLabelCount = 3U;
inline constexpr std::size_t kMagicSpellTextCount = 13U;
inline constexpr std::size_t kPocketSecretaryOptionCount = 5U;
inline constexpr std::size_t kFieldServiceTitleCount = 12U;
inline constexpr std::size_t kFieldServiceActionTitleCount = 32U;
inline constexpr std::size_t kPocketSecretaryGroupItemCount = 15U;
inline constexpr std::size_t kPocketSecretaryGroupItemsEmptyLineCount = 2U;
inline constexpr std::size_t kNotebookLabelCount = 6U;
inline constexpr std::size_t kNotebookNoRunLineCount = 3U;
inline constexpr std::size_t kNotebookTipCount = 96U;

struct FieldMenuTextCatalog {
    std::array<std::string, kCommandOverviewLabelCount> command_labels{};
    std::array<std::string, kFieldMenuArchetypeNameCount> archetype_names{};
    std::array<std::string, kFieldMenuRaceNameCount> race_names{};
    std::array<std::string, kFieldMenuClassNameCount> class_names{};
    std::array<std::string, kFieldMenuRunnerNameCount> runner_names{};
    std::array<std::string, kFieldMenuStatusNameCount> status_names{};
    std::array<std::string, kFieldMenuPowerLabelCount> power_labels{};
    std::array<std::array<std::string, kFieldMenuArmorRowCount>,
               kFieldMenuArmorCount>
        armor_labels{};
    std::array<std::string, kFieldMenuItemDetailLabelCount>
        item_detail_labels{};
    std::string physical_label;
    std::string mental_label;
    std::string attack_label;
    std::string defense_label;
    std::string damage_label;
    std::string drain_label;
    std::string none_label;
    std::string no_armor_label;
    std::string weapons_label;
    std::string guns_label;
    std::string power_label;
    // ROM 0x12634/0x127B0 source text used by the inventory discard branch.
    // These are extracted catalog values, not UI-authored replacements.
    std::string action_item_equipped_prefix;
    std::string action_item_equipped_suffix;
    std::string action_discard_prompt;
    std::string action_cannot_discard;
    std::string action_cannot_trade;
    std::string action_choice_no;
    std::string action_choice_yes;
    std::string shop_transaction_prompt;
    std::string shop_no_inventory_slot;
    std::string shop_insufficient_funds;
    std::string shop_special_action_limit;
    std::string shop_protected_sale;
    std::string shop_sale_quote_heading;
    std::string shop_purchase_price_heading;
    std::string action_trade_recipient_label;
    std::string action_trade_full_inventory_message;
    std::string action_automatic_use_message;
    std::string first_aid_prompt;
    std::string first_aid_label;
    std::string first_aid_cancel;
    std::string first_aid_not_injured;
    std::string first_aid_slap_patch_used;
    std::string first_aid_medkit_used;
    std::array<std::string, kFieldMenuClipsCommandCount> clips_commands{};
    std::array<std::string, kFieldMenuClipsCommandCount> clips_trade_commands{};
    // 0xC1A2A: eight attribute labels followed by twelve skill labels for
    // the Attributes/Skills command page.
    std::array<std::string, kAttributeSkillLabelCount>
        attribute_skill_labels{};
    std::string attributes_upgrade_header;
    std::string attributes_upgrade_maximum_level_message;
    std::string attributes_upgrade_mage_only_message;
    std::string attributes_upgrade_insufficient_karma_message;
    std::string attributes_upgrade_prompt;
    std::array<std::string, kCyberwareTitleLineCount>
        cyberware_title_lines{};
    std::string cyberware_empty_message;
    // ROM 0xFBA6 fallback branch decodes the C89C0 title into C31C.
    std::string magic_defense_title;
    std::string magic_force_posture_heading;
    std::string magic_success_label;
    std::string magic_force_label;
    std::string magic_availability_label;
    std::array<std::string, kMagicSpellTextCount> magic_spell_names{};
    std::array<std::string, kMagicSpellTextCount> magic_spell_descriptions{};
    std::string pocket_secretary_title;
    std::string pocket_secretary_separator;
    std::string hotel_title;
    std::string hotel_separator;
    std::array<std::string, 2> hotel_options{};
    std::array<std::string, 2> hotel_checkout_question{};
    std::array<std::string, 3> hotel_checkout_options{};
    std::string hotel_medical_attention_suffix;
    std::array<std::string, 5> hotel_recovery_message{};
    std::array<std::string, kFieldServiceTitleCount> field_service_titles{};
    std::array<std::uint16_t, kFieldServiceTitleCount>
        field_service_title_destinations{};
    std::array<std::string, kFieldServiceActionTitleCount>
        field_service_action_titles{};
    std::string field_service_universal_clips_title;
    std::string field_service_universal_clips_description;
    std::array<std::string, kPocketSecretaryOptionCount>
        pocket_secretary_options{};
    std::array<std::string, kCyberdeckUnavailableLineCount>
        pocket_secretary_cyberdeck_unavailable_lines{};
    std::array<std::string, kPocketSecretaryGroupItemCount>
        pocket_secretary_group_item_names{};
    std::array<std::string, kPocketSecretaryGroupItemsEmptyLineCount>
        pocket_secretary_group_items_empty_lines{};
    // ROM 0xC448 fixed text. The Dismiss separator has 12 backslashes,
    // unlike the Pocket Secretary separator's 10.
    std::array<std::string, kDismissRunnerLabelCount>
        dismiss_runner_labels{};
    std::string dismiss_runner_separator;
    // ROM F33E/F2E2 host text. Event gates are typed field-session state and
    // are intentionally not part of this static text catalog.
    std::array<std::string, kNotebookLabelCount> notebook_labels{};
    std::array<std::string, kNotebookNoRunLineCount> notebook_no_run_lines{};
    std::string notebook_tips_title;
    std::string notebook_tips_separator;
    std::string notebook_tips_page_count_label;
    std::string notebook_tips_page_number_clear;
    // ROM 0xF9A4's three pointer tables are imported offline into their own
    // named text asset and loaded as host strings before the field runtime.
    std::array<std::string, kNotebookTipCount> notebook_tips{};
    std::array<std::string, kCyberwareEntryCount> cyberware_entries{};
    std::array<std::string, 3> clips_trade_prompt{};
    std::array<std::string, 2> clips_exchange_complete{};
    std::string clips_title;
    std::string clips_separator;
    std::string clips_counter_label;
    std::array<std::string, 3> clips_reload_prompt{};
    std::array<std::string, 2> clips_reload_success{};
    std::array<std::string, 2> clips_out_of_ammo{};
    std::array<std::string, 3> clips_item_no_clips{};
    std::array<std::string, 3> clips_no_party{};
    std::array<std::string, kFieldMenuItemNameCount> item_names{};
    std::array<std::string, kFieldMenuItemNameCount> item_categories{};
    std::array<std::string, kFieldActionDetailTextCount>
        field_action_detail_texts{};
};

struct FieldMenuTextLoadResult {
    FieldMenuTextCatalog catalog{};
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Load generated field/menu text plus the separate 0xF9A4 Tips text asset
// through their manifest aliases. Missing source rows remain visible errors.
FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets);

} // namespace shadowrun::ghidra_conversion::qt_runtime

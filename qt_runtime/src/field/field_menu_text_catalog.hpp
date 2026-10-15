#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// These dimensions mirror source field-menu records. ROM-backed Lua text is
// resolved into this typed presenter catalog during bootstrap; numeric service
// routes remain in the extracted route catalog.
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
inline constexpr std::size_t kClipsTextCount = 22U;
inline constexpr std::size_t kItemUiTextCount = 14U;
inline constexpr std::size_t kItemDisplayTextCount =
    kFieldMenuPowerLabelCount +
    (kFieldMenuArmorCount * kFieldMenuArmorRowCount);
inline constexpr std::size_t kConfirmationChoiceCount = 2U;
inline constexpr std::size_t kFieldMetricTextCount = 11U;
inline constexpr std::size_t kInventoryActionChoiceCount = 4U;
inline constexpr std::size_t kAttributeSkillLabelCount = 20U;
inline constexpr std::size_t kAttributesSkillsTextCount =
    kAttributeSkillLabelCount + 5U;
inline constexpr std::size_t kCyberwareTitleLineCount = 2U;
inline constexpr std::size_t kCyberwareEntryCount = 15U;
inline constexpr std::size_t kCyberwareTextCount =
    kCyberwareTitleLineCount + 1U + kCyberwareEntryCount;
inline constexpr std::size_t kCyberdeckUnavailableLineCount = 3U;
inline constexpr std::size_t kDismissRunnerLabelCount = 3U;
inline constexpr std::size_t kMagicSpellTextCount = 13U;
inline constexpr std::size_t kMagicPanelTextCount = 7U;
inline constexpr std::size_t kFieldServiceTitleCount = 12U;
inline constexpr std::size_t kFieldServiceActionTitleCount = 32U;
inline constexpr std::size_t kPocketSecretaryGroupItemCount = 15U;
inline constexpr std::size_t kPocketSecretaryGroupItemsEmptyLineCount = 2U;
inline constexpr std::size_t kNotebookLabelCount = 6U;
inline constexpr std::size_t kNotebookChoiceCount = kNotebookLabelCount - 1U;
inline constexpr std::size_t kNotebookNoRunLineCount = 3U;
inline constexpr std::size_t kNotebookTipCount = 96U;
inline constexpr std::size_t kNotebookTipsPageCount = 3U;
inline constexpr std::size_t kNotebookTipsPageSize =
    kNotebookTipCount / kNotebookTipsPageCount;
static_assert(kNotebookTipCount % kNotebookTipsPageCount == 0U,
              "Notebook tips must divide evenly into source-ordered pages");
inline constexpr std::size_t kNotebookTipsMetadataCount = 4U;
inline constexpr std::size_t kNotebookMrJohnsonCount = 5U;
inline constexpr std::size_t kNotebookContactCount = 13U;
inline constexpr std::size_t kNotebookShadowrunnerCount = 10U;
inline constexpr std::size_t kNotebookRecordPageLabelCount = 2U;

struct FieldMenuTextCatalog {
    std::array<std::string, kCommandOverviewLabelCount> command_labels{};
    std::array<std::string, kFieldMenuArchetypeNameCount> archetype_names{};
    std::array<std::string, kFieldMenuRaceNameCount> race_names{};
    std::array<std::string, kFieldMenuRaceNameCount> race_text_ids{};
    std::array<std::string, kFieldMenuClassNameCount> class_names{};
    std::array<std::string, kFieldMenuClassNameCount> class_text_ids{};
    std::array<std::string, kFieldMenuRunnerNameCount> runner_names{};
    std::array<std::string, kFieldMenuStatusNameCount> status_names{};
    std::array<std::string, kFieldMenuStatusNameCount> status_text_ids{};
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
    std::array<std::string, kFieldMetricTextCount> field_metric_text_ids{};
    // ROM 0x12634/0x127B0 source text used by the inventory discard branch.
    // These are extracted catalog values, not UI-authored replacements.
    std::string action_item_equipped_prefix;
    std::string action_item_equipped_suffix;
    std::string action_discard_prompt;
    std::string action_cannot_discard;
    std::string action_cannot_trade;
    std::string action_choice_no;
    std::string action_choice_yes;
    std::array<std::string, kConfirmationChoiceCount>
        confirmation_choice_ids{};
    std::array<std::string, kInventoryActionChoiceCount>
        inventory_action_choice_ids{};
    // Ares shop headings and transaction copy are resolved from the Lua
    // content table during field bootstrap; the TSV stores no shop prose.
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
    std::array<std::string, kItemUiTextCount> item_ui_text_ids{};
    std::array<std::string, kItemDisplayTextCount> item_display_text_ids{};
    std::array<std::string, kFieldMenuClipsCommandCount> clips_commands{};
    std::array<std::string, kFieldMenuClipsCommandCount> clips_trade_commands{};
    std::array<std::string, kClipsTextCount> clips_text_ids{};
    // 0xC1A2A: eight attribute labels followed by twelve skill labels for
    // the Attributes/Skills command page.
    std::array<std::string, kAttributeSkillLabelCount>
        attribute_skill_labels{};
    std::array<std::string, kAttributesSkillsTextCount>
        attribute_skill_text_ids{};
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
    std::string magic_panel_metric_labels;
    std::string magic_force_posture_labels;
    std::array<std::string, kMagicPanelTextCount> magic_panel_text_ids{};
    std::array<std::string, kMagicSpellTextCount> magic_spell_ids{};
    std::array<std::string, kMagicSpellTextCount> magic_spell_names{};
    std::array<std::string, kMagicSpellTextCount> magic_spell_descriptions{};
    // Store names and category headings are Lua text. Destination words are
    // numeric route data and remain in service-routes.tsv.
    std::array<std::string, kFieldServiceTitleCount> field_service_titles{};
    std::array<std::uint16_t, kFieldServiceTitleCount>
        field_service_title_destinations{};
    std::array<std::string, kFieldServiceActionTitleCount>
        field_service_action_titles{};
    std::string field_service_universal_clips_title;
    std::string field_service_universal_clips_description;
    std::array<std::string, kCyberdeckUnavailableLineCount>
        pocket_secretary_cyberdeck_unavailable_lines{};
    std::array<std::string, kCyberdeckUnavailableLineCount>
        pocket_secretary_cyberdeck_text_ids{};
    std::array<std::string, kPocketSecretaryGroupItemCount>
        pocket_secretary_group_item_names{};
    std::array<std::string, kPocketSecretaryGroupItemCount>
        pocket_secretary_group_item_text_ids{};
    std::array<std::string, kPocketSecretaryGroupItemsEmptyLineCount>
        pocket_secretary_group_items_empty_lines{};
    std::array<std::string, kPocketSecretaryGroupItemsEmptyLineCount>
        pocket_secretary_group_items_empty_text_ids{};
    // ROM 0xC448 fixed text. The Dismiss separator has 12 backslashes,
    // unlike the Pocket Secretary separator's 10.
    std::array<std::string, kDismissRunnerLabelCount>
        dismiss_runner_labels{};
    std::array<std::string, kDismissRunnerLabelCount>
        dismiss_runner_choice_ids{};
    std::string dismiss_runner_separator;
    std::string dismiss_runner_separator_text_id;
    // ROM F33E/F2E2 host text. Event gates are typed field-session state and
    // are intentionally not part of this static text catalog.
    std::array<std::string, kNotebookLabelCount> notebook_labels{};
    std::array<std::string, kNotebookChoiceCount> notebook_choice_ids{};
    std::array<std::string, kNotebookNoRunLineCount> notebook_no_run_lines{};
    std::array<std::string, kNotebookNoRunLineCount>
        notebook_no_run_text_ids{};
    std::array<std::string, kNotebookMrJohnsonCount>
        notebook_mr_johnson_names{};
    std::array<std::string, kNotebookMrJohnsonCount>
        notebook_mr_johnson_name_text_ids{};
    std::array<std::string, kNotebookMrJohnsonCount>
        notebook_mr_johnson_details{};
    std::array<std::string, kNotebookMrJohnsonCount>
        notebook_mr_johnson_detail_text_ids{};
    std::array<std::string, kNotebookContactCount>
        notebook_contact_names{};
    std::array<std::string, kNotebookContactCount>
        notebook_contact_name_text_ids{};
    std::array<std::string, kNotebookContactCount>
        notebook_contact_details{};
    std::array<std::string, kNotebookContactCount>
        notebook_contact_detail_text_ids{};
    std::array<std::string, kNotebookShadowrunnerCount>
        notebook_shadowrunner_names{};
    std::array<std::string, kNotebookShadowrunnerCount>
        notebook_shadowrunner_name_text_ids{};
    std::array<std::string, kNotebookShadowrunnerCount>
        notebook_shadowrunner_details{};
    std::array<std::string, kNotebookShadowrunnerCount>
        notebook_shadowrunner_detail_text_ids{};
    std::array<std::string, kNotebookRecordPageLabelCount>
        notebook_record_page_labels{};
    std::array<std::string, kNotebookRecordPageLabelCount>
        notebook_record_page_text_ids{};
    std::array<std::string, kNotebookTipsMetadataCount>
        notebook_tips_metadata_text_ids{};
    std::array<std::string, kNotebookTipCount> notebook_tip_text_ids{};
    std::string notebook_tips_title;
    std::string notebook_tips_separator;
    std::string notebook_tips_page_count_label;
    std::string notebook_tips_page_number_clear;
    // ROM 0xF9A4's three pointer tables are imported offline into semantic
    // Lua text keys; typed page state consumes their text through this catalog.
    std::array<std::string, kNotebookTipCount> notebook_tips{};
    std::array<std::string, kCyberwareEntryCount> cyberware_entries{};
    std::array<std::string, kCyberwareTextCount> cyberware_text_ids{};
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
    std::string clips_function_aborted;
    struct Item {
        scripting::ScriptItemDefinition definition;
        std::string name;
        std::string category;
    };
    std::array<Item, kFieldMenuItemNameCount> items{};
    std::array<std::string, kFieldActionDetailTextCount>
        field_action_detail_texts{};
};

struct FieldMenuTextLoadResult {
    FieldMenuTextCatalog catalog{};
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Load the Lua-authored field/menu copy plus numeric service destinations
// from the extracted routing catalog. Missing source rows remain visible.
FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets,
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts);

} // namespace shadowrun::ghidra_conversion::qt_runtime

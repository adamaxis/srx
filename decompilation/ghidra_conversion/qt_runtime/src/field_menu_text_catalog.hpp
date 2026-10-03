#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// These counts are the selector/data sets used by the source command
// overview. The text file is generated offline from the ROM; the Qt runtime
// receives ordinary host strings and never follows a ROM pointer table.
inline constexpr std::size_t kCommandOverviewLabelCount = 7U;
inline constexpr std::size_t kFieldMenuArchetypeNameCount = 4U;
inline constexpr std::size_t kFieldMenuRunnerNameCount = 3U;
inline constexpr std::size_t kFieldMenuStatusNameCount = 5U;
inline constexpr std::size_t kFieldMenuPowerLabelCount = 13U;
inline constexpr std::size_t kFieldMenuArmorCount = 7U;
inline constexpr std::size_t kFieldMenuArmorRowCount = 2U;
inline constexpr std::size_t kFieldMenuItemDetailLabelCount = 3U;
inline constexpr std::size_t kFieldMenuItemNameCount = 50U;
inline constexpr std::size_t kFieldMenuClipsCommandCount = 4U;
inline constexpr std::size_t kAttributeSkillLabelCount = 20U;
inline constexpr std::size_t kCyberwareTitleLineCount = 2U;
inline constexpr std::size_t kCyberwareEntryCount = 15U;

struct FieldMenuTextCatalog {
    std::array<std::string, kCommandOverviewLabelCount> command_labels{};
    std::array<std::string, kFieldMenuArchetypeNameCount> archetype_names{};
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
    std::array<std::string, kFieldMenuClipsCommandCount> clips_commands{};
    std::array<std::string, kFieldMenuClipsCommandCount> clips_trade_commands{};
    // 0xC1A2A: eight attribute labels followed by twelve skill labels for
    // the Attributes/Skills command page.
    std::array<std::string, kAttributeSkillLabelCount>
        attribute_skill_labels{};
    std::array<std::string, kCyberwareTitleLineCount>
        cyberware_title_lines{};
    std::string cyberware_empty_message;
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
};

struct FieldMenuTextLoadResult {
    FieldMenuTextCatalog catalog{};
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Load the generated menu/field/text.tsv resource through its manifest alias.
// This is intentionally a small table loader, not a fallback label provider:
// an absent or incomplete source catalog remains a visible conversion gap.
FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets);

} // namespace shadowrun::ghidra_conversion::qt_runtime

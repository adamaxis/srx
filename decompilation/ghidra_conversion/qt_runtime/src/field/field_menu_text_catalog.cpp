#include "field_menu_text_catalog.hpp"

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::array<std::string_view, kFieldMenuClipsCommandCount>
    kClipsCommandIds{{"reload", "trade", "distribute", "cancel"}};
constexpr std::array<std::string_view, kFieldMenuClipsCommandCount>
    kClipsTradeCommandIds{{"increase_clips", "decrease_clips", "trade_clips",
                           "change_recipient"}};
constexpr std::array<std::string_view, kClipsTextCount> kClipsTextIds{{
    "header_title", "header_separator", "counter_label",
    "trade_prompt_action", "trade_prompt_clips_to",
    "trade_prompt_recipient", "exchange_complete_first",
    "exchange_complete_second", "reload_prompt_ammo_remaining",
    "reload_prompt_ammo_suffix", "reload_prompt_question",
    "reload_success_first", "reload_success_second",
    "out_of_ammo_name_suffix", "out_of_ammo_clips",
    "item_without_clips_first", "item_without_clips_second",
    "item_without_clips_third", "no_party_first", "no_party_second",
    "no_party_third"}};
constexpr std::array<std::string_view, kItemUiTextCount> kItemUiTextIds{{
    "equipped_prefix", "equipped_suffix", "discard_prompt",
    "cannot_discard", "cannot_trade", "trade_recipient",
    "trade_full_inventory", "automatic_use", "first_aid_prompt",
    "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
    "first_aid_slap_patch_used", "first_aid_medkit_used"}};
constexpr std::array<std::string_view, kAttributesSkillsTextCount>
    kAttributesSkillsTextIds{{
        "body", "quickness", "strength", "charisma", "intelligence",
        "willpower", "essence", "magic", "sorcery", "firearms", "pistols",
        "smgs", "shotguns", "melee_combat", "throwing", "computer",
        "biotech", "electronics", "reputation", "negotiation",
        "upgrade_header", "maximum_level", "mage_only",
        "insufficient_karma", "upgrade_prompt"}};
constexpr std::array<std::string_view, kNotebookChoiceCount>
    kNotebookChoiceIds{{"current_run", "tips_and_clues", "contacts",
                        "mr_johnsons", "shadowrunners"}};
constexpr std::array<std::string_view, kConfirmationChoiceCount>
    kConfirmationChoiceIds{{"yes", "no"}};
constexpr std::array<std::string_view, kInventoryActionChoiceCount>
    kInventoryActionChoiceIds{{"equip_or_use", "discard", "trade",
                               "cancel"}};
constexpr std::array<std::string_view, kMagicSpellTextCount> kMagicSpellIds{{
    "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
    "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
    "rockskin", "heal_wounds", "confusion"}};

constexpr std::string_view kMagicSpellOptionPrefix =
    "magic.spells.option.";
constexpr std::string_view kMagicSpellDescriptionPrefix =
    "magic.spells.description.";

bool magic_spell_index(const std::string_view id, std::size_t& index) {
    const auto found = std::find(kMagicSpellIds.begin(), kMagicSpellIds.end(),
                                 id);
    if (found == kMagicSpellIds.end()) {
        return false;
    }
    index = static_cast<std::size_t>(found - kMagicSpellIds.begin());
    return true;
}

bool has_clips_menu_label_prefix(const std::string_view key) {
    return key.rfind("clips.option.", 0U) == 0U ||
           key.rfind("clips.trade.option.", 0U) == 0U ||
           key.rfind("clips.text.option.", 0U) == 0U;
}

bool has_item_ui_text_label_prefix(const std::string_view key) {
    return key.rfind("items.text.option.", 0U) == 0U;
}

bool is_item_ui_text_label_key(const std::string_view key) {
    constexpr std::string_view kTextPrefix = "items.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           std::find(kItemUiTextIds.begin(), kItemUiTextIds.end(),
                     key.substr(kTextPrefix.size())) != kItemUiTextIds.end();
}

bool has_attributes_skills_text_label_prefix(const std::string_view key) {
    return key.rfind("attributes_skills.text.option.", 0U) == 0U;
}

bool is_attributes_skills_text_label_key(const std::string_view key) {
    constexpr std::string_view kTextPrefix =
        "attributes_skills.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           std::find(kAttributesSkillsTextIds.begin(),
                     kAttributesSkillsTextIds.end(),
                     key.substr(kTextPrefix.size())) !=
               kAttributesSkillsTextIds.end();
}

bool is_clips_menu_label_key(const std::string_view key) {
    constexpr std::string_view kCommandPrefix = "clips.option.";
    constexpr std::string_view kTradeCommandPrefix = "clips.trade.option.";
    if (key.rfind(kCommandPrefix, 0U) == 0U) {
        return std::find(kClipsCommandIds.begin(), kClipsCommandIds.end(),
                         key.substr(kCommandPrefix.size())) !=
               kClipsCommandIds.end();
    }
    if (key.rfind(kTradeCommandPrefix, 0U) == 0U) {
        return std::find(kClipsTradeCommandIds.begin(),
                         kClipsTradeCommandIds.end(),
                         key.substr(kTradeCommandPrefix.size())) !=
               kClipsTradeCommandIds.end();
    }
    constexpr std::string_view kTextPrefix = "clips.text.option.";
    if (key.rfind(kTextPrefix, 0U) == 0U) {
        return std::find(kClipsTextIds.begin(), kClipsTextIds.end(),
                         key.substr(kTextPrefix.size())) !=
               kClipsTextIds.end();
    }
    return false;
}

bool has_notebook_menu_label_prefix(const std::string_view key) {
    return key.rfind("notebook.option.", 0U) == 0U ||
           key == "notebook.options.title";
}

bool is_notebook_menu_label_key(const std::string_view key) {
    if (key == "notebook.options.title") {
        return true;
    }
    constexpr std::string_view kChoicePrefix = "notebook.option.";
    return key.rfind(kChoicePrefix, 0U) == 0U &&
           std::find(kNotebookChoiceIds.begin(), kNotebookChoiceIds.end(),
                     key.substr(kChoicePrefix.size())) !=
               kNotebookChoiceIds.end();
}

bool has_confirmation_menu_label_prefix(const std::string_view key) {
    return key.rfind("ui.option.", 0U) == 0U;
}

bool is_confirmation_menu_label_key(const std::string_view key) {
    constexpr std::string_view kChoicePrefix = "ui.option.";
    return key.rfind(kChoicePrefix, 0U) == 0U &&
           std::find(kConfirmationChoiceIds.begin(),
                     kConfirmationChoiceIds.end(),
                     key.substr(kChoicePrefix.size())) !=
               kConfirmationChoiceIds.end();
}

bool parse_indexed_key(const std::string& key, const std::string& prefix,
                       std::size_t& index) {
    if (key.rfind(prefix, 0) != 0U) {
        return false;
    }
    const auto number = key.substr(prefix.size());
    if (number.empty()) {
        return false;
    }
    const auto* first = number.data();
    const auto* last = first + number.size();
    std::size_t parsed = 0;
    const auto result = std::from_chars(first, last, parsed);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }
    index = parsed;
    return true;
}

bool parse_double_indexed_key(const std::string& key,
                             const std::string& prefix,
                             std::size_t& first_index,
                             std::size_t& second_index) {
    if (key.rfind(prefix, 0) != 0U) {
        return false;
    }
    const auto remainder = key.substr(prefix.size());
    const auto separator = remainder.find('.');
    if (separator == std::string::npos || separator == 0U ||
        separator + 1U >= remainder.size()) {
        return false;
    }
    const auto parse = [](const std::string& value,
                          std::size_t& output) {
        const auto* first = value.data();
        const auto* last = first + value.size();
        const auto parsed = std::from_chars(first, last, output);
        return parsed.ec == std::errc{} && parsed.ptr == last;
    };
    return parse(remainder.substr(0, separator), first_index) &&
           parse(remainder.substr(separator + 1U), second_index);
}

template <std::size_t N>
void add_missing_entries(const std::array<bool, N>& found, const char* prefix,
                         std::vector<std::string>& errors) {
    for (std::size_t index = 0; index < found.size(); ++index) {
        if (!found[index]) {
            errors.emplace_back(std::string("missing field menu text row: ") +
                                prefix + std::to_string(index));
        }
    }
}

bool is_field_menu_text_key(const std::string& key) {
    static constexpr const char* prefixes[]{
        "runner.", "archetype.", "race.", "class.", "status.",
        "cyberware.title.", "cyberware.entry.",
        "clips.", "dismiss.label.", "pocket-secretary.cyberdeck.line.",
        "pocket-secretary.group-item.", "pocket-secretary.group-items.empty.",
        "ui.option.", "notebook.option.", "notebook.no-run.",
        "notebook.mr-johnson.name.",
        "notebook.mr-johnson.detail.", "notebook.tip.",
        "magic.spells.option.", "magic.spells.description.",
        "items.text.option.", "attributes_skills.text.option.", "power.",
        "armor.",
    };
    for (const auto* prefix : prefixes) {
        if (key.rfind(prefix, 0U) == 0U) {
            return true;
        }
    }
    static constexpr const char* exact_keys[]{
        "metric.physical", "metric.mental", "metric.attack",
        "metric.defense", "metric.damage", "metric.drain", "metric.none",
        "metric.no-armor", "metric.weapons", "metric.guns", "metric.power",
        "cyberware.empty",
        "magic.defense.title",
        "magic.heading.force-posture", "magic.label.success",
        "magic.label.force", "magic.label.availability",
        "magic.panel.metric-labels", "magic.panel.force-posture-labels",
        "dismiss.separator", "notebook.tips.title",
        "notebook.tips.separator", "notebook.tips.page-count",
        "notebook.tips.page-number-clear", "notebook.options.title",
    };
    for (const auto* expected : exact_keys) {
        if (key == expected) {
            return true;
        }
    }
    return false;
}

template <std::size_t N>
void load_script_menu_labels(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id,
    const std::array<std::string_view, N>& expected_ids,
    std::array<std::string, N>& labels, std::vector<std::string>& errors,
    std::string* title = nullptr,
    std::array<std::string, N>* choice_ids = nullptr) {
    const auto menu = scripts.menu(module, function, text_profile, menu_id);
    if (!menu.has_value()) {
        errors.emplace_back("Lua menu is unavailable: " +
                            std::string(function) + ": " + scripts.failure());
        return;
    }
    if (menu->choices.size() != expected_ids.size()) {
        errors.emplace_back("Lua menu has the wrong choice count: " +
                            std::string(function));
        return;
    }
    if (title != nullptr) {
        const auto* title_text = scripts.dialogue_text(menu->title_key);
        if (title_text == nullptr) {
            errors.emplace_back("Lua menu title is missing: " +
                                menu->title_key);
            return;
        }
        *title = *title_text;
    }
    for (std::size_t index = 0; index < expected_ids.size(); ++index) {
        const auto& choice = menu->choices[index];
        if (choice.id != expected_ids[index]) {
            errors.emplace_back("Lua menu choices do not match the "
                                "native action order: " +
                                std::string(function));
            return;
        }
        if (choice_ids != nullptr) {
            (*choice_ids)[index] = choice.id;
        }
        const auto* label = scripts.dialogue_text(choice.label_key);
        if (label == nullptr) {
            errors.emplace_back("Lua menu label is missing: " +
                                choice.label_key);
            return;
        }
        labels[index] = *label;
    }
}

template <std::size_t N>
void load_script_menu_choice_ids(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id,
    const std::array<std::string_view, N>& expected_ids,
    std::array<std::string, N>& choice_ids,
    std::vector<std::string>& errors) {
    const auto menu = scripts.menu(module, function, text_profile, menu_id);
    if (!menu.has_value()) {
        errors.emplace_back("Lua menu is unavailable: " +
                            std::string(function) + ": " + scripts.failure());
        return;
    }
    if (menu->choices.size() != expected_ids.size()) {
        errors.emplace_back("Lua menu has the wrong choice count: " +
                            std::string(function));
        return;
    }
    for (std::size_t index = 0; index < expected_ids.size(); ++index) {
        if (menu->choices[index].id != expected_ids[index]) {
            errors.emplace_back("Lua menu choices do not match the "
                                "native action order: " +
                                std::string(function));
            return;
        }
        choice_ids[index] = menu->choices[index].id;
    }
}

} // namespace

FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets,
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts) {
    FieldMenuTextLoadResult result;
    const auto* record = assets.find("menu.field.service.routes");
    if (record == nullptr) {
        result.errors.emplace_back(
            "missing field service route alias: menu.field.service.routes");
        return result;
    }

    std::ifstream routing_input(record->absolute_path);
    if (!routing_input) {
        result.errors.emplace_back("cannot open field service routes: " +
                                  record->absolute_path.string());
        return result;
    }

    std::vector<std::pair<std::string, std::string>> text_rows;
    for (const auto& entry : scripts.dialogue_text_entries()) {
        if (is_field_menu_text_key(entry.first)) {
            text_rows.push_back(entry);
        }
    }
    std::sort(text_rows.begin(), text_rows.end(),
              [](const auto& left, const auto& right) {
                  return left.first < right.first;
              });
    for (const auto& [key, value] : text_rows) {
        if (value.find_first_of("\t\r") != std::string::npos) {
            result.errors.emplace_back(
                "field menu Lua text contains a TSV control character: " + key);
        }
    }

    std::string routing_line;
    std::size_t routing_line_number = 0U;
    bool routing_header_seen = false;
    while (std::getline(routing_input, routing_line)) {
        ++routing_line_number;
        if (!routing_line.empty() && routing_line.back() == '\r') {
            routing_line.pop_back();
        }
        if (routing_line.empty()) {
            continue;
        }
        if (!routing_header_seen) {
            routing_header_seen = true;
            if (routing_line != "key\ttext") {
                result.errors.emplace_back(
                    "invalid field menu routing header at line " +
                    std::to_string(routing_line_number));
            }
            continue;
        }
        const auto separator = routing_line.find('\t');
        if (separator == std::string::npos ||
            routing_line.rfind("field-service.title-destination.", 0U) != 0U) {
            result.errors.emplace_back(
                "field service route TSV contains a non-routing row at line " +
                std::to_string(routing_line_number));
            continue;
        }
        auto key = routing_line.substr(0U, separator);
        auto value = routing_line.substr(separator + 1U);
        if (value.size() >= 2U && value.front() == '"' &&
            value.back() == '"') {
            value = value.substr(1U, value.size() - 2U);
        }
        text_rows.emplace_back(std::move(key), std::move(value));
    }
    if (!routing_header_seen) {
        result.errors.emplace_back("field menu routing catalog is empty");
    }
    std::array<bool, kFieldMenuArchetypeNameCount> archetype_found{};
    std::array<bool, kFieldMenuRaceNameCount> race_found{};
    std::array<bool, kFieldMenuClassNameCount> class_found{};
    std::array<bool, kFieldMenuRunnerNameCount> runner_found{};
    std::array<bool, kFieldMenuStatusNameCount> status_found{};
    std::array<bool, kCyberwareTitleLineCount> cyberware_title_found{};
    std::array<bool, kCyberwareEntryCount> cyberware_entry_found{};
    std::array<bool, kMagicSpellTextCount> magic_spell_description_found{};
    std::array<bool, kFieldServiceTitleCount>
        field_service_title_destination_found{};
    std::array<bool, kCyberdeckUnavailableLineCount>
        pocket_secretary_cyberdeck_line_found{};
    std::array<bool, kPocketSecretaryGroupItemCount>
        pocket_secretary_group_item_found{};
    std::array<bool, kPocketSecretaryGroupItemsEmptyLineCount>
        pocket_secretary_group_items_empty_found{};
    std::array<bool, kNotebookNoRunLineCount> notebook_no_run_found{};
    std::array<bool, kNotebookMrJohnsonCount>
        notebook_mr_johnson_name_found{};
    std::array<bool, kNotebookMrJohnsonCount>
        notebook_mr_johnson_detail_found{};
    std::array<bool, kNotebookTipCount> notebook_tip_found{};
    bool notebook_tips_title_found = false;
    bool notebook_tips_separator_found = false;
    bool notebook_tips_page_count_found = false;
    bool notebook_tips_page_number_clear_found = false;
    std::array<bool, kDismissRunnerLabelCount> dismiss_runner_label_found{};
    std::array<bool, kFieldMenuPowerLabelCount> power_found{};
    std::array<std::array<bool, kFieldMenuArmorRowCount>,
               kFieldMenuArmorCount>
        armor_found{};
    bool physical_found = false;
    bool mental_found = false;
    bool attack_found = false;
    bool defense_found = false;
    bool damage_found = false;
    bool drain_found = false;
    bool none_found = false;
    bool no_armor_found = false;
    bool weapons_found = false;
    bool guns_found = false;
    bool power_metric_found = false;
    bool cyberware_empty_found = false;
    bool magic_defense_title_found = false;
    bool magic_force_posture_heading_found = false;
    bool magic_success_label_found = false;
    bool magic_force_label_found = false;
    bool magic_availability_label_found = false;
    bool magic_panel_metric_labels_found = false;
    bool magic_force_posture_labels_found = false;
    bool dismiss_runner_separator_found = false;
    std::array<std::string, kConfirmationChoiceCount> confirmation_labels{};
    load_script_menu_labels(
        scripts, "ui", "confirmation", "ui", "confirmation",
        kConfirmationChoiceIds, confirmation_labels, result.errors, nullptr,
        &result.catalog.confirmation_choice_ids);
    result.catalog.action_choice_yes = confirmation_labels[0];
    result.catalog.action_choice_no = confirmation_labels[1];
    load_script_menu_choice_ids(
        scripts, "items", "inventory_actions", "items",
        "inventory_actions", kInventoryActionChoiceIds,
        result.catalog.inventory_action_choice_ids, result.errors);
    std::array<std::string, kItemUiTextCount> item_ui_text{};
    load_script_menu_labels(
        scripts, "items", "ui_text_entries", "items.text", "catalog",
        kItemUiTextIds, item_ui_text, result.errors, nullptr,
        &result.catalog.item_ui_text_ids);
    result.catalog.action_item_equipped_prefix = item_ui_text[0];
    result.catalog.action_item_equipped_suffix = item_ui_text[1];
    result.catalog.action_discard_prompt = item_ui_text[2];
    result.catalog.action_cannot_discard = item_ui_text[3];
    result.catalog.action_cannot_trade = item_ui_text[4];
    result.catalog.action_trade_recipient_label = item_ui_text[5];
    result.catalog.action_trade_full_inventory_message = item_ui_text[6];
    result.catalog.action_automatic_use_message = item_ui_text[7];
    result.catalog.first_aid_prompt = item_ui_text[8];
    result.catalog.first_aid_label = item_ui_text[9];
    result.catalog.first_aid_cancel = item_ui_text[10];
    result.catalog.first_aid_not_injured = item_ui_text[11];
    result.catalog.first_aid_slap_patch_used = item_ui_text[12];
    result.catalog.first_aid_medkit_used = item_ui_text[13];
    std::array<std::string, kAttributesSkillsTextCount>
        attributes_skills_text{};
    load_script_menu_labels(
        scripts, "attributes_skills", "text_entries",
        "attributes_skills.text", "catalog", kAttributesSkillsTextIds,
        attributes_skills_text, result.errors, nullptr,
        &result.catalog.attribute_skill_text_ids);
    std::copy_n(attributes_skills_text.begin(), kAttributeSkillLabelCount,
                result.catalog.attribute_skill_labels.begin());
    result.catalog.attributes_upgrade_header = attributes_skills_text[20];
    result.catalog.attributes_upgrade_maximum_level_message =
        attributes_skills_text[21];
    result.catalog.attributes_upgrade_mage_only_message =
        attributes_skills_text[22];
    result.catalog.attributes_upgrade_insufficient_karma_message =
        attributes_skills_text[23];
    result.catalog.attributes_upgrade_prompt = attributes_skills_text[24];
    std::array<std::string, kMagicSpellTextCount> magic_spell_names{};
    load_script_menu_labels(
        scripts, "magic", "spells", "magic.spells", "catalog",
        kMagicSpellIds, magic_spell_names, result.errors, nullptr,
        &result.catalog.magic_spell_ids);
    result.catalog.magic_spell_names = std::move(magic_spell_names);
    load_script_menu_labels(
        scripts, "clips", "commands", "clips", "commands",
        kClipsCommandIds, result.catalog.clips_commands, result.errors);
    load_script_menu_labels(
        scripts, "clips", "trade_commands", "clips.trade", "commands",
        kClipsTradeCommandIds, result.catalog.clips_trade_commands,
        result.errors);
    std::array<std::string, kClipsTextCount> clips_text{};
    load_script_menu_labels(
        scripts, "clips", "text_entries", "clips.text", "catalog",
        kClipsTextIds, clips_text, result.errors, nullptr,
        &result.catalog.clips_text_ids);
    result.catalog.clips_title = clips_text[0];
    result.catalog.clips_separator = clips_text[1];
    result.catalog.clips_counter_label = clips_text[2];
    std::copy_n(clips_text.begin() + 3, 3,
                result.catalog.clips_trade_prompt.begin());
    std::copy_n(clips_text.begin() + 6, 2,
                result.catalog.clips_exchange_complete.begin());
    std::copy_n(clips_text.begin() + 8, 3,
                result.catalog.clips_reload_prompt.begin());
    std::copy_n(clips_text.begin() + 11, 2,
                result.catalog.clips_reload_success.begin());
    std::copy_n(clips_text.begin() + 13, 2,
                result.catalog.clips_out_of_ammo.begin());
    std::copy_n(clips_text.begin() + 15, 3,
                result.catalog.clips_item_no_clips.begin());
    std::copy_n(clips_text.begin() + 18, 3,
                result.catalog.clips_no_party.begin());
    std::array<std::string, kNotebookChoiceCount> notebook_choice_labels{};
    std::string notebook_title;
    load_script_menu_labels(
        scripts, "pocket_secretary", "notebook", "notebook", "options",
        kNotebookChoiceIds, notebook_choice_labels, result.errors,
        &notebook_title);
    result.catalog.notebook_labels[0] = std::move(notebook_title);
    std::copy(notebook_choice_labels.begin(), notebook_choice_labels.end(),
              result.catalog.notebook_labels.begin() + 1);
    for (const auto& [key, value] : text_rows) {
        if (has_confirmation_menu_label_prefix(key)) {
            if (!is_confirmation_menu_label_key(key)) {
                result.errors.emplace_back(
                    "unknown confirmation menu label key: " + key);
            }
            continue;
        }
        if (has_notebook_menu_label_prefix(key)) {
            if (!is_notebook_menu_label_key(key)) {
                result.errors.emplace_back(
                    "unknown Notebook menu label key: " + key);
            }
            continue;
        }
        if (has_clips_menu_label_prefix(key)) {
            if (!is_clips_menu_label_key(key)) {
                result.errors.emplace_back(
                    "unknown Clips menu label key: " + key);
            }
            continue;
        }
        if (has_item_ui_text_label_prefix(key)) {
            if (!is_item_ui_text_label_key(key)) {
                result.errors.emplace_back(
                    "unknown Items UI text label key: " + key);
            }
            continue;
        }
        if (has_attributes_skills_text_label_prefix(key)) {
            if (!is_attributes_skills_text_label_key(key)) {
                result.errors.emplace_back(
                    "unknown Attributes/Skills UI text label key: " + key);
            }
            continue;
        }
        std::size_t index = 0;
        if (parse_indexed_key(key, "field-service.title-destination.",
                              index) &&
            index < result.catalog.field_service_title_destinations.size()) {
            if (field_service_title_destination_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                std::uint16_t destination = 0U;
                const auto* first = value.data();
                const auto* last = first + value.size();
                const auto parsed = std::from_chars(first, last, destination);
                if (parsed.ec != std::errc{} || parsed.ptr != last) {
                    result.errors.emplace_back(
                        "invalid field-service title destination: " + key);
                } else {
                    field_service_title_destination_found[index] = true;
                    result.catalog.field_service_title_destinations[index] =
                        destination;
                }
            }
            continue;
        }
        if (parse_indexed_key(key, "runner.", index) &&
            index < result.catalog.runner_names.size()) {
            if (runner_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                runner_found[index] = true;
                result.catalog.runner_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "archetype.", index) &&
            index < result.catalog.archetype_names.size()) {
            if (archetype_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                archetype_found[index] = true;
                result.catalog.archetype_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "race.", index) &&
            index < result.catalog.race_names.size()) {
            if (race_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                race_found[index] = true;
                result.catalog.race_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "class.", index) &&
            index < result.catalog.class_names.size()) {
            if (class_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                class_found[index] = true;
                result.catalog.class_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "status.", index) &&
            index < result.catalog.status_names.size()) {
            if (status_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                status_found[index] = true;
                result.catalog.status_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "cyberware.title.", index) &&
            index < result.catalog.cyberware_title_lines.size()) {
            if (cyberware_title_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                cyberware_title_found[index] = true;
                result.catalog.cyberware_title_lines[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "cyberware.entry.", index) &&
            index < result.catalog.cyberware_entries.size()) {
            if (cyberware_entry_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                cyberware_entry_found[index] = true;
                result.catalog.cyberware_entries[index] = value;
            }
            continue;
        }
        const auto parse_named = [&](const char* expected,
                                     std::string& destination,
                                     bool& found) {
            if (key != expected) {
                return false;
            }
            if (found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                found = true;
                destination = value;
            }
            return true;
        };
        if (parse_named("metric.physical", result.catalog.physical_label,
                        physical_found) ||
            parse_named("metric.mental", result.catalog.mental_label,
                        mental_found) ||
            parse_named("metric.attack", result.catalog.attack_label,
                        attack_found) ||
            parse_named("metric.defense", result.catalog.defense_label,
                        defense_found) ||
            parse_named("metric.damage", result.catalog.damage_label,
                        damage_found) ||
            parse_named("metric.drain", result.catalog.drain_label,
                        drain_found) ||
            parse_named("metric.none", result.catalog.none_label,
                        none_found) ||
            parse_named("metric.no-armor", result.catalog.no_armor_label,
                        no_armor_found) ||
            parse_named("metric.weapons", result.catalog.weapons_label,
                        weapons_found) ||
            parse_named("metric.guns", result.catalog.guns_label, guns_found) ||
            parse_named("metric.power", result.catalog.power_label,
                        power_metric_found) ||
            parse_named("cyberware.empty",
                        result.catalog.cyberware_empty_message,
                        cyberware_empty_found) ||
            parse_named("magic.defense.title",
                        result.catalog.magic_defense_title,
                        magic_defense_title_found) ||
            parse_named("magic.heading.force-posture",
                        result.catalog.magic_force_posture_heading,
                        magic_force_posture_heading_found) ||
            parse_named("magic.label.success",
                        result.catalog.magic_success_label,
                        magic_success_label_found) ||
            parse_named("magic.label.force",
                        result.catalog.magic_force_label,
                        magic_force_label_found) ||
            parse_named("magic.label.availability",
                        result.catalog.magic_availability_label,
                        magic_availability_label_found) ||
            parse_named("magic.panel.metric-labels",
                        result.catalog.magic_panel_metric_labels,
                        magic_panel_metric_labels_found) ||
            parse_named("magic.panel.force-posture-labels",
                        result.catalog.magic_force_posture_labels,
                        magic_force_posture_labels_found) ||
            parse_named("dismiss.separator",
                        result.catalog.dismiss_runner_separator,
                        dismiss_runner_separator_found)) {
            continue;
        }
        if (parse_indexed_key(key, "dismiss.label.", index) &&
            index < result.catalog.dismiss_runner_labels.size()) {
            if (dismiss_runner_label_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                dismiss_runner_label_found[index] = true;
                result.catalog.dismiss_runner_labels[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "pocket-secretary.cyberdeck.line.",
                              index) &&
            index < result.catalog
                        .pocket_secretary_cyberdeck_unavailable_lines.size()) {
            if (pocket_secretary_cyberdeck_line_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                pocket_secretary_cyberdeck_line_found[index] = true;
                result.catalog
                    .pocket_secretary_cyberdeck_unavailable_lines[index] =
                    value;
            }
            continue;
        }
        if (parse_indexed_key(key, "pocket-secretary.group-item.", index) &&
            index < result.catalog.pocket_secretary_group_item_names.size()) {
            if (pocket_secretary_group_item_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                pocket_secretary_group_item_found[index] = true;
                result.catalog.pocket_secretary_group_item_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "pocket-secretary.group-items.empty.",
                              index) &&
            index < result.catalog
                        .pocket_secretary_group_items_empty_lines.size()) {
            if (pocket_secretary_group_items_empty_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                pocket_secretary_group_items_empty_found[index] = true;
                result.catalog.pocket_secretary_group_items_empty_lines[index] =
                    value;
            }
            continue;
        }
        if (parse_indexed_key(key, "notebook.no-run.", index) &&
            index < result.catalog.notebook_no_run_lines.size()) {
            if (notebook_no_run_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_no_run_found[index] = true;
                result.catalog.notebook_no_run_lines[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "notebook.mr-johnson.name.", index) &&
            index < result.catalog.notebook_mr_johnson_names.size()) {
            if (notebook_mr_johnson_name_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_mr_johnson_name_found[index] = true;
                result.catalog.notebook_mr_johnson_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "notebook.mr-johnson.detail.", index) &&
            index < result.catalog.notebook_mr_johnson_details.size()) {
            if (notebook_mr_johnson_detail_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_mr_johnson_detail_found[index] = true;
                result.catalog.notebook_mr_johnson_details[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "notebook.tip.", index) &&
            index < result.catalog.notebook_tips.size()) {
            if (notebook_tip_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_tip_found[index] = true;
                auto tip = value;
                std::size_t position = 0U;
                while ((position = tip.find("\\n", position)) !=
                       std::string::npos) {
                    tip.replace(position, 2U, "\n");
                    ++position;
                }
                result.catalog.notebook_tips[index] = std::move(tip);
            }
            continue;
        }
        if (key == "notebook.tips.title") {
            if (notebook_tips_title_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_tips_title_found = true;
                result.catalog.notebook_tips_title = value;
            }
            continue;
        }
        if (key == "notebook.tips.separator") {
            if (notebook_tips_separator_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_tips_separator_found = true;
                result.catalog.notebook_tips_separator = value;
            }
            continue;
        }
        if (key == "notebook.tips.page-count") {
            if (notebook_tips_page_count_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_tips_page_count_found = true;
                result.catalog.notebook_tips_page_count_label = value;
            }
            continue;
        }
        if (key == "notebook.tips.page-number-clear") {
            if (notebook_tips_page_number_clear_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_tips_page_number_clear_found = true;
                result.catalog.notebook_tips_page_number_clear = value;
            }
            continue;
        }
        if (key.rfind(kMagicSpellOptionPrefix, 0U) == 0U) {
            const auto spell_id = std::string_view(key).substr(
                kMagicSpellOptionPrefix.size());
            if (std::find(kMagicSpellIds.begin(), kMagicSpellIds.end(),
                          spell_id) == kMagicSpellIds.end()) {
                result.errors.emplace_back(
                    "unknown Magic spell option key: " + key);
            }
            continue;
        }
        if (key.rfind(kMagicSpellDescriptionPrefix, 0U) == 0U) {
            if (!magic_spell_index(
                    std::string_view(key).substr(
                        kMagicSpellDescriptionPrefix.size()),
                    index)) {
                result.errors.emplace_back(
                    "unknown Magic spell description key: " + key);
                continue;
            }
            if (magic_spell_description_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                magic_spell_description_found[index] = true;
                auto description = value;
                std::size_t position = 0U;
                while ((position = description.find("\\n", position)) !=
                       std::string::npos) {
                    description.replace(position, 2U, "\n");
                    ++position;
                }
                result.catalog.magic_spell_descriptions[index] =
                    std::move(description);
            }
            continue;
        }
        if (parse_indexed_key(key, "power.", index) &&
            index < result.catalog.power_labels.size()) {
            if (power_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                power_found[index] = true;
                result.catalog.power_labels[index] = value;
            }
            continue;
        }
        std::size_t armor_index = 0;
        std::size_t armor_row = 0;
        if (parse_double_indexed_key(key, "armor.", armor_index,
                                     armor_row) &&
            armor_index < result.catalog.armor_labels.size() &&
            armor_row < result.catalog.armor_labels[armor_index].size()) {
            if (armor_found[armor_index][armor_row]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                armor_found[armor_index][armor_row] = true;
                result.catalog.armor_labels[armor_index][armor_row] = value;
            }
            continue;
        }
        result.errors.emplace_back("unknown field menu text row: " + key);
    }
    if (text_rows.empty()) {
        result.errors.emplace_back("field menu text catalog is empty");
    }
    add_missing_entries(archetype_found, "archetype.", result.errors);
    add_missing_entries(race_found, "race.", result.errors);
    add_missing_entries(class_found, "class.", result.errors);
    add_missing_entries(runner_found, "runner.", result.errors);
    add_missing_entries(status_found, "status.", result.errors);
    add_missing_entries(cyberware_title_found, "cyberware.title.",
                        result.errors);
    add_missing_entries(cyberware_entry_found, "cyberware.entry.",
                        result.errors);
    const auto require_named = [&result](const char* key, const bool found) {
        if (!found) {
            result.errors.emplace_back(
                "missing field menu text row: " + std::string(key));
        }
    };
    require_named("metric.physical", physical_found);
    require_named("metric.mental", mental_found);
    require_named("metric.attack", attack_found);
    require_named("metric.defense", defense_found);
    require_named("metric.damage", damage_found);
    require_named("metric.drain", drain_found);
    require_named("metric.none", none_found);
    require_named("metric.no-armor", no_armor_found);
    require_named("metric.weapons", weapons_found);
    require_named("metric.guns", guns_found);
    require_named("metric.power", power_metric_found);
    require_named("cyberware.empty", cyberware_empty_found);
    require_named("magic.defense.title", magic_defense_title_found);
    require_named("magic.heading.force-posture",
                  magic_force_posture_heading_found);
    require_named("magic.label.success", magic_success_label_found);
    require_named("magic.label.force", magic_force_label_found);
    require_named("magic.label.availability",
                  magic_availability_label_found);
    require_named("magic.panel.metric-labels",
                  magic_panel_metric_labels_found);
    require_named("magic.panel.force-posture-labels",
                  magic_force_posture_labels_found);
    require_named("dismiss.separator", dismiss_runner_separator_found);
    require_named("notebook.tips.title", notebook_tips_title_found);
    require_named("notebook.tips.separator", notebook_tips_separator_found);
    require_named("notebook.tips.page-count",
                  notebook_tips_page_count_found);
    require_named("notebook.tips.page-number-clear",
                  notebook_tips_page_number_clear_found);
    add_missing_entries(power_found, "power.", result.errors);
    for (std::size_t armor = 0; armor < armor_found.size(); ++armor) {
        for (std::size_t row = 0; row < armor_found[armor].size(); ++row) {
            if (!armor_found[armor][row]) {
                result.errors.emplace_back(
                    "missing field menu text row: armor." +
                    std::to_string(armor) + "." + std::to_string(row));
            }
        }
    }
    for (std::size_t index = 0; index < magic_spell_description_found.size();
         ++index) {
        if (!magic_spell_description_found[index]) {
            result.errors.emplace_back(
                "missing field menu text row: magic.spells.description." +
                std::string(kMagicSpellIds[index]));
        }
    }
    add_missing_entries(pocket_secretary_cyberdeck_line_found,
                        "pocket-secretary.cyberdeck.line.", result.errors);
    add_missing_entries(pocket_secretary_group_item_found,
                        "pocket-secretary.group-item.", result.errors);
    add_missing_entries(pocket_secretary_group_items_empty_found,
                        "pocket-secretary.group-items.empty.", result.errors);
    add_missing_entries(notebook_no_run_found, "notebook.no-run.",
                        result.errors);
    add_missing_entries(notebook_mr_johnson_name_found,
                        "notebook.mr-johnson.name.", result.errors);
    add_missing_entries(notebook_mr_johnson_detail_found,
                        "notebook.mr-johnson.detail.", result.errors);
    add_missing_entries(notebook_tip_found, "notebook.tip.", result.errors);
    add_missing_entries(field_service_title_destination_found,
                        "field-service.title-destination.", result.errors);
    add_missing_entries(dismiss_runner_label_found, "dismiss.label.",
                        result.errors);
    const auto decode_newlines = [](std::string& value) {
        std::size_t position = 0U;
        while ((position = value.find("\\n", position)) !=
               std::string::npos) {
            value.replace(position, 2U, "\n");
            ++position;
        }
    };
    decode_newlines(result.catalog.first_aid_prompt);
    decode_newlines(result.catalog.first_aid_not_injured);
    decode_newlines(result.catalog.action_trade_full_inventory_message);
    decode_newlines(result.catalog.action_automatic_use_message);
    decode_newlines(result.catalog.magic_panel_metric_labels);
    decode_newlines(result.catalog.magic_force_posture_labels);
    for (auto& detail : result.catalog.notebook_mr_johnson_details) {
        decode_newlines(detail);
    }

    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

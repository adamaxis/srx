#include "field_menu_text_catalog.hpp"

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::string_view kMagicSpellOptionPrefix =
    "magic.spells.option.";
constexpr std::string_view kMagicSpellDescriptionPrefix =
    "magic.spells.description.";

bool has_recruited_runner_dialogue_prefix(const std::string_view key) {
    constexpr std::array<std::string_view, 10> kRunnerPrefixes{{
        "runner.ricky.",
        "runner.winston_marrs.",
        "runner.trent.",
        "runner.petr_uvehr.",
        "runner.walking_bear.",
        "runner.phantom.",
        "runner.two_fists.",
        "runner.freya.",
        "runner.rianna.",
        "runner.stark.",
    }};
    return std::any_of(
        kRunnerPrefixes.begin(), kRunnerPrefixes.end(),
        [key](const std::string_view prefix) {
            return key.rfind(prefix, 0U) == 0U;
        });
}

bool magic_spell_index(
    const std::string_view id, std::size_t& index,
    const std::array<std::string, kMagicSpellTextCount>& spell_ids) {
    const auto found = std::find_if(
        spell_ids.begin(), spell_ids.end(),
        [id](const std::string& spell_id) {
            return std::string_view(spell_id) == id;
        });
    if (found == spell_ids.end()) {
        return false;
    }
    index = static_cast<std::size_t>(found - spell_ids.begin());
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

template <std::size_t N>
bool has_menu_choice_id(const std::array<std::string, N>& choice_ids,
                        const std::string_view id) {
    return std::find_if(
               choice_ids.begin(), choice_ids.end(),
               [id](const std::string& choice_id) {
                   return std::string_view(choice_id) == id;
               }) != choice_ids.end();
}

bool is_item_ui_text_label_key(
    const std::string_view key,
    const std::array<std::string, kItemUiTextCount>& choice_ids) {
    constexpr std::string_view kTextPrefix = "items.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(choice_ids, key.substr(kTextPrefix.size()));
}

bool has_attributes_skills_text_label_prefix(const std::string_view key) {
    return key.rfind("attributes_skills.text.option.", 0U) == 0U;
}

bool is_attributes_skills_text_label_key(
    const std::string_view key,
    const std::array<std::string, kAttributesSkillsTextCount>& text_ids) {
    constexpr std::string_view kTextPrefix =
        "attributes_skills.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(text_ids, key.substr(kTextPrefix.size()));
}

bool has_item_display_text_label_prefix(const std::string_view key) {
    return key.rfind("items.display.option.", 0U) == 0U;
}

bool is_item_display_text_label_key(
    const std::string_view key,
    const std::array<std::string, kItemDisplayTextCount>& choice_ids) {
    constexpr std::string_view kTextPrefix = "items.display.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(choice_ids, key.substr(kTextPrefix.size()));
}

bool has_magic_panel_text_label_prefix(const std::string_view key) {
    return key.rfind("magic.panel.text.option.", 0U) == 0U;
}

bool is_magic_panel_text_label_key(
    const std::string_view key,
    const std::array<std::string, kMagicPanelTextCount>& text_ids) {
    constexpr std::string_view kTextPrefix = "magic.panel.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(text_ids, key.substr(kTextPrefix.size()));
}

bool has_cyberware_text_label_prefix(const std::string_view key) {
    return key.rfind("cyberware.text.option.", 0U) == 0U;
}

bool is_cyberware_text_label_key(
    const std::string_view key,
    const std::array<std::string, kCyberwareTextCount>& text_ids) {
    constexpr std::string_view kTextPrefix = "cyberware.text.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(text_ids, key.substr(kTextPrefix.size()));
}

bool has_field_metric_text_label_prefix(const std::string_view key) {
    return key.rfind("ui.field_metric.option.", 0U) == 0U;
}

bool is_field_metric_text_label_key(
    const std::string_view key,
    const std::array<std::string, kFieldMetricTextCount>& choice_ids) {
    constexpr std::string_view kTextPrefix = "ui.field_metric.option.";
    return key.rfind(kTextPrefix, 0U) == 0U &&
           has_menu_choice_id(choice_ids, key.substr(kTextPrefix.size()));
}

bool has_command_overview_text_label_prefix(const std::string_view key) {
    return key.rfind("command_overview.race.option.", 0U) == 0U ||
           key.rfind("command_overview.class.option.", 0U) == 0U ||
           key.rfind("command_overview.posture.option.", 0U) == 0U;
}

bool is_command_overview_text_label_key(
    const std::string_view key,
    const std::array<std::string, kFieldMenuRaceNameCount>& race_ids,
    const std::array<std::string, kFieldMenuClassNameCount>& class_ids,
    const std::array<std::string, kFieldMenuStatusNameCount>& posture_ids) {
    constexpr std::string_view kRacePrefix =
        "command_overview.race.option.";
    if (key.rfind(kRacePrefix, 0U) == 0U) {
        return has_menu_choice_id(race_ids, key.substr(kRacePrefix.size()));
    }
    constexpr std::string_view kClassPrefix =
        "command_overview.class.option.";
    if (key.rfind(kClassPrefix, 0U) == 0U) {
        return has_menu_choice_id(class_ids, key.substr(kClassPrefix.size()));
    }
    constexpr std::string_view kPosturePrefix =
        "command_overview.posture.option.";
    return key.rfind(kPosturePrefix, 0U) == 0U &&
           has_menu_choice_id(posture_ids,
                              key.substr(kPosturePrefix.size()));
}

bool is_clips_menu_label_key(
    const std::string_view key,
    const std::array<std::string, kFieldMenuClipsCommandCount>& commands,
    const std::array<std::string, kFieldMenuClipsCommandCount>& trade_commands,
    const std::array<std::string, kClipsTextCount>& text_ids) {
    constexpr std::string_view kCommandPrefix = "clips.option.";
    constexpr std::string_view kTradeCommandPrefix = "clips.trade.option.";
    if (key.rfind(kCommandPrefix, 0U) == 0U) {
        return has_menu_choice_id(commands, key.substr(kCommandPrefix.size()));
    }
    if (key.rfind(kTradeCommandPrefix, 0U) == 0U) {
        return has_menu_choice_id(
            trade_commands, key.substr(kTradeCommandPrefix.size()));
    }
    constexpr std::string_view kTextPrefix = "clips.text.option.";
    if (key.rfind(kTextPrefix, 0U) == 0U) {
        return has_menu_choice_id(text_ids, key.substr(kTextPrefix.size()));
    }
    return false;
}

template <std::size_t N>
bool has_semantic_label_key(const std::string_view key,
                            const std::string_view prefix,
                            const std::array<std::string, N>& ids) {
    return key.rfind(prefix, 0U) == 0U &&
           has_menu_choice_id(ids, key.substr(prefix.size()));
}

bool has_semantic_label_key(const std::string_view key,
                            const std::string_view prefix,
                            const std::string_view id) {
    return key.rfind(prefix, 0U) == 0U &&
           key.substr(prefix.size()) == id;
}

bool is_notebook_tip_text_label_key(
    const std::string_view key,
    const std::array<std::string, kNotebookTipCount>& tip_ids) {
    constexpr std::string_view kPrefix =
        "pocket_secretary.notebook.tips.option.tip_";
    if (key.rfind(kPrefix, 0U) != 0U) {
        return false;
    }
    const auto number_text = key.substr(kPrefix.size());
    if (number_text.empty() || number_text.front() == '0') {
        return false;
    }
    std::size_t number = 0U;
    const auto* first = number_text.data();
    const auto* last = first + number_text.size();
    const auto result = std::from_chars(first, last, number);
    return result.ec == std::errc{} && result.ptr == last && number > 0U &&
           number <= kNotebookTipCount &&
           has_menu_choice_id(tip_ids, "tip_" + std::to_string(number));
}

bool has_pocket_secretary_text_label_prefix(const std::string_view key) {
    return key.rfind("pocket_secretary.", 0U) == 0U;
}

bool is_pocket_secretary_text_label_key(
    const std::string_view key,
    const std::array<std::string, 5>& root_choice_ids,
    const std::array<std::string, 1>& root_text_ids,
    const FieldMenuTextCatalog& catalog) {
    if (key == "pocket_secretary.root.title" ||
        key == "pocket_secretary.notebook.options.title") {
        return true;
    }
    return has_semantic_label_key(
               key, "pocket_secretary.option.", root_choice_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.root.text.option.", root_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.dismiss.option.",
               catalog.dismiss_runner_choice_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.dismiss.text.option.",
               catalog.dismiss_runner_separator_text_id) ||
           has_semantic_label_key(
               key, "pocket_secretary.cyberdeck.option.",
               catalog.pocket_secretary_cyberdeck_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.group_item.option.",
               catalog.pocket_secretary_group_item_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.group_items.empty.option.",
               catalog.pocket_secretary_group_items_empty_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.notebook.option.",
               catalog.notebook_choice_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.notebook.no_run.option.",
               catalog.notebook_no_run_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.notebook.mr_johnson.name.option.",
               catalog.notebook_mr_johnson_name_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.notebook.mr_johnson.detail.option.",
               catalog.notebook_mr_johnson_detail_text_ids) ||
           has_semantic_label_key(
               key, "pocket_secretary.notebook.tips.option.",
               catalog.notebook_tips_metadata_text_ids) ||
           is_notebook_tip_text_label_key(key,
                                          catalog.notebook_tip_text_ids);
}

bool has_confirmation_menu_label_prefix(const std::string_view key) {
    return key.rfind("ui.option.", 0U) == 0U;
}

bool is_confirmation_menu_label_key(
    const std::string_view key,
    const std::array<std::string, kConfirmationChoiceCount>& choice_ids) {
    constexpr std::string_view kChoicePrefix = "ui.option.";
    return key.rfind(kChoicePrefix, 0U) == 0U &&
           has_menu_choice_id(choice_ids,
                              key.substr(kChoicePrefix.size()));
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
        "runner.", "archetype.",
        "clips.", "pocket_secretary.", "ui.option.",
        "magic.spells.option.", "magic.spells.description.",
        "magic.panel.text.option.", "cyberware.text.option.",
        "items.text.option.", "items.display.option.",
        "ui.field_metric.option.",
        "command_overview.race.option.",
        "command_overview.class.option.",
        "command_overview.posture.option.",
        "attributes_skills.text.option.",
    };
    for (const auto* prefix : prefixes) {
        if (key.rfind(prefix, 0U) == 0U) {
            return true;
        }
    }
    return false;
}

template <std::size_t N, typename ChoiceValidator>
void load_script_menu_labels_impl(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id,
    std::array<std::string, N>& labels, std::vector<std::string>& errors,
    const ChoiceValidator& choice_is_valid, std::string* title,
    std::array<std::string, N>* choice_ids = nullptr) {
    const auto menu = scripts.menu(module, function, text_profile, menu_id);
    if (!menu.has_value()) {
        errors.emplace_back("Lua menu is unavailable: " +
                            std::string(function) + ": " + scripts.failure());
        return;
    }
    if (menu->choices.size() != N) {
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
    std::array<std::string, N> resolved_choice_ids{};
    for (std::size_t index = 0; index < N; ++index) {
        const auto& choice = menu->choices[index];
        const auto seen_end = resolved_choice_ids.begin() +
                              static_cast<std::ptrdiff_t>(index);
        if (choice.id.empty() ||
            std::find(resolved_choice_ids.begin(), seen_end, choice.id) !=
                seen_end ||
            !choice_is_valid(index, choice)) {
            errors.emplace_back("Lua menu choices do not match the "
                                "native action order or contain an empty/"
                                "duplicate id: " + std::string(function));
            return;
        }
        resolved_choice_ids[index] = choice.id;
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

template <typename IdString, std::size_t N>
void load_script_menu_labels(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id,
    const std::array<IdString, N>& expected_ids,
    std::array<std::string, N>& labels, std::vector<std::string>& errors,
    std::string* title = nullptr,
    std::array<std::string, N>* choice_ids = nullptr) {
    const auto choice_is_valid = [&expected_ids](
                                     const std::size_t index,
                                     const auto& choice) {
        return std::string_view(choice.id) ==
               std::string_view(expected_ids[index]);
    };
    load_script_menu_labels_impl(
        scripts, module, function, text_profile, menu_id, labels, errors,
        choice_is_valid, title, choice_ids);
}

template <std::size_t N>
void load_script_menu_labels(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id,
    std::array<std::string, N>& labels, std::vector<std::string>& errors,
    std::string* title = nullptr,
    std::array<std::string, N>* choice_ids = nullptr) {
    const auto any_nonempty_choice = [](std::size_t, const auto& choice) {
        return !choice.id.empty();
    };
    load_script_menu_labels_impl(
        scripts, module, function, text_profile, menu_id, labels, errors,
        any_nonempty_choice, title, choice_ids);
}

template <std::size_t N>
void load_script_id_list(
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts,
    const std::string_view module, const std::string_view function,
    std::array<std::string, N>& choice_ids,
    std::vector<std::string>& errors) {
    const auto ids = scripts.string_list(module, function);
    if (!ids.has_value()) {
        errors.emplace_back("Lua ID list is unavailable: " +
                            std::string(function) + ": " + scripts.failure());
        return;
    }
    if (ids->size() != N) {
        errors.emplace_back("Lua ID list has the wrong count: " +
                            std::string(function));
        return;
    }
    for (std::size_t index = 0; index < N; ++index) {
        const auto seen_end = choice_ids.begin() +
                              static_cast<std::ptrdiff_t>(index);
        if ((*ids)[index].empty() ||
            std::find(choice_ids.begin(), seen_end, (*ids)[index]) !=
                seen_end) {
            errors.emplace_back("Lua ID list contains an empty or duplicate "
                                "id: " + std::string(function));
            return;
        }
        choice_ids[index] = (*ids)[index];
    }
}

} // namespace

FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets,
    shadowrun::ghidra_conversion::scripting::GameScriptRuntime& scripts) {
    FieldMenuTextLoadResult result;
    std::array<std::string, 5> pocket_secretary_root_choice_ids{};
    load_script_id_list(
        scripts, "pocket_secretary", "root",
        pocket_secretary_root_choice_ids, result.errors);
    std::array<std::string, 1> pocket_secretary_root_text_ids{};
    std::array<std::string, 1> pocket_secretary_root_text{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "root_text_entries",
        "pocket_secretary.root.text", "catalog", pocket_secretary_root_text,
        result.errors, nullptr, &pocket_secretary_root_text_ids);
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
    std::array<bool, kFieldMenuRunnerNameCount> runner_found{};
    std::array<bool, kMagicSpellTextCount> magic_spell_description_found{};
    std::array<bool, kFieldServiceTitleCount>
        field_service_title_destination_found{};
    std::array<std::string, kConfirmationChoiceCount> confirmation_labels{};
    load_script_menu_labels(
        scripts, "ui", "confirmation", "ui", "confirmation",
        confirmation_labels, result.errors, nullptr,
        &result.catalog.confirmation_choice_ids);
    result.catalog.action_choice_yes = confirmation_labels[0];
    result.catalog.action_choice_no = confirmation_labels[1];
    std::array<std::string, kFieldMetricTextCount> field_metric_labels{};
    load_script_menu_labels(
        scripts, "ui", "field_metric_labels", "ui.field_metric", "catalog",
        field_metric_labels, result.errors, nullptr,
        &result.catalog.field_metric_text_ids);
    result.catalog.physical_label = field_metric_labels[0];
    result.catalog.mental_label = field_metric_labels[1];
    result.catalog.attack_label = field_metric_labels[2];
    result.catalog.defense_label = field_metric_labels[3];
    result.catalog.damage_label = field_metric_labels[4];
    result.catalog.drain_label = field_metric_labels[5];
    result.catalog.none_label = field_metric_labels[6];
    result.catalog.no_armor_label = field_metric_labels[7];
    result.catalog.weapons_label = field_metric_labels[8];
    result.catalog.guns_label = field_metric_labels[9];
    result.catalog.power_label = field_metric_labels[10];
    std::array<std::string, kFieldMenuRaceNameCount> race_labels{};
    load_script_menu_labels(
        scripts, "command_overview", "races", "command_overview.race",
        "catalog", race_labels, result.errors,
        nullptr, &result.catalog.race_text_ids);
    result.catalog.race_names = std::move(race_labels);
    std::array<std::string, kFieldMenuClassNameCount> class_labels{};
    load_script_menu_labels(
        scripts, "command_overview", "classes", "command_overview.class",
        "catalog", class_labels, result.errors,
        nullptr, &result.catalog.class_text_ids);
    result.catalog.class_names = std::move(class_labels);
    std::array<std::string, kFieldMenuStatusNameCount> posture_labels{};
    load_script_menu_labels(
        scripts, "command_overview", "postures",
        "command_overview.posture", "catalog", posture_labels,
        result.errors, nullptr,
        &result.catalog.status_text_ids);
    result.catalog.status_names = std::move(posture_labels);
    load_script_id_list(
        scripts, "items", "inventory_actions",
        result.catalog.inventory_action_choice_ids, result.errors);
    std::array<std::string, kItemUiTextCount> item_ui_text{};
    load_script_menu_labels(
        scripts, "items", "ui_text_entries", "items.text", "catalog",
        item_ui_text, result.errors, nullptr,
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
    std::array<std::string, kItemDisplayTextCount> item_display_text{};
    load_script_menu_labels(
        scripts, "items", "display_text_entries", "items.display",
        "catalog", item_display_text, result.errors,
        nullptr, &result.catalog.item_display_text_ids);
    std::copy_n(item_display_text.begin(), kFieldMenuPowerLabelCount,
                result.catalog.power_labels.begin());
    for (std::size_t armor = 0; armor < kFieldMenuArmorCount; ++armor) {
        for (std::size_t row = 0; row < kFieldMenuArmorRowCount; ++row) {
            const auto index = kFieldMenuPowerLabelCount +
                               (armor * kFieldMenuArmorRowCount) + row;
            result.catalog.armor_labels[armor][row] =
                item_display_text[index];
        }
    }
    std::array<std::string, kAttributesSkillsTextCount>
        attributes_skills_text{};
    load_script_menu_labels(
        scripts, "attributes_skills", "text_entries",
        "attributes_skills.text", "catalog", attributes_skills_text,
        result.errors, nullptr,
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
        magic_spell_names, result.errors, nullptr,
        &result.catalog.magic_spell_ids);
    result.catalog.magic_spell_names = std::move(magic_spell_names);
    std::array<std::string, kMagicSpellTextCount> magic_spell_descriptions{};
    load_script_menu_labels(
        scripts, "magic", "spell_descriptions",
        "magic.spells.description", "catalog", result.catalog.magic_spell_ids,
        magic_spell_descriptions, result.errors);
    result.catalog.magic_spell_descriptions =
        std::move(magic_spell_descriptions);
    std::array<std::string, kMagicPanelTextCount> magic_panel_text{};
    load_script_menu_labels(
        scripts, "magic", "panel_text_entries", "magic.panel.text",
        "catalog", magic_panel_text, result.errors,
        nullptr, &result.catalog.magic_panel_text_ids);
    result.catalog.magic_defense_title = magic_panel_text[0];
    result.catalog.magic_force_posture_heading = magic_panel_text[1];
    result.catalog.magic_success_label = magic_panel_text[2];
    result.catalog.magic_force_label = magic_panel_text[3];
    result.catalog.magic_availability_label = magic_panel_text[4];
    result.catalog.magic_panel_metric_labels = magic_panel_text[5];
    result.catalog.magic_force_posture_labels = magic_panel_text[6];
    std::array<std::string, kCyberwareTextCount> cyberware_text{};
    load_script_menu_labels(
        scripts, "cyberware", "text_entries", "cyberware.text", "catalog",
        cyberware_text, result.errors, nullptr,
        &result.catalog.cyberware_text_ids);
    std::copy_n(cyberware_text.begin(), kCyberwareTitleLineCount,
                result.catalog.cyberware_title_lines.begin());
    result.catalog.cyberware_empty_message = cyberware_text[2];
    std::copy_n(cyberware_text.begin() + 3, kCyberwareEntryCount,
                result.catalog.cyberware_entries.begin());
    std::array<std::string, kFieldMenuClipsCommandCount> clips_command_ids{};
    load_script_menu_labels(
        scripts, "clips", "commands", "clips", "commands",
        result.catalog.clips_commands, result.errors, nullptr,
        &clips_command_ids);
    std::array<std::string, kFieldMenuClipsCommandCount>
        clips_trade_command_ids{};
    load_script_menu_labels(
        scripts, "clips", "trade_commands", "clips.trade", "commands",
        result.catalog.clips_trade_commands, result.errors, nullptr,
        &clips_trade_command_ids);
    std::array<std::string, kClipsTextCount> clips_text{};
    load_script_menu_labels(
        scripts, "clips", "text_entries", "clips.text", "catalog",
        clips_text, result.errors, nullptr,
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
        scripts, "pocket_secretary", "notebook",
        "pocket_secretary.notebook", "options",
        notebook_choice_labels, result.errors,
        &notebook_title, &result.catalog.notebook_choice_ids);
    result.catalog.notebook_labels[0] = std::move(notebook_title);
    std::copy(notebook_choice_labels.begin(), notebook_choice_labels.end(),
              result.catalog.notebook_labels.begin() + 1);
    std::array<std::string, kDismissRunnerLabelCount> dismiss_labels{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "dismiss_options",
        "pocket_secretary.dismiss", "options", dismiss_labels,
        result.errors, nullptr,
        &result.catalog.dismiss_runner_choice_ids);
    result.catalog.dismiss_runner_labels = std::move(dismiss_labels);
    std::array<std::string, 1> dismiss_separator_text{};
    std::array<std::string, 1> dismiss_separator_ids{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "dismiss_separator_text",
        "pocket_secretary.dismiss.text", "catalog",
        dismiss_separator_text, result.errors, nullptr,
        &dismiss_separator_ids);
    result.catalog.dismiss_runner_separator = dismiss_separator_text[0];
    result.catalog.dismiss_runner_separator_text_id =
        dismiss_separator_ids[0];
    std::array<std::string, kCyberdeckUnavailableLineCount>
        cyberdeck_unavailable_text{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "cyberdeck_unavailable_lines",
        "pocket_secretary.cyberdeck", "catalog",
        cyberdeck_unavailable_text, result.errors, nullptr,
        &result.catalog.pocket_secretary_cyberdeck_text_ids);
    result.catalog.pocket_secretary_cyberdeck_unavailable_lines =
        std::move(cyberdeck_unavailable_text);
    std::array<std::string, kPocketSecretaryGroupItemCount>
        group_item_names{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "group_item_names",
        "pocket_secretary.group_item", "catalog",
        group_item_names, result.errors,
        nullptr, &result.catalog.pocket_secretary_group_item_text_ids);
    result.catalog.pocket_secretary_group_item_names =
        std::move(group_item_names);
    std::array<std::string, kPocketSecretaryGroupItemsEmptyLineCount>
        group_items_empty_text{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "group_items_empty_lines",
        "pocket_secretary.group_items.empty", "catalog",
        group_items_empty_text, result.errors, nullptr,
        &result.catalog.pocket_secretary_group_items_empty_text_ids);
    result.catalog.pocket_secretary_group_items_empty_lines =
        std::move(group_items_empty_text);
    std::array<std::string, kNotebookNoRunLineCount> notebook_no_run_text{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "notebook_no_run_lines",
        "pocket_secretary.notebook.no_run", "catalog", notebook_no_run_text,
        result.errors, nullptr,
        &result.catalog.notebook_no_run_text_ids);
    result.catalog.notebook_no_run_lines = std::move(notebook_no_run_text);
    std::array<std::string, kNotebookMrJohnsonCount> mr_johnson_names{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "mr_johnson_contacts",
        "pocket_secretary.notebook.mr_johnson.name", "catalog",
        mr_johnson_names, result.errors, nullptr,
        &result.catalog.notebook_mr_johnson_name_text_ids);
    result.catalog.notebook_mr_johnson_names = std::move(mr_johnson_names);
    std::array<std::string, kNotebookMrJohnsonCount> mr_johnson_details{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "mr_johnson_details",
        "pocket_secretary.notebook.mr_johnson.detail", "catalog",
        mr_johnson_details, result.errors, nullptr,
        &result.catalog.notebook_mr_johnson_detail_text_ids);
    result.catalog.notebook_mr_johnson_details =
        std::move(mr_johnson_details);
    std::array<std::string, kNotebookTipsMetadataCount> notebook_tips_metadata{};
    load_script_menu_labels(
        scripts, "pocket_secretary", "notebook_tips_metadata",
        "pocket_secretary.notebook.tips", "metadata",
        notebook_tips_metadata, result.errors, nullptr,
        &result.catalog.notebook_tips_metadata_text_ids);
    result.catalog.notebook_tips_title = notebook_tips_metadata[0];
    result.catalog.notebook_tips_separator = notebook_tips_metadata[1];
    result.catalog.notebook_tips_page_count_label = notebook_tips_metadata[2];
    result.catalog.notebook_tips_page_number_clear =
        notebook_tips_metadata[3];

    constexpr std::array<std::string_view, kNotebookTipsPageCount>
        kNotebookTipsPageFunctions{{"notebook_tip_page_1",
                                    "notebook_tip_page_2",
                                    "notebook_tip_page_3"}};
    constexpr std::array<std::string_view, kNotebookTipsPageCount>
        kNotebookTipsPageMenuIds{{"page_1", "page_2", "page_3"}};
    std::array<std::string, kNotebookTipsPageSize> tip_page_text{};
    std::array<std::string, kNotebookTipsPageSize> tip_page_text_ids{};
    for (std::size_t page = 0; page < kNotebookTipsPageCount; ++page) {
        const auto first_tip = page * kNotebookTipsPageSize;
        load_script_menu_labels(
            scripts, "pocket_secretary",
            kNotebookTipsPageFunctions[page],
            "pocket_secretary.notebook.tips", kNotebookTipsPageMenuIds[page],
            tip_page_text, result.errors, nullptr,
            &tip_page_text_ids);
        std::copy(tip_page_text.begin(), tip_page_text.end(),
                  result.catalog.notebook_tips.begin() +
                      static_cast<std::ptrdiff_t>(first_tip));
        std::copy(tip_page_text_ids.begin(), tip_page_text_ids.end(),
                  result.catalog.notebook_tip_text_ids.begin() +
                      static_cast<std::ptrdiff_t>(first_tip));
    }
    for (std::size_t index = 0;
         index < result.catalog.notebook_tip_text_ids.size(); ++index) {
        const auto& tip_id = result.catalog.notebook_tip_text_ids[index];
        const auto previous_end = result.catalog.notebook_tip_text_ids.begin() +
                                  static_cast<std::ptrdiff_t>(index);
        if (tip_id.empty() ||
            std::find(result.catalog.notebook_tip_text_ids.begin(),
                      previous_end, tip_id) != previous_end) {
            result.errors.emplace_back(
                "Notebook Tips Lua pages contain an empty or duplicate ID");
        }
    }
    for (const auto& [key, value] : text_rows) {
        if (has_confirmation_menu_label_prefix(key)) {
            if (!is_confirmation_menu_label_key(
                    key, result.catalog.confirmation_choice_ids)) {
                result.errors.emplace_back(
                    "unknown confirmation menu label key: " + key);
            }
            continue;
        }
        if (has_field_metric_text_label_prefix(key)) {
            if (!is_field_metric_text_label_key(
                    key, result.catalog.field_metric_text_ids)) {
                result.errors.emplace_back(
                    "unknown field metric text label key: " + key);
            }
            continue;
        }
        if (has_command_overview_text_label_prefix(key)) {
            if (!is_command_overview_text_label_key(
                    key, result.catalog.race_text_ids,
                    result.catalog.class_text_ids,
                    result.catalog.status_text_ids)) {
                result.errors.emplace_back(
                    "unknown command overview text label key: " + key);
            }
            continue;
        }
        if (has_pocket_secretary_text_label_prefix(key)) {
            if (!is_pocket_secretary_text_label_key(
                    key, pocket_secretary_root_choice_ids,
                    pocket_secretary_root_text_ids, result.catalog)) {
                result.errors.emplace_back(
                    "unknown Pocket Secretary text label key: " + key);
            }
            continue;
        }
        if (has_clips_menu_label_prefix(key)) {
            if (!is_clips_menu_label_key(
                    key, clips_command_ids, clips_trade_command_ids,
                    result.catalog.clips_text_ids)) {
                result.errors.emplace_back(
                    "unknown Clips menu label key: " + key);
            }
            continue;
        }
        if (has_item_ui_text_label_prefix(key)) {
            if (!is_item_ui_text_label_key(
                    key, result.catalog.item_ui_text_ids)) {
                result.errors.emplace_back(
                    "unknown Items UI text label key: " + key);
            }
            continue;
        }
        if (has_attributes_skills_text_label_prefix(key)) {
            if (!is_attributes_skills_text_label_key(
                    key, result.catalog.attribute_skill_text_ids)) {
                result.errors.emplace_back(
                    "unknown Attributes/Skills UI text label key: " + key);
            }
            continue;
        }
        if (has_item_display_text_label_prefix(key)) {
            if (!is_item_display_text_label_key(
                    key, result.catalog.item_display_text_ids)) {
                result.errors.emplace_back(
                    "unknown item display text label key: " + key);
            }
            continue;
        }
        if (has_magic_panel_text_label_prefix(key)) {
            if (!is_magic_panel_text_label_key(
                    key, result.catalog.magic_panel_text_ids)) {
                result.errors.emplace_back(
                    "unknown Magic panel text label key: " + key);
            }
            continue;
        }
        if (has_cyberware_text_label_prefix(key)) {
            if (!is_cyberware_text_label_key(
                    key, result.catalog.cyberware_text_ids)) {
                result.errors.emplace_back(
                    "unknown Cyberware text label key: " + key);
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
        // The field-menu text catalog also loads the shared script-text.lua
        // table, but recruited-runner dialogue belongs to the dialogue panel
        // namespace and is validated by dialogue-panel QA.
        if (has_recruited_runner_dialogue_prefix(key)) {
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
        if (key.rfind(kMagicSpellOptionPrefix, 0U) == 0U) {
            const auto spell_id = std::string_view(key).substr(
                kMagicSpellOptionPrefix.size());
            if (!has_menu_choice_id(result.catalog.magic_spell_ids, spell_id)) {
                result.errors.emplace_back(
                    "unknown Magic spell option key: " + key);
            }
            continue;
        }
        if (key.rfind(kMagicSpellDescriptionPrefix, 0U) == 0U) {
            if (!magic_spell_index(
                    std::string_view(key).substr(
                        kMagicSpellDescriptionPrefix.size()),
                    index, result.catalog.magic_spell_ids)) {
                result.errors.emplace_back(
                    "unknown Magic spell description key: " + key);
                continue;
            }
            if (magic_spell_description_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                magic_spell_description_found[index] = true;
                if (value.empty()) {
                    result.errors.emplace_back(
                        "empty Magic spell description text: " + key);
                }
            }
            continue;
        }
        result.errors.emplace_back("unknown field menu text row: " + key);
    }
    if (text_rows.empty()) {
        result.errors.emplace_back("field menu text catalog is empty");
    }
    add_missing_entries(archetype_found, "archetype.", result.errors);
    add_missing_entries(runner_found, "runner.", result.errors);
    for (std::size_t index = 0; index < magic_spell_description_found.size();
         ++index) {
        if (!magic_spell_description_found[index]) {
            result.errors.emplace_back(
                "missing field menu text row: magic.spells.description." +
                result.catalog.magic_spell_ids[index]);
        }
    }
    add_missing_entries(field_service_title_destination_found,
                        "field-service.title-destination.", result.errors);
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
    for (auto& description : result.catalog.magic_spell_descriptions) {
        decode_newlines(description);
    }
    for (auto& detail : result.catalog.notebook_mr_johnson_details) {
        decode_newlines(detail);
    }
    for (auto& tip : result.catalog.notebook_tips) {
        decode_newlines(tip);
    }

    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

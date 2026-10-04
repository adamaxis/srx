#include "field_menu_text_catalog.hpp"

#include <charconv>
#include <fstream>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

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

} // namespace

FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets) {
    FieldMenuTextLoadResult result;
    const auto* record = assets.find("menu.field.text");
    if (record == nullptr) {
        result.errors.emplace_back("missing field menu text alias: menu.field.text");
        return result;
    }

    std::ifstream input(record->absolute_path);
    if (!input) {
        result.errors.emplace_back("cannot open field menu text: " +
                                  record->absolute_path.string());
        return result;
    }

    std::array<bool, kCommandOverviewLabelCount> command_found{};
    std::array<bool, kFieldMenuArchetypeNameCount> archetype_found{};
    std::array<bool, kFieldMenuRaceNameCount> race_found{};
    std::array<bool, kFieldMenuClassNameCount> class_found{};
    std::array<bool, kFieldMenuRunnerNameCount> runner_found{};
    std::array<bool, kFieldMenuStatusNameCount> status_found{};
    std::array<bool, kAttributeSkillLabelCount> attribute_skill_found{};
    std::array<bool, kCyberwareTitleLineCount> cyberware_title_found{};
    std::array<bool, kCyberwareEntryCount> cyberware_entry_found{};
    std::array<bool, kMagicSpellTextCount> magic_spell_name_found{};
    std::array<bool, kMagicSpellTextCount> magic_spell_description_found{};
    std::array<bool, kPocketSecretaryOptionCount>
        pocket_secretary_option_found{};
    std::array<bool, kFieldServiceTitleCount> field_service_title_found{};
    std::array<bool, kFieldServiceTitleCount>
        field_service_title_destination_found{};
    std::array<bool, kFieldServiceActionTitleCount>
        field_service_action_title_found{};
    bool field_service_universal_clips_title_found = false;
    bool field_service_universal_clips_description_found = false;
    std::array<bool, kCyberdeckUnavailableLineCount>
        pocket_secretary_cyberdeck_line_found{};
    std::array<bool, kPocketSecretaryGroupItemCount>
        pocket_secretary_group_item_found{};
    std::array<bool, kPocketSecretaryGroupItemsEmptyLineCount>
        pocket_secretary_group_items_empty_found{};
    std::array<bool, kNotebookLabelCount> notebook_label_found{};
    std::array<bool, kNotebookNoRunLineCount> notebook_no_run_found{};
    bool notebook_tips_title_found = false;
    bool notebook_tips_separator_found = false;
    bool notebook_tips_page_count_found = false;
    bool notebook_tips_page_number_clear_found = false;
    std::array<bool, kDismissRunnerLabelCount> dismiss_runner_label_found{};
    std::array<bool, kFieldMenuPowerLabelCount> power_found{};
    std::array<std::array<bool, kFieldMenuArmorRowCount>,
               kFieldMenuArmorCount>
        armor_found{};
    std::array<bool, kFieldMenuItemDetailLabelCount> item_detail_found{};
    std::array<bool, kFieldMenuItemNameCount> item_found{};
    std::array<bool, kFieldMenuItemNameCount> item_category_found{};
    std::array<bool, kFieldActionDetailTextCount>
        field_action_detail_text_found{};
    std::array<bool, kFieldMenuClipsCommandCount> clips_found{};
    std::array<bool, kFieldMenuClipsCommandCount> clips_trade_found{};
    std::array<bool, 3> clips_trade_prompt_found{};
    std::array<bool, 2> clips_exchange_complete_found{};
    std::array<bool, 3> clips_reload_found{};
    std::array<bool, 2> clips_success_found{};
    std::array<bool, 2> clips_out_of_ammo_found{};
    std::array<bool, 3> clips_item_no_clips_found{};
    std::array<bool, 3> clips_no_party_found{};
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
    bool action_item_equipped_prefix_found = false;
    bool action_item_equipped_suffix_found = false;
    bool action_discard_prompt_found = false;
    bool action_cannot_discard_found = false;
    bool action_cannot_trade_found = false;
    bool action_choice_no_found = false;
    bool action_choice_yes_found = false;
    bool shop_transaction_prompt_found = false;
    bool shop_no_inventory_slot_found = false;
    bool shop_insufficient_funds_found = false;
    bool shop_special_action_limit_found = false;
    bool shop_protected_sale_found = false;
    bool shop_sale_quote_heading_found = false;
    bool shop_purchase_price_heading_found = false;
    bool action_trade_recipient_found = false;
    bool action_trade_full_inventory_found = false;
    bool action_automatic_use_message_found = false;
    bool first_aid_prompt_found = false;
    bool first_aid_label_found = false;
    bool first_aid_cancel_found = false;
    bool first_aid_not_injured_found = false;
    bool first_aid_slap_patch_used_found = false;
    bool first_aid_medkit_used_found = false;
    bool clips_title_found = false;
    bool clips_separator_found = false;
    bool clips_counter_found = false;
    bool cyberware_empty_found = false;
    bool magic_defense_title_found = false;
    bool magic_force_posture_heading_found = false;
    bool magic_success_label_found = false;
    bool magic_force_label_found = false;
    bool magic_availability_label_found = false;
    bool pocket_secretary_title_found = false;
    bool pocket_secretary_separator_found = false;
    bool dismiss_runner_separator_found = false;
    std::string line;
    std::size_t line_number = 0;
    bool header_seen = false;
    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (!header_seen) {
            header_seen = true;
            if (line != "key\ttext") {
                result.errors.emplace_back(
                    "invalid field menu text header at line " +
                    std::to_string(line_number));
            }
            continue;
        }

        const auto separator = line.find('\t');
        if (separator == std::string::npos) {
            result.errors.emplace_back(
                "invalid field menu text row at line " +
                std::to_string(line_number));
            continue;
        }
        const auto key = line.substr(0, separator);
        const auto encoded_value = line.substr(separator + 1U);
        std::string value;
        if (encoded_value.size() >= 2U &&
            encoded_value.front() == '"' &&
            encoded_value.back() == '"') {
            value = encoded_value.substr(1U, encoded_value.size() - 2U);
        } else {
            value = encoded_value;
        }
        std::size_t index = 0;
        if (parse_indexed_key(key, "field-service.title.", index) &&
            index < result.catalog.field_service_titles.size()) {
            if (field_service_title_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                field_service_title_found[index] = true;
                result.catalog.field_service_titles[index] = value;
            }
            continue;
        }
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
        if (parse_indexed_key(key, "field-service.action-title.", index) &&
            index < result.catalog.field_service_action_titles.size()) {
            if (field_service_action_title_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                field_service_action_title_found[index] = true;
                result.catalog.field_service_action_titles[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "field-action.detail-text.", index) &&
            index < result.catalog.field_action_detail_texts.size()) {
            if (field_action_detail_text_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                field_action_detail_text_found[index] = true;
                result.catalog.field_action_detail_texts[index] = value;
            }
            continue;
        }
        if (key == "field-service.universal-clips.title") {
            if (field_service_universal_clips_title_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                field_service_universal_clips_title_found = true;
                result.catalog.field_service_universal_clips_title = value;
            }
            continue;
        }
        if (key == "field-service.universal-clips.description") {
            if (field_service_universal_clips_description_found) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                field_service_universal_clips_description_found = true;
                result.catalog.field_service_universal_clips_description =
                    value;
            }
            continue;
        }
        if (parse_indexed_key(key, "command.", index) &&
            index < result.catalog.command_labels.size()) {
            if (command_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                command_found[index] = true;
                result.catalog.command_labels[index] = value;
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
        if (parse_indexed_key(key, "attribute.skill.", index) &&
            index < result.catalog.attribute_skill_labels.size()) {
            if (attribute_skill_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                attribute_skill_found[index] = true;
                result.catalog.attribute_skill_labels[index] = value;
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
        if (parse_indexed_key(key, "clips.trade.", index) &&
            index < result.catalog.clips_trade_commands.size()) {
            if (clips_trade_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_trade_found[index] = true;
                result.catalog.clips_trade_commands[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.trade-prompt.", index) &&
            index < result.catalog.clips_trade_prompt.size()) {
            if (clips_trade_prompt_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_trade_prompt_found[index] = true;
                result.catalog.clips_trade_prompt[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.exchange-complete.", index) &&
            index < result.catalog.clips_exchange_complete.size()) {
            if (clips_exchange_complete_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_exchange_complete_found[index] = true;
                result.catalog.clips_exchange_complete[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.", index) &&
            index < result.catalog.clips_commands.size()) {
            if (clips_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_found[index] = true;
                result.catalog.clips_commands[index] = value;
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
            parse_named("action.item-equipped.prefix",
                        result.catalog.action_item_equipped_prefix,
                        action_item_equipped_prefix_found) ||
            parse_named("action.item-equipped.suffix",
                        result.catalog.action_item_equipped_suffix,
                        action_item_equipped_suffix_found) ||
            parse_named("action.discard-prompt",
                        result.catalog.action_discard_prompt,
                        action_discard_prompt_found) ||
            parse_named("action.cannot-discard",
                        result.catalog.action_cannot_discard,
                        action_cannot_discard_found) ||
            parse_named("action.cannot-trade",
                        result.catalog.action_cannot_trade,
                        action_cannot_trade_found) ||
            parse_named("action.choice-no", result.catalog.action_choice_no,
                        action_choice_no_found) ||
            parse_named("action.choice-yes", result.catalog.action_choice_yes,
                        action_choice_yes_found) ||
            parse_named("shop.transaction-prompt",
                        result.catalog.shop_transaction_prompt,
                        shop_transaction_prompt_found) ||
            parse_named("shop.no-inventory-slot",
                        result.catalog.shop_no_inventory_slot,
                        shop_no_inventory_slot_found) ||
            parse_named("shop.insufficient-funds",
                        result.catalog.shop_insufficient_funds,
                        shop_insufficient_funds_found) ||
            parse_named("shop.special-action-limit",
                        result.catalog.shop_special_action_limit,
                        shop_special_action_limit_found) ||
            parse_named("shop.protected-sale",
                        result.catalog.shop_protected_sale,
                        shop_protected_sale_found) ||
            parse_named("shop.sale-quote-heading",
                        result.catalog.shop_sale_quote_heading,
                        shop_sale_quote_heading_found) ||
            parse_named("shop.purchase-price-heading",
                        result.catalog.shop_purchase_price_heading,
                        shop_purchase_price_heading_found) ||
            parse_named("action.trade-recipient",
                        result.catalog.action_trade_recipient_label,
                        action_trade_recipient_found) ||
            parse_named("action.trade-full-inventory",
                        result.catalog.action_trade_full_inventory_message,
                        action_trade_full_inventory_found) ||
            parse_named("action.automatic-use.message",
                        result.catalog.action_automatic_use_message,
                        action_automatic_use_message_found) ||
            parse_named("first-aid.prompt",
                        result.catalog.first_aid_prompt,
                        first_aid_prompt_found) ||
            parse_named("first-aid.label",
                        result.catalog.first_aid_label,
                        first_aid_label_found) ||
            parse_named("first-aid.cancel",
                        result.catalog.first_aid_cancel,
                        first_aid_cancel_found) ||
            parse_named("first-aid.not-injured",
                        result.catalog.first_aid_not_injured,
                        first_aid_not_injured_found) ||
            parse_named("first-aid.slap-patch-used",
                        result.catalog.first_aid_slap_patch_used,
                        first_aid_slap_patch_used_found) ||
            parse_named("first-aid.medkit-used",
                        result.catalog.first_aid_medkit_used,
                        first_aid_medkit_used_found) ||
            parse_named("clips.title", result.catalog.clips_title,
                        clips_title_found) ||
            parse_named("clips.separator", result.catalog.clips_separator,
                        clips_separator_found) ||
            parse_named("clips.counter", result.catalog.clips_counter_label,
                        clips_counter_found) ||
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
            parse_named("pocket-secretary.title",
                        result.catalog.pocket_secretary_title,
                        pocket_secretary_title_found) ||
            parse_named("pocket-secretary.separator",
                        result.catalog.pocket_secretary_separator,
                        pocket_secretary_separator_found) ||
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
        if (parse_indexed_key(key, "pocket-secretary.option.", index) &&
            index < result.catalog.pocket_secretary_options.size()) {
            if (pocket_secretary_option_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                pocket_secretary_option_found[index] = true;
                result.catalog.pocket_secretary_options[index] = value;
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
        if (parse_indexed_key(key, "notebook.label.", index) &&
            index < result.catalog.notebook_labels.size()) {
            if (notebook_label_found[index]) {
                result.errors.emplace_back(
                    "duplicate field menu text row: " + key);
            } else {
                notebook_label_found[index] = true;
                result.catalog.notebook_labels[index] = value;
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
        if (parse_indexed_key(key, "magic.spell.name.", index) &&
            index < result.catalog.magic_spell_names.size()) {
            if (magic_spell_name_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                magic_spell_name_found[index] = true;
                result.catalog.magic_spell_names[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "magic.spell.description.", index) &&
            index < result.catalog.magic_spell_descriptions.size()) {
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
        if (parse_indexed_key(key, "clips.reload.", index) &&
            index < result.catalog.clips_reload_prompt.size()) {
            if (clips_reload_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_reload_found[index] = true;
                result.catalog.clips_reload_prompt[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.success.", index) &&
            index < result.catalog.clips_reload_success.size()) {
            if (clips_success_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_success_found[index] = true;
                result.catalog.clips_reload_success[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.out-of-ammo.", index) &&
            index < result.catalog.clips_out_of_ammo.size()) {
            if (clips_out_of_ammo_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_out_of_ammo_found[index] = true;
                result.catalog.clips_out_of_ammo[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.item-no-clips.", index) &&
            index < result.catalog.clips_item_no_clips.size()) {
            if (clips_item_no_clips_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_item_no_clips_found[index] = true;
                result.catalog.clips_item_no_clips[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "clips.no-party.", index) &&
            index < result.catalog.clips_no_party.size()) {
            if (clips_no_party_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                clips_no_party_found[index] = true;
                result.catalog.clips_no_party[index] = value;
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
        if (parse_indexed_key(key, "item-detail.", index) &&
            index < result.catalog.item_detail_labels.size()) {
            if (item_detail_found[index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                item_detail_found[index] = true;
                result.catalog.item_detail_labels[index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "item.", index) && index >= 1U &&
            index <= result.catalog.item_names.size()) {
            const auto item_index = index - 1U;
            if (item_found[item_index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                item_found[item_index] = true;
                result.catalog.item_names[item_index] = value;
            }
            continue;
        }
        if (parse_indexed_key(key, "item-category.", index) && index >= 1U &&
            index <= result.catalog.item_categories.size()) {
            const auto item_index = index - 1U;
            if (item_category_found[item_index]) {
                result.errors.emplace_back("duplicate field menu text row: " +
                                          key);
            } else {
                item_category_found[item_index] = true;
                result.catalog.item_categories[item_index] = value;
            }
            continue;
        }
        result.errors.emplace_back("unknown field menu text row: " + key);
    }
    if (!header_seen) {
        result.errors.emplace_back("field menu text catalog is empty");
    }
    add_missing_entries(command_found, "command.", result.errors);
    add_missing_entries(archetype_found, "archetype.", result.errors);
    add_missing_entries(race_found, "race.", result.errors);
    add_missing_entries(class_found, "class.", result.errors);
    add_missing_entries(runner_found, "runner.", result.errors);
    add_missing_entries(status_found, "status.", result.errors);
    add_missing_entries(attribute_skill_found, "attribute.skill.",
                        result.errors);
    add_missing_entries(cyberware_title_found, "cyberware.title.",
                        result.errors);
    add_missing_entries(cyberware_entry_found, "cyberware.entry.",
                        result.errors);
    add_missing_entries(clips_found, "clips.", result.errors);
    add_missing_entries(clips_trade_found, "clips.trade.", result.errors);
    add_missing_entries(clips_trade_prompt_found, "clips.trade-prompt.",
                        result.errors);
    add_missing_entries(clips_exchange_complete_found,
                        "clips.exchange-complete.", result.errors);
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
    require_named("action.item-equipped.prefix",
                  action_item_equipped_prefix_found);
    require_named("action.item-equipped.suffix",
                  action_item_equipped_suffix_found);
    require_named("action.discard-prompt", action_discard_prompt_found);
    require_named("action.cannot-discard", action_cannot_discard_found);
    require_named("action.cannot-trade", action_cannot_trade_found);
    require_named("action.choice-no", action_choice_no_found);
    require_named("action.choice-yes", action_choice_yes_found);
    require_named("shop.transaction-prompt", shop_transaction_prompt_found);
    require_named("shop.no-inventory-slot", shop_no_inventory_slot_found);
    require_named("shop.insufficient-funds", shop_insufficient_funds_found);
    require_named("shop.special-action-limit",
                  shop_special_action_limit_found);
    require_named("shop.protected-sale", shop_protected_sale_found);
    require_named("shop.sale-quote-heading", shop_sale_quote_heading_found);
    require_named("shop.purchase-price-heading",
                  shop_purchase_price_heading_found);
    require_named("action.trade-recipient", action_trade_recipient_found);
    require_named("action.trade-full-inventory",
                  action_trade_full_inventory_found);
    require_named("action.automatic-use.message",
                  action_automatic_use_message_found);
    require_named("first-aid.prompt", first_aid_prompt_found);
    require_named("first-aid.label", first_aid_label_found);
    require_named("first-aid.cancel", first_aid_cancel_found);
    require_named("first-aid.not-injured", first_aid_not_injured_found);
    require_named("first-aid.slap-patch-used",
                  first_aid_slap_patch_used_found);
    require_named("first-aid.medkit-used", first_aid_medkit_used_found);
    require_named("clips.title", clips_title_found);
    require_named("clips.separator", clips_separator_found);
    require_named("clips.counter", clips_counter_found);
    require_named("cyberware.empty", cyberware_empty_found);
    require_named("magic.defense.title", magic_defense_title_found);
    require_named("magic.heading.force-posture",
                  magic_force_posture_heading_found);
    require_named("magic.label.success", magic_success_label_found);
    require_named("magic.label.force", magic_force_label_found);
    require_named("magic.label.availability",
                  magic_availability_label_found);
    require_named("pocket-secretary.title", pocket_secretary_title_found);
    require_named("pocket-secretary.separator",
                  pocket_secretary_separator_found);
    require_named("dismiss.separator", dismiss_runner_separator_found);
    require_named("notebook.tips.title", notebook_tips_title_found);
    require_named("notebook.tips.separator", notebook_tips_separator_found);
    require_named("notebook.tips.page-count",
                  notebook_tips_page_count_found);
    require_named("notebook.tips.page-number-clear",
                  notebook_tips_page_number_clear_found);
    add_missing_entries(clips_reload_found, "clips.reload.", result.errors);
    add_missing_entries(clips_success_found, "clips.success.", result.errors);
    add_missing_entries(clips_out_of_ammo_found, "clips.out-of-ammo.",
                       result.errors);
    add_missing_entries(clips_item_no_clips_found, "clips.item-no-clips.",
                       result.errors);
    add_missing_entries(clips_no_party_found, "clips.no-party.",
                       result.errors);
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
    add_missing_entries(item_detail_found, "item-detail.", result.errors);
    add_missing_entries(item_found, "item.", result.errors);
    add_missing_entries(item_category_found, "item-category.", result.errors);
    add_missing_entries(magic_spell_name_found, "magic.spell.name.",
                        result.errors);
    add_missing_entries(magic_spell_description_found,
                        "magic.spell.description.", result.errors);
    add_missing_entries(pocket_secretary_option_found,
                        "pocket-secretary.option.", result.errors);
    add_missing_entries(pocket_secretary_cyberdeck_line_found,
                        "pocket-secretary.cyberdeck.line.", result.errors);
    add_missing_entries(pocket_secretary_group_item_found,
                        "pocket-secretary.group-item.", result.errors);
    add_missing_entries(pocket_secretary_group_items_empty_found,
                        "pocket-secretary.group-items.empty.", result.errors);
    add_missing_entries(notebook_label_found, "notebook.label.", result.errors);
    add_missing_entries(notebook_no_run_found, "notebook.no-run.",
                        result.errors);
    add_missing_entries(field_service_title_found, "field-service.title.",
                        result.errors);
    add_missing_entries(field_service_title_destination_found,
                        "field-service.title-destination.", result.errors);
    add_missing_entries(field_service_action_title_found,
                        "field-service.action-title.", result.errors);
    add_missing_entries(field_action_detail_text_found,
                        "field-action.detail-text.", result.errors);
    require_named("field-service.universal-clips.title",
                  field_service_universal_clips_title_found);
    require_named("field-service.universal-clips.description",
                  field_service_universal_clips_description_found);
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
    decode_newlines(result.catalog.shop_no_inventory_slot);
    decode_newlines(result.catalog.shop_insufficient_funds);
    decode_newlines(result.catalog.shop_special_action_limit);
    for (auto& detail_text : result.catalog.field_action_detail_texts) {
        decode_newlines(detail_text);
    }
    decode_newlines(result.catalog.field_service_universal_clips_title);
    decode_newlines(
        result.catalog.field_service_universal_clips_description);

    const auto* tips_record =
        assets.find("menu.pocket_secretary.notebook.tips");
    if (tips_record == nullptr) {
        result.errors.emplace_back(
            "missing Notebook Tips text alias: "
            "menu.pocket_secretary.notebook.tips");
    } else {
        std::ifstream tips_input(tips_record->absolute_path);
        if (!tips_input) {
            result.errors.emplace_back(
                "cannot open Notebook Tips text: " +
                tips_record->absolute_path.string());
        } else {
            std::array<bool, kNotebookTipCount> tip_found{};
            std::string tip_line;
            std::size_t tip_line_number = 0U;
            bool tip_header_seen = false;
            while (std::getline(tips_input, tip_line)) {
                ++tip_line_number;
                if (!tip_line.empty() && tip_line.back() == '\r') {
                    tip_line.pop_back();
                }
                if (tip_line.empty()) {
                    continue;
                }
                if (!tip_header_seen) {
                    tip_header_seen = true;
                    if (tip_line != "tip_index\ttext") {
                        result.errors.emplace_back(
                            "invalid Notebook Tips header at line " +
                            std::to_string(tip_line_number));
                    }
                    continue;
                }
                const auto separator = tip_line.find('\t');
                if (separator == std::string::npos) {
                    result.errors.emplace_back(
                        "invalid Notebook Tips row at line " +
                        std::to_string(tip_line_number));
                    continue;
                }
                const auto index_text = tip_line.substr(0U, separator);
                std::size_t tip_index = 0U;
                const auto* first = index_text.data();
                const auto* last = first + index_text.size();
                const auto parsed =
                    std::from_chars(first, last, tip_index);
                if (parsed.ec != std::errc{} || parsed.ptr != last ||
                    tip_index >= tip_found.size()) {
                    result.errors.emplace_back(
                        "invalid Notebook Tips index at line " +
                        std::to_string(tip_line_number));
                    continue;
                }
                if (tip_found[tip_index]) {
                    result.errors.emplace_back(
                        "duplicate Notebook Tips entry: " + index_text);
                    continue;
                }
                tip_found[tip_index] = true;
                auto value = tip_line.substr(separator + 1U);
                if (value.size() >= 2U && value.front() == '"' &&
                    value.back() == '"') {
                    value = value.substr(1U, value.size() - 2U);
                }
                std::size_t position = 0U;
                while ((position = value.find("\\n", position)) !=
                       std::string::npos) {
                    value.replace(position, 2U, "\n");
                    ++position;
                }
                result.catalog.notebook_tips[tip_index] = std::move(value);
            }
            if (!tip_header_seen) {
                result.errors.emplace_back("Notebook Tips text asset is empty");
            }
            add_missing_entries(tip_found, "notebook.tip.", result.errors);
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

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
    std::array<bool, kFieldMenuRunnerNameCount> runner_found{};
    std::array<bool, kFieldMenuStatusNameCount> status_found{};
    std::array<bool, kFieldMenuPowerLabelCount> power_found{};
    std::array<std::array<bool, kFieldMenuArmorRowCount>,
               kFieldMenuArmorCount>
        armor_found{};
    std::array<bool, kFieldMenuItemDetailLabelCount> item_detail_found{};
    std::array<bool, kFieldMenuItemNameCount> item_found{};
    std::array<bool, kFieldMenuItemNameCount> item_category_found{};
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
                        power_metric_found)) {
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
    add_missing_entries(runner_found, "runner.", result.errors);
    add_missing_entries(status_found, "status.", result.errors);
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
    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

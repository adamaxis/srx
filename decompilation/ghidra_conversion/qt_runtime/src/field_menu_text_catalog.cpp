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
    std::array<bool, kFieldMenuItemNameCount> item_found{};
    std::array<bool, kFieldMenuItemNameCount> item_category_found{};
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
        const auto value = encoded_value == "\"\""
                               ? std::string{}
                               : encoded_value;
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
    add_missing_entries(item_found, "item.", result.errors);
    add_missing_entries(item_category_found, "item-category.", result.errors);
    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

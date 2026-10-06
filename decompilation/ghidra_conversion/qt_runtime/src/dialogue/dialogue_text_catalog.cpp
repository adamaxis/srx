#include "dialogue_text_catalog.hpp"

#include <fstream>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

std::string unquote(std::string value) {
    if (value.size() >= 2U && value.front() == '"' && value.back() == '"') {
        value = value.substr(1U, value.size() - 2U);
    }
    return value;
}

} // namespace

const std::string* DialogueTextCatalog::find(const std::string_view key) const {
    const auto found = entries.find(std::string(key));
    return found == entries.end() ? nullptr : &found->second;
}

DialogueTextLoadResult load_dialogue_text_catalog(
    const ConversionAssetCatalog& assets,
    const std::string_view resource_alias) {
    DialogueTextLoadResult result;
    const auto* record = assets.find(resource_alias);
    if (record == nullptr) {
        result.errors.emplace_back("missing dialogue text alias: " +
                                  std::string(resource_alias));
        return result;
    }
    if (record->kind != "text") {
        result.errors.emplace_back("dialogue alias is not a text resource: " +
                                  std::string(resource_alias));
        return result;
    }

    std::ifstream input(record->absolute_path, std::ios::binary);
    if (!input) {
        result.errors.emplace_back("cannot open dialogue text: " +
                                  record->absolute_path.string());
        return result;
    }
    std::string line;
    std::size_t line_number = 0U;
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
                    "invalid dialogue text header at line " +
                    std::to_string(line_number));
            }
            continue;
        }
        const auto separator = line.find('\t');
        if (separator == std::string::npos || separator == 0U) {
            result.errors.emplace_back(
                "invalid dialogue text row at line " +
                std::to_string(line_number));
            continue;
        }
        auto key = line.substr(0U, separator);
        auto value = unquote(line.substr(separator + 1U));
        if (value.empty()) {
            result.errors.emplace_back("empty dialogue text row: " + key);
            continue;
        }
        if (!result.catalog.entries.emplace(std::move(key), std::move(value))
                 .second) {
            result.errors.emplace_back("duplicate dialogue text row at line " +
                                      std::to_string(line_number));
        }
    }
    if (!header_seen) {
        result.errors.emplace_back("dialogue text catalog is empty");
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

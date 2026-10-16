#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <charconv>
#include <fstream>
#include <system_error>

namespace shadowrun::ghidra_conversion {
namespace {

std::vector<std::string> split_tsv(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (start <= line.size()) {
        const auto separator = line.find('\t', start);
        const auto end = separator == std::string::npos ? line.size()
                                                         : separator;
        fields.emplace_back(line.substr(start, end - start));
        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1U;
    }
    return fields;
}

bool parse_integer(const std::string& text, std::int32_t& value) {
    const auto* first = text.data();
    const auto* last = first + text.size();
    const auto parsed = std::from_chars(first, last, value);
    return parsed.ec == std::errc{} && parsed.ptr == last;
}

bool path_stays_under_root(const std::filesystem::path& relative) {
    if (relative.empty() || relative.is_absolute()) {
        return false;
    }
    for (const auto& component : relative) {
        if (component == "..") {
            return false;
        }
    }
    return true;
}

} // namespace

const ConversionAssetRecord* ConversionAssetCatalog::find(
    const std::string_view alias) const {
    for (const auto& record : records) {
        if (record.alias == alias) {
            return &record;
        }
    }
    return nullptr;
}

AssetCatalogLoadResult load_conversion_asset_catalog(
    const std::filesystem::path& asset_root) {
    AssetCatalogLoadResult result;
    result.catalog.asset_root = asset_root;

    const auto manifest_path = asset_root / "resources.tsv";
    std::ifstream manifest(manifest_path);
    if (!manifest) {
        result.errors.emplace_back("cannot open " + manifest_path.string());
        return result;
    }

    std::string line;
    std::size_t line_number = 0;
    bool header_seen = false;
    while (std::getline(manifest, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line.front() == '#') {
            continue;
        }

        const auto fields = split_tsv(line);
        if (!header_seen) {
            header_seen = true;
            if (fields.size() != 5U || fields[0] != "alias" ||
                fields[1] != "path" || fields[2] != "kind") {
                result.errors.emplace_back("invalid resources.tsv header at line " +
                                           std::to_string(line_number));
            }
            continue;
        }

        if (fields.size() != 5U) {
            result.errors.emplace_back("invalid resources.tsv row at line " +
                                       std::to_string(line_number));
            continue;
        }

        ConversionAssetRecord record;
        record.alias = fields[0];
        record.relative_path = fields[1];
        record.kind = fields[2];
        if (record.alias.empty() ||
            result.catalog.find(record.alias) != nullptr) {
            result.errors.emplace_back("duplicate or empty asset alias at line " +
                                       std::to_string(line_number));
            continue;
        }
        if (!parse_integer(fields[3], record.origin_x) ||
            !parse_integer(fields[4], record.origin_y)) {
            result.errors.emplace_back("invalid asset origin at line " +
                                       std::to_string(line_number));
            continue;
        }

        const std::filesystem::path relative(record.relative_path);
        if (!path_stays_under_root(relative)) {
            result.errors.emplace_back("asset path escapes root at line " +
                                       std::to_string(line_number));
            continue;
        }

        record.absolute_path = asset_root / relative;
        std::error_code file_error;
        if (!std::filesystem::is_regular_file(record.absolute_path,
                                              file_error)) {
            result.errors.emplace_back("missing asset: " + record.alias +
                                       " -> " + record.relative_path);
        } else {
            record.byte_size = std::filesystem::file_size(record.absolute_path,
                                                           file_error);
            if (file_error) {
                result.errors.emplace_back("cannot stat asset: " +
                                           record.alias);
            }
        }
        result.catalog.records.push_back(std::move(record));
    }

    if (!header_seen) {
        result.errors.emplace_back("resources.tsv is empty");
    }
    return result;
}

AssetSetReport verify_conversion_asset_set(
    const ConversionAssetCatalog& catalog,
    const std::vector<std::string>& required_aliases) {
    AssetSetReport result;
    result.required_aliases = required_aliases;
    for (const auto& alias : required_aliases) {
        const auto* record = catalog.find(alias);
        if (record == nullptr) {
            result.missing_aliases.push_back(alias);
            continue;
        }
        result.total_bytes += record->byte_size;
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion

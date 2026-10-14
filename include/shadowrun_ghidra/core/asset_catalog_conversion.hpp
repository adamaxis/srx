#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

// One row from assets/resources.tsv. The conversion project consumes the
// already extracted host file; it never decodes a ROM resource at runtime.
struct ConversionAssetRecord {
    std::string alias;
    std::string relative_path;
    std::string kind;
    std::int32_t origin_x = 0;
    std::int32_t origin_y = 0;
    std::filesystem::path absolute_path;
    std::uintmax_t byte_size = 0;
};

struct ConversionAssetCatalog {
    std::filesystem::path asset_root;
    std::vector<ConversionAssetRecord> records;

    const ConversionAssetRecord* find(std::string_view alias) const;
};

struct AssetCatalogLoadResult {
    ConversionAssetCatalog catalog;
    std::vector<std::string> errors;

    bool complete() const { return errors.empty() && !catalog.records.empty(); }
};

// Read the direct-asset alias table from <asset_root>/resources.tsv. Rows are
// rejected when they escape the asset root, duplicate an alias, or point to a
// missing file. The catalog is still returned so callers can print every
// problem in one pass instead of failing on the first row.
AssetCatalogLoadResult load_conversion_asset_catalog(
    const std::filesystem::path& asset_root);

struct AssetSetReport {
    std::vector<std::string> required_aliases;
    std::vector<std::string> missing_aliases;
    std::uintmax_t total_bytes = 0;

    bool complete() const { return missing_aliases.empty(); }
};

// Verify one semantic asset set by alias. This is used by the conversion boot
// path to select only the resources needed to reach the title/startup system;
// unrelated extracted assets are not pulled into the startup path.
AssetSetReport verify_conversion_asset_set(
    const ConversionAssetCatalog& catalog,
    const std::vector<std::string>& required_aliases);

} // namespace shadowrun::ghidra_conversion

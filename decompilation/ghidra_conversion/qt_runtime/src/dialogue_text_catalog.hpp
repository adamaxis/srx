#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

struct DialogueTextCatalog {
    std::unordered_map<std::string, std::string> entries;

    const std::string* find(std::string_view key) const;
};

struct DialogueTextLoadResult {
    DialogueTextCatalog catalog;
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Load one direct key/text dialogue resource. The values retain the source
// segment separator ('|') emitted by the offline extractor; the panel adapter
// turns it into its typed segment array.
DialogueTextLoadResult load_dialogue_text_catalog(
    const ConversionAssetCatalog& assets,
    std::string_view resource_alias);

} // namespace shadowrun::ghidra_conversion::qt_runtime

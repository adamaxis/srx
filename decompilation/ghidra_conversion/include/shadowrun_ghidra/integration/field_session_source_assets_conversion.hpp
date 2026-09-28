#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

constexpr std::size_t kFieldSessionActionProfileByteCount = 5U * 0x22U;
constexpr std::size_t kFieldSessionModeSelectionWordCount = 0x17U;

// Direct source records extracted from the US ROM. These are host-owned
// inputs for the existing field-session conversion, not a runtime ROM view.
struct FieldSessionSourceAssets {
    // ROM 0x166AD8, copied by 0x566A2.
    std::vector<std::uint8_t> action_selection_profile_source;
    // ROM 0x14C1F4, copied by 0x26938.
    std::vector<std::uint16_t> field_mode_selection_state_source;
};

struct FieldSessionSourceAssetLoadResult {
    std::optional<FieldSessionSourceAssets> assets;
    std::vector<std::string> errors;

    bool complete() const { return assets.has_value() && errors.empty(); }
};

// Reads the two named records from resources.tsv and decodes only the
// big-endian words required by FieldSessionBootstrapInput. Missing, malformed,
// or wrong-sized records are reported; no fallback data is synthesized.
FieldSessionSourceAssetLoadResult load_field_session_source_assets(
    const ConversionAssetCatalog& catalog);

} // namespace shadowrun::ghidra_conversion::integration

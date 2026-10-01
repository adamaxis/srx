#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// These counts are the selector/data sets used by the source command
// overview. The text file is generated offline from the ROM; the Qt runtime
// receives ordinary host strings and never follows a ROM pointer table.
inline constexpr std::size_t kCommandOverviewLabelCount = 7U;
inline constexpr std::size_t kFieldMenuArchetypeNameCount = 4U;
inline constexpr std::size_t kFieldMenuRunnerNameCount = 3U;
inline constexpr std::size_t kFieldMenuStatusNameCount = 5U;
inline constexpr std::size_t kFieldMenuItemNameCount = 50U;

struct FieldMenuTextCatalog {
    std::array<std::string, kCommandOverviewLabelCount> command_labels{};
    std::array<std::string, kFieldMenuArchetypeNameCount> archetype_names{};
    std::array<std::string, kFieldMenuRunnerNameCount> runner_names{};
    std::array<std::string, kFieldMenuStatusNameCount> status_names{};
    std::array<std::string, kFieldMenuItemNameCount> item_names{};
    std::array<std::string, kFieldMenuItemNameCount> item_categories{};
};

struct FieldMenuTextLoadResult {
    FieldMenuTextCatalog catalog{};
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Load the generated menu/field/text.tsv resource through its manifest alias.
// This is intentionally a small table loader, not a fallback label provider:
// an absent or incomplete source catalog remains a visible conversion gap.
FieldMenuTextLoadResult load_field_menu_text_catalog(
    const ConversionAssetCatalog& assets);

} // namespace shadowrun::ghidra_conversion::qt_runtime

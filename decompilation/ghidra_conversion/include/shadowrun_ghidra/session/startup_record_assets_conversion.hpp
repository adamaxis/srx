#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_template_conversion.hpp"
#include "shadowrun_ghidra/session/runner_roster_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

inline constexpr std::string_view kJoshuaArchetypeAssetAlias =
    "records.startup.joshua.archetypes";
inline constexpr std::string_view kRecruitedRunnerAssetAlias =
    "records.startup.recruited.runners";
inline constexpr std::string_view kSaveRosterHeaderAssetAlias =
    "records.startup.save.roster.header";

constexpr std::size_t kJoshuaArchetypeCount = 3U;
constexpr std::size_t kRecruitedRunnerRecordCount = 12U;

// The source startup path consumes three complete records at 0x7291A and
// twelve 0x100-byte records from 0x72C1A. The files are deliberately raw
// record assets: the extraction tool preserves the source record bytes, while
// the conversion code gives each consumer an ordinary fixed-size host value.
struct StartupRecordAssets {
    std::array<JoshuaArchetypeTemplate, kJoshuaArchetypeCount>
        joshua_archetypes{};
    std::array<RunnerRosterTemplate, kSaveRosterTemplateCount>
        save_roster_templates{};
    std::array<std::uint8_t, 0x14U> save_roster_header{};
};

struct StartupRecordAssetLoadResult {
    std::optional<StartupRecordAssets> assets;
    std::vector<std::string> errors;

    bool complete() const { return assets.has_value() && errors.empty(); }
};

// Load the source records needed by 0x42B0 and 0x59318 from named direct
// assets. This is an offline/conversion-side asset boundary; it does not read
// ROM addresses or depend on the native-port state serializer.
StartupRecordAssetLoadResult load_startup_record_assets(
    const ConversionAssetCatalog& catalog);

} // namespace shadowrun::ghidra_conversion

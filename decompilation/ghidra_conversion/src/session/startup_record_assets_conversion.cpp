#include "shadowrun_ghidra/session/startup_record_assets_conversion.hpp"

#include <fstream>
#include <utility>

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Record, std::size_t Count>
bool read_record_table(const ConversionAssetCatalog& catalog,
                       const std::string_view alias,
                       std::array<Record, Count>& destination,
                       std::vector<std::string>& errors) {
    const auto* asset = catalog.find(alias);
    if (asset == nullptr) {
        errors.emplace_back("missing startup record asset alias: " +
                           std::string(alias));
        return false;
    }

    constexpr auto record_size = sizeof(destination.front().record);
    constexpr auto expected_size = Count * record_size;
    if (asset->kind != "state" || asset->byte_size != expected_size) {
        errors.emplace_back("invalid startup record asset size or kind: " +
                           std::string(alias));
        return false;
    }

    std::ifstream input(asset->absolute_path, std::ios::binary);
    if (!input) {
        errors.emplace_back("cannot open startup record asset: " +
                           asset->absolute_path.string());
        return false;
    }
    for (auto& record : destination) {
        input.read(reinterpret_cast<char*>(record.record.data()),
                   static_cast<std::streamsize>(record.record.size()));
        if (!input) {
            errors.emplace_back("truncated startup record asset: " +
                               asset->absolute_path.string());
            return false;
        }
    }
    return true;
}

bool read_header(const ConversionAssetCatalog& catalog,
                 std::array<std::uint8_t, 0x14U>& destination,
                 std::vector<std::string>& errors) {
    const auto* asset = catalog.find(kSaveRosterHeaderAssetAlias);
    if (asset == nullptr) {
        errors.emplace_back("missing startup record asset alias: " +
                           std::string(kSaveRosterHeaderAssetAlias));
        return false;
    }
    if (asset->kind != "state" || asset->byte_size != destination.size()) {
        errors.emplace_back("invalid startup record asset size or kind: " +
                           std::string(kSaveRosterHeaderAssetAlias));
        return false;
    }

    std::ifstream input(asset->absolute_path, std::ios::binary);
    if (!input) {
        errors.emplace_back("cannot open startup record asset: " +
                           asset->absolute_path.string());
        return false;
    }
    input.read(reinterpret_cast<char*>(destination.data()),
               static_cast<std::streamsize>(destination.size()));
    if (!input) {
        errors.emplace_back("truncated startup record asset: " +
                           asset->absolute_path.string());
        return false;
    }
    return true;
}

} // namespace

StartupRecordAssetLoadResult load_startup_record_assets(
    const ConversionAssetCatalog& catalog) {
    StartupRecordAssetLoadResult result;
    StartupRecordAssets assets{};

    const bool archetypes_loaded = read_record_table(
        catalog, kJoshuaArchetypeAssetAlias, assets.joshua_archetypes,
        result.errors);

    std::array<RunnerRosterTemplate, kRecruitedRunnerRecordCount>
        recruited_records{};
    const bool recruited_loaded = read_record_table(
        catalog, kRecruitedRunnerAssetAlias, recruited_records, result.errors);
    const bool header_loaded =
        read_header(catalog, assets.save_roster_header, result.errors);

    if (!archetypes_loaded || !recruited_loaded || !header_loaded) {
        return result;
    }

    // ROM 0x59318 seeds compact save record zero from Joshua's first
    // archetype, then projects the twelve records beginning at 0x72C1A.
    assets.save_roster_templates[0].record =
        assets.joshua_archetypes[0].record;
    for (std::size_t index = 0; index < recruited_records.size(); ++index) {
        assets.save_roster_templates[index + 1U].record =
            recruited_records[index].record;
    }
    result.assets = std::move(assets);
    return result;
}

} // namespace shadowrun::ghidra_conversion

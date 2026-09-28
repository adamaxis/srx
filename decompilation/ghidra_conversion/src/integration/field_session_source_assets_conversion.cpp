#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <fstream>
#include <utility>

namespace shadowrun::ghidra_conversion::integration {
namespace {

constexpr const char* kActionProfilesAlias =
    "records.field.action.selection.profiles";
constexpr const char* kModeSelectionAlias =
    "records.field.mode.selection.state";

std::optional<std::vector<std::uint8_t>> read_record_bytes(
    const ConversionAssetCatalog& catalog, const char* alias,
    const std::size_t expected_size, std::vector<std::string>& errors) {
    const auto* record = catalog.find(alias);
    if (record == nullptr) {
        errors.emplace_back(std::string("missing field-session source record: ") +
                            alias);
        return std::nullopt;
    }
    if (record->kind != "state") {
        errors.emplace_back(std::string("field-session source record is not state data: ") +
                            alias);
        return std::nullopt;
    }
    if (record->byte_size != expected_size) {
        errors.emplace_back(
            std::string("field-session source record has wrong size: ") + alias);
        return std::nullopt;
    }

    std::ifstream input(record->absolute_path, std::ios::binary);
    if (!input) {
        errors.emplace_back(std::string("cannot open field-session source record: ") +
                            alias);
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes(expected_size);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (!input || input.gcount() != static_cast<std::streamsize>(bytes.size())) {
        errors.emplace_back(std::string("cannot read field-session source record: ") +
                            alias);
        return std::nullopt;
    }
    return bytes;
}

std::uint16_t read_big_endian_word(const std::vector<std::uint8_t>& bytes,
                                   const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        static_cast<std::uint16_t>(bytes[offset + 1U]));
}

} // namespace

FieldSessionSourceAssetLoadResult load_field_session_source_assets(
    const ConversionAssetCatalog& catalog) {
    FieldSessionSourceAssetLoadResult result;
    const auto action_profiles = read_record_bytes(
        catalog, kActionProfilesAlias, kFieldSessionActionProfileByteCount,
        result.errors);
    const auto mode_selection = read_record_bytes(
        catalog, kModeSelectionAlias,
        kFieldSessionModeSelectionWordCount * sizeof(std::uint16_t),
        result.errors);
    if (!action_profiles.has_value() || !mode_selection.has_value()) {
        return result;
    }

    FieldSessionSourceAssets assets;
    assets.action_selection_profile_source = *action_profiles;
    assets.field_mode_selection_state_source.reserve(
        kFieldSessionModeSelectionWordCount);
    for (std::size_t offset = 0; offset < mode_selection->size();
         offset += sizeof(std::uint16_t)) {
        assets.field_mode_selection_state_source.push_back(
            read_big_endian_word(*mode_selection, offset));
    }
    result.assets = std::move(assets);
    return result;
}

} // namespace shadowrun::ghidra_conversion::integration

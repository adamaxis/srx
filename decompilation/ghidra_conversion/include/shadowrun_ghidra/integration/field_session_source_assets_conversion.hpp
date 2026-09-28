#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

constexpr std::size_t kFieldSessionActionProfileByteCount = 5U * 0x22U;
constexpr std::size_t kFieldSessionModeSelectionWordCount = 0x17U;
constexpr std::size_t kFieldSceneProfileWordCount = 48U;
constexpr std::size_t kFieldSceneProfileByteCount =
    kFieldSceneProfileWordCount * sizeof(std::uint16_t);
constexpr std::size_t kFieldSessionTerrainLayerWordCount = 0xE10U;
constexpr std::size_t kFieldSessionTerrainLayerByteCount =
    kFieldSessionTerrainLayerWordCount * sizeof(std::uint16_t);
constexpr std::size_t kFieldSessionTerrainStreamByteCount = 0x3500U;

// Direct source records extracted from the US ROM. These are host-owned
// inputs for the existing field-session conversion, not a runtime ROM view.
struct FieldSessionSourceAssets {
    // ROM 0x166AD8, copied by 0x566A2.
    std::vector<std::uint8_t> action_selection_profile_source;
    // ROM 0x14C1F4, copied by 0x26938.
    std::vector<std::uint16_t> field_mode_selection_state_source;
    // ROM 0x0D5456, first pointer-table profile at 0x3232.
    std::array<std::uint16_t, kFieldSceneProfileWordCount>
        normal_scene_profile_source{};
    // ROM 0x17D5C4, second pointer-table profile at 0x3232.
    std::array<std::uint16_t, kFieldSceneProfileWordCount>
        alternate_scene_profile_source{};
    // ROM 0x186EDC/0x187CEC, copied by 0x0313A as the initial mode-2
    // terrain work layers. The flagged 0x188AFD source stream is represented
    // separately below after its 0x01610 source-history contract is verified.
    std::vector<std::uint16_t> mode2_terrain_layer_a_source;
    std::vector<std::uint16_t> mode2_terrain_layer_b_source;
    // Offline result of ROM 0x01610 applied to the flagged mode-2 source
    // stream at 0x188AFC; this is direct host data, not a runtime decoder
    // input or a ROM address token.
    std::vector<std::uint8_t> mode2_terrain_stream_source;
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

// Build the initial mode-2 field-scene input from already-loaded direct
// records. This is the conversion seam used by the separate Qt composition
// layer; it does not consult the ROM or reconstruct a source address table.
FieldSceneBufferInput make_initial_mode2_field_scene_input(
    const FieldSessionSourceAssets& assets);

} // namespace shadowrun::ghidra_conversion::integration

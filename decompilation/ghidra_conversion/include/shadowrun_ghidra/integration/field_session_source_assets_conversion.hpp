#pragma once

#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"
#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_map_conversion.hpp"
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
constexpr std::size_t kFieldMapDescriptorByteCount = 0x58U;
constexpr std::size_t kFieldMode0TerrainCollisionRectangleCount = 65U;
constexpr std::size_t kFieldMode0TerrainCollisionRectangleByteCount =
    kFieldMode0TerrainCollisionRectangleCount * 10U;
constexpr std::size_t kFieldMode2TerrainCollisionRectangleCount = 101U;
constexpr std::size_t kFieldMode2TerrainCollisionRectangleByteCount =
    kFieldMode2TerrainCollisionRectangleCount * 10U;
constexpr std::size_t kFieldMapEventTableByteCount = 32U;
constexpr std::size_t kFieldMovementDirectionCount = 8U;
constexpr std::size_t kFieldMovementDirectionTableByteCount =
    kFieldMovementDirectionCount * 2U * sizeof(std::uint16_t);
constexpr std::size_t kFieldCollisionProbeOffsetBankCount = 4U;
constexpr std::size_t kFieldCollisionProbePointCount = 3U;
constexpr std::size_t kFieldCollisionProbeOffsetBankByteCount =
    kFieldCollisionProbeOffsetBankCount * kFieldCollisionProbePointCount *
    2U * sizeof(std::uint16_t);
constexpr std::size_t kFieldControllerHeadingTableCount = 16U;
constexpr std::size_t kFieldControllerHeadingTableByteCount =
    kFieldControllerHeadingTableCount * sizeof(std::uint16_t);
constexpr std::size_t kFieldMovementRouteCount = 4U;
constexpr std::size_t kFieldMovementRouteByteCount =
    kFieldMovementRouteCount * 6U;

struct FieldMovementRouteSourceRecord {
    // Indexes into the source collision sample bytes FFEF10..13. The ROM
    // stores pointers to those four bytes; extraction resolves them into
    // these semantic indexes so runtime code never follows a RAM pointer.
    std::uint8_t primary_sample_index = 0;
    std::uint8_t secondary_sample_index = 0;
    std::uint16_t primary_fallback_slot = 0;
    std::uint16_t secondary_fallback_slot = 0;
};

struct FieldMapSourceDescriptor {
    // Map descriptor words +0x00/+0x02. These are source fixed-point words;
    // callers decide how to project them into host coordinates.
    std::uint16_t initial_camera_x_fixed = 0;
    std::uint16_t initial_camera_y_fixed = 0;
    // ROM words +0x42..+0x4C, the three initial field-record positions.
    std::array<SceneRunnerPosition, 3> initial_field_records{};
    // ROM words +0x0C..+0x12, used as the field terrain collision bounds.
    std::uint16_t terrain_x_exclusive_upper = 0;
    std::uint16_t terrain_y_exclusive_upper = 0;
    std::uint16_t terrain_x_inclusive_lower = 0;
    std::uint16_t terrain_y_inclusive_lower = 0;
    // Copied from the descriptor's event-table pointer.
    std::array<std::uint8_t, kFieldMapEventTableByteCount>
        event_mode_table{};
};

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
    // The normal profile is the map-index-0 descriptor selected by the
    // startup field-mode path. Its pointed-to collision/event records are
    // kept separately below; the mode-2 map descriptor remains a distinct
    // direct record.
    FieldMapSourceDescriptor mode0_descriptor{};
    std::vector<TerrainCollisionRectangle>
        mode0_terrain_collision_rectangles;
    std::array<std::uint8_t, 0x1000> mode0_terrain_collision_map{};
    // ROM 0x0C002, the eight signed fixed-point direction vectors consumed by
    // the converted 0x0BABC/0x0B77E movement probe path.
    std::array<MovementOffset, kFieldMovementDirectionCount>
        field_movement_direction_offsets{};
    // ROM 0x0BC36 selects one of four three-point banks at 0x0BD34 after
    // rotating the signed terrain-probe bytes. These are typed signed
    // offsets, not source pointers or runtime address tokens.
    CollisionProbeOffsetBank field_collision_probe_offset_banks{};
    // ROM 0x019D2, normalized D-pad nibble to target heading.
    std::array<std::uint16_t, kFieldControllerHeadingTableCount>
        field_controller_heading_table{};
    // ROM 0xBBF6, the four aligned-heading route descriptors used by
    // 0xBB86. Each descriptor is six host bytes: two collision-sample
    // indexes followed by two big-endian fallback direction slots.
    std::array<FieldMovementRouteSourceRecord, kFieldMovementRouteCount>
        field_movement_routes{};
    // ROM 0x172C80, interpreted as the distinct map-index-2 metadata record.
    FieldMapSourceDescriptor mode2_descriptor{};
    // ROM 0x172DAA, five-word records consumed by the terrain-map builder.
    std::vector<TerrainCollisionRectangle>
        mode2_terrain_collision_rectangles;
    // Host projection of the same source rectangles through the converted
    // ROM 0xEE10 rasterization contract. This is derived conversion state,
    // not another extracted asset.
    std::array<std::uint8_t, 0x1000> mode2_terrain_collision_map{};
};

struct FieldSessionSourceAssetLoadResult {
    std::optional<FieldSessionSourceAssets> assets;
    std::vector<std::string> errors;

    bool complete() const { return assets.has_value() && errors.empty(); }
};

// Reads the named field-session records from resources.tsv and decodes only
// the big-endian words required by FieldSessionBootstrapInput. Missing,
// malformed, or wrong-sized records are reported; no fallback data is
// synthesized.
FieldSessionSourceAssetLoadResult load_field_session_source_assets(
    const ConversionAssetCatalog& catalog);

// Build the initial mode-2 field-scene input from already-loaded direct
// records. This is the conversion seam used by the separate Qt composition
// layer; it does not consult the ROM or reconstruct a source address table.
FieldSceneBufferInput make_initial_mode2_field_scene_input(
    const FieldSessionSourceAssets& assets);

} // namespace shadowrun::ghidra_conversion::integration

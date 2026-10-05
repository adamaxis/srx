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
constexpr std::uint16_t kFirstWorldActivityResourceScene = 0x15U;
constexpr std::uint16_t kWorldActivityResourceSceneEnd = 0x2CU;
constexpr std::size_t kFieldSessionTerrainLayerWordCount = 0xE10U;
constexpr std::size_t kFieldSessionTerrainLayerByteCount =
    kFieldSessionTerrainLayerWordCount * sizeof(std::uint16_t);
// Scene 2's profile points at the 0x170102 compressed resource. Its DBF
// header expands to 0x4F40 bytes; the old 0x3500 size belonged to the row-0
// profile's 0x188AFC stream and was another symptom of selecting the wrong
// scene record.
constexpr std::size_t kScene2TerrainStreamByteCount = 0x4F40U;
constexpr std::size_t kFieldMapDescriptorByteCount = 0x58U;
constexpr std::size_t kScene0TerrainCollisionRectangleCount = 65U;
constexpr std::size_t kScene0TerrainCollisionRectangleByteCount =
    kScene0TerrainCollisionRectangleCount * 10U;
constexpr std::size_t kScene2TerrainCollisionRectangleCount = 101U;
constexpr std::size_t kScene2TerrainCollisionRectangleByteCount =
    kScene2TerrainCollisionRectangleCount * 10U;
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
    // Copied from the descriptor's event-table pointer when the scene has one.
    bool event_table_available = false;
    std::array<std::uint8_t, kFieldMapEventTableByteCount>
        event_mode_table{};
};

struct FieldSceneSourceRecord {
    // One complete scene-row resource set selected by ROM 0x3232. All source
    // pointers have been resolved by the offline importer; the conversion
    // runtime receives only typed host records and direct byte/word vectors.
    std::uint16_t scene_reference = 0;
    std::vector<std::uint16_t> terrain_layer_a_source;
    std::vector<std::uint16_t> terrain_layer_b_source;
    std::vector<std::uint8_t> terrain_stream_source;
    // ROM 0x167AE samples this scene-specific bound into FFE12C. The offline
    // importer resolves FFDE1E and its first bundle longword into this value.
    std::optional<std::uint16_t> world_activity_random_index_bound;
    FieldMapSourceDescriptor descriptor{};
    std::vector<TerrainCollisionRectangle>
        terrain_collision_rectangles;
    std::array<std::uint8_t, 0x1000> terrain_collision_map{};
};

// Direct source records extracted from the US ROM. These are host-owned
// inputs for the existing field-session conversion, not a runtime ROM view.
struct FieldSessionSourceAssets {
    // ROM 0x166AD8, copied by 0x566A2.
    std::vector<std::uint8_t> action_selection_profile_source;
    // ROM 0x14C1F4, copied by 0x26938.
    std::vector<std::uint16_t> field_mode_selection_state_source;
    // ROM 0x3232: the selected scene id indexes all 56 copied 48-word
    // profiles. The importer resolves the pointers once and stores the
    // complete table as one direct host record; runtime code never follows
    // the ROM pointer table.
    FieldSceneProfileTable scene_profile_table_source{};
    // One entry for every row in ROM 0x3232. Missing entries are retained as
    // explicit optionals so a partial extraction cannot silently become a
    // fabricated field scene.
    std::array<std::optional<FieldSceneSourceRecord>, kFieldSceneProfileCount>
        field_scene_resources{};
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

// Build a selected field-scene input from already-loaded direct records. This
// is the conversion seam used by the separate Qt composition layer; it does
// not consult the ROM or reconstruct a source address table.
std::optional<FieldSceneBufferInput> make_field_scene_input(
    const FieldSessionSourceAssets& assets, std::uint16_t scene_reference);

// Compatibility-named convenience for the ROM's opening scene (row 2). An
// incomplete source catalog returns an empty input; the bootstrap's normal
// validation then reports source_data_missing rather than inventing records.
FieldSceneBufferInput make_initial_mode2_field_scene_input(
    const FieldSessionSourceAssets& assets);

} // namespace shadowrun::ghidra_conversion::integration

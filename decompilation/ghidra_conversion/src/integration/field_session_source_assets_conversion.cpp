#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace shadowrun::ghidra_conversion::integration {
namespace {

constexpr const char* kActionProfilesAlias =
    "records.field.action.selection.profiles";
constexpr const char* kModeSelectionAlias =
    "records.field.mode.selection.state";
constexpr const char* kNormalSceneProfileAlias =
    "records.field.scene.profile.normal";
constexpr const char* kAlternateSceneProfileAlias =
    "records.field.scene.profile.alternate";
constexpr const char* kMode0TerrainCollisionRectanglesAlias =
    "records.field.mode.0.terrain.collision.rectangles";
constexpr const char* kMode0EventTableAlias =
    "records.field.mode.0.event.table";
constexpr const char* kFieldMovementDirectionTableAlias =
    "records.field.entity.movement.direction.table";
constexpr const char* kMode2TerrainLayerAAlias =
    "records.field.mode.2.terrain.layer.a";
constexpr const char* kMode2TerrainLayerBAlias =
    "records.field.mode.2.terrain.layer.b";
constexpr const char* kMode2TerrainStreamAlias =
    "records.field.mode.2.terrain.stream";
constexpr const char* kMode2DescriptorAlias =
    "records.field.mode.2.descriptor";
constexpr const char* kMode2TerrainCollisionRectanglesAlias =
    "records.field.mode.2.terrain.collision.rectangles";
constexpr const char* kMode2EventTableAlias =
    "records.field.mode.2.event.table";

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

std::array<std::uint16_t, kFieldSceneProfileWordCount>
decode_scene_profile(const std::vector<std::uint8_t>& bytes) {
    std::array<std::uint16_t, kFieldSceneProfileWordCount> profile{};
    for (std::size_t index = 0; index < profile.size(); ++index) {
        profile[index] = read_big_endian_word(bytes, index * sizeof(std::uint16_t));
    }
    return profile;
}

FieldMapSourceDescriptor decode_field_map_descriptor(
    const std::vector<std::uint8_t>& descriptor_bytes,
    const std::vector<std::uint8_t>& event_table_bytes) {
    FieldMapSourceDescriptor descriptor;
    descriptor.initial_camera_x_fixed =
        read_big_endian_word(descriptor_bytes, 0x00U);
    descriptor.initial_camera_y_fixed =
        read_big_endian_word(descriptor_bytes, 0x02U);
    descriptor.initial_field_records[0] = {
        read_big_endian_word(descriptor_bytes, 0x42U),
        read_big_endian_word(descriptor_bytes, 0x44U)};
    descriptor.initial_field_records[1] = {
        read_big_endian_word(descriptor_bytes, 0x46U),
        read_big_endian_word(descriptor_bytes, 0x48U)};
    descriptor.initial_field_records[2] = {
        read_big_endian_word(descriptor_bytes, 0x4AU),
        read_big_endian_word(descriptor_bytes, 0x4CU)};
    descriptor.terrain_x_exclusive_upper =
        read_big_endian_word(descriptor_bytes, 0x0CU);
    descriptor.terrain_y_exclusive_upper =
        read_big_endian_word(descriptor_bytes, 0x0EU);
    descriptor.terrain_x_inclusive_lower =
        read_big_endian_word(descriptor_bytes, 0x10U);
    descriptor.terrain_y_inclusive_lower =
        read_big_endian_word(descriptor_bytes, 0x12U);
    std::copy(event_table_bytes.begin(), event_table_bytes.end(),
              descriptor.event_mode_table.begin());
    return descriptor;
}

std::vector<TerrainCollisionRectangle>
decode_field_terrain_collision_rectangles(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t expected_rectangle_count) {
    std::vector<TerrainCollisionRectangle> rectangles;
    rectangles.reserve(expected_rectangle_count);
    for (std::size_t offset = 0; offset < bytes.size();
         offset += 10U) {
        const auto x_start = read_big_endian_word(bytes, offset + 0x00U);
        if ((x_start & 0x8000U) != 0U) {
            throw std::runtime_error(
                "terrain collision record stream contains an early sentinel");
        }
        const auto x_end = read_big_endian_word(bytes, offset + 0x02U);
        const auto y_start = read_big_endian_word(bytes, offset + 0x04U);
        const auto y_end = read_big_endian_word(bytes, offset + 0x06U);
        const auto value = read_big_endian_word(bytes, offset + 0x08U);
        rectangles.push_back({x_start, x_end, y_start, y_end,
                              static_cast<std::uint8_t>(value)});
    }
    if (rectangles.size() != expected_rectangle_count) {
        throw std::runtime_error(
            "terrain collision record count changed");
    }
    return rectangles;
}

std::array<MovementOffset, kFieldMovementDirectionCount>
decode_field_movement_direction_offsets(
    const std::vector<std::uint8_t>& bytes) {
    std::array<MovementOffset, kFieldMovementDirectionCount> offsets{};
    for (std::size_t index = 0; index < offsets.size(); ++index) {
        const auto byte_offset = index * 4U;
        offsets[index] = MovementOffset{
            static_cast<std::int16_t>(read_big_endian_word(bytes, byte_offset)),
            static_cast<std::int16_t>(
                read_big_endian_word(bytes, byte_offset + 2U))};
    }
    return offsets;
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
    const auto normal_profile = read_record_bytes(
        catalog, kNormalSceneProfileAlias, kFieldSceneProfileByteCount,
        result.errors);
    const auto alternate_profile = read_record_bytes(
        catalog, kAlternateSceneProfileAlias, kFieldSceneProfileByteCount,
        result.errors);
    const auto mode0_collision_rectangles = read_record_bytes(
        catalog, kMode0TerrainCollisionRectanglesAlias,
        kFieldMode0TerrainCollisionRectangleByteCount, result.errors);
    const auto mode0_event_table = read_record_bytes(
        catalog, kMode0EventTableAlias, kFieldMapEventTableByteCount,
        result.errors);
    const auto movement_direction_table = read_record_bytes(
        catalog, kFieldMovementDirectionTableAlias,
        kFieldMovementDirectionTableByteCount, result.errors);
    const auto terrain_layer_a = read_record_bytes(
        catalog, kMode2TerrainLayerAAlias, kFieldSessionTerrainLayerByteCount,
        result.errors);
    const auto terrain_layer_b = read_record_bytes(
        catalog, kMode2TerrainLayerBAlias, kFieldSessionTerrainLayerByteCount,
        result.errors);
    const auto terrain_stream = read_record_bytes(
        catalog, kMode2TerrainStreamAlias, kFieldSessionTerrainStreamByteCount,
        result.errors);
    const auto mode2_descriptor = read_record_bytes(
        catalog, kMode2DescriptorAlias, kFieldMapDescriptorByteCount,
        result.errors);
    const auto mode2_collision_rectangles = read_record_bytes(
        catalog, kMode2TerrainCollisionRectanglesAlias,
        kFieldMode2TerrainCollisionRectangleByteCount, result.errors);
    const auto mode2_event_table = read_record_bytes(
        catalog, kMode2EventTableAlias, kFieldMapEventTableByteCount,
        result.errors);
    if (!action_profiles.has_value() || !mode_selection.has_value() ||
        !normal_profile.has_value() || !alternate_profile.has_value() ||
        !mode0_collision_rectangles.has_value() ||
        !mode0_event_table.has_value() ||
        !movement_direction_table.has_value() ||
        !terrain_layer_a.has_value() || !terrain_layer_b.has_value() ||
        !terrain_stream.has_value() || !mode2_descriptor.has_value() ||
        !mode2_collision_rectangles.has_value() ||
        !mode2_event_table.has_value()) {
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
    assets.normal_scene_profile_source = decode_scene_profile(*normal_profile);
    assets.alternate_scene_profile_source =
        decode_scene_profile(*alternate_profile);
    assets.mode0_descriptor =
        decode_field_map_descriptor(*normal_profile, *mode0_event_table);
    assets.mode0_terrain_collision_rectangles =
        decode_field_terrain_collision_rectangles(
            *mode0_collision_rectangles,
            kFieldMode0TerrainCollisionRectangleCount);
    bool rebuild_requested = true;
    rebuild_terrain_collision_map(
        assets.mode0_terrain_collision_map, rebuild_requested,
        assets.mode0_terrain_collision_rectangles, [](auto&) {});
    assets.field_movement_direction_offsets =
        decode_field_movement_direction_offsets(*movement_direction_table);
    assets.mode2_terrain_layer_a_source.reserve(
        kFieldSessionTerrainLayerWordCount);
    assets.mode2_terrain_layer_b_source.reserve(
        kFieldSessionTerrainLayerWordCount);
    for (std::size_t offset = 0; offset < terrain_layer_a->size();
         offset += sizeof(std::uint16_t)) {
        assets.mode2_terrain_layer_a_source.push_back(
            read_big_endian_word(*terrain_layer_a, offset));
        assets.mode2_terrain_layer_b_source.push_back(
            read_big_endian_word(*terrain_layer_b, offset));
    }
    assets.mode2_terrain_stream_source = *terrain_stream;
    assets.mode2_descriptor =
        decode_field_map_descriptor(*mode2_descriptor, *mode2_event_table);
    assets.mode2_terrain_collision_rectangles =
        decode_field_terrain_collision_rectangles(
            *mode2_collision_rectangles,
            kFieldMode2TerrainCollisionRectangleCount);
    rebuild_requested = true;
    rebuild_terrain_collision_map(
        assets.mode2_terrain_collision_map, rebuild_requested,
        assets.mode2_terrain_collision_rectangles, [](auto&) {});
    result.assets = std::move(assets);
    return result;
}

FieldSceneBufferInput make_initial_mode2_field_scene_input(
    const FieldSessionSourceAssets& assets) {
    FieldSceneBufferInput input;
    input.normal_profile = assets.normal_scene_profile_source;
    input.alternate_profile = assets.alternate_scene_profile_source;
    input.scene_reference = 2U;
    input.terrain_stream = assets.mode2_terrain_stream_source;
    input.layer_a = assets.mode2_terrain_layer_a_source;
    input.layer_b = assets.mode2_terrain_layer_b_source;
    input.source_metadata.available = true;
    // Field mode 2 enters the normal scene profile, whose pointer-table row
    // is map index 0. The separate mode-2 descriptor is a different map
    // record and must not be projected onto this startup scene.
    input.source_metadata.initial_camera_x_fixed =
        assets.mode0_descriptor.initial_camera_x_fixed;
    input.source_metadata.initial_camera_y_fixed =
        assets.mode0_descriptor.initial_camera_y_fixed;
    input.source_metadata.initial_field_records =
        assets.mode0_descriptor.initial_field_records;
    input.source_metadata.terrain_x_exclusive_upper =
        assets.mode0_descriptor.terrain_x_exclusive_upper;
    input.source_metadata.terrain_y_exclusive_upper =
        assets.mode0_descriptor.terrain_y_exclusive_upper;
    input.source_metadata.terrain_x_inclusive_lower =
        assets.mode0_descriptor.terrain_x_inclusive_lower;
    input.source_metadata.terrain_y_inclusive_lower =
        assets.mode0_descriptor.terrain_y_inclusive_lower;
    input.source_metadata.event_mode_table =
        assets.mode0_descriptor.event_mode_table;
    input.source_metadata.terrain_collision_rectangles =
        assets.mode0_terrain_collision_rectangles;
    input.source_metadata.terrain_collision_map =
        assets.mode0_terrain_collision_map;
    return input;
}

} // namespace shadowrun::ghidra_conversion::integration

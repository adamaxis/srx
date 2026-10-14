#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace shadowrun::ghidra_conversion::integration {
namespace {

constexpr const char* kActionProfilesAlias =
    "records.field.action.selection.profiles";
constexpr const char* kModeSelectionAlias =
    "records.field.mode.selection.state";
constexpr const char* kSceneProfileTableAlias =
    "records.field.scene.profiles";
constexpr const char* kElevatorSceneGroupsAlias =
    "records.field.elevator.scene.groups";
constexpr const char* kFieldMovementDirectionTableAlias =
    "records.field.entity.movement.direction.table";
constexpr const char* kFieldCollisionProbeOffsetBanksAlias =
    "records.field.entity.collision.probe.offset.banks";
constexpr const char* kFieldControllerHeadingTableAlias =
    "records.field.entity.controller.heading.table";
constexpr const char* kFieldMovementRouteTableAlias =
    "records.field.entity.movement.route.table";
constexpr const char* kFieldSceneResourceAliasPrefix =
    "records.field.scenes.scene.";

std::string field_scene_resource_alias(const std::size_t scene_id,
                                       const char* suffix) {
    std::ostringstream alias;
    alias << kFieldSceneResourceAliasPrefix << std::setfill('0')
          << std::setw(2) << scene_id << '.' << suffix;
    return alias.str();
}

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

std::optional<std::vector<std::uint8_t>> read_optional_record_bytes(
    const ConversionAssetCatalog& catalog, const std::string& alias,
    const std::size_t expected_size, std::vector<std::string>& errors) {
    const auto* record = catalog.find(alias);
    if (record == nullptr) {
        return std::nullopt;
    }
    if (record->kind != "state") {
        errors.emplace_back(
            std::string("field-scene optional record is not state data: ") +
            alias);
        return std::nullopt;
    }
    if (record->byte_size != expected_size) {
        errors.emplace_back(
            std::string("field-scene optional record has wrong size: ") +
            alias);
        return std::nullopt;
    }
    std::ifstream input(record->absolute_path, std::ios::binary);
    if (!input) {
        errors.emplace_back(
            std::string("cannot open field-scene optional record: ") + alias);
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes(expected_size);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (!input || input.gcount() != static_cast<std::streamsize>(bytes.size())) {
        errors.emplace_back(
            std::string("cannot read field-scene optional record: ") + alias);
        return std::nullopt;
    }
    return bytes;
}

std::optional<std::vector<std::uint8_t>> read_variable_record_bytes(
    const ConversionAssetCatalog& catalog, const std::string& alias,
    std::vector<std::string>& errors) {
    const auto* record = catalog.find(alias);
    if (record == nullptr) {
        errors.emplace_back(std::string("missing field-scene source record: ") +
                            alias);
        return std::nullopt;
    }
    if (record->kind != "state" || record->byte_size == 0U) {
        errors.emplace_back(
            std::string("field-scene source record is invalid: ") + alias);
        return std::nullopt;
    }
    std::ifstream input(record->absolute_path, std::ios::binary);
    if (!input) {
        errors.emplace_back(std::string("cannot open field-scene source record: ") +
                            alias);
        return std::nullopt;
    }
    std::vector<std::uint8_t> bytes(record->byte_size);
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    if (!input || input.gcount() != static_cast<std::streamsize>(bytes.size())) {
        errors.emplace_back(std::string("cannot read field-scene source record: ") +
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
    const std::optional<std::vector<std::uint8_t>>& event_table_bytes) {
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
    if (event_table_bytes.has_value()) {
        descriptor.event_table_available = true;
        std::copy(event_table_bytes->begin(), event_table_bytes->end(),
                  descriptor.event_mode_table.begin());
    }
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

CollisionProbeOffsetBank decode_field_collision_probe_offset_banks(
    const std::vector<std::uint8_t>& bytes) {
    CollisionProbeOffsetBank banks{};
    std::size_t offset = 0;
    for (auto& bank : banks) {
        for (auto& point : bank) {
            point.x = static_cast<std::int16_t>(
                read_big_endian_word(bytes, offset));
            offset += sizeof(std::uint16_t);
            point.y = static_cast<std::int16_t>(
                read_big_endian_word(bytes, offset));
            offset += sizeof(std::uint16_t);
        }
    }
    return banks;
}

std::array<std::uint16_t, kFieldControllerHeadingTableCount>
decode_field_controller_heading_table(
    const std::vector<std::uint8_t>& bytes) {
    std::array<std::uint16_t, kFieldControllerHeadingTableCount> table{};
    for (std::size_t index = 0; index < table.size(); ++index) {
        table[index] = read_big_endian_word(bytes, index * 2U);
    }
    return table;
}

std::array<FieldMovementRouteSourceRecord, kFieldMovementRouteCount>
decode_field_movement_routes(const std::vector<std::uint8_t>& bytes) {
    std::array<FieldMovementRouteSourceRecord, kFieldMovementRouteCount>
        routes{};
    for (std::size_t index = 0; index < routes.size(); ++index) {
        const auto byte_offset = index * 6U;
        const auto primary_sample_index = bytes[byte_offset];
        const auto secondary_sample_index = bytes[byte_offset + 1U];
        if (primary_sample_index >= 4U || secondary_sample_index >= 4U) {
            throw std::runtime_error(
                "field movement route sample index changed");
        }
        routes[index] = FieldMovementRouteSourceRecord{
            primary_sample_index,
            secondary_sample_index,
            read_big_endian_word(bytes, byte_offset + 2U),
            read_big_endian_word(bytes, byte_offset + 4U)};
    }
    return routes;
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
    const auto scene_profiles = read_record_bytes(
        catalog, kSceneProfileTableAlias,
        kFieldSceneProfileCount * kFieldSceneProfileByteCount,
        result.errors);
    const auto elevator_scene_groups = read_record_bytes(
        catalog, kElevatorSceneGroupsAlias,
        kFieldElevatorSceneGroupTableByteCount, result.errors);
    const auto movement_direction_table = read_record_bytes(
        catalog, kFieldMovementDirectionTableAlias,
        kFieldMovementDirectionTableByteCount, result.errors);
    const auto collision_probe_offset_banks = read_record_bytes(
        catalog, kFieldCollisionProbeOffsetBanksAlias,
        kFieldCollisionProbeOffsetBankByteCount, result.errors);
    const auto controller_heading_table = read_record_bytes(
        catalog, kFieldControllerHeadingTableAlias,
        kFieldControllerHeadingTableByteCount, result.errors);
    const auto movement_route_table = read_record_bytes(
        catalog, kFieldMovementRouteTableAlias, kFieldMovementRouteByteCount,
        result.errors);
    if (!action_profiles.has_value() || !mode_selection.has_value() ||
        !scene_profiles.has_value() || !elevator_scene_groups.has_value() ||
        !movement_direction_table.has_value() ||
        !collision_probe_offset_banks.has_value() ||
        !controller_heading_table.has_value() ||
        !movement_route_table.has_value()) {
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
    for (std::size_t scene_id = 0; scene_id < kFieldSceneProfileCount;
         ++scene_id) {
        const auto offset = scene_id * kFieldSceneProfileByteCount;
        const std::vector<std::uint8_t> profile_bytes(
            scene_profiles->begin() +
                static_cast<std::ptrdiff_t>(offset),
            scene_profiles->begin() + static_cast<std::ptrdiff_t>(
                offset + kFieldSceneProfileByteCount));
        assets.scene_profile_table_source[scene_id] =
            decode_scene_profile(profile_bytes);
        const auto group_offset =
            scene_id * kFieldElevatorSceneGroupRecordByteCount;
        auto& group = assets.field_elevator_scene_groups[scene_id];
        group.count = (*elevator_scene_groups)[group_offset];
        if (group.count > kFieldElevatorSceneGroupMemberCount) {
            result.errors.emplace_back(
                "field elevator scene-group count is out of range: " +
                std::to_string(scene_id));
            return result;
        }
        for (std::size_t member = 0; member < group.scene_ids.size(); ++member) {
            const auto scene_reference =
                (*elevator_scene_groups)[group_offset + 1U + member];
            if ((member < group.count &&
                 scene_reference >= kFieldSceneProfileCount) ||
                (member >= group.count && scene_reference != 0U)) {
                result.errors.emplace_back(
                    "field elevator scene-group member is invalid: " +
                    std::to_string(scene_id));
                return result;
            }
            group.scene_ids[member] = scene_reference;
        }
    }
    bool rebuild_requested = true;
    assets.field_movement_direction_offsets =
        decode_field_movement_direction_offsets(*movement_direction_table);
    assets.field_collision_probe_offset_banks =
        decode_field_collision_probe_offset_banks(*collision_probe_offset_banks);
    assets.field_controller_heading_table =
        decode_field_controller_heading_table(*controller_heading_table);
    assets.field_movement_routes =
        decode_field_movement_routes(*movement_route_table);
    for (std::size_t scene_id = 0; scene_id < kFieldSceneProfileCount;
         ++scene_id) {
        const auto layer_a = read_record_bytes(
            catalog,
            field_scene_resource_alias(scene_id, "terrain.layer.a").c_str(),
            kFieldSessionTerrainLayerByteCount, result.errors);
        const auto layer_b = read_record_bytes(
            catalog,
            field_scene_resource_alias(scene_id, "terrain.layer.b").c_str(),
            kFieldSessionTerrainLayerByteCount, result.errors);
        const auto terrain_stream = read_variable_record_bytes(
            catalog, field_scene_resource_alias(scene_id, "terrain.stream"),
            result.errors);
        const auto descriptor = read_record_bytes(
            catalog,
            field_scene_resource_alias(scene_id, "descriptor").c_str(),
            kFieldMapDescriptorByteCount, result.errors);
        const auto collision_rectangles = read_variable_record_bytes(
            catalog,
            field_scene_resource_alias(scene_id,
                                       "terrain.collision.rectangles"),
            result.errors);
        const auto event_table = read_optional_record_bytes(
            catalog, field_scene_resource_alias(scene_id, "event.table"),
            kFieldMapEventTableByteCount, result.errors);
        const auto activity_random_index_bound = read_optional_record_bytes(
            catalog,
            field_scene_resource_alias(
                scene_id, "world.activity.random.index.bound"),
            sizeof(std::uint16_t), result.errors);
        const auto activity_resource_cursor = read_optional_record_bytes(
            catalog,
            field_scene_resource_alias(
                scene_id, "world.activity.resource.cursor"),
            3U, result.errors);
        const bool expects_activity_bound =
            scene_id >= kFirstWorldActivityResourceScene &&
            scene_id < kWorldActivityResourceSceneEnd;
        if (expects_activity_bound &&
            !activity_random_index_bound.has_value()) {
            result.errors.emplace_back(
                "field-scene world-activity random-index bound is missing: " +
                std::to_string(scene_id));
        } else if (!expects_activity_bound &&
                   activity_random_index_bound.has_value()) {
            result.errors.emplace_back(
                "field-scene has an unexpected world-activity random-index bound: " +
                std::to_string(scene_id));
        }
        if (expects_activity_bound && !activity_resource_cursor.has_value()) {
            result.errors.emplace_back(
                "field-scene world-activity resource cursor is missing: " +
                std::to_string(scene_id));
        } else if (!expects_activity_bound &&
                   activity_resource_cursor.has_value()) {
            result.errors.emplace_back(
                "field-scene has an unexpected world-activity resource cursor: " +
                std::to_string(scene_id));
        }
        if (!layer_a.has_value() || !layer_b.has_value() ||
            !terrain_stream.has_value() || !descriptor.has_value() ||
            !collision_rectangles.has_value()) {
            continue;
        }
        if (collision_rectangles->size() == 0U ||
            collision_rectangles->size() % 10U != 0U) {
            result.errors.emplace_back(
                "field-scene collision record stream is not record-aligned: " +
                std::to_string(scene_id));
            continue;
        }

        FieldSceneSourceRecord scene;
        scene.scene_reference = static_cast<std::uint16_t>(scene_id);
        scene.terrain_layer_a_source.reserve(
            kFieldSessionTerrainLayerWordCount);
        scene.terrain_layer_b_source.reserve(
            kFieldSessionTerrainLayerWordCount);
        for (std::size_t offset = 0; offset < layer_a->size();
             offset += sizeof(std::uint16_t)) {
            scene.terrain_layer_a_source.push_back(
                read_big_endian_word(*layer_a, offset));
            scene.terrain_layer_b_source.push_back(
                read_big_endian_word(*layer_b, offset));
        }
        scene.terrain_stream_source = *terrain_stream;
        if (activity_random_index_bound.has_value()) {
            scene.world_activity_random_index_bound =
                read_big_endian_word(*activity_random_index_bound, 0U);
            if (*scene.world_activity_random_index_bound == 0U ||
                *scene.world_activity_random_index_bound > 6U) {
                result.errors.emplace_back(
                    "field-scene world-activity random-index bound is invalid: " +
                    std::to_string(scene_id));
            }
        }
        if (activity_resource_cursor.has_value()) {
            scene.world_activity_resource_cursor.cursor =
                static_cast<std::int8_t>((*activity_resource_cursor)[0]);
            scene.world_activity_resource_cursor.resource_index =
                read_big_endian_word(*activity_resource_cursor, 1U);
        }
        scene.descriptor = decode_field_map_descriptor(*descriptor, event_table);
        scene.terrain_collision_rectangles =
            decode_field_terrain_collision_rectangles(
                *collision_rectangles, collision_rectangles->size() / 10U);
        rebuild_requested = true;
        std::array<std::uint8_t, 0x1000> terrain_collision_map{};
        rebuild_terrain_collision_map(
            terrain_collision_map, rebuild_requested,
            scene.terrain_collision_rectangles, [](auto&) {});
        scene.terrain_collision_map.assign(terrain_collision_map.begin(),
                                           terrain_collision_map.end());
        assets.field_scene_resources[scene_id] = std::move(scene);
    }
    for (std::size_t scene_id = 0; scene_id < kFieldSceneProfileCount;
         ++scene_id) {
        if (!assets.field_scene_resources[scene_id].has_value()) {
            result.errors.emplace_back(
                "field-scene resource set is incomplete at scene " +
                std::to_string(scene_id));
        }
    }
    if (!result.errors.empty()) {
        return result;
    }
    result.assets = std::move(assets);
    return result;
}

std::optional<FieldSceneBufferInput> make_field_scene_input(
    const FieldSessionSourceAssets& assets,
    const std::uint16_t scene_reference) {
    if (scene_reference >= kFieldSceneProfileCount ||
        !assets.field_scene_resources[scene_reference].has_value()) {
        return std::nullopt;
    }
    const auto& scene = *assets.field_scene_resources[scene_reference];
    FieldSceneBufferInput input;
    input.scene_profiles = assets.scene_profile_table_source;
    input.scene_profile_table_available = true;
    input.scene_reference = scene_reference;
    input.terrain_stream = scene.terrain_stream_source;
    input.layer_a = scene.terrain_layer_a_source;
    input.layer_b = scene.terrain_layer_b_source;
    input.source_metadata.available = true;
    input.source_metadata.initial_camera_x_fixed =
        scene.descriptor.initial_camera_x_fixed;
    input.source_metadata.initial_camera_y_fixed =
        scene.descriptor.initial_camera_y_fixed;
    input.source_metadata.initial_field_records =
        scene.descriptor.initial_field_records;
    input.source_metadata.terrain_x_exclusive_upper =
        scene.descriptor.terrain_x_exclusive_upper;
    input.source_metadata.terrain_y_exclusive_upper =
        scene.descriptor.terrain_y_exclusive_upper;
    input.source_metadata.terrain_x_inclusive_lower =
        scene.descriptor.terrain_x_inclusive_lower;
    input.source_metadata.terrain_y_inclusive_lower =
        scene.descriptor.terrain_y_inclusive_lower;
    input.source_metadata.event_table_available =
        scene.descriptor.event_table_available;
    input.source_metadata.event_mode_table = scene.descriptor.event_mode_table;
    input.source_metadata.world_activity_random_index_bound =
        scene.world_activity_random_index_bound;
    input.source_metadata.world_activity_resource_cursor =
        scene.world_activity_resource_cursor;
    input.source_metadata.terrain_collision_rectangles =
        scene.terrain_collision_rectangles;
    if (scene.terrain_collision_map.size() !=
        input.source_metadata.terrain_collision_map.size()) {
        return std::nullopt;
    }
    std::copy(scene.terrain_collision_map.begin(),
              scene.terrain_collision_map.end(),
              input.source_metadata.terrain_collision_map.begin());
    return input;
}

FieldSceneBufferInput make_initial_mode2_field_scene_input(
    const FieldSessionSourceAssets& assets) {
    const auto input = make_field_scene_input(assets, 2U);
    return input.has_value() ? *input : FieldSceneBufferInput{};
}

} // namespace shadowrun::ghidra_conversion::integration

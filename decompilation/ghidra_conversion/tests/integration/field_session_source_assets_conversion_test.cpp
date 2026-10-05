#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <array>
#include <cstdint>
#include <filesystem>

#ifndef SHADOWRUN_CONVERSION_ASSET_ROOT
#error "The field-session source asset test needs the direct asset root"
#endif

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::integration;

    const auto loaded = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_CONVERSION_ASSET_ROOT));
    if (!loaded.complete()) {
        return 1;
    }
    const auto source = load_field_session_source_assets(loaded.catalog);
    if (!source.complete() ||
        source.assets->action_selection_profile_source.size() !=
            kFieldSessionActionProfileByteCount ||
        source.assets->field_mode_selection_state_source.size() !=
            kFieldSessionModeSelectionWordCount) {
        return 2;
    }
    if (loaded.catalog.find("records.field.mode.2.descriptor") != nullptr ||
        loaded.catalog.find("records.field.scenes.scene.02.descriptor") ==
            nullptr) {
        return 22;
    }

    const auto& profiles = source.assets->action_selection_profile_source;
    if (profiles[0] != 0x00U || profiles[1] != 0x03U || profiles[2] != 0x05U ||
        profiles[0x22U] != 0x00U || profiles[0x24U] != 0x00U ||
        profiles[0x25U] != 0x02U) {
        return 3;
    }
    const auto& mode_state = source.assets->field_mode_selection_state_source;
    if (mode_state.front() != 0x0300U || mode_state[1] != 0x001EU ||
        mode_state[2] != 0x0064U || mode_state.back() != 0x0000U) {
        return 4;
    }
    const auto& scene_profiles = source.assets->scene_profile_table_source;
    const auto& normal_profile = scene_profiles[0];
    if (normal_profile[0] != 0x0A00U || normal_profile[1] != 0x0280U ||
        normal_profile[4] != 0x6700U || normal_profile[15] != 0x7CECU ||
        normal_profile.back() != 0x1215U) {
        return 5;
    }
    const auto& alternate_profile = scene_profiles[1];
    if (alternate_profile[0] != 0x4080U || alternate_profile[1] != 0x4C80U ||
        alternate_profile[4] != 0x5300U || alternate_profile[15] != 0xAD3AU ||
        alternate_profile.back() != 0x1A0DU) {
        return 6;
    }
    if (scene_profiles[2][0] != 0x3400U ||
        scene_profiles[2][1] != 0x2280U ||
        scene_profiles[2][33] != 0x3CE0U ||
        scene_profiles[2][34] != 0x2D00U) {
        return 18;
    }
    for (const auto& scene : source.assets->field_scene_resources) {
        if (!scene.has_value() ||
            scene->terrain_layer_a_source.size() !=
                kFieldSessionTerrainLayerWordCount ||
            scene->terrain_layer_b_source.size() !=
                kFieldSessionTerrainLayerWordCount ||
            scene->terrain_stream_source.empty() ||
            scene->terrain_collision_rectangles.empty()) {
            return 19;
        }
    }
    constexpr std::array<std::uint16_t, 0x17U> expected_activity_bounds{{
        4U, 4U, 4U, 4U, 4U, 3U, 3U, 3U, 3U, 4U, 4U, 4U,
        4U, 4U, 5U, 5U, 5U, 5U, 5U, 5U, 2U, 2U, 2U}};
    for (std::size_t offset = 0U;
         offset < expected_activity_bounds.size(); ++offset) {
        const auto scene_id = static_cast<std::size_t>(
            kFirstWorldActivityResourceScene + offset);
        if (!source.assets->field_scene_resources[scene_id]
                 ->world_activity_random_index_bound.has_value() ||
            *source.assets->field_scene_resources[scene_id]
                 ->world_activity_random_index_bound !=
                expected_activity_bounds[offset]) {
            return 23;
        }
    }
    for (std::size_t scene_id = 0U;
         scene_id < source.assets->field_scene_resources.size(); ++scene_id) {
        if ((scene_id < kFirstWorldActivityResourceScene ||
             scene_id >= kWorldActivityResourceSceneEnd) &&
            source.assets->field_scene_resources[scene_id]
                ->world_activity_random_index_bound.has_value()) {
            return 25;
        }
    }
    const auto& scene55 = *source.assets->field_scene_resources[55U];
    if (scene55.descriptor.event_table_available ||
        scene55.terrain_stream_source.size() != 1984U) {
        return 20;
    }
    if (!make_field_scene_input(*source.assets, 55U).has_value() ||
        make_field_scene_input(*source.assets, 56U).has_value()) {
        return 21;
    }
    const auto& scene2 = *source.assets->field_scene_resources[2U];
    const auto& terrain_a = scene2.terrain_layer_a_source;
    const auto& terrain_b = scene2.terrain_layer_b_source;
    if (terrain_a.size() != kFieldSessionTerrainLayerWordCount ||
        terrain_b.size() != kFieldSessionTerrainLayerWordCount ||
        terrain_a.front() != 0x0000U || terrain_a[0x708U] != 0x01A2U ||
        terrain_a.back() != 0x021FU || terrain_b.front() != 0x020EU ||
        terrain_b[0x708U] != 0x020EU || terrain_b.back() != 0xA133U) {
        return 7;
    }
    const auto& terrain_stream = scene2.terrain_stream_source;
    if (terrain_stream.size() != kScene2TerrainStreamByteCount ||
        terrain_stream[0] != 0x02U || terrain_stream[1] != 0x01U ||
        terrain_stream[2] != 0x02U || terrain_stream[3] != 0x02U ||
        terrain_stream[terrain_stream.size() - 2U] != 0x02U ||
        terrain_stream.back() != 0x62U) {
        return 8;
    }
    const auto& descriptor = scene2.descriptor;
    if (descriptor.initial_camera_x_fixed != 0x3400U ||
        descriptor.initial_camera_y_fixed != 0x2280U ||
        descriptor.initial_field_records[0].x != 0x3CE0U ||
        descriptor.initial_field_records[0].y != 0x2D00U ||
        descriptor.initial_field_records[1].x != 0x3BA0U ||
        descriptor.initial_field_records[1].y != 0x2D00U ||
        descriptor.initial_field_records[2].x != 0x3DA0U ||
        descriptor.initial_field_records[2].y != 0x2D00U ||
        descriptor.terrain_x_exclusive_upper != 0x0754U ||
        descriptor.terrain_y_exclusive_upper != 0x0518U ||
        descriptor.terrain_x_inclusive_lower != 0x002CU ||
        descriptor.terrain_y_inclusive_lower != 0x002CU ||
         !descriptor.event_table_available ||
         descriptor.event_mode_table[0] != 0x14U ||
        descriptor.event_mode_table[1] != 0x1AU) {
        return 9;
    }
    const auto& mode0_descriptor =
        source.assets->field_scene_resources[0U]->descriptor;
    if (mode0_descriptor.initial_camera_x_fixed != 0x0A00U ||
        mode0_descriptor.initial_camera_y_fixed != 0x0280U ||
        mode0_descriptor.initial_field_records[0].x != 0x1310U ||
        mode0_descriptor.initial_field_records[0].y != 0x0980U ||
        mode0_descriptor.initial_field_records[1].x != 0x1220U ||
        mode0_descriptor.initial_field_records[1].y != 0x08C0U ||
        mode0_descriptor.initial_field_records[2].x != 0x1400U ||
        mode0_descriptor.initial_field_records[2].y != 0x08C0U ||
        mode0_descriptor.terrain_x_exclusive_upper != 0x0770U ||
        mode0_descriptor.terrain_y_exclusive_upper != 0x03BCU ||
        mode0_descriptor.terrain_x_inclusive_lower != 0x000CU ||
        mode0_descriptor.terrain_y_inclusive_lower != 0x0020U ||
        mode0_descriptor.event_mode_table[0] != 0x15U ||
        mode0_descriptor.event_mode_table[1] != 0x0CU) {
        return 12;
    }
    const auto& mode0_collision_rectangles =
        source.assets->field_scene_resources[0U]->terrain_collision_rectangles;
    if (mode0_collision_rectangles.size() !=
            kScene0TerrainCollisionRectangleCount ||
        mode0_collision_rectangles.front().left != 0x0000U ||
        mode0_collision_rectangles.front().right != 0x005FU ||
        mode0_collision_rectangles.front().top != 0x0000U ||
        mode0_collision_rectangles.front().bottom != 0x007FU ||
        mode0_collision_rectangles.front().value != 0x7FU ||
        mode0_collision_rectangles.back().left != 0x0740U ||
        mode0_collision_rectangles.back().right != 0x075FU ||
        mode0_collision_rectangles.back().top != 0x0360U ||
        mode0_collision_rectangles.back().bottom != 0x037FU ||
        mode0_collision_rectangles.back().value != 0xDFU ||
         source.assets->field_scene_resources[0U]
                 ->terrain_collision_map[0U] != 0x7FU) {
        return 13;
    }
    const auto& movement_offsets =
        source.assets->field_movement_direction_offsets;
    const std::array<MovementOffset, kFieldMovementDirectionCount>
        expected_movement_offsets{{
            {0, -40}, {40, -40}, {40, 0}, {40, 40},
            {0, 40}, {-40, 40}, {-40, 0}, {-40, -40}}};
    for (std::size_t index = 0; index < movement_offsets.size(); ++index) {
        if (movement_offsets[index].x != expected_movement_offsets[index].x ||
            movement_offsets[index].y != expected_movement_offsets[index].y) {
            return 14;
        }
    }
    const auto& collision_probe_banks =
        source.assets->field_collision_probe_offset_banks;
    const CollisionProbeOffsetBank expected_collision_probe_banks{{
        {{{0, -320}, {-320, -320}, {320, -320}}},
        {{{480, 0}, {320, 320}, {320, -320}}},
        {{{0, 320}, {320, 320}, {-320, 320}}},
        {{{-480, 0}, {-256, 320}, {-256, -256}}}}};
    for (std::size_t bank = 0; bank < collision_probe_banks.size(); ++bank) {
        for (std::size_t point = 0;
             point < collision_probe_banks[bank].size(); ++point) {
            if (collision_probe_banks[bank][point].x !=
                    expected_collision_probe_banks[bank][point].x ||
                collision_probe_banks[bank][point].y !=
                    expected_collision_probe_banks[bank][point].y) {
                return 17;
            }
        }
    }
    const std::array<std::uint16_t, kFieldControllerHeadingTableCount>
        expected_heading_table{{
            0xFFFFU, 0x0000U, 0x8000U, 0xFFFFU,
            0xC000U, 0xE000U, 0xA000U, 0xFFFFU,
            0x4000U, 0x2000U, 0x6000U, 0xFFFFU,
            0xFFFFU, 0xFFFFU, 0xFFFFU, 0xFFFFU}};
    for (std::size_t index = 0; index < expected_heading_table.size();
         ++index) {
        if (source.assets->field_controller_heading_table[index] !=
            expected_heading_table[index]) {
            return 15;
        }
    }
    const std::array<FieldMovementRouteSourceRecord,
                     kFieldMovementRouteCount>
        expected_routes{{
            {0U, 1U, 0x0006U, 0x0002U},
            {1U, 3U, 0x0000U, 0x0004U},
            {2U, 3U, 0x0006U, 0x0002U},
            {0U, 2U, 0x0000U, 0x0004U},
        }};
    for (std::size_t index = 0; index < expected_routes.size(); ++index) {
        const auto& actual = source.assets->field_movement_routes[index];
        const auto& expected = expected_routes[index];
        if (actual.primary_sample_index != expected.primary_sample_index ||
            actual.secondary_sample_index != expected.secondary_sample_index ||
            actual.primary_fallback_slot != expected.primary_fallback_slot ||
            actual.secondary_fallback_slot != expected.secondary_fallback_slot) {
            return 16;
        }
    }
    const auto& collision_rectangles = scene2.terrain_collision_rectangles;
    if (collision_rectangles.size() !=
            kScene2TerrainCollisionRectangleCount ||
        collision_rectangles.front().left != 0x0420U ||
        collision_rectangles.front().right != 0x043FU ||
        collision_rectangles.front().top != 0x0020U ||
        collision_rectangles.front().bottom != 0x007FU ||
        collision_rectangles.front().value != 0x7FU ||
        collision_rectangles.back().left != 0x0200U ||
        collision_rectangles.back().right != 0x021FU ||
        collision_rectangles.back().top != 0x0080U ||
        collision_rectangles.back().bottom != 0x009FU ||
        collision_rectangles.back().value != 0xDFU ||
         scene2.terrain_collision_map[97U] != 0x7FU) {
        return 10;
    }
    const auto scene_input_result = make_field_scene_input(*source.assets, 2U);
    if (!scene_input_result.has_value()) {
        return 11;
    }
    const auto& scene_input = *scene_input_result;
    if (scene_input.scene_reference != 2U ||
        !scene_input.scene_profile_table_available ||
        scene_input.scene_profiles != source.assets->scene_profile_table_source ||
        scene_input.layer_a != scene2.terrain_layer_a_source ||
        scene_input.layer_b != scene2.terrain_layer_b_source ||
        scene_input.terrain_stream != scene2.terrain_stream_source ||
        !scene_input.source_metadata.available ||
        scene_input.source_metadata.initial_camera_x_fixed != 0x3400U ||
        scene_input.source_metadata.initial_field_records[0].x != 0x3CE0U ||
        scene_input.source_metadata.initial_field_records[0].y != 0x2D00U ||
        scene_input.source_metadata.terrain_collision_rectangles.size() !=
            kScene2TerrainCollisionRectangleCount ||
        scene_input.source_metadata.terrain_collision_map[97U] != 0x7FU ||
        scene_input.source_metadata.world_activity_random_index_bound ||
         !scene_input.source_metadata.event_table_available ||
         scene_input.source_metadata.event_mode_table[0] != 0x14U) {
        return 11;
    }
    const auto scene_activity_input =
        make_field_scene_input(*source.assets, 0x1AU);
    if (!scene_activity_input.has_value() ||
        scene_activity_input->source_metadata
                .world_activity_random_index_bound != 3U) {
        return 24;
    }
    return 0;
}

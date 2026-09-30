#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(scene_uses_alternate_profile(6) &&
                scene_uses_alternate_profile(9) &&
                scene_uses_alternate_profile(0x2C) &&
                scene_uses_alternate_profile(0x31) &&
                !scene_uses_alternate_profile(5) &&
                !scene_uses_alternate_profile(10) &&
                !scene_uses_alternate_profile(0x2B) &&
                !scene_uses_alternate_profile(0x32) &&
                !scene_uses_alternate_profile(-1))) {
        return 1;
    }

    const SceneProfileDefaults defaults{
        0x1111U, 0x2222U, {{{0x0100U, 0x0200U}, {0x0300U, 0x0400U},
                            {0x0500U, 0x0600U}}}};
    SceneProfileState state{0U, 0U, {}, 0x1234, 0x5678};
    std::optional<std::array<std::uint16_t, 8>> no_pending_profile;
    apply_scene_profile_selection(state, no_pending_profile, defaults);
    if (!expect(state.camera_x == 0x1111U && state.camera_y == 0x2222U &&
                state.runner_positions[2].y == 0x0600U &&
                state.camera_transition_x == 0x1234 &&
                state.camera_transition_y == 0x5678)) {
        return 2;
    }

    auto pending_profile = std::optional<std::array<std::uint16_t, 8>>{
        std::array<std::uint16_t, 8>{0x12FFU, 0x23FFU, 0x1001U, 0x1002U,
                                     0x2001U, 0x2002U, 0x3001U, 0x3002U}};
    apply_scene_profile_selection(state, pending_profile, defaults);
    if (!expect(state.camera_x == 0x1280U && state.camera_y == 0x2380U &&
                state.runner_positions[0].x == 0x1001U &&
                state.runner_positions[2].y == 0x3002U &&
                state.camera_transition_x == -1 &&
                state.camera_transition_y == -1 && !pending_profile.has_value())) {
        return 3;
    }

    SceneResourceCursorState cursor{0, 0};
    copy_scene_resource_cursor(cursor, SceneResourceProfileFields{-3, 0x4567U});
    if (!expect(cursor.cursor == -3 && cursor.resource_index == 0x4567U)) {
        return 4;
    }

    SceneRecordPlacement source_record{0x0100, -0x20, 0, 0};
    SceneRecordPlacement destination_record{0, 0, 0, 0x04U};
    if (!expect(spawn_scene_record_offset(
                    destination_record, source_record, 7,
                    [] { return false; }) &&
                destination_record.x == 0x0180 &&
                destination_record.y == 0x0060 &&
                destination_record.kind == 7 &&
                destination_record.flags == 0x05U)) {
        return 5;
    }
    if (!expect(!spawn_scene_record_offset(
                    destination_record, source_record, 9,
                    [] { return true; }))) {
        return 6;
    }

    if (!expect(field_scene_profile_table_entry(2) == 0x00172C80U &&
                field_scene_profile_table_entry(0) == 0x000D5456U &&
                field_scene_profile_table_entry(0x38U) == 0U)) {
        return 7;
    }

    std::array<std::uint16_t, kFieldSceneProfileWordCount> source{};
    source[12] = 0x0018U;
    source[13] = 0x8AFDU;
    source[0] = 0x3400U;
    source[1] = 0x2280U;
    FieldSceneLoopState loop{};
    loop.scene_id = 2;
    run_field_loop(loop, source);
    if (!expect(loop.profile[0] == 0x3400U && loop.profile[12] == 0x0018U &&
                loop.profile[46] == 0x0018U && loop.profile[47] == 0x8AFDU &&
                loop.override_lock == 0U)) {
        return 8;
    }

    loop = FieldSceneLoopState{};
    loop.alternate_profile = 1;
    loop.override_lock = 1;
    loop.override_x = 0x0B00U;
    loop.override_y = 0x0800U;
    run_field_loop(loop, source);
    if (!expect(loop.profile[0] == 0x3400U && loop.override_lock == 0U)) {
        return 9;
    }

    loop = FieldSceneLoopState{};
    loop.alternate_profile = 1;
    loop.override_x = 0x0B00U;
    loop.override_y = 0x0800U;
    run_field_loop(loop, source);
    if (!expect(loop.profile[0] == 0x0100U && loop.profile[1] == 0x0100U &&
                loop.profile[33] == 0x0B00U && loop.profile[34] == 0x0800U &&
                loop.profile[37] == 0x0B00U && loop.profile[38] == 0x0800U)) {
        return 10;
    }

    loop = FieldSceneLoopState{};
    loop.alternate_profile = 1;
    loop.override_x = 0x0900U;
    loop.override_y = 0x0600U;
    run_field_loop(loop, source);
    if (!expect(loop.profile[0] == 0U && loop.profile[1] == 0U)) {
        return 11;
    }

    std::vector<std::uint32_t> copied;
    if (!expect(copy_terrain_layers(
                    0x00FFDDDAU, 0x00FFDDE2U,
                    [&](const std::uint32_t source_pointer,
                        const std::uint32_t destination,
                        const std::size_t words) {
                        copied.push_back(source_pointer);
                        copied.push_back(destination);
                        copied.push_back(words);
                    }) == 0x1C20U &&
                copied == std::vector<std::uint32_t>{
                              0x00FFDDDAU, 0x00FF2700U, 0x0E10U, 0x00FFDDE2U,
                              0x00FF4700U, 0x0E10U})) {
        return 12;
    }

    FieldSceneVariantState variant{};
    variant.field_mode = 2;
    variant.scene_id = 2;
    variant.camera_x = 0x1000U;
    if (!expect(!toggle_field_scene_variant(variant) &&
                variant.scene_id == 2 && !variant.pending_profile.has_value())) {
        return 13;
    }

    variant.field_mode = 0x10U;
    variant.scene_id = 0;
    variant.camera_x = 0x13FFU;
    if (!expect(toggle_field_scene_variant(variant) && variant.scene_id == 3U &&
                variant.pending_profile == kFieldSceneVariantProfile0x3222)) {
        return 14;
    }

    variant.scene_id = 0;
    variant.camera_x = 0x1400U;
    if (!expect(toggle_field_scene_variant(variant) && variant.scene_id == 5U &&
                variant.pending_profile == kFieldSceneVariantProfile0x3212)) {
        return 15;
    }

    variant.scene_id = 5U;
    if (!expect(toggle_field_scene_variant(variant) && variant.scene_id == 0U &&
                variant.pending_profile == kFieldSceneVariantProfile0x31F2)) {
        return 16;
    }

    variant.scene_id = 3U;
    if (!expect(toggle_field_scene_variant(variant) && variant.scene_id == 0U &&
                variant.pending_profile == kFieldSceneVariantProfile0x3202)) {
        return 17;
    }

    SceneProfileState applied{};
    apply_scene_profile_selection(applied, variant.pending_profile, defaults);
    if (!expect(applied.camera_x == 0x0000U && applied.camera_y == 0x2380U &&
                applied.runner_positions[0].x == 0x0300U &&
                applied.runner_positions[2].y == 0x2B80U &&
                !variant.pending_profile.has_value())) {
        return 18;
    }

    std::array<std::uint16_t, kFieldSceneRuntimeStateWordCount> backup{};
    backup[0] = 2U;
    backup[1] = 0x3400U;
    backup[2] = 0x2280U;
    backup[48] = 0xABCDU;
    FieldSceneRuntimeState runtime{};
    FieldSceneRuntimeInput input{};
    input.prepared_camera_x = 0x0A00U;
    input.prepared_camera_y = 0x0700U;
    input.source_runner_positions =
        {{{0x0100U, 0x0200U}, {0x0300U, 0x0400U}, {0x0500U, 0x0600U}}};
    std::vector<int> order;
    initialize_field_scene_state(
        runtime, backup, input,
        {[&order] { order.push_back(1); }, [&order] { order.push_back(2); },
         [&order] { order.push_back(3); }});
    if (!expect(runtime.scene_gate == 0U && runtime.fade_marker == -1 &&
                runtime.scene_byte == 2U && runtime.alternate_profile == 0U &&
                runtime.profile[0] == 2U && runtime.profile[1] == 0x3400U &&
                runtime.profile[2] == 0x2280U &&
                runtime.profile[48] == 0xABCDU &&
                runtime.camera_x == 0x0A00U && runtime.camera_y == 0x0700U &&
                runtime.runner_positions[2].y == 0x0600U &&
                order == std::vector<int>{1, 2, 3})) {
        return 19;
    }

    backup[0] = 6U;
    input = {};
    input.prepared_camera_x = 0x1000U;
    input.prepared_camera_y = 0x2000U;
    initialize_field_scene_state(runtime, backup, input, {});
    if (!expect(runtime.scene_byte == 6U && runtime.alternate_profile == 1U &&
                runtime.camera_x == 0x1000U &&
                runtime.camera_y == 0x2000U)) {
        return 20;
    }

    backup[0] = 5U;
    input = {};
    input.prepared_camera_x = 1U;
    input.prepared_camera_y = 2U;
    initialize_field_scene_state(runtime, backup, input, {});
    if (!expect(runtime.alternate_profile == 0U && runtime.scene_byte == 5U)) {
        return 21;
    }

    backup[0] = 0x2CU;
    input = {};
    input.prepared_camera_x = 3U;
    input.prepared_camera_y = 4U;
    input.source_runner_positions =
        {{{0x0100U, 0x0200U}, {0x0300U, 0x0400U}, {0x0500U, 0x0600U}}};
    input.world_effect_player_spawn_pending = true;
    input.pending_profile =
        std::array<std::uint16_t, 8>{0x12FFU, 0x23FFU, 0x1001U, 0x1002U,
                                     0x2001U, 0x2002U, 0x3001U, 0x3002U};
    std::size_t published_slot = 99U;
    SceneRunnerPosition published_position{};
    bool find_ready_called = false;
    initialize_field_scene_state(
        runtime, backup, input,
        { {}, {}, {},
          [&find_ready_called] {
              find_ready_called = true;
              return std::optional<std::size_t>{4U};
          },
          [&published_slot, &published_position](
              const std::size_t slot, const SceneRunnerPosition position) {
              published_slot = slot;
              published_position = position;
          } });
    if (!expect(runtime.alternate_profile == 1U &&
                runtime.scene_byte == 0x2CU && find_ready_called &&
                runtime.ready_entity_repositioned &&
                runtime.ready_entity_slot == std::optional<std::size_t>{4U} &&
                published_slot == 4U && published_position.x == 0x0180U &&
                published_position.y == 0x0200U &&
                runtime.camera_x == 0x1280U &&
                runtime.camera_y == 0x2380U &&
                runtime.runner_positions[0].x == 0x1001U &&
                runtime.runner_positions[2].y == 0x3002U &&
                runtime.camera_transition_x == -1 &&
                runtime.camera_transition_y == -1 &&
                runtime.pending_profile_consumed &&
                !input.pending_profile.has_value())) {
        return 22;
    }

    return 0;
}

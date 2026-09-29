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
    return 0;
}

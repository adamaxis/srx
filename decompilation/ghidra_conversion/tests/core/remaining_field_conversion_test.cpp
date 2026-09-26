#include "shadowrun_ghidra/core/remaining_field_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    TitleResourcePreparation title{};
    std::vector<std::uint16_t> uploads;
    prepare_title_resources(title, 1, 2, 3, 4, {0x100, 0x103, 0},
                            {7, 1, 2, 3, 4, 5, 6, 0},
                            [&](const TitleResourceTransfer& item) {
                                uploads.push_back(item.source_id);
                            },
                            {});
    if (title.transfers.size() != 2 || uploads[1] != 0x102 ||
        title.sprite_streams.size() != 1 || !title.display_dirty) {
        return 1;
    }

    std::vector<std::uint16_t> words;
    if (render_hud_sprite_stream({1, 0xC3, 2, 0}, 0, 4, 0x8000,
                                 { {}, [&](std::uint16_t value) {
                                       words.push_back(value);
                                   }, {} }) != 4 ||
        words.size() != 4) {
        return 2;
    }

    FieldModeTransitionState mode{0, 0, 0, 0, 0};
    int started = 0;
    advance_field_mode_transition(mode, {0x13}, {4}, {9, 10},
                                  [&] { ++started; }, {});
    if (mode.mode != 0x13 || started != 1 || mode.camera_x != 9) {
        return 3;
    }

    FieldSceneState scene{};
    initialize_field_scene_state(scene, {1, 2, 3}, 5, 10, 20,
                                 std::vector<std::uint16_t>{0x180, 0x200, 3, 4,
                                                            5, 6, 7, 8},
                                 {}, {}, {}, {});
    if (scene.profile_words.size() != 3 || scene.camera_x != 0x180 ||
        scene.entity_effect_y != 8 || !scene.pending_position_consumed) {
        return 4;
    }

    if (select_hud_panel_variant(false, 1).asset != 0x86BAU) {
        return 5;
    }
    EntityMeters meters{0, 0, 0, 3, 1, 0};
    int writes = 0;
    advance_entity_delay_meter(meters, 4, [&] { ++writes; });
    advance_entity_behavior_meter(meters, 8, [&] { ++writes; });
    if (meters.delay != 4 || meters.behavior != 8 || writes != 2) {
        return 6;
    }

    FieldCameraState camera{0, 0, -10, 30, -20, 40, 0xFFFFU, 0xFFFFU, 0};
    int edges = 0;
    update_field_camera(camera, 130, 0, [&](std::uint16_t) { ++edges; }, {}, {});
    if (camera.x != 30 || edges != 1) {
        return 7;
    }
    std::vector<std::uint16_t> rendered;
    render_field_tile_edge(3, 2, 0x8000, [](std::uint16_t key, std::size_t offset) {
        return static_cast<std::uint16_t>(key + offset);
    }, [&](std::uint16_t value) { rendered.push_back(value); }, {});
    if (rendered.size() != 2 || rendered[0] != static_cast<std::uint16_t>(3 ^ 0x8000)) {
        return 8;
    }
    return 0;
}

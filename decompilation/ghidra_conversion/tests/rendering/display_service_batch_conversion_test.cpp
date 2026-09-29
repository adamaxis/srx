#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> events;
    DisplayServiceState display{};
    display.dirty_flags = 0x0FU;
    display.render_normal = true;
    display.render_alternate = true;
    display.flush_primary = true;
    display.flush_secondary = true;
    service_display_frame(
        display,
        {[&] { events.push_back(1); }, [&] { events.push_back(2); },
         [&] { events.push_back(3); }, [&] { events.push_back(4); },
         [&] { events.push_back(5); }, [&] { events.push_back(6); },
         [&] { events.push_back(7); }, [&] { events.push_back(8); },
         [&] { events.push_back(9); }});
    if (display.frame_counter != 1 || display.dirty_flags != 0 ||
        events != std::vector<int>{1, 2, 4, 5, 6, 7, 8, 9}) {
        return 1;
    }
    service_display_frame_wrapper(display, {});
    if (display.frame_counter != 2) {
        return 2;
    }

    int pair_calls = 0;
    write_alternating_word_pair_wrapper([&] { ++pair_calls; });
    if (pair_calls != 1) {
        return 3;
    }

    FieldSceneBuffers scene{};
    FieldSceneBufferInput scene_input{};
    scene_input.normal_profile[0] = 0x0100;
    scene_input.normal_profile[1] = 0x0200;
    scene_input.normal_profile[3] = 12;
    scene_input.alternate_profile[0] = 0x0300;
    scene_input.alternate_profile[1] = 0x0400;
    scene_input.alternate_profile[3] = 34;
    scene_input.scene_reference = 0x2C;
    scene_input.terrain_stream = {0x12, 0x34, 0x56, 0x78};
    scene_input.pending_position_available = true;
    scene_input.pending_position = {0x0B00, 0x0800};
    scene_input.layer_a.assign(kFieldTerrainLayerWordCount, 1U);
    scene_input.layer_b.assign(kFieldTerrainLayerWordCount, 3U);
    scene_input.layer_a[1] = 2U;
    scene_input.layer_b[1] = 4U;
    int initialized = 0;
    prepare_field_scene_buffers(scene, scene_input,
                                {[&] { ++initialized; }, {},
                                 [] { return true; }});
    if (!scene.initialized || initialized != 1 || scene.scene_mode != 1 ||
        scene.field_resource_mode != 0x2C || scene.profile[3] != 34 ||
        scene.camera_x != 0x100 || scene.camera_y != 0x100 ||
        scene.terrain_stream != scene_input.terrain_stream ||
        scene.pending_field_targets[2].x != 0x0B00 ||
        scene.pending_field_targets[2].y != 0x0800 ||
        !scene.pending_position_consumed || !scene.pending_position_cleared ||
        scene.layer_a.size() != kFieldTerrainLayerWordCount ||
        scene.layer_b.size() != kFieldTerrainLayerWordCount ||
        scene.layer_a[0] != 1U || scene.layer_a[1] != 2U ||
        scene.layer_b[0] != 3U || scene.layer_b[1] != 4U) {
        return 4;
    }

    FieldSceneBuffers normal_scene{};
    FieldSceneBufferInput normal_input{};
    normal_input.normal_profile[0] = 0x1200;
    normal_input.normal_profile[1] = 0x2300;
    normal_input.scene_reference = 2;
    normal_input.pending_position_available = true;
    normal_input.pending_position_in_use = true;
    normal_input.layer_a.assign(kFieldTerrainLayerWordCount, 0U);
    normal_input.layer_b.assign(kFieldTerrainLayerWordCount, 0U);
    if (!prepare_field_scene_buffers(normal_scene, normal_input,
                                     {{}, [] { return false; }})) {
        return 5;
    }
    if (normal_scene.scene_mode != 0U ||
        normal_scene.field_resource_mode != 2U ||
        normal_scene.camera_x != 0x1200 || normal_scene.camera_y != 0x2300 ||
        normal_scene.pending_position_consumed ||
        !normal_scene.pending_position_cleared) {
        return 6;
    }

    FieldSceneBuffers incomplete_scene{};
    if (prepare_field_scene_buffers(incomplete_scene, {}, {}) ||
        incomplete_scene.initialized) {
        return 7;
    }

    std::vector<std::uint16_t> words;
    int transfer = 0;
    write_hud_panel_transfer(
        10,
        {[&]() -> std::optional<std::size_t> {
             ++transfer;
             return transfer <= 2 ? std::optional<std::size_t>(transfer) :
                                    std::nullopt;
         },
         [&words](const std::uint16_t word) { words.push_back(word); }});
    if (words != std::vector<std::uint16_t>{10, 11, 12} || transfer != 3) {
        return 6;
    }

    std::vector<std::uint16_t> column_words;
    const auto column_count = render_field_tile_column(
        {127, 4, 0x8000, 0},
        {[&] {}, [](const std::uint16_t key, const std::size_t offset) {
             return static_cast<std::uint16_t>(key + offset);
         },
         [&column_words](const std::uint16_t word) { column_words.push_back(word); }});
    if (column_count != 44 || column_words.size() != 44U ||
        column_words.front() != static_cast<std::uint16_t>(0x8004U)) {
        return 7;
    }

    std::vector<std::uint16_t> split_column_words;
    int split_transfers = 0;
    const auto split_column_count = render_field_tile_column(
        {118, 4, 0x0000, 0},
        {[&] { ++split_transfers; },
         [](const std::uint16_t key, const std::size_t offset) {
             return static_cast<std::uint16_t>(key + offset);
         },
         [&split_column_words](const std::uint16_t word) {
             split_column_words.push_back(word);
         }});
    if (split_column_count != 44 || split_column_words.size() != 44U ||
        split_transfers != 2 || split_column_words.front() != 4U) {
        return 8;
    }

    int rendered_rows = 0;
    render_field_tile_rows(
        0x0010, 0x2345,
        {[&] { ++rendered_rows; }, [] { return 8; },
         [&rendered_rows](const std::int16_t, const std::uint16_t,
                          const std::int16_t) { ++rendered_rows; }});
    if (rendered_rows != 32) {
        return 9;
    }

    std::vector<std::uint16_t> map(17U, 0U);
    map[16] = 0x0301;
    if (calculate_field_tile_source_offset({0, 1}, 1, 0, map).value_or(-1) !=
        0x2000) {
        return 10;
    }

    ControllerDelayState delay{0x40, {0, 0}, 0};
    if (!arm_controller_activity_delay(delay) || delay.delay != 0x3C) {
        return 11;
    }
    delay.entity_flags[0] = 1;
    if (arm_controller_activity_delay(delay)) {
        return 12;
    }

    events.clear();
    dispatch_entity_collision_behavior(
        0, true,
        {[&] { events.push_back(1); }, [&] { events.push_back(2); },
         [&] { events.push_back(3); }});
    if (events != std::vector<int>{1, 3}) {
        return 12;
    }

    TitleSpriteWaveState wave{0, 10, 20, 4, true};
    int sprites = 0;
    std::uint16_t first_x = 0;
    std::uint16_t first_y = 0;
    if (!advance_title_sprite_wave(
            wave, {1, 2, 3, 4},
            {[&](const std::uint16_t x, const std::uint16_t y,
                 const std::uint16_t offset, const std::uint32_t palette) {
                 ++sprites;
                 if (offset == 0U) {
                     first_x = x;
                     first_y = y;
                 }
                 if (palette != 2) {
                     sprites = -100;
                 }
             }}) ||
        wave.timer != 8 || sprites != 32 ||
        first_x != 6U || first_y != 20U) {
        return 13;
    }

    int callback_calls = 0;
    load_title_font([&] { ++callback_calls; });
    write_menu_grid_with_metadata([&] { ++callback_calls; });
    render_menu_grid_number_pair([&] { ++callback_calls; },
                                  [&] { ++callback_calls; });
    prepare_hotel_options_display([&] { ++callback_calls; }, [&] { ++callback_calls; },
                                  [&] { ++callback_calls; }, [&] { ++callback_calls; });
    render_attributes_skills_heading([&] { ++callback_calls; });
    if (callback_calls != 9) {
        return 14;
    }

    std::vector<std::uint16_t> random_bounds;
    std::vector<std::uint16_t> random_values{0U, 7U, 2U};
    std::size_t random_index = 0;
    int grids = 0;
    RandomGridEffectState grid_state{};
    RandomGridEffectPlan submitted_plan{};
    advance_random_grid_effect(
        grid_state,
        {[&](const std::uint16_t bound) {
             random_bounds.push_back(bound);
             return random_values[random_index++];
         },
         [&](const RandomGridEffectPlan& plan) {
             ++grids;
             submitted_plan = plan;
         }});
    write_menu_footer_grid([&] { ++grids; });
    write_menu_grid_quadruple([&] { ++grids; });
    if (grid_state.random_samples != 3U || random_bounds !=
            std::vector<std::uint16_t>{2U, 8U, 3U} || grids != 6 ||
        !grid_state.grid_write_requested || submitted_plan.first_selection_nonzero ||
        submitted_plan.primary_source != RandomGridPrimarySource::branch_zero_table ||
        submitted_plan.primary_entry_index != 7U ||
        submitted_plan.variant_source !=
            RandomGridVariantSource::fixed_three_variant_resources ||
        submitted_plan.variant_entry_index != 2U ||
        submitted_plan.grid_row_count != 2U) {
        return 15;
    }
    const auto metadata_grid = write_menu_grid_with_metadata();
    const auto footer_grid = write_menu_footer_grid();
    if (metadata_grid.destination != kMenuMetadataGridDestination ||
        metadata_grid.width != kMenuMetadataGridWidth ||
        metadata_grid.height != kMenuMetadataGridHeight ||
        footer_grid.destination != kMenuFooterGridDestination ||
        footer_grid.width != kMenuFooterGridWidth ||
        footer_grid.height != kMenuFooterGridHeight) {
        return 18;
    }
    const auto quad = write_menu_grid_quadruple(0);
    if (quad[0].destination != 0xC112U || quad[0].glyph_source != 0x000C8650U ||
        quad[0].height != 4U || quad[1].destination != 0xC292U ||
        quad[2].destination != 0xC192U || quad[2].width != 1U ||
        quad[3].destination != 0xC19AU ||
        quad[3].glyph_source != 0x000C8668U) {
        return 19;
    }
    const auto quad3 = write_menu_grid_quadruple(3);
    if (quad3[0].glyph_source != 0x000C86A4U ||
        write_menu_grid_quadruple(4)[0].glyph_source != 0U) {
        return 20;
    }

    random_bounds.clear();
    random_values = {1U, 3U, 3U};
    random_index = 0;
    advance_random_grid_effect(
        grid_state,
        {[&](const std::uint16_t bound) {
             random_bounds.push_back(bound);
             return random_values[random_index++];
         },
         [&](const RandomGridEffectPlan& plan) { submitted_plan = plan; }});
    if (random_bounds != std::vector<std::uint16_t>{2U, 4U, 4U} ||
        !submitted_plan.first_selection_nonzero ||
        submitted_plan.primary_source !=
            RandomGridPrimarySource::branch_nonzero_table ||
        submitted_plan.primary_entry_index != 3U ||
        submitted_plan.variant_source !=
            RandomGridVariantSource::indexed_four_variant_resources ||
        submitted_plan.variant_entry_index != 3U) {
        return 16;
    }
    return 0;
}

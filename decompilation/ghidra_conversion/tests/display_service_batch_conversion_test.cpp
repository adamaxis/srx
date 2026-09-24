#include "shadowrun_ghidra/display_service_batch_conversion.hpp"

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
    scene.recenter_pending = true;
    std::array<std::uint16_t, 48> normal{};
    std::array<std::uint16_t, 48> alternate{};
    normal[3] = 12;
    alternate[3] = 34;
    int initialized = 0;
    prepare_field_scene_buffers(
        scene, normal, alternate, 0x1234, 0x0B00, 0x0800, {1, 2}, {3, 4},
        {[&] { ++initialized; }, [] { return true; }});
    if (!scene.initialized || initialized != 1 || scene.scene_mode != 1 ||
        scene.profile[3] != 34 || scene.camera_x != 0x100 ||
        scene.camera_y != 0x100 || scene.layer_a != std::vector<std::uint16_t>{1, 2} ||
        scene.layer_b != std::vector<std::uint16_t>{3, 4}) {
        return 4;
    }

    int hook_calls = 0;
    run_hud_panel_post_hook([&] { ++hook_calls; });
    if (hook_calls != 1) {
        return 5;
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

    int rendered_rows = 0;
    render_field_tile_rows(
        0x0010, 0x2345,
        {[&] { ++rendered_rows; }, [] { return 8; },
         [&rendered_rows](const std::int16_t, const std::uint16_t,
                          const std::int16_t) { ++rendered_rows; }});
    if (rendered_rows != 32) {
        return 8;
    }

    std::vector<std::uint16_t> map(17U, 0U);
    map[16] = 0x0301;
    if (calculate_field_tile_source_offset({0, 1}, 1, 0, map).value_or(-1) !=
        0x2000) {
        return 9;
    }

    ControllerDelayState delay{0x40, {0, 0}, 0};
    if (!arm_controller_activity_delay(delay) || delay.delay != 0x3C) {
        return 10;
    }
    delay.entity_flags[0] = 1;
    if (arm_controller_activity_delay(delay)) {
        return 11;
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
    if (!advance_title_sprite_wave(
            wave, {1, 2, 3, 4},
            {[&](const std::uint16_t, const std::uint16_t,
                 const std::uint16_t, const std::uint32_t palette) {
                 ++sprites;
                 if (palette != 2) {
                     sprites = -100;
                 }
             }}) ||
        wave.timer != 8 || sprites != 32) {
        return 13;
    }

    int callback_calls = 0;
    load_title_font([&] { ++callback_calls; });
    write_menu_grid_with_metadata([&] { ++callback_calls; });
    render_menu_grid_number_pair([&] { ++callback_calls; },
                                  [&] { ++callback_calls; });
    prepare_title_mode_display([&] { ++callback_calls; }, [&] { ++callback_calls; },
                                [&] { ++callback_calls; }, [&] { ++callback_calls; });
    render_title_text_wrapper([&] { ++callback_calls; });
    if (callback_calls != 9) {
        return 14;
    }

    int samples = 0;
    int grids = 0;
    advance_random_grid_effect([&] { return static_cast<std::int16_t>(++samples); },
                               [&] { ++grids; });
    write_menu_footer_grid([&] { ++grids; });
    write_menu_grid_quadruple([&] { ++grids; });
    if (samples != 3 || grids != 6) {
        return 15;
    }
    return 0;
}

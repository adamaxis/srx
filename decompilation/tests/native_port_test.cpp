#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace {

std::vector<std::uint8_t> read_rom() {
    std::ifstream input(SHADOWRUN_TEST_ROM, std::ios::binary);
    assert(input);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

shadowrun::native::NativePortAssets native_assets_from_rom(
    const std::vector<std::uint8_t>& rom) {
    return shadowrun::graphics::decode_genesis_native_assets(rom);
}

bool cab_cursor_tile_matches(const shadowrun::native::SoftwareScreen& actual,
                             const std::int32_t x, const std::int32_t y,
                             const shadowrun::graphics::RgbaImage& tile) {
    using shadowrun::native::Rgba8;
    using shadowrun::native::SoftwareScreen;
    SoftwareScreen expected;
    expected.clear(Rgba8{0, 0, 0, 0xFF});
    expected.blit_rgba(x, y, tile.width, tile.height, tile.pixels);
    std::size_t visible_pixels = 0;
    for (std::int32_t tile_y = 0; tile_y < tile.height; ++tile_y) {
        for (std::int32_t tile_x = 0; tile_x < tile.width; ++tile_x) {
            const auto& pixel = tile.pixels[
                static_cast<std::size_t>(tile_y) * tile.width + tile_x];
            if (pixel.alpha == 0U) {
                continue;
            }
            ++visible_pixels;
            if (actual.pixel(x + tile_x, y + tile_y) !=
                expected.pixel(x + tile_x, y + tile_y)) {
                return false;
            }
        }
    }
    return visible_pixels != 0U;
}

bool thin_gold_inventory_selector_matches(
    const shadowrun::native::SoftwareScreen& actual,
    const std::int32_t left, const std::int32_t top) {
    constexpr std::uint32_t kSelectorGold = 0x00C4BE27U;
    constexpr std::int32_t kWidth = 28;
    constexpr std::int32_t kHeight = 20;
    for (std::int32_t y = 0; y < kHeight; ++y) {
        for (std::int32_t x = 0; x < kWidth; ++x) {
            const bool edge = x == 0 || x == kWidth - 1 ||
                              y == 0 || y == kHeight - 1;
            if ((actual.pixel(left + x, top + y) == kSelectorGold) != edge) {
                return false;
            }
        }
    }
    return true;
}

void enter_field_from_start(shadowrun::native::NativePortShell& shell) {
    using namespace shadowrun::native;
    shell.finish_boot();
    shell.update(InputState{button_mask(Button::Start),
                             button_mask(Button::Start)});
    for (int frame = 0; frame < 48 && shell.scene() == NativeScene::Title;
         ++frame) {
        shell.update(InputState{});
    }
    assert(shell.scene() == NativeScene::StartOptions);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::Intro);
    shell.update(InputState{button_mask(Button::Start),
                             button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::Field);
}

void enter_intro_from_start(shadowrun::native::NativePortShell& shell) {
    using namespace shadowrun::native;
    shell.finish_boot();
    shell.update(InputState{button_mask(Button::Start),
                             button_mask(Button::Start)});
    for (int frame = 0; frame < 48 && shell.scene() == NativeScene::Title;
         ++frame) {
        shell.update(InputState{});
    }
    assert(shell.scene() == NativeScene::StartOptions);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::Intro);
}

void press_menu_direction(shadowrun::native::NativePortShell& shell,
                          shadowrun::native::Button button) {
    const auto mask = shadowrun::native::button_mask(button);
    shell.update(shadowrun::native::InputState{mask, mask});
    // The Genesis input sampler reports an edge only when the button was
    // released between menu steps. This also makes the test independent of
    // the held-repeat delay used by the native menu loop.
    shell.update(shadowrun::native::InputState{});
}

} // namespace

int main() {
    using namespace shadowrun::native;

    const auto decoded_input = decode_genesis_controller_word(
        static_cast<std::uint16_t>(
            kGenesisInputUp | kGenesisInputA |
            static_cast<std::uint16_t>(kGenesisInputC << 8U) |
            static_cast<std::uint16_t>(kGenesisInputStart << 8U)));
    assert(decoded_input.is_held(Button::Up));
    assert(decoded_input.is_held(Button::A));
    assert(!decoded_input.is_held(Button::C));
    assert(decoded_input.was_pressed(Button::C));
    assert(decoded_input.was_pressed(Button::Start));
    assert(!decoded_input.was_pressed(Button::A));

    const auto decoded_b = decode_genesis_controller_word(
        static_cast<std::uint16_t>(kGenesisInputB << 8U));
    assert(decoded_b.was_pressed(Button::B));
    assert(!decoded_b.was_pressed(Button::C));

    SoftwareScreen screen;
    NativePortShell shell("SHADOWRUN");
    shell.render(screen);
    assert(screen.presented_frames() == 1);
    assert(screen.pixel(319, 223) == 0x00060818U);
    assert(screen.pixel(20, 20) != 0x00060818U);

    const auto start_x = shell.marker_x();
    const auto start_y = shell.marker_y();
    shell.update(InputState{
        button_mask(Button::Right) | button_mask(Button::Down), 0});
    assert(shell.marker_x() == start_x + 2);
    assert(shell.marker_y() == start_y + 2);

    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    shell.render(screen);
    assert(shell.frame_number() == 2);
    assert(screen.presented_frames() == 2);

    shell.update(InputState{
        button_mask(Button::Start), button_mask(Button::Start)});
    assert(shell.marker_x() == kScreenWidth / 2);
    assert(shell.marker_y() == kScreenHeight / 2);

    const auto rom = read_rom();
    NativePortShell rom_shell("SHADOWRUN", native_assets_from_rom(rom));
    assert(rom_shell.has_assets());
    assert(rom_shell.has_field_map());
    assert(rom_shell.has_terrain());
    assert(rom_shell.has_entity_frame());
    assert(rom_shell.has_boot());
    assert(rom_shell.scene() == NativeScene::Bootstrap);
    rom_shell.finish_boot();
    assert(rom_shell.scene() == NativeScene::Title);
    rom_shell.render(screen);
    assert(screen.presented_frames() == 3);

    // 0x1A9E's first Start edge fades via 0x131C into 0x59092. Holding Start
    // without a pressed edge must not skip the fade or jump to the field.
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::Title);
    for (int frame = 0; frame < 48 &&
         rom_shell.scene() == NativeScene::Title; ++frame) {
        rom_shell.update(InputState{});
    }
    assert(rom_shell.scene() == NativeScene::StartOptions);
    assert(rom_shell.start_options_selection() == 0);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    assert(rom_shell.scene() == NativeScene::ArchetypeMenu);
    assert(rom_shell.archetype_selection() == 0);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    assert(rom_shell.scene() == NativeScene::Intro);
    assert(rom_shell.intro_frame() == 0);
    assert(rom_shell.intro_story_line_count() == 22);
    assert(rom_shell.intro_line_start() == 0);
    // The native surface runs at 60 Hz, while intro_frame is the ROM's
    // 30 Hz source-pass clock. The first host tick holds source pass zero.
    rom_shell.update(InputState{});
    rom_shell.update(InputState{});
    assert(rom_shell.intro_frame() == 1);
    assert(rom_shell.scene() == NativeScene::Intro);

    // The wilderness loop is the ROM's 0x5A1EA pass, not a storyboard
    // sequence.  The pan plate initially contains the runners; the source
    // map/body phases then replace the lower-right and Indian body regions,
    // and the late pass changes Michael to the trivid-left pose.
    NativePortShell ambush_shell("SHADOWRUN", native_assets_from_rom(rom));
    enter_intro_from_start(ambush_shell);
    while (ambush_shell.intro_frame() < 0x308U) {
        ambush_shell.update(InputState{});
    }
    SoftwareScreen ambush_initial;
    ambush_shell.render(ambush_initial);
    const auto count_ink = [](const SoftwareScreen& image,
                              std::int32_t left, std::int32_t top,
                              std::int32_t width, std::int32_t height) {
        std::size_t count = 0;
        for (std::int32_t y = top; y < top + height; ++y) {
            for (std::int32_t x = left; x < left + width; ++x) {
                if (image.pixel(x, y) != 0U) {
                    ++count;
                }
            }
        }
        return count;
    };
    // 0x59D7C scrolls Plane A left while the 0x1DF1B2 head SAT advances from
    // (0x138,0xF1) to (0x178,0xD1).  The source body/head pair must therefore
    // already occupy one connected composition on the first pan frame; a
    // detached component here is the regression the video exposed.
    while (rom_shell.intro_frame() < 0x267U) {
        rom_shell.update(InputState{});
    }
    SoftwareScreen pan_open;
    rom_shell.render(pan_open);
    assert(count_ink(pan_open, 160, 56, 80, 72) > 500U);
    assert(count_ink(pan_open, 240, 56, 80, 72) < 10U);

    assert(count_ink(ambush_initial, 160, 148, 88, 56) > 200U);
    while (ambush_shell.intro_frame() < 0x3E8U) {
        ambush_shell.update(InputState{});
    }
    SoftwareScreen ambush_after_deaths;
    ambush_shell.render(ambush_after_deaths);
    assert(count_ink(ambush_after_deaths, 160, 148, 88, 56) < 40U);
    while (ambush_shell.intro_frame() < 0x4E8U) {
        ambush_shell.update(InputState{});
    }
    SoftwareScreen michael_slain;
    ambush_shell.render(michael_slain);
    // The terminal 0x10 ambush passes hand off to 0x5AC0E. The standing
    // Michael region is cleared and the source trivid-left pose remains.
    // The source trivid_left_0_0 resource is a 52x43 sprite at (51,148).
    // Do not turn this into a large guessed presentation plate just to make
    // a pixel-count threshold pass.
    assert(count_ink(michael_slain, 51, 148, 52, 43) > 20U);
    assert(count_ink(michael_slain, 0, 112, 36, 112) < 10U);
    // 0x5A1EA consumes the 0x1F0 ambush passes, then 0x5AA96 starts the
    // separate 0x114-pass trivid loop. The native terminal hand-off exposes
    // the same 0x5AC0E slain pose before the black/cockpit setup plate.
    while (ambush_shell.intro_frame() < 0x4F9U) {
        ambush_shell.update(InputState{});
    }
    SoftwareScreen ambush_late;
    ambush_shell.render(ambush_late);
    std::size_t michael_transition_delta = 0;
    for (std::int32_t y = 128; y < 224; ++y) {
        for (std::int32_t x = 0; x < 152; ++x) {
            if (ambush_after_deaths.pixel(x, y) != ambush_late.pixel(x, y)) {
                ++michael_transition_delta;
            }
        }
    }
    assert(michael_transition_delta > 20U);

    // 0x5AA96's first pass clears the old plane and draws trivid-left before
    // 0x1728 uploads the cockpit map. The source 0x12F6 setup wait keeps that
    // black/slain state visible for 0x11*2 source frame advances; a test that
    // only checks the eventual cockpit would miss that transition ordering.
    SoftwareScreen trivid_black;
    ambush_shell.render(trivid_black);
    assert(count_ink(trivid_black, 0, 0, 320, 224) > 20U);
    assert(trivid_black.pixel(0, 112) == 0U);
    constexpr auto kTrividMapSetupFrames = 0x11U * 2U;
    while (ambush_shell.intro_frame() <
           0x4F9U + 1U + kTrividMapSetupFrames) {
        ambush_shell.update(InputState{});
    }
    SoftwareScreen trivid_cockpit;
    ambush_shell.render(trivid_cockpit);
    // The native layer preserves transparent name-table holes so the moving
    // SAT list can remain behind the cockpit. Verify the cockpit by its
    // framebuffer ink, not by assuming the upper-left hole is opaque black.
    assert(count_ink(trivid_cockpit, 0, 0, 320, 224) > 1000U);

    // The airport is source-timed, not a handful of storyboard stills. At
    // the craft pass the Plane-A body and the off-screen-preserved rotor strip
    // must both reach the framebuffer. After the city settles, the late
    // 0x5B1B6 SAT movement must still change pixels; the old $150 cutoff
    // rendered a static skyline through this part of the ROM loop.
    const auto extracted_cab_cursor =
        shadowrun::graphics::decode_genesis_cab_cursor_tiles(rom);
    NativePortShell intro_shell("SHADOWRUN", native_assets_from_rom(rom));
    enter_intro_from_start(intro_shell);
    while (intro_shell.intro_frame() < 1614U) {
        intro_shell.update(InputState{});
    }
    SoftwareScreen rotor_before;
    intro_shell.render(rotor_before);
    while (intro_shell.intro_frame() < 1620U) {
        intro_shell.update(InputState{});
    }
    SoftwareScreen aircraft_screen;
    intro_shell.render(aircraft_screen);
    std::size_t rotor_ink = 0;
    std::size_t body_ink = 0;
    for (std::int32_t y = 90; y < 116; ++y) {
        for (std::int32_t x = 0; x < kScreenWidth; ++x) {
            if (aircraft_screen.pixel(x, y) != 0U) {
                ++rotor_ink;
            }
        }
    }
    for (std::int32_t y = 116; y < 184; ++y) {
        for (std::int32_t x = 0; x < kScreenWidth; ++x) {
            if (aircraft_screen.pixel(x, y) != 0U) {
                ++body_ink;
            }
        }
    }
    assert(rotor_ink > 20U);
    assert(body_ink > 200U);
    std::size_t rotor_phase_delta = 0;
    for (std::int32_t y = 90; y < 116; ++y) {
        for (std::int32_t x = 0; x < kScreenWidth; ++x) {
            if (rotor_before.pixel(x, y) != aircraft_screen.pixel(x, y)) {
                ++rotor_phase_delta;
            }
        }
    }
    assert(rotor_phase_delta > 10U);
    while (intro_shell.intro_frame() < 2300U) {
        intro_shell.update(InputState{});
    }
    SoftwareScreen landing_before;
    intro_shell.render(landing_before);
    while (intro_shell.intro_frame() < 2340U) {
        intro_shell.update(InputState{});
    }
    SoftwareScreen landing_after;
    intro_shell.render(landing_after);
    std::size_t landing_delta = 0;
    for (std::int32_t y = 80; y < 184; ++y) {
        for (std::int32_t x = 0; x < kScreenWidth; ++x) {
            if (landing_before.pixel(x, y) != landing_after.pixel(x, y)) {
                ++landing_delta;
            }
        }
    }
    assert(intro_shell.scene() == NativeScene::Intro);
    assert(landing_delta > 20U);

    // 0x13726 consumes a five-state shrinking cursor for each cab-map
    // destination. The second destination restarts at its large outline;
    // it must not inherit the terminal zero-size state from Downtown.
    while (intro_shell.scene() == NativeScene::Intro) {
        intro_shell.update(InputState{});
    }
    assert(intro_shell.scene() == NativeScene::CabMap);
    SoftwareScreen downtown_map;
    intro_shell.render(downtown_map);
    assert(cab_cursor_tile_matches(downtown_map, 32, 32,
                                   extracted_cab_cursor.tiles[0]));
    for (std::size_t frame = 0; frame < 5U * 6U; ++frame) {
        intro_shell.update(InputState{});
    }
    assert(intro_shell.scene() == NativeScene::CabMap);
    SoftwareScreen redmond_map;
    intro_shell.render(redmond_map);
    std::size_t cab_destination_delta = 0;
    for (std::int32_t y = 0; y < kScreenHeight; ++y) {
        for (std::int32_t x = 0; x < kScreenWidth; ++x) {
            if (downtown_map.pixel(x, y) != redmond_map.pixel(x, y)) {
                ++cab_destination_delta;
            }
        }
    }
    assert(cab_destination_delta > 200U);
    for (std::size_t frame = 0; frame < 5U * 6U; ++frame) {
        intro_shell.update(InputState{});
    }
    SoftwareScreen redmond_map_final;
    intro_shell.render(redmond_map_final);
    // The final Redmond outline uses D4464 position 0x03AE relative to
    // Plane-A base 0xC208: (216,88) on the host frame, with no map offset.
    assert(cab_cursor_tile_matches(redmond_map_final, 216, 88,
                                   extracted_cab_cursor.tiles[0]));

    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::Field);
    assert(rom_shell.field_entity_visible());
    rom_shell.render(screen);
    // HUD is the right 64px panel. The playfield (x<256) must not be covered
    // by the 64x80 command portrait that used to sit at (256,8).
    bool hud_ink = false;
    for (std::int32_t y = 0; y < kScreenHeight && !hud_ink; ++y) {
        for (std::int32_t x = kFieldPlayfieldWidth; x < kScreenWidth; ++x) {
            if (screen.pixel(x, y) != 0U) {
                hud_ink = true;
                break;
            }
        }
    }
    assert(hud_ink);
    assert(screen.pixel(200, 40) != 0U || screen.pixel(120, 80) != 0U);
    // Mode 2's +0x42/+0x44 pair is copied by ROM 0x2B34 into the first
    // field record. 0x5A62 writes VDP coordinates with a 0x80 origin; the
    // native accessor exposes the adapted host pixels.
    assert(rom_shell.field_entity_screen_x() == 142);
    assert(rom_shell.field_entity_screen_y() == 168);
    assert(rom_shell.field_collision_status() == 0);
    assert(rom_shell.field_raw_status() == 0);
    assert(!rom_shell.field_collision_blocked());
    assert(rom_shell.field_mode() == 2);
    // The first pointer table at ROM 0x6378 is the idle directional table;
    // walking selectors are selected only after an accepted movement step.
    assert(rom_shell.field_entity_state_selector() == 0);
    assert(rom_shell.field_entity_frame_index() == 0);
    const auto extracted_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    // These are the direct 0x12CEE results for Joshua's startup record:
    // 0x55ED6 gives base 4, 0x55F20 supplies the posture penalty, 0x12C74
    // selects skill +0x81 for equipped item 2, and the defense branch adds
    // +0x77 plus the three uninstalled cyberware adjustments.
    assert((extracted_assets.protagonist_archetype_records[0]
                .attack_bar_indices_by_posture ==
            std::array<std::uint8_t, 5>{{3, 4, 5, 6, 7}}));
    assert((extracted_assets.protagonist_archetype_records[0]
                .defense_bar_indices_by_posture ==
            std::array<std::uint8_t, 5>{{19, 18, 17, 16, 15}}));

    // Archetype selection mutates Joshua's one live template before the
    // opening; it does not create a switchable Decker/Shaman runner.
    NativePortShell decker_shell("SHADOWRUN", extracted_assets);
    decker_shell.finish_boot();
    decker_shell.update(InputState{button_mask(Button::Start),
                                   button_mask(Button::Start)});
    for (int frame = 0; frame < 48 &&
         decker_shell.scene() == NativeScene::Title; ++frame) {
        decker_shell.update(InputState{});
    }
    assert(decker_shell.scene() == NativeScene::StartOptions);
    decker_shell.update(InputState{button_mask(Button::A),
                                   button_mask(Button::A)});
    press_menu_direction(decker_shell, Button::Down);
    assert(decker_shell.archetype_selection() == 1);
    decker_shell.update(InputState{button_mask(Button::A),
                                   button_mask(Button::A)});
    assert(decker_shell.selected_archetype() == 1);
    assert(decker_shell.scene() == NativeScene::Intro);
    decker_shell.update(InputState{button_mask(Button::Start),
                                   button_mask(Button::Start)});
    assert(decker_shell.scene() == NativeScene::Field);
    decker_shell.update(InputState{button_mask(Button::C),
                                   button_mask(Button::C)});
    assert(decker_shell.field_active_record_index() == 0);
    assert(decker_shell.field_runner_switch_rejections() == 1);

    // Redmond's C0 doorway class is a blocked terrain cell, but the ROM keeps
    // its raw byte and dispatches the low status through the mode-2 event
    // table. Put a synthetic runner on that reviewed doorway rectangle and
    // verify that the native shell crosses into the extracted mode-20 room.
    auto transition_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    transition_assets.initial_field_records[0].position =
        {static_cast<std::uint16_t>(240U * 16U),
         static_cast<std::uint16_t>(624U * 16U)};
    transition_assets.field_areas[0].initial_field_records[0].position =
        transition_assets.initial_field_records[0].position;
    NativePortShell transition_shell("SHADOWRUN", std::move(transition_assets));
    enter_field_from_start(transition_shell);
    assert(transition_shell.field_mode() == 2);
    assert(transition_shell.field_raw_status() == 0xC0);
    transition_shell.update(InputState{
        button_mask(Button::Right), button_mask(Button::Right)});
    // The reviewed mode-2 C0/event-0 tuple must load the motel (0x14) and
    // immediately enter the source interaction callback, not fall through to
    // the unrelated mode-26 security interior.
    assert(transition_shell.scene() == NativeScene::FieldDialog);
    assert(transition_shell.field_mode() == 0x14);
    assert(transition_shell.field_area_transition_count() == 1);
    assert(transition_shell.field_world_x() == (0x5A00 >> 4));
    assert(transition_shell.field_world_y() == (0x0800 >> 4));
    assert(transition_shell.field_dialog_line().find("Stoker") !=
           std::string_view::npos);
    transition_shell.render(screen);
    // 0xC3E6 panel at (24,24) is black; the old cyan host border is gone.
    assert(screen.pixel(kDialogPanelX, kDialogPanelY) == 0U);
    assert(screen.pixel(8, 92) != 0x0000A8C8U);
    transition_shell.update(InputState{button_mask(Button::A),
                                       button_mask(Button::A)});
    assert(transition_shell.field_dialog_line_index() == 1);
    assert(transition_shell.scene() == NativeScene::FieldDialog);
    for (std::size_t line = 1; line <= 7; ++line) {
        transition_shell.update(InputState{});
        transition_shell.update(InputState{button_mask(Button::A),
                                           button_mask(Button::A)});
    }
    assert(transition_shell.scene() == NativeScene::Field);

    // The first three records at 0x7291A are Joshua archetype templates, not
    // three active runners. New Game therefore starts with Joshua only.
    assert(extracted_assets.initial_field_records[0].record_type == 0);
    assert(extracted_assets.initial_field_records[1].record_type == 1);
    assert(extracted_assets.initial_field_records[2].record_type == 2);
    assert(rom_shell.field_active_record_index() == 0);
    rom_shell.update(InputState{button_mask(Button::C),
                                button_mask(Button::C)});
    assert(rom_shell.field_active_record_index() == 0);
    assert(rom_shell.field_runner_switch_attempts() == 1);
    assert(rom_shell.field_runner_switch_rejections() == 1);
    assert(extracted_assets.protagonist_archetype_records[0].archetype_index == 2);
    assert(extracted_assets.protagonist_archetype_records[1].archetype_index == 3);
    assert(extracted_assets.protagonist_archetype_records[2].archetype_index == 1);

    // ROM 0x0075E dispatches Start to 0x10D30, which opens the field command
    // screen. It does not reset the selected record or camera. The selector
    // starts at six (0x10F50), and the reviewed 0x1170C label path resolves
    // that value to Pocket Secretary.
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::FieldMenu);
    assert(rom_shell.field_menu_open());
    assert(rom_shell.field_menu_selection() == 6);
    assert(rom_shell.field_menu_row() == 0);
    assert(rom_shell.field_active_record_index() == 0);

    // Up from a non-inventory command enters the real posture focus. The
    // source field is +0x4D, with Neutral at index 2 for Joshua's template.
    assert(rom_shell.field_posture_index() == 2);
    const auto neutral_attack_bar = rom_shell.field_attack_bar_index();
    const auto neutral_defense_bar = rom_shell.field_defense_bar_index();
    press_menu_direction(rom_shell, Button::Up);
    assert(rom_shell.field_menu_posture_active());
    press_menu_direction(rom_shell, Button::Left);
    assert(rom_shell.field_posture_index() == 1);
    assert(rom_shell.field_attack_bar_index() ==
           extracted_assets.protagonist_archetype_records[0]
               .attack_bar_indices_by_posture[1]);
    assert(rom_shell.field_defense_bar_index() ==
           extracted_assets.protagonist_archetype_records[0]
               .defense_bar_indices_by_posture[1]);
    assert(rom_shell.field_attack_bar_index() != neutral_attack_bar ||
           rom_shell.field_defense_bar_index() != neutral_defense_bar);
    press_menu_direction(rom_shell, Button::Right);
    assert(rom_shell.field_posture_index() == 2);
    assert(rom_shell.field_attack_bar_index() == neutral_attack_bar);
    assert(rom_shell.field_defense_bar_index() == neutral_defense_bar);
    rom_shell.update(InputState{button_mask(Button::B), button_mask(Button::B)});
    assert(!rom_shell.field_menu_posture_active());

    press_menu_direction(rom_shell, Button::Left);
    assert(rom_shell.field_menu_selection() == 5);
    for (int step = 0; step < 4; ++step) {
        press_menu_direction(rom_shell, Button::Left);
    }
    assert(rom_shell.field_menu_selection() == 1);
    for (int step = 0; step < 3; ++step) {
        press_menu_direction(rom_shell, Button::Down);
    }
    assert(rom_shell.field_menu_row() == 3);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    // ROM 0x11740 only opens the item-action loop for a non-empty slot. The
    // selected final cell is empty in Joshua's source record, so this first
    // press is a source no-op rather than an invented command count.
    assert(rom_shell.field_inventory_item_code(7) == 0);
    assert(!rom_shell.field_menu_action_active());
    assert(rom_shell.field_command_count() == 0);
    assert(rom_shell.field_last_command_selection() == 1);
    assert(rom_shell.field_last_command_row() == 3);
    assert(rom_shell.scene() == NativeScene::FieldMenu);

    // Joshua's extracted record, rather than a fabricated third runner, owns
    // the inventory and ammunition state shown here.
    assert(rom_shell.field_inventory_item_code(0) == 2);
    assert(rom_shell.field_inventory_item_code(1) == 0x2C);
    assert(rom_shell.field_ammo_rounds() == 0x20);
    assert(rom_shell.field_ammo_clips() == 0x0A);
    // The equipped item is code 2, so 0x12A8C selects the source "ammo"
    // branch. This is intentionally separate from the later medkit Uses
    // branch; neither is a generic Damage field.
    assert(rom_shell.field_active_item_detail_label() == "ammo");

    auto medical_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    NativePortShell medical_shell("SHADOWRUN", std::move(medical_assets));
    enter_field_from_start(medical_shell);
    medical_shell.update(InputState{button_mask(Button::Start),
                                    button_mask(Button::Start)});
    for (int step = 0; step < 6; ++step) {
        press_menu_direction(medical_shell, Button::Left);
    }
    press_menu_direction(medical_shell, Button::Down);
    press_menu_direction(medical_shell, Button::Right);
    medical_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    assert(medical_shell.field_inventory_item_code(3) == 0x10);
    assert(medical_shell.field_menu_action_active());
    assert(medical_shell.field_menu_action_selection() == 2);
    assert(medical_shell.field_menu_action_label() == "    First Aid    ");
    assert(medical_shell.field_selected_item_detail_label() == "uses");
    // First Aid is the medical handler selected by the ROM's D1E6C action
    // table. Its second edge opens the source target list rather than
    // pretending to equip the medkit as a weapon.
    const auto medical_equipped_item = medical_shell.field_active_item_code();
    medical_shell.update(InputState{});
    medical_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    assert(medical_shell.field_first_aid_active());
    assert(medical_shell.field_first_aid_target_count() == 1);
    assert(medical_shell.field_first_aid_target_record() == 0);
    medical_shell.update(InputState{button_mask(Button::B),
                                    button_mask(Button::B)});
    assert(!medical_shell.field_menu_action_active());
    assert(medical_shell.field_active_item_code() == medical_equipped_item);

    // Inventory possession is not equipment. Joshua starts with Model 101T
    // (slot 0, item 2) equipped; selecting the Scatter gear in slot 2 must
    // execute the source 0x121E0 weapon/gear path and change +0x56/+0x57, while
    // selecting that equipped cell again must set +0x56=0x7F and clear +0x57.
    auto equipment_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    NativePortShell equipment_shell("SHADOWRUN", std::move(equipment_assets));
    enter_field_from_start(equipment_shell);
    equipment_shell.update(InputState{button_mask(Button::Start),
                                      button_mask(Button::Start)});
    for (int step = 0; step < 6; ++step) {
        press_menu_direction(equipment_shell, Button::Left);
    }
    assert(equipment_shell.field_menu_selection() == 0);
    press_menu_direction(equipment_shell, Button::Down);
    assert(equipment_shell.field_menu_row() == 1);
    SoftwareScreen inventory_selector_screen;
    equipment_shell.render(inventory_selector_screen);
    // The selected Scatter cell has a continuous one-pixel warm-gold outline
    // inset two pixels from the cell. It has no segmented or interior marks.
    assert(thin_gold_inventory_selector_matches(
        inventory_selector_screen, 14, 142));
    equipment_shell.update(InputState{button_mask(Button::A),
                                      button_mask(Button::A)});
    assert(equipment_shell.field_menu_action_active());
    assert(equipment_shell.field_menu_action_selection() == 2);
    assert(equipment_shell.field_menu_action_label() == "  Equip/Unequip  ");
    assert(equipment_shell.field_active_item_code() == 2);
    press_menu_direction(equipment_shell, Button::Left);
    assert(equipment_shell.field_menu_action_selection() == 5);
    assert(equipment_shell.field_menu_action_label() == "     Cancel      ");
    press_menu_direction(equipment_shell, Button::Right);
    assert(equipment_shell.field_menu_action_selection() == 2);
    equipment_shell.update(InputState{});
    equipment_shell.update(InputState{button_mask(Button::A),
                                      button_mask(Button::A)});
    assert(equipment_shell.field_active_item_code() == 0x0E);
    assert(equipment_shell.field_equipped_weapon_slot() == 2);
    // Item 0x0E is a non-ammunition gear class in 0x121E0; the source
    // preserves the converted byte value rather than applying item 0x0E's
    // catalog factor as if it were a firearm.
    assert(equipment_shell.field_ammo_rounds() == 11);
    const auto equipped_attack = equipment_shell.field_attack_bar_index();
    equipment_shell.update(InputState{});
    equipment_shell.update(InputState{button_mask(Button::A),
                                      button_mask(Button::A)});
    assert(equipment_shell.field_menu_action_active());
    assert(equipment_shell.field_active_item_code() == 0x0E);
    equipment_shell.update(InputState{});
    equipment_shell.update(InputState{button_mask(Button::A),
                                      button_mask(Button::A)});
    assert(equipment_shell.field_active_item_code() == 0);
    assert(equipment_shell.field_equipped_weapon_slot() == 0x7F);
    assert(equipment_shell.field_ammo_rounds() != 0);
    assert(equipment_shell.field_attack_bar_index() != equipped_attack ||
           !equipment_shell.field_entity_visible());

    // The same source toggle is rendered by 0x12D74 as Empty/Hand with the
    // unarmed Damage/Power values; it must not leave the previous weapon
    // detail blank or painted over by the selected inventory cell.
    equipment_shell.update(InputState{button_mask(Button::Start),
                                      button_mask(Button::Start)});
    equipment_shell.update(InputState{});
    equipment_shell.update(InputState{button_mask(Button::Start),
                                      button_mask(Button::Start)});
    SoftwareScreen unarmed_menu;
    equipment_shell.render(unarmed_menu);
    assert(count_ink(unarmed_menu, 112, 56, 128, 40) > 40U);

    // ROM 0x12634 is a separate Yes/No loop, not an immediate delete. The
    // source starts on No; only the explicit Yes choice reaches 0x1275C.
    auto discard_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    NativePortShell discard_shell("SHADOWRUN", std::move(discard_assets));
    enter_field_from_start(discard_shell);
    discard_shell.update(InputState{button_mask(Button::Start),
                                    button_mask(Button::Start)});
    for (int step = 0; step < 6; ++step) {
        press_menu_direction(discard_shell, Button::Left);
    }
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    press_menu_direction(discard_shell, Button::Right);
    assert(discard_shell.field_menu_action_selection() == 3);
    discard_shell.update(InputState{});
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    assert(discard_shell.field_menu_action_active());
    assert(discard_shell.field_discard_confirmation_active());
    assert(!discard_shell.field_discard_confirmation_yes());
    assert(discard_shell.field_discard_message() ==
           "This item is equipped");
    assert(discard_shell.field_inventory_item_code(0) == 2);
    discard_shell.update(InputState{});
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    assert(!discard_shell.field_menu_action_active());
    assert(discard_shell.field_inventory_item_code(0) == 2);
    assert(discard_shell.field_active_item_code() == 2);

    // Re-enter the same action, toggle the source Yes/No choice once, and
    // confirm. This clears possession and live equipment because the
    // selected cell owns the equipped weapon.
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    press_menu_direction(discard_shell, Button::Right);
    discard_shell.update(InputState{});
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    press_menu_direction(discard_shell, Button::Right);
    assert(discard_shell.field_discard_confirmation_yes());
    discard_shell.update(InputState{});
    discard_shell.update(InputState{button_mask(Button::A),
                                    button_mask(Button::A)});
    assert(!discard_shell.field_menu_action_active());
    assert(!discard_shell.field_discard_confirmation_active());
    assert(discard_shell.field_inventory_item_code(0) == 0);
    assert(discard_shell.field_active_item_code() == 0);
    assert(discard_shell.field_equipped_weapon_slot() == 0x7F);

    rom_shell.render(screen);

    // Top-level Clips enters ROM 0x11B58's independent selector loop. The
    // native submenu starts at selector two, wraps through 2..5, and gates
    // its actions on the extracted +0x4F clip count via 0x12062.
    press_menu_direction(rom_shell, Button::Right);
    assert(rom_shell.field_menu_selection() == 2);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    assert(rom_shell.scene() == NativeScene::ClipsMenu);
    assert(rom_shell.clips_menu_open());
    assert(rom_shell.clips_menu_selection() == 2);
    press_menu_direction(rom_shell, Button::Left);
    assert(rom_shell.clips_menu_selection() == 5);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    // Selector five is the source D1E00 Cancel path; 0x11C74 returns
    // without an action or mutation.
    assert(rom_shell.clips_action_count() == 0);
    rom_shell.render(screen);
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::FieldMenu);
    assert(rom_shell.field_menu_selection() == 2);

    // ROM 0x11950 is the display-only Cyberware handler. It reads +0x90 as
    // fifteen bit flags and emits the pointer-table name for each set bit.
    // Joshua's active record owns this bitfield; the catalog name remains
    // available even when its bit is clear.
    press_menu_direction(rom_shell, Button::Right);
    press_menu_direction(rom_shell, Button::Right);
    assert(rom_shell.field_menu_selection() == 4);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    assert(rom_shell.scene() == NativeScene::CyberwareMenu);
    assert(rom_shell.cyberware_menu_open());
    assert(rom_shell.field_cyberware_flags() ==
           extracted_assets.protagonist_archetype_records[0].cyberware_flags);
    assert(rom_shell.field_cyberware_installed(1) ==
           ((extracted_assets.protagonist_archetype_records[0].cyberware_flags &
             (1U << 1U)) != 0));
    assert(rom_shell.field_cyberware_installed(2) ==
           ((extracted_assets.protagonist_archetype_records[0].cyberware_flags &
             (1U << 2U)) != 0));
    assert(rom_shell.field_cyberware_name(2) == "Hand Razors");
    rom_shell.render(screen);
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::FieldMenu);
    assert(rom_shell.field_menu_selection() == 4);

    // ROM 0xD01C renders the direct attribute/skill fields. The twenty names
    // come from 0xC1A2A; the startup record's values stay host-native while
    // the unresolved A/B/C upgrade mutations remain disabled.
    press_menu_direction(rom_shell, Button::Left);
    assert(rom_shell.field_menu_selection() == 3);
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    assert(rom_shell.scene() == NativeScene::AttributesMenu);
    assert(rom_shell.attributes_menu_open());
    assert(rom_shell.attributes_menu_column() == 0);
    assert(rom_shell.attributes_menu_row() == 0);
    assert(rom_shell.field_attribute_skill_name(0) == "Body");
    assert(rom_shell.field_attribute_value(0) == 4);
    assert(rom_shell.field_attribute_value(7) == 5);
    assert(rom_shell.field_skill_value(0) == 0);
    assert(rom_shell.field_skill_value(5) == 3);
    assert(rom_shell.field_attribute_skill_auxiliary(3) == 9);
    rom_shell.update(InputState{button_mask(Button::Right),
                                button_mask(Button::Right)});
    rom_shell.update(InputState{button_mask(Button::Down),
                                button_mask(Button::Down)});
    assert(rom_shell.attributes_menu_column() == 1);
    assert(rom_shell.attributes_menu_row() == 1);
    assert(rom_shell.field_attribute_skill_name(11) == "SMGs");
    rom_shell.render(screen);
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::FieldMenu);
    assert(rom_shell.field_menu_selection() == 3);

    // C follows the same 0x391A record-selection path from the command
    // screen, restoring the menu selector to six before continuing.
    rom_shell.update(InputState{button_mask(Button::C),
                                button_mask(Button::C)});
    assert(rom_shell.field_active_record_index() == 0);
    assert(rom_shell.field_menu_selection() == 6);
    assert(rom_shell.field_menu_row() == 0);

    // The 0x391A selector skips records that are no longer active. If the
    // party has no other eligible runner, C still produces the input/audio
    // action but leaves the controlled record untouched.
    auto no_runner_assets = shadowrun::graphics::decode_genesis_native_assets(
        rom);
    no_runner_assets.initial_field_records[1].flags_10 = 0;
    no_runner_assets.initial_field_records[2].flags_10 = 0;
    NativePortShell no_runner_shell("SHADOWRUN", std::move(no_runner_assets));
    enter_field_from_start(no_runner_shell);
    no_runner_shell.update(InputState{button_mask(Button::C),
                                      button_mask(Button::C)});
    assert(no_runner_shell.field_active_record_index() == 0);
    assert(no_runner_shell.field_runner_switch_attempts() == 1);
    assert(no_runner_shell.field_runner_switch_rejections() == 1);

    const auto field_start_x = rom_shell.marker_x();
    const auto field_start_y = rom_shell.marker_y();
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::Field);
    assert(rom_shell.marker_x() == field_start_x);
    assert(rom_shell.marker_y() == field_start_y);

    rom_shell.update(InputState{button_mask(Button::Start), 0});
    assert(rom_shell.scene() == NativeScene::Field);
    assert(rom_shell.marker_x() == field_start_x);

    const auto initial_world_x = rom_shell.field_world_x();
    const auto initial_world_y = rom_shell.field_world_y();
    rom_shell.update(InputState{
        button_mask(Button::Right) | button_mask(Button::Down), 0});
    // The source does not snap the facing direction to the requested
    // diagonal.  0xB530 advances +0x2E by 0x0800 first; the first C002
    // vector is therefore the current direction's vector, not the host's
    // requested pixel delta.
    assert(rom_shell.field_world_x() == initial_world_x);
    assert(rom_shell.field_world_y() < initial_world_y);
    assert(rom_shell.field_entity_state_selector() == 0x0060);
    assert(rom_shell.field_entity_state_index() == 0);
    assert(rom_shell.field_entity_frame_index() == 0);
    assert(rom_shell.field_entity_screen_x() == rom_shell.marker_x());
    assert(rom_shell.field_entity_screen_y() == rom_shell.marker_y());
    rom_shell.render(screen);
    const auto initial_camera_x = rom_shell.field_camera_x();
    for (int frame = 0; frame < 100; ++frame) {
        rom_shell.update(InputState{button_mask(Button::Right), 0});
    }
    // The source C002 movement vector is 0x28 fixed-point units (2.5
    // pixels), and the angle slew settles on direction two (0x68 walking
    // state). 0x3BAC catches the camera up so the runner stays on-screen.
    assert(rom_shell.field_entity_visible());
    assert(rom_shell.field_entity_state_selector() == 0x0068);
    const auto frame_before_source_cadence =
        rom_shell.field_entity_frame_index();
    for (int frame = 0; frame < 6; ++frame) {
        rom_shell.update(InputState{button_mask(Button::Right), 0});
    }
    // 0x600E selects multiplier 0x0010 from the reviewed +0xCA timing word;
    // the list's 0x0300 auxiliary word is not the clock. Six source passes
    // therefore do not cross the +0x1000 threshold.
    assert(rom_shell.field_entity_frame_index() ==
           frame_before_source_cadence);
    for (int frame = 0; frame < 256; ++frame) {
        rom_shell.update(InputState{button_mask(Button::Right), 0});
    }
    assert(rom_shell.field_entity_frame_index() !=
           frame_before_source_cadence);
    assert(rom_shell.field_world_x() > initial_camera_x);
    assert(rom_shell.field_camera_x() > initial_camera_x);
    rom_shell.render(screen);
    bool rejected = false;
    auto world_at_rejection = std::int32_t{};
    for (int frame = 0; frame < 1000; ++frame) {
        const auto before = rom_shell.field_world_x();
        rom_shell.update(InputState{button_mask(Button::Right), 0});
        if (rom_shell.field_move_rejected()) {
            rejected = true;
            world_at_rejection = before;
            assert(rom_shell.field_world_x() == world_at_rejection);
            assert(rom_shell.field_collision_blocked());
            break;
        }
    }
    assert(rejected);
    const auto before_action_frame = rom_shell.frame_number();
    rom_shell.update(InputState{button_mask(Button::A),
                                button_mask(Button::A)});
    rom_shell.render(screen);
    assert(rom_shell.frame_number() == before_action_frame + 1);
    assert(screen.presented_frames() == 12);

    // The runtime constructor below receives only host-owned native state and
    // individually extracted assets. The ROM is used by extraction, not by
    // the native update/render loop.
    auto native_assets = shadowrun::graphics::decode_genesis_native_assets(
        rom);
    assert(native_assets.recruited_runner_templates.size() == 10);
    assert(native_assets.recruited_runner_templates[0].display_name ==
           "Ricky");
    assert(native_assets.recruited_runner_templates[9].display_name ==
           "Stark");
    assert(native_assets.recruited_runner_templates[0].record.active_item_code ==
           1U);
    assert(native_assets.recruited_runner_templates[3].record.cyberware_flags ==
           1U);
    assert(native_assets.field_menu_cursor.width == 24);
    assert(native_assets.field_menu_cursor.height == 32);
    assert(native_assets.field_menu_selected_cells[0].width == 40);
    assert(native_assets.field_menu_selected_cells[0].height == 32);
    assert(native_assets.field_menu_item_art[1].width == 72);
    assert(native_assets.field_menu_item_art[1].height == 40);
    assert(native_assets.field_menu_damage_bars[0].width == 56);
    assert(native_assets.field_menu_damage_bars[0].height == 8);
    assert(native_assets.field_menu_power_labels[5] == "Light");
    assert(native_assets.protagonist_archetype_records[0]
               .attack_bar_indices_by_posture ==
           native_assets.protagonist_archetype_records[0]
               .attack_bar_indices_by_posture);
    assert(native_assets.protagonist_archetype_records[0]
               .defense_bar_indices_by_posture ==
           native_assets.protagonist_archetype_records[0]
               .defense_bar_indices_by_posture);
    NativePortShell direct_shell("SHADOWRUN", native_assets);
    assert(direct_shell.has_assets());
    assert(direct_shell.recruited_runner_template_count() == 10);
    const auto* ricky = direct_shell.recruited_runner_template(0);
    assert(ricky != nullptr && ricky->display_name == "Ricky" &&
           ricky->record.inventory_item_codes[1] == 0x2D);
    assert(direct_shell.recruited_runner_template(10) == nullptr);
    assert(direct_shell.has_field_map());
    assert(direct_shell.has_terrain());
    assert(direct_shell.has_entity_frame());
    assert(direct_shell.has_boot());
    assert(direct_shell.scene() == NativeScene::Bootstrap);
    SoftwareScreen direct_screen;
    enter_field_from_start(direct_shell);
    assert(direct_shell.scene() == NativeScene::Field);
    assert(direct_shell.field_entity_visible());
    const auto direct_start_x = direct_shell.marker_x();
    const auto direct_start_y = direct_shell.marker_y();
    direct_shell.update(InputState{button_mask(Button::Right), 0});
    assert(direct_shell.marker_x() == direct_start_x);
    assert(direct_shell.marker_y() < direct_start_y);
    assert(direct_shell.field_entity_state_selector() == 0x0060);
    direct_shell.render(direct_screen);
    assert(direct_screen.presented_frames() == 1);

    // ROM 0x36FC suppresses the separately rendered equipped-item sprite
    // when template byte +0x56 is negative; the projected runner body still
    // remains present.
    auto hidden_assets = native_assets;
    hidden_assets.entity_template.display_state = -1;
    hidden_assets.protagonist_archetype_records[0].display_state = -1;
    hidden_assets.field_entity_records[0].display_state = -1;
    NativePortShell hidden_shell("SHADOWRUN", std::move(hidden_assets));
    enter_field_from_start(hidden_shell);
    assert(hidden_shell.scene() == NativeScene::Field);
    assert(hidden_shell.field_entity_visible());
    return 0;
}

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

void enter_field_from_start(shadowrun::native::NativePortShell& shell) {
    using namespace shadowrun::native;
    shell.update(InputState{button_mask(Button::Start),
                             button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::StartOptions);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::Intro);
    shell.update(InputState{button_mask(Button::Start),
                             button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::Field);
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
    NativePortShell rom_shell("SHADOWRUN", rom);
    assert(rom_shell.has_assets());
    assert(rom_shell.has_field_map());
    assert(rom_shell.has_terrain());
    assert(rom_shell.has_entity_frame());
    assert(rom_shell.scene() == NativeScene::Title);
    rom_shell.render(screen);
    assert(screen.presented_frames() == 3);

    // Start is an edge-triggered title transition. Holding it without a
    // pressed edge must not repeatedly re-enter or reset the field scene.
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
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
    rom_shell.update(InputState{});
    assert(rom_shell.intro_frame() == 1);
    assert(rom_shell.scene() == NativeScene::Intro);
    rom_shell.update(InputState{button_mask(Button::Start),
                                button_mask(Button::Start)});
    assert(rom_shell.scene() == NativeScene::Field);
    assert(rom_shell.field_entity_visible());
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

    // Archetype selection mutates Joshua's one live template before the
    // opening; it does not create a switchable Decker/Shaman runner.
    NativePortShell decker_shell("SHADOWRUN", extracted_assets);
    decker_shell.update(InputState{button_mask(Button::Start),
                                   button_mask(Button::Start)});
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
    assert(transition_shell.field_mode() == 0x14);
    assert(transition_shell.field_area_transition_count() == 1);
    assert(transition_shell.field_world_x() == (0x5A00 >> 4));
    assert(transition_shell.field_world_y() == (0x0800 >> 4));

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
    assert(rom_shell.field_command_count() == 1);
    assert(rom_shell.field_last_command_selection() == 1);
    assert(rom_shell.field_last_command_row() == 3);
    assert(rom_shell.scene() == NativeScene::FieldMenu);

    // Joshua's extracted record, rather than a fabricated third runner, owns
    // the inventory and ammunition state shown here.
    assert(rom_shell.field_inventory_item_code(0) == 2);
    assert(rom_shell.field_inventory_item_code(1) == 0x2C);
    assert(rom_shell.field_ammo_rounds() == 0x20);
    assert(rom_shell.field_ammo_clips() == 0x0A);
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
    assert(rom_shell.clips_action_count() == 1);
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

    rom_shell.update(InputState{
        button_mask(Button::Right) | button_mask(Button::Down), 0});
    assert(rom_shell.marker_x() == field_start_x + 2);
    assert(rom_shell.marker_y() == 168);
    assert(rom_shell.field_entity_state_selector() == 0x006C);
    assert(rom_shell.field_entity_state_index() == 0);
    assert(rom_shell.field_entity_frame_index() == 0);
    assert(rom_shell.field_entity_screen_x() == rom_shell.marker_x());
    assert(rom_shell.field_entity_screen_y() == rom_shell.marker_y());
    rom_shell.render(screen);
    const auto field_before_scroll = screen.pixels();
    const auto initial_camera_x = rom_shell.field_camera_x();
    for (int frame = 0; frame < 100; ++frame) {
        rom_shell.update(InputState{button_mask(Button::Right), 0});
    }
    assert(rom_shell.marker_x() == 301);
    assert(rom_shell.field_entity_state_selector() == 0x0068);
    // Kind-3's 0x8002 walking lists carry auxiliary=0x0300 and the entity
    // timer threshold is 0x1000. After the direction change, 100 field ticks
    // therefore advance 18 times and land back on frame zero.
    assert(rom_shell.field_entity_state_index() == 0);
    assert(rom_shell.field_entity_frame_index() == 0);
    assert(rom_shell.field_world_x() > initial_camera_x);
    assert(rom_shell.field_camera_x() > initial_camera_x);
    rom_shell.render(screen);
    bool background_scrolled = false;
    for (std::int32_t y = 36; y < 204 && !background_scrolled; ++y) {
        for (std::int32_t x = 0; x < 140; ++x) {
            if (screen.pixel(x, y) !=
                field_before_scroll[static_cast<std::size_t>(y) *
                                     screen.width() + x]) {
                background_scrolled = true;
                break;
            }
        }
    }
    assert(background_scrolled);
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
    assert(screen.presented_frames() == 10);

    // The runtime constructor below receives only the host asset pack. This
    // is the PC-port path: the ROM is used by extraction, not by the native
    // update/render loop.
    auto native_assets = shadowrun::graphics::decode_genesis_native_assets(
        rom);
    assert(native_assets.field_menu_cursor.width == 24);
    assert(native_assets.field_menu_cursor.height == 32);
    assert(native_assets.field_menu_selected_cells[0].width == 40);
    assert(native_assets.field_menu_selected_cells[0].height == 32);
    assert(native_assets.field_menu_item_art[1].width == 72);
    assert(native_assets.field_menu_item_art[1].height == 40);
    assert(native_assets.field_menu_damage_bars[0].width == 56);
    assert(native_assets.field_menu_damage_bars[0].height == 8);
    assert(native_assets.field_menu_power_labels[5] == "Light");
    const auto pack_path = std::filesystem::temp_directory_path() /
                           "shadowrun_native_port_runtime_test.srna";
    shadowrun::graphics::write_native_asset_pack(
        pack_path.string(), native_assets);
    auto packed_assets = shadowrun::graphics::read_native_asset_pack(
        pack_path.string());
    assert(packed_assets.protagonist_archetype_records[0]
               .attack_bar_indices_by_posture ==
           native_assets.protagonist_archetype_records[0]
               .attack_bar_indices_by_posture);
    assert(packed_assets.protagonist_archetype_records[0]
               .defense_bar_indices_by_posture ==
           native_assets.protagonist_archetype_records[0]
               .defense_bar_indices_by_posture);
    NativePortShell pack_shell("SHADOWRUN", std::move(packed_assets));
    std::filesystem::remove(pack_path);
    assert(pack_shell.has_assets());
    assert(pack_shell.has_field_map());
    assert(pack_shell.has_terrain());
    assert(pack_shell.has_entity_frame());
    assert(pack_shell.scene() == NativeScene::Title);
    SoftwareScreen pack_screen;
    enter_field_from_start(pack_shell);
    assert(pack_shell.scene() == NativeScene::Field);
    assert(pack_shell.field_entity_visible());
    const auto pack_start_x = pack_shell.marker_x();
    pack_shell.update(InputState{button_mask(Button::Right), 0});
    assert(pack_shell.marker_x() == pack_start_x + 2);
    pack_shell.render(pack_screen);
    assert(pack_screen.presented_frames() == 1);

    // ROM 0x36FC suppresses an otherwise projected entity when template
    // byte +0x56 is negative. The native asset boundary must preserve that
    // rule independently of the ROM-backed constructor.
    auto hidden_assets = native_assets;
    hidden_assets.entity_template.display_state = -1;
    hidden_assets.protagonist_archetype_records[0].display_state = -1;
    hidden_assets.field_entity_records[0].display_state = -1;
    NativePortShell hidden_shell("SHADOWRUN", std::move(hidden_assets));
    enter_field_from_start(hidden_shell);
    assert(hidden_shell.scene() == NativeScene::Field);
    assert(!hidden_shell.field_entity_visible());
    return 0;
}

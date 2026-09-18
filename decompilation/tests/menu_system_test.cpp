#include "shadowrun/graphics.hpp"
#include "shadowrun/native_port.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using shadowrun::native::Button;
using shadowrun::native::InputState;
using shadowrun::native::NativeImage;
using shadowrun::native::NativePortAssets;
using shadowrun::native::NativePortShell;
using shadowrun::native::NativeScene;
using shadowrun::native::Rgba8;
using shadowrun::native::SoftwareScreen;
using shadowrun::native::button_mask;

std::vector<std::uint8_t> read_rom() {
    std::ifstream input(SHADOWRUN_TEST_ROM, std::ios::binary);
    assert(input);
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void press(NativePortShell& shell, Button button) {
    const auto mask = button_mask(button);
    shell.update(InputState{mask, mask});
    shell.update(InputState{});
}

void enter_start_options(NativePortShell& shell) {
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::Title);
    shell.update(InputState{});
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::StartOptions);
}

void enter_field(NativePortShell& shell) {
    enter_start_options(shell);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::Intro);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::Field);
}

std::uint32_t pack(Rgba8 color) {
    return (static_cast<std::uint32_t>(color.red) << 16U) |
           (static_cast<std::uint32_t>(color.green) << 8U) |
           color.blue;
}

// This is deliberately a source-image comparison rather than a guessed
// color check. It proves that the visible overlay contains pixels from the
// extracted Genesis resource at its source destination. SoftwareScreen's
// blitter is overwrite-on-nontransparent, so an exact match is expected for
// at least one uncovered pixel in each reviewed layer.
bool has_image_pixel(const SoftwareScreen& screen, const NativeImage& image,
                     std::int32_t destination_x,
                     std::int32_t destination_y) {
    for (std::size_t y = 0; y < image.height; ++y) {
        for (std::size_t x = 0; x < image.width; ++x) {
            const auto& source = image.pixels[y * image.width + x];
            if (source.alpha == 0U) {
                continue;
            }
            if (screen.pixel(destination_x + static_cast<std::int32_t>(x),
                             destination_y + static_cast<std::int32_t>(y)) ==
                pack(source)) {
                return true;
            }
        }
    }
    return false;
}

void assert_cursor_at(const SoftwareScreen& screen, const NativeImage& cursor,
                      std::int32_t x, std::int32_t y) {
    assert(!cursor.pixels.empty());
    assert(has_image_pixel(screen, cursor, x, y));
}

void select_field_inventory_slot(NativePortShell& shell, std::size_t slot) {
    assert(slot < 8U);
    while (shell.field_menu_selection() != 0U) {
        press(shell, Button::Left);
    }
    while (shell.field_menu_row() != 0U) {
        press(shell, Button::Up);
    }
    for (std::size_t row = 0; row < slot / 2U; ++row) {
        press(shell, Button::Down);
    }
    if ((slot & 1U) != 0U) {
        press(shell, Button::Right);
    }
    assert(shell.field_menu_selection() == slot % 2U);
    assert(shell.field_menu_row() == slot / 2U);
}

void test_startup_behavior_and_rendering(const std::vector<std::uint8_t>& rom,
                                         const NativePortAssets& assets) {
    NativePortShell shell("SHADOWRUN", assets);
    SoftwareScreen screen;

    // Screenshot evidence: the reference title captures show the circuit
    // plane, progressive logo reveal, sprite phases, and delayed text. The
    // extracted title layers are the automated pixel-level golden here.
    shell.render(screen);
    const auto first_title = screen.pixels();
    assert(first_title != std::vector<std::uint32_t>{});
    for (std::size_t frame = 0; frame < 120U; ++frame) {
        shell.update(InputState{});
    }
    shell.render(screen);
    assert(first_title != screen.pixels());
    assert(has_image_pixel(screen, assets.title_background, 0, 0));

    // Bytecode/source evidence: 0x0BF2/0x1A9E consumes the first Start edge
    // as a title skip; the following edge enters the source 0x59092 options
    // block. It must not skip directly into New Game.
    enter_start_options(shell);
    assert(shell.start_options_selection() == 0U);
    shell.render(screen);
    assert(screen.pixel(0, 0) == 0U);
    assert_cursor_at(screen, assets.startup_cursor, 104, 80);

    // 0x59092's two choices wrap on the vertical selector. The cursor moves
    // with the selected row, rather than being a checklist rendered by the
    // host UI.
    press(shell, Button::Down);
    assert(shell.start_options_selection() == 1U);
    shell.render(screen);
    assert_cursor_at(screen, assets.startup_cursor, 104, 96);
    press(shell, Button::Down);
    assert(shell.start_options_selection() == 0U);
    press(shell, Button::Up);
    assert(shell.start_options_selection() == 1U);

    // 0x59172 opens Save/Load. An empty SRAM slot displays the source error
    // after the slot action and remains on Save/Load; it is not New Game.
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::SaveLoad);
    assert(shell.save_load_selection() == 0U);
    shell.render(screen);
    assert_cursor_at(screen, assets.startup_cursor, 136, 112);
    press(shell, Button::Down);
    assert(shell.save_load_selection() == 1U);
    press(shell, Button::Up);
    assert(shell.save_load_selection() == 0U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.save_load_message_visible());
    assert(shell.scene() == NativeScene::SaveLoad);
    shell.update(InputState{button_mask(Button::B), button_mask(Button::B)});
    assert(shell.scene() == NativeScene::StartOptions);

    // The archetype source path has exactly three choices. Test both wrap
    // directions and select the third choice to prove it changes Joshua's
    // startup template rather than creating a switchable party member.
    shell.update(InputState{button_mask(Button::Up), button_mask(Button::Up)});
    assert(shell.start_options_selection() == 0U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ArchetypeMenu);
    shell.render(screen);
    assert_cursor_at(screen, assets.startup_cursor, 152, 88);
    press(shell, Button::Up);
    assert(shell.archetype_selection() == 2U);
    press(shell, Button::Down);
    assert(shell.archetype_selection() == 0U);
    press(shell, Button::Down);
    press(shell, Button::Down);
    assert(shell.archetype_selection() == 2U);
    shell.render(screen);
    assert_cursor_at(screen, assets.startup_cursor, 152, 120);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.selected_archetype() == 2U);
    assert(shell.scene() == NativeScene::Intro);

    // 0x59C5E is a timed presentation. An empty update advances it, while a
    // Start edge exits through the same field entry boundary.
    const auto intro_frame = shell.intro_frame();
    shell.update(InputState{});
    assert(shell.intro_frame() == intro_frame + 1U);
    shell.render(screen);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::Field);

    (void)rom;
}

void test_field_menu_behavior_and_rendering(
    const std::vector<std::uint8_t>& rom, const NativePortAssets& assets) {
    NativePortShell shell("SHADOWRUN", assets);
    enter_field(shell);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::FieldMenu);
    assert(shell.field_menu_selection() == 6U);
    assert(shell.field_menu_row() == 0U);

    SoftwareScreen screen;
    shell.render(screen);
    // 0x10F50/0x1232 supplies the complete plane-B base. The menu must use
    // the extracted panel rather than a newly invented checklist layout.
    assert(has_image_pixel(screen, assets.field_menu_background, 0, 0));
    assert(has_image_pixel(screen, assets.field_menu_inventory_boxes, 0, 0));
    assert(has_image_pixel(screen, assets.field_menu_runner_portraits[0],
                          16, 16));
    assert(has_image_pixel(screen, assets.field_menu_status_bars[2],
                          176, 168));

    // 0x114A0 wraps all seven top-level command values. 0x11506 enters a
    // separate posture focus for values 2..6; it does not trap the cursor.
    press(shell, Button::Right);
    assert(shell.field_menu_selection() == 0U);
    press(shell, Button::Left);
    assert(shell.field_menu_selection() == 6U);
    press(shell, Button::Up);
    assert(shell.field_menu_posture_active());
    const auto neutral_posture = shell.field_posture_index();
    for (int step = 0; step < 8; ++step) {
        press(shell, Button::Right);
    }
    assert(shell.field_posture_index() == 4U);
    for (int step = 0; step < 8; ++step) {
        press(shell, Button::Left);
    }
    assert(shell.field_posture_index() == 0U);
    press(shell, Button::Down);
    assert(!shell.field_menu_posture_active());
    assert(shell.field_posture_index() != neutral_posture);

    // Inventory is a two-column/four-row selector. Its action loop is the
    // source 0x11740/0x118C6 table: Equip, Discard, Trade, Cancel, wrapped by
    // Left/Right and confirmed by a second A/B edge.
    // Slot 7 is the source-empty cell in Joshua's startup record. Checking
    // the cursor there isolates the extracted selector sprite from item-art
    // and equipment overlays, which are intentionally composited later.
    select_field_inventory_slot(shell, 7U);
    shell.render(screen);
    assert_cursor_at(screen, assets.field_menu_cursor, 52, 188);
    select_field_inventory_slot(shell, 0U);
    assert(shell.field_inventory_item_code(0U) != 0U);
    shell.render(screen);
    assert(has_image_pixel(screen, assets.field_menu_selected_cells[0], 8, 112));
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.field_menu_action_active());
    assert(shell.field_menu_action_selection() == 2U);
    assert(shell.field_menu_action_label() == "  Equip/Unequip  ");
    const std::array<std::size_t, 4> action_cycle{{3U, 4U, 5U, 2U}};
    for (const auto expected : action_cycle) {
        press(shell, Button::Right);
        assert(shell.field_menu_action_selection() == expected);
    }
    press(shell, Button::Left);
    assert(shell.field_menu_action_selection() == 5U);
    shell.update(InputState{button_mask(Button::B), button_mask(Button::B)});
    assert(!shell.field_menu_action_active());
    assert(shell.field_menu_selection() == 0U);

    // Action 3 is a confirmation state, not an immediate deletion. The
    // default is No from 0x12694; a second horizontal edge selects Yes.
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    press(shell, Button::Right);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.field_discard_confirmation_active());
    assert(!shell.field_discard_confirmation_yes());
    shell.render(screen);
    press(shell, Button::Right);
    assert(shell.field_discard_confirmation_yes());
    shell.update(InputState{button_mask(Button::B), button_mask(Button::B)});
    assert(!shell.field_discard_confirmation_active());

    // Top-level submenus each preserve their parent selector and have their
    // source-specific exit semantics.
    while (shell.field_menu_selection() != 2U) {
        press(shell, Button::Right);
    }
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::ClipsMenu);
    assert(shell.clips_menu_selection() == 2U);
    for (std::size_t expected = 3U; expected <= 5U; ++expected) {
        press(shell, Button::Right);
        assert(shell.clips_menu_selection() == expected);
    }
    press(shell, Button::Right);
    assert(shell.clips_menu_selection() == 2U);
    shell.render(screen);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::FieldMenu);
    assert(shell.field_menu_selection() == 2U);

    press(shell, Button::Right);
    press(shell, Button::Right);
    assert(shell.field_menu_selection() == 4U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::CyberwareMenu);
    const auto cyberware_flags = shell.field_cyberware_flags();
    shell.update(InputState{button_mask(Button::Left) | button_mask(Button::Down),
                            button_mask(Button::Left) | button_mask(Button::Down)});
    assert(shell.field_cyberware_flags() == cyberware_flags);
    shell.render(screen);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::FieldMenu);
    assert(shell.field_menu_selection() == 4U);

    press(shell, Button::Left);
    assert(shell.field_menu_selection() == 3U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.scene() == NativeScene::AttributesMenu);
    assert(shell.attributes_menu_column() == 0U);
    assert(shell.attributes_menu_row() == 0U);
    press(shell, Button::Up);
    assert(shell.attributes_menu_row() == 0U);
    for (int step = 0; step < 12; ++step) {
        press(shell, Button::Down);
    }
    assert(shell.attributes_menu_row() == 9U);
    press(shell, Button::Right);
    assert(shell.attributes_menu_column() == 1U);
    press(shell, Button::Left);
    assert(shell.attributes_menu_column() == 0U);
    shell.render(screen);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    assert(shell.scene() == NativeScene::FieldMenu);
    assert(shell.field_menu_selection() == 3U);

    // C is the source runner-switch command. It resets the top-level command
    // selector to Pocket Secretary after either a successful switch or the
    // sound-only no-runner path.
    shell.update(InputState{button_mask(Button::C), button_mask(Button::C)});
    assert(shell.field_menu_selection() == 6U);
    (void)rom;
}

void test_no_clip_gate(const std::vector<std::uint8_t>& rom) {
    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.protagonist_archetype_records[0].ammo_clips = 0U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    press(shell, Button::Right);
    press(shell, Button::Right);
    assert(shell.field_menu_selection() == 1U);
    // Return to Clips through the source top-level order, not a direct test
    // hook. This also protects the selector transitions from a future UI shim.
    press(shell, Button::Right);
    assert(shell.field_menu_selection() == 2U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.clips_menu_open());
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.clips_action_count() == 0U);
    assert(shell.clips_action_message() == "is out of ammo clips!");
}

void test_clip_item_gate(const std::vector<std::uint8_t>& rom) {
    // 0x11CCA rejects Reload when +0x56 is the source no-equipment marker or
    // +0x57 names a non-weapon item. It displays D1B40 and does not mutate
    // ammunition or count the action as a reload.
    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.protagonist_archetype_records[0].display_state = 0x7F;
    assets.protagonist_archetype_records[0].display_substate = 0;
    assets.protagonist_archetype_records[0].active_item_code = 0;
    assets.protagonist_archetype_records[0].ammo_clips = 3U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    press(shell, Button::Start);
    while (shell.field_menu_selection() != 2U) {
        press(shell, Button::Right);
    }
    press(shell, Button::A);
    assert(shell.clips_menu_open());
    press(shell, Button::A);
    assert(shell.clips_action_count() == 0U);
    assert(shell.clips_action_message() ==
           "This item does not use clips!!");
    assert(shell.field_ammo_clips() == 3U);
}

void test_clip_trade_menu(const std::vector<std::uint8_t>& rom) {
    // 0x11DD2 builds the active recipient list, then 0x11E7E enters the
    // nested D1E24 quantity/recipient loop. This verifies that Trade is not
    // mistaken for the top-level Distribute action.
    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.protagonist_archetype_records[0].ammo_clips = 5U;
    assets.initial_field_records[1].flags_10 |= 0x01U;
    assets.field_entity_records[1].flags_10 |= 0x01U;
    assets.field_entity_records[1].ammo_clips = 1U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    press(shell, Button::Start);
    while (shell.field_menu_selection() != 2U) {
        press(shell, Button::Right);
    }
    press(shell, Button::A);
    assert(shell.clips_menu_open());
    press(shell, Button::Right);
    assert(shell.clips_menu_selection() == 3U);
    press(shell, Button::A);
    assert(shell.clips_trade_menu_open());
    assert(shell.clips_trade_menu_selection() == 2U);
    assert(shell.clips_trade_recipient_record() == 1U);
    assert(shell.clips_trade_quantity() == 5U);

    // D2=1/negative one in 0x120E4 wraps the quantity in the inclusive
    // range 1..source +0x4F; the source starts at the full quantity.
    press(shell, Button::Right);
    assert(shell.clips_trade_menu_selection() == 3U);
    press(shell, Button::A);
    assert(shell.clips_trade_quantity() == 4U);
    press(shell, Button::Right);
    press(shell, Button::A);
    assert(!shell.clips_trade_menu_open());
    assert(shell.clips_action_message() == "Exchange complete");
    assert(shell.field_ammo_clips() == 1U);
    assert(shell.field_record_ammo_clips(1U) == 5U);
}

void test_clip_distribution(const std::vector<std::uint8_t>& rom) {
    // 0x11FDE averages +0x4F across at least two active records. With an
    // even total this isolates the quotient write from the bit-1 remainder
    // assignment at 0x1202A.
    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.protagonist_archetype_records[0].ammo_clips = 5U;
    assets.initial_field_records[1].flags_10 |= 0x01U;
    assets.field_entity_records[1].flags_10 |= 0x01U;
    assets.field_entity_records[1].ammo_clips = 1U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    press(shell, Button::Start);
    while (shell.field_menu_selection() != 2U) {
        press(shell, Button::Right);
    }
    press(shell, Button::A);
    assert(shell.clips_menu_open());
    press(shell, Button::Right);
    press(shell, Button::Right);
    assert(shell.clips_menu_selection() == 4U);
    press(shell, Button::A);
    assert(!shell.clips_trade_menu_open());
    assert(shell.clips_action_message() == "Exchange complete");
    assert(shell.field_ammo_clips() == 3U);
    assert(shell.field_record_ammo_clips(1U) == 3U);
}

void open_medical_action(NativePortShell& shell) {
    enter_field(shell);
    press(shell, Button::Start);
    select_field_inventory_slot(shell, 3U);
    assert(shell.field_inventory_item_code(3U) >= 0x10U);
    press(shell, Button::A);
    assert(shell.field_menu_action_active());
    assert(shell.field_menu_action_selection() == 2U);
    press(shell, Button::A);
    assert(shell.field_first_aid_active());
}

void test_first_aid_handler(const std::vector<std::uint8_t>& rom) {
    // 0x12298 creates a target list from active record bits and adds a
    // count-entry CANCEL row. It does not reuse the runner-switch eligibility
    // predicate, and Up/Down therefore moves through target rows, not posture.
    auto selector_assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    selector_assets.initial_field_records[1].flags_10 |= 0x01U;
    selector_assets.field_entity_records[1].flags_10 |= 0x01U;
    NativePortShell selector("SHADOWRUN", std::move(selector_assets));
    open_medical_action(selector);
    assert(selector.field_first_aid_target_count() == 2U);
    assert(selector.field_first_aid_target_index() == 0U);
    assert(selector.field_first_aid_target_record() == 0U);
    press(selector, Button::Down);
    assert(selector.field_first_aid_target_record() == 1U);
    press(selector, Button::Down);
    assert(selector.field_first_aid_cancel_selected());
    press(selector, Button::Up);
    assert(selector.field_first_aid_target_record() == 1U);
    press(selector, Button::Up);
    assert(selector.field_first_aid_target_record() == 0U);
    SoftwareScreen selector_screen;
    selector.render(selector_screen);
    press(selector, Button::B);
    assert(!selector.field_first_aid_active());
    assert(!selector.field_menu_action_active());

    // The shipped Joshua record starts with a damaged physical value and a
    // six-use medkit. 0x1253E uses the source Biotech roll, 0x3674/0x37EC
    // clamp both health values, and 0x124DC consumes exactly one phase unit.
    auto medkit_assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    medkit_assets.protagonist_archetype_records[0].physical_value = 4U;
    medkit_assets.protagonist_archetype_records[0].mental_value = 8U;
    NativePortShell medkit("SHADOWRUN", std::move(medkit_assets));
    open_medical_action(medkit);
    const auto physical_before = medkit.field_record_physical_value(0U);
    const auto mental_before = medkit.field_record_mental_value(0U);
    const auto phase_before = medkit.field_inventory_phase(3U);
    assert(phase_before == 6U);
    press(medkit, Button::A);
    assert(medkit.field_first_aid_message_active());
    assert(medkit.field_first_aid_message() == "Medkit used.");
    assert(medkit.field_record_physical_value(0U) > physical_before);
    assert(medkit.field_record_mental_value(0U) > mental_before);
    assert(medkit.field_inventory_phase(3U) == phase_before - 1U);
    press(medkit, Button::A);
    assert(!medkit.field_first_aid_active());
    assert(!medkit.field_menu_action_active());

    // 0x12548/0x12552 is a no-use result: a fully healed target keeps the
    // item and displays the source D1F28/D1F37 message.
    auto full_assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    full_assets.protagonist_archetype_records[0].physical_value = 10U;
    full_assets.protagonist_archetype_records[0].mental_value = 10U;
    NativePortShell full("SHADOWRUN", std::move(full_assets));
    open_medical_action(full);
    assert(full.field_inventory_phase(3U) == 6U);
    press(full, Button::A);
    assert(full.field_first_aid_message() == "This person is not injured!");
    assert(full.field_inventory_item_code(3U) == 0x10U);
    assert(full.field_inventory_phase(3U) == 6U);
    press(full, Button::A);
    assert(!full.field_menu_action_active());

    // 0x1247E/0x1248C is the mental slap-patch (+2 on the 0..10 scale).
    auto mental_patch_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    mental_patch_assets.protagonist_archetype_records[0].mental_value = 4U;
    mental_patch_assets.protagonist_archetype_records[0].inventory_item_codes[3] =
        0x11U;
    mental_patch_assets.protagonist_archetype_records[0].inventory_phase_bytes[3] =
        1U;
    NativePortShell mental_patch("SHADOWRUN", std::move(mental_patch_assets));
    open_medical_action(mental_patch);
    const auto mental_patch_before =
        mental_patch.field_record_mental_value(0U);
    press(mental_patch, Button::A);
    assert(mental_patch.field_first_aid_message() == "Slap-patch used.");
    assert(mental_patch.field_record_mental_value(0U) ==
           mental_patch_before + 2U);
    assert(mental_patch.field_inventory_item_code(3U) == 0U);
    assert(mental_patch.field_inventory_phase(3U) == 0U);

    // Codes 0x12/0x13 share the physical patch path at 0x12466. This guards
    // the source distinction from the medkit's Biotech-based branch.
    auto physical_patch_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    physical_patch_assets.protagonist_archetype_records[0].physical_value = 4U;
    physical_patch_assets.protagonist_archetype_records[0].inventory_item_codes[3] =
        0x12U;
    physical_patch_assets.protagonist_archetype_records[0].inventory_phase_bytes[3] =
        2U;
    NativePortShell physical_patch("SHADOWRUN", std::move(physical_patch_assets));
    open_medical_action(physical_patch);
    const auto physical_patch_before =
        physical_patch.field_record_physical_value(0U);
    press(physical_patch, Button::A);
    assert(physical_patch.field_first_aid_message() == "Slap-patch used.");
    assert(physical_patch.field_record_physical_value(0U) ==
           physical_patch_before + 2U);
    assert(physical_patch.field_inventory_phase(3U) == 1U);
}

void test_automatic_use_item_handler(const std::vector<std::uint8_t>& rom) {
    // 0x12168 sends 0x14, 0x16, and 0x18..0x2B to 0x121C0. That routine
    // displays D202F and waits; it does not equip, consume, or reinterpret
    // possession as equipment. Exercise the path through the real action
    // selector rather than calling a native helper directly.
    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.protagonist_archetype_records[0].inventory_item_codes[0] = 0x14U;
    assets.protagonist_archetype_records[0].inventory_phase_bytes[0] = 7U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    press(shell, Button::Start);
    select_field_inventory_slot(shell, 0U);
    const auto active_before = shell.field_active_item_code();
    const auto slot_before = shell.field_equipped_weapon_slot();
    press(shell, Button::A);
    assert(shell.field_menu_action_active());
    assert(shell.field_menu_action_selection() == 2U);
    press(shell, Button::A);
    assert(shell.field_item_message_active());
    assert(shell.field_item_message() ==
           "This item is automatically used.");
    assert(shell.field_inventory_item_code(0U) == 0x14U);
    assert(shell.field_inventory_phase(0U) == 7U);
    assert(shell.field_active_item_code() == active_before);
    assert(shell.field_equipped_weapon_slot() == slot_before);

    // 0x0D94 acknowledgement returns to the parent action loop without
    // silently applying an equipment change.
    press(shell, Button::B);
    assert(!shell.field_item_message_active());
    assert(!shell.field_menu_action_active());
    assert(shell.field_inventory_item_code(0U) == 0x14U);
}

void test_trade_handler(const std::vector<std::uint8_t>& rom) {
    // The shipped startup party has no eligible recipient. The reviewed
    // source path does not fabricate a new runner or an error panel here.
    auto no_recipient_assets =
        shadowrun::graphics::decode_genesis_native_assets(rom);
    NativePortShell no_recipient("SHADOWRUN", std::move(no_recipient_assets));
    enter_field(no_recipient);
    no_recipient.update(InputState{button_mask(Button::Start),
                                   button_mask(Button::Start)});
    select_field_inventory_slot(no_recipient, 0U);
    no_recipient.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    press(no_recipient, Button::Right);
    press(no_recipient, Button::Right);
    no_recipient.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(!no_recipient.field_trade_active());
    assert(no_recipient.field_menu_action_active());
    assert(no_recipient.field_inventory_item_code(0U) == 2U);

    auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    assets.initial_field_records[1].flags_10 |= 0x01U;
    assets.field_entity_records[1].flags_10 |= 0x01U;
    NativePortShell shell("SHADOWRUN", std::move(assets));
    enter_field(shell);
    shell.update(InputState{button_mask(Button::Start),
                            button_mask(Button::Start)});
    select_field_inventory_slot(shell, 0U);
    assert(shell.field_inventory_item_code(0U) == 2U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    press(shell, Button::Right);
    press(shell, Button::Right);
    assert(shell.field_menu_action_selection() == 4U);
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.field_trade_active());
    assert(!shell.field_trade_cannot_trade());
    assert(shell.field_trade_message().empty());
    const auto recipient = shell.field_trade_recipient_record();
    const auto recipient_slot = shell.field_trade_recipient_slot();
    assert(recipient != 0U);
    assert(recipient_slot < 8U);
    assert(!shell.field_trade_confirmation_yes());

    // 0x127B0 starts at No. The source only mutates at 0x129FE after the
    // affirmative A edge, so the first confirmation cannot move possession.
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(shell.field_inventory_item_code(0U) == 2U);
    assert(shell.field_trade_active());
    press(shell, Button::Right);
    assert(shell.field_trade_confirmation_yes());
    shell.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(!shell.field_trade_active());
    assert(!shell.field_menu_action_active());
    assert(shell.field_inventory_item_code(0U) == 0U);
    assert(shell.field_active_item_code() == 0U);
    assert(shell.field_equipped_weapon_slot() == 0x7F);

    // The destination is the source-selected recipient's first empty cell.
    // Switch through the actual C roster path and inspect the transferred
    // possession, rather than adding a hidden recipient accessor to the test.
    shell.update(InputState{button_mask(Button::C), button_mask(Button::C)});
    assert(shell.field_active_record_index() == recipient);
    assert(shell.field_record_inventory_item_code(recipient, recipient_slot) ==
           2U);

    // 0x1283A..0x12846 rejects magic/special codes 0x17..0x2B before the
    // recipient scan and displays D2015/D2022. It must not mutate equipment
    // or possession.
    auto rejected_assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    rejected_assets.initial_field_records[1].flags_10 |= 0x01U;
    rejected_assets.field_entity_records[1].flags_10 |= 0x01U;
    rejected_assets.protagonist_archetype_records[0].inventory_item_codes[0] =
        0x17U;
    NativePortShell rejected("SHADOWRUN", std::move(rejected_assets));
    enter_field(rejected);
    rejected.update(InputState{button_mask(Button::Start),
                               button_mask(Button::Start)});
    select_field_inventory_slot(rejected, 0U);
    rejected.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    press(rejected, Button::Right);
    press(rejected, Button::Right);
    rejected.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(rejected.field_trade_active());
    assert(rejected.field_trade_cannot_trade());
    assert(rejected.field_trade_message() == "Cannot trade magic items!");
    assert(rejected.field_inventory_item_code(0U) == 0x17U);
    rejected.update(InputState{button_mask(Button::B), button_mask(Button::B)});
    assert(!rejected.field_trade_active());
    assert(rejected.field_inventory_item_code(0U) == 0x17U);

    // 0x128BE..0x12904 reports the first eligible recipient's full inventory
    // rather than dropping the source item or silently choosing another one.
    auto full_assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    full_assets.initial_field_records[1].flags_10 |= 0x01U;
    full_assets.field_entity_records[1].flags_10 |= 0x01U;
    for (auto& code : full_assets.field_entity_records[1].inventory_item_codes) {
        code = 2U;
    }
    full_assets.field_entity_records[1].flags_10 |= 0x01U;
    NativePortShell full("SHADOWRUN", std::move(full_assets));
    enter_field(full);
    full.update(InputState{button_mask(Button::Start),
                           button_mask(Button::Start)});
    select_field_inventory_slot(full, 0U);
    full.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    press(full, Button::Right);
    press(full, Button::Right);
    full.update(InputState{button_mask(Button::A), button_mask(Button::A)});
    assert(full.field_trade_active());
    assert(full.field_trade_message().find("full inventory") !=
           std::string_view::npos);
    assert(full.field_inventory_item_code(0U) == 2U);
}

} // namespace

int main() {
    const auto rom = read_rom();
    const auto assets = shadowrun::graphics::decode_genesis_native_assets(rom);
    test_startup_behavior_and_rendering(rom, assets);
    test_field_menu_behavior_and_rendering(rom, assets);
    test_no_clip_gate(rom);
    test_clip_item_gate(rom);
    test_clip_trade_menu(rom);
    test_clip_distribution(rom);
    test_first_aid_handler(rom);
    test_automatic_use_item_handler(rom);
    test_trade_handler(rom);
    return 0;
}

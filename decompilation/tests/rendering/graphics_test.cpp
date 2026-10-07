#include "shadowrun/rendering/graphics.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
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

} // namespace

int main() {
    using namespace shadowrun::graphics;

    const auto white = decode_genesis_cram_word(0x0EEE);
    assert(white.red == 255 && white.green == 255 && white.blue == 255);
    const auto red = decode_genesis_cram_word(0x000E);
    assert(red.red == 255 && red.green == 0 && red.blue == 0);

    std::vector<std::uint8_t> tile(32, 0);
    tile[0] = 0x12;
    tile[1] = 0x48;
    const auto indexed = decode_genesis_tile(tile.data(), tile.size());
    assert(indexed.pixels[0] == 1);
    assert(indexed.pixels[1] == 2);
    assert(indexed.pixels[2] == 4);
    assert(indexed.pixels[3] == 8);
    assert(indexed.pixels[4] == 0);

    const std::vector<std::uint8_t> compressed_run = {
        0x00, 0x02, 0x00, 0xAB, 0xAB, 0xAB
    };
    const auto decoded_run = decode_genesis_resource_stream(
        compressed_run, 0);
    assert((decoded_run == std::vector<std::uint8_t>{0xAB, 0xAB, 0xAB}));

    const std::vector<std::uint8_t> compressed_backref = {
        0x00, 0x03, 0x40, 0x41, 0x90, 0x00
    };
    const auto decoded_backref = decode_genesis_resource_stream(
        compressed_backref, 0);
    assert((decoded_backref == std::vector<std::uint8_t>{
        0x41, 0x41, 0x41, 0x41}));

    std::array<shadowrun::native::Rgba8, 16> palette{};
    palette[0] = {0, 0, 0, 255};
    palette[1] = {255, 0, 0, 255};
    palette[2] = {0, 255, 0, 255};
    palette[4] = {0, 0, 255, 255};
    palette[8] = {255, 255, 255, 255};
    const auto atlas = decode_tile_atlas(tile, 0, 1, palette, 1, 0);
    assert(atlas.width == 8 && atlas.height == 8);
    assert(atlas.pixels[0].alpha == 255 && atlas.pixels[0].red == 255);
    assert(atlas.pixels[1].green == 255);
    assert(atlas.pixels[2].blue == 255);
    assert(atlas.pixels[3].red == 255 && atlas.pixels[3].green == 255);
    assert(atlas.pixels[4].alpha == 0);

    const auto rom = read_rom();
    // ROM 0x5ACF6 writes D6=-0x2000 for the three timed tri-vid SAT lists.
    // Preserve the resulting palette-line-3/high-priority attributes in the
    // offline extractor so rebuilding named assets cannot recolor the timed
    // tri-vid sprite lists with palette line 1.
    const auto intro_sprites = decode_genesis_intro_sprite_resources(rom);
    std::array<bool, 3> trivid_timed_lists{};
    for (const auto& sprite : intro_sprites) {
        if (sprite.role != "trivid-timed") {
            continue;
        }
        assert(sprite.table_offset == 0x5AD02U);
        assert(sprite.table_index < trivid_timed_lists.size());
        assert(sprite.attr_or == 0xE000U);
        assert(sprite.image.width > 0U && sprite.image.height > 0U);
        assert(!sprite.image.pixels.empty());
        trivid_timed_lists[sprite.table_index] = true;
    }
    assert(std::all_of(trivid_timed_lists.begin(), trivid_timed_lists.end(),
                       [](const bool present) { return present; }));

    const auto stoker_portrait = decode_genesis_field_dialogue_portrait(rom, 0U);
    assert(stoker_portrait.width == 56 && stoker_portrait.height == 72);
    assert(stoker_portrait.pixels.size() ==
           static_cast<std::size_t>(stoker_portrait.width) *
               stoker_portrait.height);
    const auto stoker_opaque = std::count_if(
        stoker_portrait.pixels.begin(), stoker_portrait.pixels.end(),
        [](const auto& pixel) { return pixel.alpha != 0U; });
    assert(stoker_opaque > 500U);

    const auto stoker_arrival_text =
        read_genesis_dialogue_segments(rom, 0x1167EBU);
    assert((stoker_arrival_text ==
            std::vector<std::string>{
                "<The battered sign reads Stoker's Coffin Motel.",
                "",
                "Motels like these are cheap, each room only large enough to fit a small bed.",
                "",
                "Dumps like these tend to attract the dregs of humanity.>"}));
    const auto stoker_more_text =
        read_genesis_dialogue_segments(rom, 0x0FE5BFU);
    assert((stoker_more_text ==
            std::vector<std::string>{"", "          < MORE >"}));
    const auto stoker_package_option =
        read_genesis_dialogue_segments(rom, 0x103A55U);
    assert((stoker_package_option ==
            std::vector<std::string>{"I'm here to pick up Michael's stuff."}));

    const auto field_tile = decode_genesis_tile(
        rom.data() + 0x166B82, 32);
    assert(field_tile.pixels[0] == 0);
    assert(field_tile.pixels[1] == 0);
    assert(field_tile.pixels[2] == 0x0D);
    assert(field_tile.pixels[3] == 0x0C);
    assert(field_tile.pixels[8] == 0x0D);
    assert(field_tile.pixels[9] == 0x0D);
    const auto command_labels = read_genesis_command_overview_labels(rom);
    assert((command_labels == std::array<std::string, 7>{
        "Inventory", "Inventory", "Clips", "Attributes/Skills",
        "Cyberware", "Magic", "Pocket Secretary"}));
    const auto command_names = read_genesis_command_overview_names(rom);
    assert((command_names.races == std::array<std::string, 5>{
        "Human", "  Elf", "Dwarf", "  Orc", "Troll"}));
    assert((command_names.classes == std::array<std::string, 4>{
        "Mage   ", "Shaman ", "Samurai", "Decker "}));
    const auto command_cursor = decode_genesis_field_menu_cursor(rom);
    assert(command_cursor.width == 32U);
    assert(command_cursor.height == 24U);
    const auto item_detail_labels =
        read_genesis_field_menu_item_detail_labels(rom);
    assert((item_detail_labels ==
            std::array<std::string, 3>{"ammo", "uses", "rtng"}));
    const auto field_labels = read_genesis_field_menu_labels(rom);
    assert(field_labels.physical == "Physical");
    assert(field_labels.mental == "Mental");
    assert(field_labels.attack == "Atk");
    assert(field_labels.defense == "Def");
    assert(field_labels.damage == "Damage");
    assert(field_labels.drain == "Drain");
    assert(field_labels.none == "None");
    assert(field_labels.no_armor == "No Armor");
    assert(field_labels.weapons == "Wpns");
    assert(field_labels.guns == "Guns");
    assert(field_labels.power == "Power");
    const auto field_service_titles = read_genesis_field_service_titles(rom);
    assert(field_service_titles[0].text == "Weapons World");
    assert(field_service_titles[1].text == "Ares Weapon Emporium");
    assert(field_service_titles[1].destination_word == 0xC110U);
    const auto ares_mode_sprite =
        decode_genesis_field_mode_profile_sprite(rom, 0x1220C0U);
    assert(ares_mode_sprite.width == 56U);
    assert(ares_mode_sprite.height == 72U);
    assert(std::any_of(ares_mode_sprite.pixels.begin(),
                       ares_mode_sprite.pixels.end(),
                       [](const shadowrun::native::Rgba8& pixel) {
                           return pixel.alpha != 0U;
                       }));
    const auto entity_template =
        read_genesis_field_entity_template(rom);
    assert(entity_template.record_offset == 0x7291A);
    assert(entity_template.projection_word_08 == 0);
    assert(entity_template.kind == 3);
    assert(entity_template.flags_10 == 0x0F);
    assert(entity_template.flags_11 == 0);
    assert(entity_template.state_pointer == 0);
    assert(entity_template.state_selector == 0);
    assert(entity_template.timing_multiplier == 0x1000);
    assert(entity_template.timing_accumulator == 0);
    assert(entity_template.timing_threshold == 0x1000);
    assert(entity_template.state_index == 0);
    assert(entity_template.display_state == 0);
    assert(entity_template.display_substate == 2);
    assert((entity_template.inventory_item_codes ==
            std::array<std::uint8_t, 8>{2, 0x2C, 0x0E, 0x10, 0, 0, 0, 0}));
    assert((entity_template.inventory_phase_bytes ==
            std::array<std::uint8_t, 8>{0, 0, 6, 6, 0, 0, 0, 0}));
    assert(entity_template.equipped_armor_index == 1);
    assert(entity_template.ammo_rounds == 0x20);
    assert(entity_template.ammo_clips == 0x0A);
    assert(entity_template.cyberware_flags == 0x0004);
    assert((entity_template.attribute_values ==
            std::array<std::uint8_t, 8>{4, 3, 4, 1, 2, 4, 3, 5}));
    assert((entity_template.skill_values ==
            std::array<std::uint8_t, 12>{0, 4, 0, 0, 0, 3, 2, 0, 1, 0,
                                         1, 1}));
    assert((entity_template.attribute_skill_auxiliary ==
            std::array<std::uint8_t, 4>{0, 0, 0, 9}));
    const auto recruited_roster = read_genesis_recruited_roster(rom);
    assert(recruited_roster.size() == 10);
    assert(recruited_roster[0].record_offset == 0x72C1A);
    assert(recruited_roster[0].display_name == "Ricky");
    assert(recruited_roster[1].display_name == "Winston Marrs");
    assert(recruited_roster[3].display_name == "Petr Uvehr");
    assert(recruited_roster[4].display_name == "Walking Bear");
    assert(recruited_roster[9].display_name == "Stark");
    assert(recruited_roster[0].record_type == 3);
    assert(recruited_roster[0].active_item_code == 1);
    assert((recruited_roster[0].inventory_item_codes ==
            std::array<std::uint8_t, 8>{1, 0x2D, 0x11, 0x12, 0x13, 0, 0,
                                        0x29}));
    assert(recruited_roster[3].cyberware_flags == 1);
    assert(recruited_roster[9].cyberware_flags == 0x282A);
    const auto inventory_catalog = read_genesis_inventory_catalog(rom);
    assert(inventory_catalog.size() == 50);
    assert(inventory_catalog[1].code == 2);
    assert(inventory_catalog[1].source_combat_factor == 12);
    assert(inventory_catalog[1].name == "Model 101T");
    assert(inventory_catalog[1].category == "Light Pistol");
    assert(inventory_catalog[2].code == 3);
    assert(inventory_catalog[2].source_combat_factor == 11);
    assert(inventory_catalog[43].code == 44);
    assert(inventory_catalog[43].name == "Armor Vest");
    const auto cyberware_catalog = read_genesis_cyberware_catalog(rom);
    assert(cyberware_catalog.size() == 15);
    assert(cyberware_catalog[0] == "Datajack");
    assert(cyberware_catalog[1] == "CyberEyes");
    assert(cyberware_catalog[2] == "Hand Razors");
    assert(cyberware_catalog[14] == "Wired Reflexes(3)");
    const auto cyberware_text = read_genesis_cyberware_text(rom);
    assert(cyberware_text.title ==
           std::string(" CYBERWARE\n[") + std::string(10U, '\\'));
    assert(cyberware_text.empty_message == "NONE");
    assert((cyberware_text.entries ==
            std::array<std::string, 15>{
                "    Datajack", "    CyberEyes", "   Hand Razors",
                "      Spurs", "    Smartlink", "Muscle Replace(1)",
                "Muscle Replace(2)", "Muscle Replace(3)",
                "Muscle Replace(4)", "Dermal Plating(1)",
                "Dermal Plating(2)", "Dermal Plating(3)",
                "Wired Reflexes(1)", "Wired Reflexes(2)",
                "Wired Reflexes(3)"}));
    const auto field_action_text =
        read_genesis_field_menu_action_text(rom);
    assert(field_action_text.automatic_use_message ==
           "This item is\nautomatically\n    used.");
    assert(field_action_text.trade_recipient_label == "Give to");
    assert(field_action_text.trade_full_inventory_message ==
           "  has a full\n  inventory.\n\nPress a button\n to continue.");
    assert(field_action_text.transaction_prompt == "Complete Transaction?");
    assert(field_action_text.no_inventory_slot_message ==
           "Your inventory\n   is full!");
    assert(field_action_text.insufficient_funds_message ==
           " You do not have the\nnuyen for this item!!");
    assert(field_action_text.special_action_limit_message ==
           "  You are already\ncarrying the maximum\n  number ");
    assert(field_action_text.protected_sale_message ==
           "Totems cannot be sold!!");
    assert(field_action_text.sale_quote_heading == "Item sells for       $");
    assert(field_action_text.purchase_price_heading == "Price        $");
    const auto field_action_detail_texts =
        read_genesis_field_action_detail_texts(rom);
    assert(field_action_detail_texts.size() == 32U);
    assert(field_action_detail_texts[16] ==
           "A slap patch that instantly\nincreases mental health\nby 20%.");
    const auto action_confirmation_background =
        decode_genesis_field_menu_action_confirmation_background(rom);
    assert(action_confirmation_background.width == 120U);
    assert(action_confirmation_background.height == 16U);
    const auto action_background =
        decode_genesis_field_menu_action_background(rom);
    const auto clips_background =
        decode_genesis_field_menu_clips_background(rom);
    const auto clips_trade_background =
        decode_genesis_field_menu_clips_trade_background(rom);
    assert(clips_background.width == 120U);
    assert(clips_background.height == 16U);
    assert(clips_trade_background.width == 120U);
    assert(clips_trade_background.height == 16U);
    const auto images_differ = [](const RgbaImage& left,
                                  const RgbaImage& right) {
        if (left.width != right.width || left.height != right.height ||
            left.pixels.size() != right.pixels.size()) {
            return true;
        }
        for (std::size_t index = 0; index < left.pixels.size(); ++index) {
            const auto& a = left.pixels[index];
            const auto& b = right.pixels[index];
            if (a.red != b.red || a.green != b.green || a.blue != b.blue ||
                a.alpha != b.alpha) {
                return true;
            }
        }
        return false;
    };
    assert(images_differ(action_background, clips_background));
    assert(images_differ(clips_background, clips_trade_background));
    assert(images_differ(clips_trade_background,
                         action_confirmation_background));
    const auto action_confirmation_focus =
        decode_genesis_field_menu_action_confirmation_focus(rom);
    assert(action_confirmation_focus.size() == 2U);
    for (const auto& focus : action_confirmation_focus) {
        assert(focus.width == 16U);
        assert(focus.height == 16U);
        assert(focus.pixels.size() == 16U * 16U);
        assert(std::all_of(focus.pixels.begin(), focus.pixels.end(),
                           [](const auto& pixel) {
                               return pixel.alpha == 255U;
                           }));
    }
    const auto dialogue_frame =
        decode_genesis_field_dialogue_panel_background(rom, false);
    const auto dialogue_frame_alternate =
        decode_genesis_field_dialogue_panel_background(rom, true);
    assert(dialogue_frame.width == 320U && dialogue_frame.height == 224U);
    assert(dialogue_frame_alternate.width == 320U &&
           dialogue_frame_alternate.height == 224U);
    const auto field_service_background =
        decode_genesis_field_service_background(rom);
    assert(field_service_background.width == 320U &&
           field_service_background.height == 224U);
    assert(field_service_background.pixels.size() == 320U * 224U);
    assert(std::all_of(field_service_background.pixels.begin(),
                       field_service_background.pixels.end(),
                       [](const auto& pixel) { return pixel.alpha == 255U; }));
    assert(std::any_of(field_service_background.pixels.begin(),
                       field_service_background.pixels.end(),
                       [](const auto& pixel) {
                           return pixel.red != 0U || pixel.green != 0U ||
                                  pixel.blue != 0U;
                       }));
    const auto service_pixel = [&](std::size_t x, std::size_t y)
        -> const shadowrun::native::Rgba8& {
        return field_service_background.pixels[
            y * field_service_background.width + x];
    };
    const auto& service_frame_edge = service_pixel(12U, 12U);
    assert(service_frame_edge.red == 219U &&
           service_frame_edge.green == 219U &&
           service_frame_edge.blue == 219U);
    const auto& service_text_well = service_pixel(16U, 16U);
    assert(service_text_well.red == 0U && service_text_well.green == 0U &&
           service_text_well.blue == 0U);
    const auto& service_right_panel = service_pixel(300U, 20U);
    assert(service_right_panel.red == 73U &&
           service_right_panel.green == 73U &&
           service_right_panel.blue == 73U);
    const auto& service_lower_right_clear = service_pixel(319U, 223U);
    assert(service_lower_right_clear.red == 0U &&
           service_lower_right_clear.green == 0U &&
           service_lower_right_clear.blue == 0U);
    bool dialogue_left_panel_differs = false;
    for (std::size_t y = 0; y < dialogue_frame.height; ++y) {
        for (std::size_t x = 0; x < dialogue_frame.width; ++x) {
            const auto index = y * dialogue_frame.width + x;
            const auto& standard = dialogue_frame.pixels[index];
            const auto& alternate = dialogue_frame_alternate.pixels[index];
            if (x < 80U &&
                (standard.red != alternate.red ||
                 standard.green != alternate.green ||
                 standard.blue != alternate.blue ||
                 standard.alpha != alternate.alpha)) {
                dialogue_left_panel_differs = true;
            }
            if (x >= 80U) {
                assert(standard.red == alternate.red &&
                       standard.green == alternate.green &&
                       standard.blue == alternate.blue &&
                       standard.alpha == alternate.alpha);
            }
        }
    }
    assert(dialogue_left_panel_differs);
    const auto dialogue_buttons =
        decode_genesis_field_dialogue_option_markers(rom);
    for (const auto& button : dialogue_buttons) {
        assert(button.width == 16U && button.height == 16U);
        assert(button.pixels.size() == 16U * 16U);
        assert(std::any_of(button.pixels.begin(), button.pixels.end(),
                           [](const auto& pixel) {
                               return pixel.alpha != 0U;
                           }));
    }
    for (std::size_t left = 0; left < dialogue_buttons.size(); ++left) {
        for (std::size_t right = left + 1U;
             right < dialogue_buttons.size(); ++right) {
            bool differs = false;
            for (std::size_t pixel = 0;
                 pixel < dialogue_buttons[left].pixels.size(); ++pixel) {
                const auto& a = dialogue_buttons[left].pixels[pixel];
                const auto& b = dialogue_buttons[right].pixels[pixel];
                differs = differs || a.red != b.red || a.green != b.green ||
                          a.blue != b.blue || a.alpha != b.alpha;
            }
            assert(differs);
        }
    }
    const auto attribute_skill_catalog =
        read_genesis_attribute_skill_catalog(rom);
    assert(attribute_skill_catalog.size() == 20);
    assert(attribute_skill_catalog[0] == "Body");
    assert(attribute_skill_catalog[1] == "Quickness");
    assert(attribute_skill_catalog[7] == "Magic");
    assert(attribute_skill_catalog[8] == "Sorcery");
    assert(attribute_skill_catalog[19] == "Negotiation");
    const auto intro = read_genesis_intro_data(rom);
    assert(intro.start_options_title == "Start Options");
    assert((intro.start_options ==
            std::array<std::string, 2>{"New Game", "Load Game"}));
    assert(intro.archetype_title == "Choose Archetype");
    assert((intro.archetypes ==
            std::array<std::string, 4>{"Samurai", "Decker", "Gator",
                                       "Shaman"}));
    assert(!intro.start_options_layout.empty());
    assert(intro.start_options_layout[0].text.find("Start Options") !=
           std::string::npos);
    bool saw_title_underline = false;
    for (const auto& line : intro.start_options_layout) {
        if (!line.text.empty() && line.text.front() == '[') {
            saw_title_underline = true;
        }
    }
    assert(saw_title_underline);
    const auto portraits = decode_genesis_field_menu_runner_portraits(rom);
    assert(!portraits[0].pixels.empty());
    assert(!portraits[1].pixels.empty());
    assert(!portraits[2].pixels.empty());
    const auto portraits_differ = [](const auto& left, const auto& right) {
        if (left.pixels.size() != right.pixels.size()) {
            return true;
        }
        for (std::size_t index = 0; index < left.pixels.size(); ++index) {
            const auto& a = left.pixels[index];
            const auto& b = right.pixels[index];
            if (a.red != b.red || a.green != b.green || a.blue != b.blue ||
                a.alpha != b.alpha) {
                return true;
            }
        }
        return false;
    };
    assert(portraits_differ(portraits[0], portraits[1]));
    assert(portraits_differ(portraits[0], portraits[2]));
    assert(portraits_differ(portraits[1], portraits[2]));
    const auto magic_posture_bars =
        decode_genesis_field_magic_posture_bars(rom);
    const auto magic_force_meter_tiles =
        decode_genesis_field_magic_force_meter_tiles(rom);
    const auto magic_availability_bars =
        decode_genesis_field_magic_availability_bars(rom);
    const auto magic_damage_bars =
        decode_genesis_field_magic_damage_bars(rom);
    const auto magic_menu_text = read_genesis_magic_menu_text(rom);
    for (const auto& bar : magic_posture_bars) {
        assert(bar.width == 64U && bar.height == 8U);
        assert(bar.pixels.size() == 64U * 8U);
        assert(std::any_of(bar.pixels.begin(), bar.pixels.end(),
                           [](const auto& pixel) {
                               return pixel.alpha != 0U;
                           }));
    }
    for (const auto& tile : magic_force_meter_tiles) {
        assert(tile.width == 8U && tile.height == 8U);
        assert(tile.pixels.size() == 64U);
    }
    for (const auto& bar : magic_availability_bars) {
        assert(bar.width == 56U && bar.height == 8U);
        assert(bar.pixels.size() == 56U * 8U);
    }
    for (const auto& bar : magic_damage_bars) {
        assert(bar.width == 56U && bar.height == 8U);
        assert(bar.pixels.size() == 56U * 8U);
    }
    assert(portraits_differ(magic_posture_bars[0],
                            magic_posture_bars[4]));
    assert(portraits_differ(magic_force_meter_tiles[3],
                            magic_force_meter_tiles[4]));
    assert(magic_menu_text.set_force_and_posture ==
           "Set Force & Posture.");
    assert(magic_menu_text.success_label == "Success");
    assert(magic_menu_text.force_label == "Force");
    assert(magic_menu_text.availability_label == "Damage");
    const auto field_menu_font = decode_genesis_field_menu_font(rom);
    const auto field_menu_font_palette_2 =
        decode_genesis_field_menu_font(rom, 2U);
    assert(field_menu_font.width == 128U && field_menu_font.height == 48U);
    assert(field_menu_font_palette_2.width == field_menu_font.width &&
           field_menu_font_palette_2.height == field_menu_font.height);
    assert(std::any_of(
        field_menu_font_palette_2.pixels.begin(),
        field_menu_font_palette_2.pixels.end(), [](const auto& pixel) {
            return pixel.alpha != 0U && pixel.red == 109U &&
                   pixel.green == 73U && pixel.blue == 146U;
        }));
    const auto pocket_secretary_grid =
        decode_genesis_pocket_secretary_grid(rom);
    assert(pocket_secretary_grid.width == 224U &&
           pocket_secretary_grid.height == 168U &&
           pocket_secretary_grid.pixels.size() == 224U * 168U);
    for (std::size_t y = 0; y < pocket_secretary_grid.height; ++y) {
        for (std::size_t x = 0; x < pocket_secretary_grid.width; ++x) {
            const auto repeated_tile_pixel =
                (y % 8U) * pocket_secretary_grid.width + (x % 8U);
            const auto& actual = pocket_secretary_grid.pixels[
                y * pocket_secretary_grid.width + x];
            const auto& expected =
                pocket_secretary_grid.pixels[repeated_tile_pixel];
            assert(actual.red == expected.red &&
                   actual.green == expected.green &&
                   actual.blue == expected.blue &&
                   actual.alpha == expected.alpha);
        }
    }
    const auto startup_cursor = decode_genesis_startup_cursor(rom);
    const auto field_font_ink = std::find_if(
        field_menu_font.pixels.begin(), field_menu_font.pixels.end(),
        [](const auto& pixel) { return pixel.alpha != 0U; });
    assert(field_font_ink != field_menu_font.pixels.end());
    assert(std::any_of(
        startup_cursor.pixels.begin(), startup_cursor.pixels.end(),
        [](const auto& pixel) { return pixel.alpha != 0U; }));
    for (const auto& pixel : startup_cursor.pixels) {
        if (pixel.alpha != 0U) {
            assert(pixel.red == field_font_ink->red &&
                   pixel.green == field_font_ink->green &&
                   pixel.blue == field_font_ink->blue);
        }
    }
    assert(intro.story_lines.size() == 22);
    assert(intro.story_lines.front() == "The Time:  January 31, 2058");
    assert(intro.story_lines.back() ==
           "in the Redmond Barrens.");
    const auto metatile_resource = decode_genesis_resource_stream(rom, 0x170102);
    assert(metatile_resource.size() == 0x4F40);
    const auto field_resources = read_genesis_mode2_field_resources(rom);
    assert(field_resources.record_offset == 0x172C80);
    assert(field_resources.initial_camera_x_fixed == 0x3400);
    assert(field_resources.initial_camera_y_fixed == 0x2280);
    assert(field_resources.initial_field_records[0].world_x_fixed == 0x3CE0);
    assert(field_resources.initial_field_records[0].world_y_fixed == 0x2D00);
    assert(field_resources.initial_field_records[1].world_x_fixed == 0x3BA0);
    assert(field_resources.initial_field_records[1].world_y_fixed == 0x2D00);
    assert(field_resources.initial_field_records[2].world_x_fixed == 0x3DA0);
    assert(field_resources.initial_field_records[2].world_y_fixed == 0x2D00);
    assert(field_resources.terrain_x_exclusive_upper == 0x0754);
    assert(field_resources.terrain_y_exclusive_upper == 0x0518);
    assert(field_resources.terrain_x_inclusive_lower == 0x002C);
    assert(field_resources.terrain_y_inclusive_lower == 0x002C);
    assert(field_resources.terrain_record_offset == 0x172DAA);
    assert(field_resources.setup_block_offset == 0x172C50);
    assert(field_resources.palette_offset == 0x172D8A);
    assert(field_resources.pattern_data_offset == 0x166B82);
    assert(field_resources.pattern_byte_count == 0x6E20);
    assert(field_resources.metatile_stream_offset == 0x170102);
    assert(field_resources.primary_tilemap_offset == 0x16D9A2);
    assert(field_resources.secondary_tilemap_offset == 0x16ED52);
    assert(field_resources.tilemap_width == 60);
    assert(field_resources.tilemap_height == 42);
    assert(field_resources.event_table_offset == 0x172CDC);
    const std::array<std::uint8_t, 12> redmond_event_modes{
        0x14, 0x1A, 0x1A, 0x1A, 0x0D, 0x13,
        0x1A, 0x1A, 0x34, 0x1A, 0x32, 0x33,
    };
    for (std::size_t index = 0; index < redmond_event_modes.size();
         ++index) {
        assert(field_resources.event_mode_table[index] ==
               redmond_event_modes[index]);
    }

    const auto mode20_resources =
        read_genesis_field_mode_resources(rom, 20);
    assert(mode20_resources.record_offset == 0x1D917A);
    assert(mode20_resources.metatile_stream_offset == 0x1CD29A);
    assert(mode20_resources.metatile_encoding ==
           GenesisMetatileEncoding::RawNameWords);
    assert(mode20_resources.tilemap_width == 50);
    assert(mode20_resources.tilemap_height == 30);
    assert(mode20_resources.initial_field_records[0].world_x_fixed ==
           0x5A00);
    assert(mode20_resources.initial_field_records[0].world_y_fixed == 0x0800);
    const auto mode20_map =
        decode_genesis_field_mode_map(rom, mode20_resources);
    assert(mode20_map.width == 1600);
    assert(mode20_map.height == 960);

    // ROM 0x3074 uses the same descriptor layout for the small indoor-style
    // maps. Their two layer pointers are sometimes reversed relative to the
    // mode-2 record, so the reader derives the equal layer size from their
    // absolute separation rather than assuming pointer order.
    const auto mode10_resources =
        read_genesis_field_mode_resources(rom, 10);
    assert(mode10_resources.record_offset == 0x1D609A);
    assert(mode10_resources.primary_tilemap_offset == 0x1B25F4);
    assert(mode10_resources.secondary_tilemap_offset == 0x1B2568);
    assert(mode10_resources.tilemap_width == 10);
    assert(mode10_resources.tilemap_height == 7);
    assert(mode10_resources.pattern_data_offset == 0x1ACA20);
    assert(mode10_resources.pattern_byte_count == 0x53A0);
    const auto mode10_map =
        decode_genesis_field_mode_map(rom, mode10_resources);
    assert(mode10_map.width == 320);
    assert(mode10_map.height == 224);

    const auto mode11_resources =
        read_genesis_field_mode_resources(rom, 11);
    assert(mode11_resources.record_offset == 0x1D5A44);
    assert(mode11_resources.tilemap_width == 10);
    assert(mode11_resources.tilemap_height == 21);
    assert(mode11_resources.primary_tilemap_offset == 0x1B1ED8);
    assert(mode11_resources.secondary_tilemap_offset == 0x1B207C);
    const auto mode11_map =
        decode_genesis_field_mode_map(rom, mode11_resources);
    assert(mode11_map.width == 320);
    assert(mode11_map.height == 672);

    const auto mode15_resources =
        read_genesis_field_mode_resources(rom, 15);
    assert(mode15_resources.tilemap_width == 10);
    assert(mode15_resources.tilemap_height == 7);
    assert(mode15_resources.primary_tilemap_offset == 0x1B2194);
    assert(mode15_resources.secondary_tilemap_offset == 0x1B2108);
    const auto mode15_map =
        decode_genesis_field_mode_map(rom, mode15_resources);
    assert(mode15_map.width == 320);
    assert(mode15_map.height == 224);

    const auto terrain = decode_genesis_mode2_terrain_map(rom, field_resources);
    assert(terrain.region_count == 101);
    assert(terrain.statuses.size() ==
           GenesisTerrainMap::kWidth * GenesisTerrainMap::kHeight);
    assert(terrain.statuses[(552U / 32U) * GenesisTerrainMap::kWidth +
                             (832U / 32U)] == 0);
    assert(terrain.statuses[(536U / 32U) * GenesisTerrainMap::kWidth +
                             (864U / 32U)] == 0x7FU);
    const auto clear_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(832U * 16U),
        static_cast<std::uint16_t>(552U * 16U));
    assert(clear_probe.raw_status == 0 && clear_probe.status == 0 &&
           !clear_probe.blocked &&
           !clear_probe.out_of_bounds);
    const auto blocked_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(864U * 16U),
        static_cast<std::uint16_t>(536U * 16U));
    assert(blocked_probe.raw_status == 0x7FU &&
           blocked_probe.status == 0x7FU && blocked_probe.blocked &&
           !blocked_probe.out_of_bounds);
    const auto rectangle_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(600U * 16U),
        static_cast<std::uint16_t>(1250U * 16U));
    assert(rectangle_probe.raw_status == 1 &&
           rectangle_probe.status == 0x7FU && rectangle_probe.blocked);
    const auto positive_status_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(0x240U * 16U),
        static_cast<std::uint16_t>(0x4E0U * 16U), false);
    assert(positive_status_probe.raw_status == 1 &&
           positive_status_probe.status == 0 &&
           !positive_status_probe.blocked);
    const auto bounds_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(43U * 16U),
        static_cast<std::uint16_t>(552U * 16U));
    assert(bounds_probe.raw_status == 0x7FU &&
           bounds_probe.status == 0x7FU && bounds_probe.blocked &&
           bounds_probe.out_of_bounds);
    const auto field_map = decode_genesis_mode2_field_map(
        rom, field_resources);
    assert(field_map.width == 1920 && field_map.height == 1344);
    assert(field_map.pixels.size() ==
           static_cast<std::size_t>(field_map.width) * field_map.height);
    assert(field_map.pixels.front().alpha == 255);
    const auto first_pixel = field_map.pixels.front();
    bool varied = false;
    for (const auto pixel : field_map.pixels) {
        if (pixel.red != first_pixel.red || pixel.green != first_pixel.green ||
            pixel.blue != first_pixel.blue) {
            varied = true;
            break;
        }
    }
    assert(varied);

    const auto entity_frame = decode_genesis_entity_frame(rom);
    assert(entity_frame.width == 24 && entity_frame.height == 24);
    assert(entity_frame.pixels.size() ==
           static_cast<std::size_t>(entity_frame.width) * entity_frame.height);
    bool entity_has_transparency = false;
    bool entity_has_color = false;
    bool entity_has_chroma = false;
    for (const auto& pixel : entity_frame.pixels) {
        entity_has_transparency |= pixel.alpha == 0;
        entity_has_color |= pixel.alpha != 0;
        if (pixel.alpha != 0) {
            const auto maxc = std::max({pixel.red, pixel.green, pixel.blue});
            const auto minc = std::min({pixel.red, pixel.green, pixel.blue});
            entity_has_chroma |= (maxc - minc) > 40;
        }
    }
    assert(entity_has_transparency && entity_has_color);
    assert(entity_has_chroma);
    const auto entity_gray = make_grayscale_image(entity_frame);
    assert(entity_gray.pixels.size() == entity_frame.pixels.size());
    bool gray_is_grey = true;
    for (const auto& pixel : entity_gray.pixels) {
        if (pixel.alpha != 0 &&
            (pixel.red != pixel.green || pixel.green != pixel.blue)) {
            gray_is_grey = false;
        }
    }
    assert(gray_is_grey);

    const auto entity_animation = decode_genesis_entity_animation(rom);
    assert(entity_animation.size() == 7);
    assert(entity_animation.front().width == 24);
    assert(entity_animation.front().height == 24);
    for (const auto& frame : entity_animation) {
        assert(frame.width != 0 && frame.height != 0);
        assert(frame.pixels.size() ==
               static_cast<std::size_t>(frame.width) * frame.height);
    }

    const auto field_entity_animation =
        decode_genesis_field_entity_animation(rom);
    assert(field_entity_animation.size() == 8);
    assert(field_entity_animation.front().width == 24);
    assert(field_entity_animation.front().height == 32);
    const std::array<std::uint16_t, 8> field_entity_widths{
        24, 24, 16, 24, 32, 24, 16, 24};
    bool field_entity_directions_vary = false;
    for (std::size_t index = 0; index < field_entity_animation.size();
         ++index) {
        const auto& frame = field_entity_animation[index];
        assert(frame.width == field_entity_widths[index]);
        assert(frame.height == 32);
        assert(frame.pixels.size() ==
               static_cast<std::size_t>(frame.width) * frame.height);
        if (index != 0) {
            for (std::size_t pixel = 0; pixel < frame.pixels.size(); ++pixel) {
                const auto& left = frame.pixels[pixel];
                const auto& right = field_entity_animation.front().pixels[pixel];
                if (left.red != right.red || left.green != right.green ||
                    left.blue != right.blue || left.alpha != right.alpha) {
                    field_entity_directions_vary = true;
                    break;
                }
            }
        }
    }
    assert(field_entity_directions_vary);

    // The field record's +0x11 bit 5 is clear, so its 0x8002 source is
    // uploaded as raw packed tile bytes. Its +0x2C word selects palette line
    // one after the 0x172D8A/0x18AC56 field palette composition. Keep this
    // source check here so the fire/checkerboard regression cannot return.
    const auto field_raw_frame = decode_genesis_entity_frame(
        rom, 0x9D6E, 0x3EDC, 0x172D8A, false, 1, true);
    bool field_frame_matches_raw_source =
        field_entity_animation.front().width == field_raw_frame.width &&
        field_entity_animation.front().height == field_raw_frame.height &&
        field_entity_animation.front().pixels.size() ==
            field_raw_frame.pixels.size();
    if (field_frame_matches_raw_source) {
        for (std::size_t pixel = 0;
             pixel < field_raw_frame.pixels.size(); ++pixel) {
            const auto& left = field_entity_animation.front().pixels[pixel];
            const auto& right = field_raw_frame.pixels[pixel];
            if (left.red != right.red || left.green != right.green ||
                left.blue != right.blue || left.alpha != right.alpha) {
                field_frame_matches_raw_source = false;
                break;
            }
        }
    }
    assert(field_frame_matches_raw_source);

    const auto field_entity_animations =
        decode_genesis_field_entity_animation_sequences(rom);
    assert(field_entity_animations.size() == 8);
    const auto field_movement_animations =
        decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC062, 0x3EDC, 0x172D8A, 8);
    assert(field_movement_animations.size() == 8);
    // The behavior-transition table remains separate from the ordinary
    // accepted-movement selector at 0xC062.
    const auto hurt_animations = decode_genesis_field_entity_animation_sequences(
        rom, 0x7291A, 0x62AC, 0xC072, 0x3EDC, 0x172D8A, 8);
    assert(hurt_animations.size() == 8);
    const auto attack_animations =
        decode_genesis_field_entity_animation_sequences(
            rom, 0x7291A, 0x62AC, 0xC042, 0x3EDC, 0x172D8A, 8);
    assert(attack_animations.size() == 8);
    for (std::size_t index = 0; index < field_entity_animations.size();
         ++index) {
        const auto& animation = field_entity_animations[index];
        // ROM 0xB330/0xAFF2 select the finite C0B2 behavior-transition
        // lists. They are eight descriptors and are not ordinary walking.
        assert(animation.state_selector ==
               static_cast<std::uint16_t>(0xE8U + index * 4U));
        assert(animation.state_marker == 0x8008);
        assert(animation.state_auxiliary_word == 0x0200);
        assert(animation.source_timing_value == 0x76F0);
        assert(animation.timing_multiplier == 0x0010);
        assert(animation.frames.size() == 8);
        for (const auto& frame : animation.frames) {
            assert(frame.width != 0 && frame.height != 0);
            assert(frame.pixels.size() ==
                   static_cast<std::size_t>(frame.width) * frame.height);
        }
        // The transition sequence's frame data remains available to native
        // behavior code, but Qt field movement consumes the separate C062 art.
        const auto pixels_differ = [](const auto& left, const auto& right) {
            if (left.pixels.size() != right.pixels.size()) {
                return true;
            }
            for (std::size_t pixel = 0; pixel < left.pixels.size(); ++pixel) {
                const auto& a = left.pixels[pixel];
                const auto& b = right.pixels[pixel];
                if (a.red != b.red || a.green != b.green ||
                    a.blue != b.blue || a.alpha != b.alpha) {
                    return true;
                }
            }
            return false;
        };
        bool transition_has_distinct_frames = false;
        for (std::size_t frame = 1; frame < animation.frames.size();
             ++frame) {
            if (pixels_differ(animation.frames[0], animation.frames[frame])) {
                transition_has_distinct_frames = true;
                break;
            }
        }
        assert(transition_has_distinct_frames);
        // 0xAFF2's +0x4C behavior path selects the 0x60 + 4*index hurt
        // lists; those lists are separate from C0B2 behavior transitions.
        assert(hurt_animations[index].frames.size() == 2);
        for (const auto& walking_frame : animation.frames) {
            assert(pixels_differ(walking_frame,
                                 hurt_animations[index].frames[0]));
            assert(pixels_differ(walking_frame,
                                 hurt_animations[index].frames[1]));
        }
        // 0xB39A's weapon-timed attack pose (0xC042) is a different held
        // frame from the behavior transition.
        assert(attack_animations[index].frames.size() == 1);
        for (const auto& walking_frame : animation.frames) {
            assert(pixels_differ(walking_frame,
                                 attack_animations[index].frames[0]));
        }
    }
    for (std::size_t index = 0; index < field_movement_animations.size();
         ++index) {
        const auto& movement = field_movement_animations[index];
        assert(movement.state_selector ==
               static_cast<std::uint16_t>(0x20U + index * 4U));
        assert(movement.state_marker == 4U);
        assert(movement.state_auxiliary_word == 0x0180U);
        assert(movement.frames.size() == 4U);
        for (const auto& frame : movement.frames) {
            assert(frame.width != 0U && frame.height != 0U);
            assert(frame.pixels.size() ==
                   static_cast<std::size_t>(frame.width) * frame.height);
        }
        const auto& c0b2_transition = field_entity_animations[index];
        assert(c0b2_transition.state_selector ==
               static_cast<std::uint16_t>(0xE8U + index * 4U));
        assert(c0b2_transition.state_marker == 0x8008U);
    }

    auto native_assets = decode_genesis_native_assets(rom);
    const auto opaque_pixel_count = [](const auto& image) {
        return std::count_if(
            image.pixels.begin(), image.pixels.end(),
            [](const auto& pixel) { return pixel.alpha != 0U; });
    };
    // ROM 0x5D886 ORs $4000 into the FASA grid, selecting palette line 2.
    // Its first visible entry is $0E60 (#006DFF), not the cyan line-0 color.
    const auto fasa_blue = decode_genesis_cram_word(0x0E60U, false);
    const auto fasa_blue_pixels = std::count_if(
        native_assets.boot_fasa.pixels.begin(),
        native_assets.boot_fasa.pixels.end(),
        [&fasa_blue](const auto& pixel) {
            return pixel.alpha != 0U && pixel.red == fasa_blue.red &&
                   pixel.green == fasa_blue.green &&
                   pixel.blue == fasa_blue.blue;
        });
    assert(fasa_blue.red == 0U && fasa_blue.green == 109U &&
           fasa_blue.blue == 255U && fasa_blue_pixels > 500U);
    assert(opaque_pixel_count(native_assets.field_menu_item_art[12]) > 10U);
    assert(opaque_pixel_count(native_assets.field_menu_item_art[49]) > 10U);
    assert(opaque_pixel_count(native_assets.field_menu_item_art[40]) == 0U);
    {
        const auto lights = decode_genesis_title_circuit_lights(rom);
        std::size_t light_delta = 0;
        const auto& first = lights.front();
        for (std::size_t index = 1; index < lights.size(); ++index) {
            const auto& other = lights[index];
            assert(other.pixels.size() == first.pixels.size());
            for (std::size_t pixel = 0; pixel < first.pixels.size(); ++pixel) {
                const auto& left = first.pixels[pixel];
                const auto& right = other.pixels[pixel];
                if (left.red != right.red || left.green != right.green ||
                    left.blue != right.blue) {
                    ++light_delta;
                }
            }
        }
        assert(light_delta > 100U);
        assert(native_assets.title_circuit_lights[0].pixels.size() ==
               first.pixels.size());
        // The first ROM delta run emits two cells after its initial cell;
        // losing the final cell shifts the rest of the 40-cell row and
        // visibly disconnects the circuit board. These are deliberately
        // source-position checks, not a comparison against extracted BMPs.
        const auto& first_row_trace = first.pixels[16U];
        assert(first_row_trace.red == 0U && first_row_trace.green > 0U &&
               first_row_trace.blue == 0U && first_row_trace.alpha != 0U);
        const auto& first_row_gap = first.pixels[45U];
        assert(first_row_gap.red == 0U && first_row_gap.green == 0U &&
               first_row_gap.blue == 0U && first_row_gap.alpha != 0U);
    }
    assert(native_assets.field_areas.size() == 56);
    assert(native_assets.field_areas[0].mode_id == 2);
    for (std::size_t index = 1; index < native_assets.field_areas.size();
         ++index) {
        const auto expected_mode = index <= 2U
            ? static_cast<std::uint16_t>(index - 1U)
            : static_cast<std::uint16_t>(index);
        assert(native_assets.field_areas[index].mode_id == expected_mode);
    }
    assert(native_assets.field_areas[20].map.width == 1600);
    assert(native_assets.field_areas[20].map.height == 960);
    assert(native_assets.field_areas[20].initial_field_records[0]
               .position.world_x_fixed == 0x5A00);
    assert(native_assets.field_areas[20].initial_field_records[0]
               .position.world_y_fixed == 0x0800);
    for (const auto& focus : native_assets.field_menu_command_focus) {
        assert(focus.width == 16U);
        assert(focus.height == 16U);
        assert(!focus.pixels.empty());
    }
    assert(native_assets.field_menu_attributes_background.width == 320U);
    assert(native_assets.field_menu_attributes_background.height == 224U);
    std::size_t attributes_ink = 0;
    for (const auto& pixel :
         native_assets.field_menu_attributes_background.pixels) {
        if (pixel.alpha != 0U &&
            (pixel.red != 0U || pixel.green != 0U || pixel.blue != 0U)) {
            ++attributes_ink;
        }
    }
    assert(attributes_ink > 1000U);
    for (const auto& focus : native_assets.field_menu_clips_focus) {
        assert(focus.width == 16U);
        assert(focus.height == 16U);
        assert(!focus.pixels.empty());
    }
    assert(native_assets.group_item_names[0] == "Prototype Cyber-heart");
    assert(native_assets.group_item_names[14] == "Ork Armband");
    assert(native_assets.cyberdeck_unavailable_lines[0] == " You do not have");
    assert(native_assets.cyberdeck_unavailable_lines[1] == "  a cyberdeck!!");
    assert(native_assets.cyberdeck_unavailable_lines[2] == " Keep looking.");
    assert(native_assets.dismiss_runner_labels[0] == "   Dismiss");
    assert(native_assets.dismiss_runner_labels[1] == "    Who??");
    assert(native_assets.dismiss_runner_labels[2] == "    Abort");
    const auto random_grid_effects =
        decode_genesis_field_menu_random_grid_effects(rom);
    for (const auto& effect : random_grid_effects) {
        assert(effect.width == 16U);
        assert(effect.height == 8U);
        assert(effect.pixels.size() == 16U * 8U);
        assert(std::any_of(effect.pixels.begin(), effect.pixels.end(),
                           [](const auto& pixel) {
                               return pixel.red != 0U || pixel.green != 0U ||
                                      pixel.blue != 0U;
                           }));
    }
    assert(read_genesis_dismiss_runner_separator(rom) ==
           std::string("[") + std::string(12U, static_cast<char>(92)));
    const auto notebook_labels = read_genesis_notebook_labels(rom);
    assert((notebook_labels == std::array<std::string, 6>{
        "  Notebook", "Current Run", "Tips & Clues", "Contacts",
        "Mr. Johnsons", "Shadowrunners"}));
    assert((read_genesis_notebook_no_run_text(rom) ==
            std::array<std::string, 3>{
                " You are not currently", "engaged in a shadowrun.",
                "     Get a job!!"}));
    const auto pocket_secretary_text =
        read_genesis_pocket_secretary_text(rom);
    assert(pocket_secretary_text.title == " Main Menu");
    assert(pocket_secretary_text.separator ==
           std::string("[") + std::string(10U, static_cast<char>(92)));
    assert((pocket_secretary_text.options ==
            std::array<std::string, 5>{
                "Notebook", "Group Items", "Cyberdeck", "Dismiss Runner",
                "Save/Load Game"}));
    const auto hotel_text = read_genesis_hotel_options_text(rom);
    assert(hotel_text.title == " Hotel Options");
    assert(hotel_text.separator ==
           std::string("[") + std::string(14U, static_cast<char>(92)));
    assert((hotel_text.options ==
            std::array<std::string, 2>{
                "  Karma Screen", "  Check Out"}));
    assert((hotel_text.checkout_question ==
            std::array<std::string, 2>{
                "Do you wish to", "  check out?"}));
    assert((hotel_text.checkout_options ==
            std::array<std::string, 3>{"Yes", "", "No"}));
    assert(hotel_text.medical_attention_suffix ==
           " needs medical attention!");
    assert(hotel_text.recovery_message[0].empty());
    assert(hotel_text.recovery_message[1].empty());
    assert(hotel_text.recovery_message[2] == "     A steaming mug of");
    assert(hotel_text.recovery_message[3] == "   soykaf would be just");
    assert(hotel_text.recovery_message[4] == "   wiz about now.");
    const auto hotel_cursor = decode_genesis_hotel_options_cursor(rom);
    const auto hotel_checkout_cursor =
        decode_genesis_hotel_options_cursor(rom, 2U);
    assert(hotel_cursor.width == 8U && hotel_cursor.height == 8U);
    assert(hotel_checkout_cursor.width == 8U &&
           hotel_checkout_cursor.height == 8U);
    // C376 clears with tile 0x5AB, then writes the selected arrow from tile
    // 0x5FF in the C1064 font upload.
    assert(std::any_of(hotel_cursor.pixels.begin(), hotel_cursor.pixels.end(),
                       [](const auto& pixel) { return pixel.alpha != 0U; }));
    assert(std::any_of(
        hotel_checkout_cursor.pixels.begin(), hotel_checkout_cursor.pixels.end(),
        [](const auto& pixel) {
            return pixel.alpha != 0U && pixel.red == 109U &&
                   pixel.green == 73U && pixel.blue == 146U;
        }));
    assert(native_assets.notebook_labels == notebook_labels);
    assert(native_assets.notebook_no_run_lines ==
           read_genesis_notebook_no_run_text(rom));
    assert((native_assets.notebook_event_gates ==
            std::array<std::uint8_t, 3>{1, 0, 0}));
    const auto notebook_tips = read_genesis_notebook_tip_catalog(rom);
    assert(notebook_tips.size() == 96U);
    assert(notebook_tips[0].find("Michael's last credstick\ntransaction:") ==
           0U);
    assert(notebook_tips[32].find("Ilene is doing time") == 0U);
    assert(notebook_tips[95].find("You have deleted the") == 0U);
    assert(native_assets.notebook_tip_text == notebook_tips);
    const auto mr_johnson_names = read_genesis_notebook_mr_johnson_names(rom);
    assert((mr_johnson_names ==
            std::array<std::string, 5>{
                "Mr. Gunderson", "Julius Strouther", "Mortimer Reed",
                "Vigore & Jarl", "Caleb Brightmore"}));
    assert(native_assets.notebook_mr_johnson_names == mr_johnson_names);
    assert((native_assets.notebook_tip_flags ==
            std::array<std::uint32_t, 3>{1U, 0U, 0U}));
    native_assets.notebook_tip_flags =
        std::array<std::uint32_t, 3>{0x80000001U, 0x00000000U,
                                     0x00000000U};
    // NativePortAssets is an extraction-side value object. Runtime coverage
    // loads the individually named files through the direct asset-directory
    // test; this source-analysis test must not create an aggregate pack.
    const auto& loaded_assets = native_assets;
    assert(loaded_assets.boot_background.width ==
           native_assets.boot_background.width);
    assert(loaded_assets.boot_background.pixels.size() ==
           native_assets.boot_background.pixels.size());
    assert(loaded_assets.boot_static.pixels.size() ==
           native_assets.boot_static.pixels.size());
    assert(loaded_assets.boot_logo_strobe[0].pixels.size() ==
           native_assets.boot_logo_strobe[0].pixels.size());
    assert(loaded_assets.boot_fasa.pixels.size() ==
           native_assets.boot_fasa.pixels.size());
    assert(loaded_assets.startup_background.pixels.size() ==
           native_assets.startup_background.pixels.size());
    assert(loaded_assets.field_menu_attributes_background.width ==
           native_assets.field_menu_attributes_background.width);
    assert(loaded_assets.field_menu_attributes_background.height ==
           native_assets.field_menu_attributes_background.height);
    assert(loaded_assets.field_menu_attributes_background.pixels.size() ==
           native_assets.field_menu_attributes_background.pixels.size());
    for (std::size_t pixel = 0;
         pixel < native_assets.field_menu_attributes_background.pixels.size();
         ++pixel) {
        const auto& loaded =
            loaded_assets.field_menu_attributes_background.pixels[pixel];
        const auto& native =
            native_assets.field_menu_attributes_background.pixels[pixel];
        assert(loaded.red == native.red);
        assert(loaded.green == native.green);
        assert(loaded.blue == native.blue);
        assert(loaded.alpha == native.alpha);
    }
    const auto intro_stages = decode_genesis_intro_stages(rom);
    for (std::size_t index = 0; index < intro_stages.size(); ++index) {
        assert(intro_stages[index].width == 320);
        assert(intro_stages[index].height == 224);
        std::size_t ink = 0;
        for (const auto& pixel : intro_stages[index].pixels) {
            if (pixel.red != 0 || pixel.green != 0 || pixel.blue != 0) {
                ++ink;
            }
        }
        assert(ink > 100U);
        if (index == 1U) {
            // 0x1728 BGE runs must fill the 17x16 wilderness stamp. A
            // count-not-plus-one decode left 32 empty cells and sheared
            // every character row.
            assert(ink > 8000U);
        }
        assert(native_assets.intro_stages[index].pixels.size() ==
               intro_stages[index].pixels.size());
        assert(loaded_assets.intro_stages[index].pixels.size() ==
               native_assets.intro_stages[index].pixels.size());
    }
    const auto field_hud = decode_genesis_field_hud(rom);
    assert(field_hud.width == 320);
    assert(field_hud.height == 224);
    {
        std::size_t hud_ink = 0;
        std::size_t right_ink = 0;
        for (std::uint16_t y = 0; y < field_hud.height; ++y) {
            for (std::uint16_t x = 0; x < field_hud.width; ++x) {
                const auto& pixel =
                    field_hud.pixels[static_cast<std::size_t>(y) *
                                         field_hud.width +
                                     x];
                if (pixel.alpha == 0U) {
                    continue;
                }
                ++hud_ink;
                if (x >= 256U) {
                    ++right_ink;
                }
            }
        }
        assert(hud_ink > 20U);
        assert(right_ink == hud_ink);
        std::size_t hud_top_ink = 0;
        std::size_t hud_bottom_ink = 0;
        for (std::uint16_t y = 0; y < field_hud.height; ++y) {
            for (std::uint16_t x = 256; x < field_hud.width; ++x) {
                const auto& pixel =
                    field_hud.pixels[static_cast<std::size_t>(y) *
                                         field_hud.width +
                                     x];
                if (pixel.alpha == 0U) {
                    continue;
                }
                if (y < 48U) {
                    ++hud_top_ink;
                } else {
                    ++hud_bottom_ink;
                }
            }
        }
        // Capture t1720: metal + face occupy the top of the column, not
        // only the 40x6 strip at row 22.
        assert(hud_top_ink > 20U);
        assert(hud_bottom_ink > 20U);
        assert(native_assets.field_hud.pixels.size() == field_hud.pixels.size());
        assert(loaded_assets.field_hud.pixels.size() ==
               native_assets.field_hud.pixels.size());
        for (std::size_t meter = 0;
             meter < native_assets.field_hud_delay_meter_counters.size();
             ++meter) {
            const auto& delay =
                native_assets.field_hud_delay_meter_counters[meter];
            assert(delay.width == 8U && delay.height == 32U);
            assert(std::any_of(delay.pixels.begin(), delay.pixels.end(),
                               [](const auto& pixel) {
                                   return pixel.alpha != 0U;
                               }));
            for (std::size_t profile = 0;
                 profile <
                     native_assets.field_hud_behavior_meter_counters.size();
                 ++profile) {
                const auto& behavior =
                    native_assets.field_hud_behavior_meter_counters[profile]
                        [meter];
                assert(behavior.width == 32U && behavior.height == 8U);
                assert(std::any_of(behavior.pixels.begin(),
                                   behavior.pixels.end(), [](const auto& pixel) {
                                       return pixel.alpha != 0U;
                                   }));
            }
        }
        const auto meter_index = [](const std::uint8_t value) {
            return std::min<std::size_t>(value, 10U);
        };
        const auto same_pixels = [](
                                     const auto& left,
                                     const auto& right) {
            return left.size() == right.size() &&
                   std::equal(left.begin(), left.end(), right.begin(),
                              [](const auto& a, const auto& b) {
                                  return a.red == b.red &&
                                         a.green == b.green &&
                                         a.blue == b.blue &&
                                         a.alpha == b.alpha;
                              });
        };
        {
            const std::array<GenesisFieldEntityTemplate, 3> hud_records{{
                read_genesis_field_entity_template(rom, 0x7291AU),
                read_genesis_field_entity_template(rom, 0x72A1AU),
                read_genesis_field_entity_template(rom, 0x72B1AU),
            }};
            auto zero_palette_rom = rom;
            const auto portrait_record = hud_records[0].record_offset;
            zero_palette_rom[portrait_record + 0x2CU] = 0U;
            zero_palette_rom[portrait_record + 0x2DU] = 0U;
            const auto zero_palette_layers = decode_genesis_field_hud_layers(
                zero_palette_rom, hud_records);
            assert(same_pixels(zero_palette_layers.faces[0].pixels,
                               native_assets.field_hud_faces[0].pixels));
        }
        for (std::size_t spell = 0;
             spell < native_assets.field_hud_magic_item_blocks.size();
             ++spell) {
            const auto& low_palette =
                native_assets.field_hud_magic_item_blocks[spell][0U];
            const auto& high_palette =
                native_assets.field_hud_magic_item_blocks[spell][1U];
            const auto& loaded_low =
                loaded_assets.field_hud_magic_item_blocks[spell][0U];
            const auto& loaded_high =
                loaded_assets.field_hud_magic_item_blocks[spell][1U];
            assert(low_palette.width == 24U && low_palette.height == 16U);
            assert(high_palette.width == 24U && high_palette.height == 16U);
            assert(std::any_of(low_palette.pixels.begin(),
                               low_palette.pixels.end(), [](const auto& pixel) {
                                   return pixel.alpha != 0U;
                               }));
            assert(std::any_of(high_palette.pixels.begin(),
                               high_palette.pixels.end(), [](const auto& pixel) {
                                   return pixel.alpha != 0U;
                               }));
            assert(!same_pixels(low_palette.pixels, high_palette.pixels));
            assert(loaded_low.width == 24U && loaded_low.height == 16U);
            assert(loaded_high.width == 24U && loaded_high.height == 16U);
            assert(same_pixels(low_palette.pixels, loaded_low.pixels));
            assert(same_pixels(high_palette.pixels, loaded_high.pixels));
        }
        const auto party_hud = decode_genesis_field_hud_party_assets(rom);
        const auto gator_hud_record =
            read_genesis_field_entity_template(rom, 0x72B1AU);
        assert(gator_hud_record.active_item_code == 2U);
        assert(gator_hud_record.ammo_rounds == 32U);
        assert(gator_hud_record.display_state == 0);
        assert(gator_hud_record.display_palette_state == 1U);
        assert(same_pixels(native_assets.field_hud_faces[2].pixels,
                           party_hud.portraits[2][0].pixels));
        assert(same_pixels(native_assets.field_hud_item_blocks[2].pixels,
                           party_hud.items[1][2].pixels));
        bool portrait_palette_differs = false;
        for (const auto& family : party_hud.portraits) {
            for (const auto& portrait : family) {
                assert(portrait.width == 32U && portrait.height == 32U);
                assert(portrait.pixels.size() == 32U * 32U);
                assert(std::any_of(portrait.pixels.begin(), portrait.pixels.end(),
                                   [](const auto& pixel) {
                                       return pixel.alpha != 0U;
                                   }));
            }
            portrait_palette_differs |=
                !same_pixels(family[0].pixels, family[1].pixels) ||
                !same_pixels(family[1].pixels, family[2].pixels);
        }
        assert(party_hud.portraits.size() == 13U);
        assert(portrait_palette_differs);
        bool item_palette_differs = false;
        for (const auto& item : party_hud.items) {
            for (const auto& image : item) {
                assert(image.width == 24U && image.height == 16U);
                assert(image.pixels.size() == 24U * 16U);
            }
            item_palette_differs |=
                !same_pixels(item[0].pixels, item[1].pixels) ||
                !same_pixels(item[1].pixels, item[2].pixels);
        }
        assert(party_hud.items.size() == 50U);
        assert(item_palette_differs);
        for (const auto& item : party_hud.unarmed_items) {
            for (const auto& image : item) {
                assert(image.width == 24U && image.height == 16U);
                assert(image.pixels.size() == 24U * 16U);
            }
        }
        for (std::size_t profile = 0;
             profile < native_assets.protagonist_archetype_records.size();
             ++profile) {
            const auto& record =
                native_assets.protagonist_archetype_records[profile];
            const auto delay = (record.flags_10 & 0x01U) != 0U
                ? meter_index(record.physical_value)
                : 0U;
            const auto behavior_profile = profile == 0U ? 0U : 1U;
            const auto behavior = meter_index(record.mental_value);
            assert(same_pixels(
                native_assets.field_hud_vertical_counters[profile].pixels,
                native_assets.field_hud_delay_meter_counters[delay].pixels));
            assert(same_pixels(
                native_assets.field_hud_horizontal_counters[profile].pixels,
                native_assets.field_hud_behavior_meter_counters
                    [behavior_profile][behavior]
                        .pixels));
        }
        assert(!native_assets.entity_frames_gray.empty());
        assert(native_assets.entity_frames_gray.size() ==
               native_assets.entity_frames.size());
        const auto ps = decode_genesis_pocket_secretary_background(rom);
        assert(ps.width == 320);
        assert(ps.height == 224);
        std::size_t ps_ink = 0;
        for (const auto& pixel : ps.pixels) {
            if (pixel.alpha != 0U &&
                (pixel.red != 0 || pixel.green != 0 || pixel.blue != 0)) {
                ++ps_ink;
            }
        }
        assert(ps_ink > 100U);
        assert(native_assets.pocket_secretary_background.pixels.size() ==
               ps.pixels.size());
    }
    const auto heli_frames = decode_genesis_intro_heli_frames(rom);
    {
        // 0x5B100's SAT source starts at D1=$1FC. Its first phase extends
        // past the right edge of the Genesis viewport, so the native asset
        // must retain a wider source canvas instead of clipping that rotor
        // strip at extraction time.
        std::uint16_t min_x = 512U;
        std::uint16_t max_x = 0U;
        std::size_t total_ink = 0;
        for (const auto& frame : heli_frames) {
            assert(frame.width == 512U);
            assert(frame.height == 224U);
            assert(frame.pixels.size() ==
                   static_cast<std::size_t>(frame.width) * frame.height);
            for (std::uint16_t y = 0; y < frame.height; ++y) {
                for (std::uint16_t x = 0; x < frame.width; ++x) {
                    const auto& pixel =
                        frame.pixels[static_cast<std::size_t>(y) *
                                         frame.width + x];
                    if (pixel.alpha == 0U ||
                        (pixel.red == 0U && pixel.green == 0U &&
                         pixel.blue == 0U)) {
                        continue;
                    }
                    ++total_ink;
                    min_x = std::min(min_x, x);
                    max_x = std::max(max_x, x);
                }
            }
        }
        assert(total_ink > 100U);
        assert(min_x > 250U);
        assert(max_x > 400U);
    }
    const auto plane_a = decode_genesis_intro_aircraft_plane_a(rom);
    {
        std::size_t ink = 0;
        std::uint16_t min_x = plane_a.width;
        std::uint16_t max_x = 0U;
        std::uint16_t min_y = plane_a.height;
        std::uint16_t max_y = 0U;
        for (std::uint16_t y = 0; y < plane_a.height; ++y) {
            for (std::uint16_t x = 0; x < plane_a.width; ++x) {
                const auto& pixel =
                    plane_a.pixels[static_cast<std::size_t>(y) *
                                       plane_a.width + x];
                if (pixel.alpha == 0U ||
                    (pixel.red == 0U && pixel.green == 0U &&
                     pixel.blue == 0U)) {
                    continue;
                }
                ++ink;
                min_x = std::min(min_x, x);
                max_x = std::max(max_x, x);
                min_y = std::min(min_y, y);
                max_y = std::max(max_y, y);
            }
        }
        // The normalized 27x8 Plane-A map is the detailed craft body. These
        // bounds reject the former one-byte-offset decode and the clipped SAT
        // strip as substitutes for the authored map.
        assert(plane_a.width == 320U);
        assert(plane_a.height == 224U);
        assert(ink > 1000U);
        assert(min_x < 80U && max_x > 200U);
        assert(min_y < 80U && max_y > 50U);
    }
    assert(native_assets.intro_aircraft_plane_a.width == plane_a.width);
    assert(native_assets.intro_aircraft_plane_a.pixels.size() ==
           plane_a.pixels.size());
    for (const auto& movement : native_assets.intro_aircraft_movement_frames) {
        assert(movement.width == 320U);
        assert(movement.height == 224U);
        assert(movement.pixels.size() ==
               static_cast<std::size_t>(movement.width) * movement.height);
    }
    const auto title_sprites = decode_genesis_title_sprite_frames(rom);
    // List 1 object 0: SAT X=$FFF1 Y=$FF9F size 4x4 at D1=$9C D2=$110.
    // Host pixel origin is that SAT pair minus $80.
    {
        const auto& sprite = title_sprites[1];
        std::size_t ink = 0;
        for (std::size_t y = 47; y < 79 && y < sprite.height; ++y) {
            for (std::size_t x = 13; x < 45 && x < sprite.width; ++x) {
                const auto& pixel = sprite.pixels[y * sprite.width + x];
                if (pixel.alpha != 0U &&
                    (pixel.red != 0 || pixel.green != 0 || pixel.blue != 0)) {
                    ++ink;
                }
            }
        }
        assert(ink > 200U);
    }
    assert(loaded_assets.boot_guy.pixels.size() ==
           native_assets.boot_guy.pixels.size());
    assert(loaded_assets.title_scene.width == native_assets.title_scene.width);
    assert(loaded_assets.title_scene.height == native_assets.title_scene.height);
    assert(loaded_assets.title_scene.pixels.size() ==
           native_assets.title_scene.pixels.size());
    for (std::size_t index = 0;
         index < native_assets.field_menu_command_focus.size(); ++index) {
        assert(loaded_assets.field_menu_command_focus[index].width ==
               native_assets.field_menu_command_focus[index].width);
        assert(loaded_assets.field_menu_command_focus[index].height ==
               native_assets.field_menu_command_focus[index].height);
        assert(loaded_assets.field_menu_command_focus[index].pixels.size() ==
               native_assets.field_menu_command_focus[index].pixels.size());
        for (std::size_t pixel = 0;
             pixel < native_assets.field_menu_command_focus[index].pixels.size();
             ++pixel) {
            const auto& loaded =
                loaded_assets.field_menu_command_focus[index].pixels[pixel];
            const auto& native =
                native_assets.field_menu_command_focus[index].pixels[pixel];
            assert(loaded.red == native.red);
            assert(loaded.green == native.green);
            assert(loaded.blue == native.blue);
            assert(loaded.alpha == native.alpha);
        }
    }
    assert(loaded_assets.field_map.width == native_assets.field_map.width);
    assert(loaded_assets.field_map.height == native_assets.field_map.height);
    assert(loaded_assets.field_map.pixels.size() ==
           native_assets.field_map.pixels.size());
    assert(loaded_assets.entity_frames.size() ==
           native_assets.entity_frames.size());
    for (std::size_t index = 0;
         index < native_assets.field_entity_animations.size(); ++index) {
        assert(loaded_assets.field_entity_animations[index].state_selector ==
               native_assets.field_entity_animations[index].state_selector);
        assert(loaded_assets.field_entity_animations[index].frames.size() ==
               native_assets.field_entity_animations[index].frames.size());
    }
    assert(loaded_assets.field_terrain.statuses ==
           native_assets.field_terrain.statuses);
    assert(loaded_assets.initial_camera_x_fixed ==
           native_assets.initial_camera_x_fixed);
    assert(loaded_assets.initial_camera_y_fixed ==
           native_assets.initial_camera_y_fixed);
    assert(loaded_assets.field_direction_table ==
           native_assets.field_direction_table);
    assert(loaded_assets.group_item_names == native_assets.group_item_names);
    assert(loaded_assets.cyberdeck_unavailable_lines ==
           native_assets.cyberdeck_unavailable_lines);
    assert(loaded_assets.dismiss_runner_labels ==
           native_assets.dismiss_runner_labels);
    assert(loaded_assets.notebook_labels == native_assets.notebook_labels);
    assert(loaded_assets.notebook_no_run_lines ==
           native_assets.notebook_no_run_lines);
    assert(loaded_assets.notebook_event_gates ==
           native_assets.notebook_event_gates);
    assert(loaded_assets.notebook_tip_text == native_assets.notebook_tip_text);
    assert(loaded_assets.notebook_tip_flags == native_assets.notebook_tip_flags);
    assert(loaded_assets.notebook_mr_johnson_names ==
           native_assets.notebook_mr_johnson_names);
    assert(loaded_assets.field_event_mode_table ==
           native_assets.field_event_mode_table);
    assert(loaded_assets.field_areas.size() ==
           native_assets.field_areas.size());
    for (std::size_t index = 0; index < native_assets.field_areas.size();
         ++index) {
        assert(loaded_assets.field_areas[index].mode_id ==
               native_assets.field_areas[index].mode_id);
        assert(loaded_assets.field_areas[index].map.width ==
               native_assets.field_areas[index].map.width);
        assert(loaded_assets.field_areas[index].map.height ==
               native_assets.field_areas[index].map.height);
        assert(loaded_assets.field_areas[index].map.pixels.size() ==
               native_assets.field_areas[index].map.pixels.size());
        for (std::size_t pixel = 0;
             pixel < native_assets.field_areas[index].map.pixels.size();
             ++pixel) {
            const auto& loaded =
                loaded_assets.field_areas[index].map.pixels[pixel];
            const auto& native =
                native_assets.field_areas[index].map.pixels[pixel];
            assert(loaded.red == native.red && loaded.green == native.green &&
                   loaded.blue == native.blue && loaded.alpha == native.alpha);
        }
        assert(loaded_assets.field_areas[index].terrain.statuses ==
               native_assets.field_areas[index].terrain.statuses);
        assert(loaded_assets.field_areas[index].event_mode_table ==
               native_assets.field_areas[index].event_mode_table);
        for (std::size_t record = 0; record < 3; ++record) {
            const auto& loaded =
                loaded_assets.field_areas[index].initial_field_records[record];
            const auto& native =
                native_assets.field_areas[index].initial_field_records[record];
            assert(loaded.position.world_x_fixed ==
                       native.position.world_x_fixed &&
                   loaded.position.world_y_fixed ==
                       native.position.world_y_fixed &&
                   loaded.record_type == native.record_type &&
                   loaded.flags_10 == native.flags_10 &&
                   loaded.flags_11 == native.flags_11);
        }
    }
    assert(loaded_assets.entity_template.kind ==
           native_assets.entity_template.kind);
    assert(loaded_assets.entity_template.record_type ==
           native_assets.entity_template.record_type);
    assert(loaded_assets.intro.start_options_title ==
           native_assets.intro.start_options_title);
    assert(loaded_assets.intro.start_options == native_assets.intro.start_options);
    assert(loaded_assets.intro.archetype_title ==
           native_assets.intro.archetype_title);
    assert(loaded_assets.intro.archetypes == native_assets.intro.archetypes);
    assert(loaded_assets.intro.story_lines == native_assets.intro.story_lines);
    assert(loaded_assets.entity_template.display_state ==
           native_assets.entity_template.display_state);
    assert(loaded_assets.entity_template.display_substate ==
           native_assets.entity_template.display_substate);
    assert(loaded_assets.entity_template.inventory_item_codes ==
           native_assets.entity_template.inventory_item_codes);
    assert(loaded_assets.entity_template.inventory_phase_bytes ==
           native_assets.entity_template.inventory_phase_bytes);
    assert(loaded_assets.entity_template.equipped_armor_index ==
           native_assets.entity_template.equipped_armor_index);
    assert(loaded_assets.entity_template.ammo_rounds ==
           native_assets.entity_template.ammo_rounds);
    assert(loaded_assets.entity_template.ammo_clips ==
           native_assets.entity_template.ammo_clips);
    assert(loaded_assets.entity_template.cyberware_flags ==
           native_assets.entity_template.cyberware_flags);
    assert(loaded_assets.entity_template.attribute_values ==
           native_assets.entity_template.attribute_values);
    assert(loaded_assets.entity_template.skill_values ==
           native_assets.entity_template.skill_values);
    assert(loaded_assets.entity_template.attribute_skill_auxiliary ==
           native_assets.entity_template.attribute_skill_auxiliary);
    assert(loaded_assets.inventory_items.size() ==
           native_assets.inventory_items.size());
    assert(loaded_assets.inventory_items[1].source_combat_factor ==
           native_assets.inventory_items[1].source_combat_factor);
    assert(loaded_assets.inventory_items[1].name ==
           native_assets.inventory_items[1].name);
    assert(loaded_assets.inventory_items[43].category ==
           native_assets.inventory_items[43].category);
    assert(loaded_assets.cyberware_names == native_assets.cyberware_names);
    assert(loaded_assets.attribute_skill_names ==
           native_assets.attribute_skill_names);
    return 0;
}

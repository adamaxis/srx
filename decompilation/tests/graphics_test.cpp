#include "shadowrun/graphics.hpp"

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
    const auto field_tile = decode_genesis_tile(
        rom.data() + 0x166B82, 32);
    assert(field_tile.pixels[0] == 0);
    assert(field_tile.pixels[1] == 0);
    assert(field_tile.pixels[2] == 0x0D);
    assert(field_tile.pixels[3] == 0x0C);
    assert(field_tile.pixels[8] == 0x0D);
    assert(field_tile.pixels[9] == 0x0D);
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
    const auto inventory_catalog = read_genesis_inventory_catalog(rom);
    assert(inventory_catalog.size() == 50);
    assert(inventory_catalog[1].code == 2);
    assert(inventory_catalog[1].source_combat_factor == 12);
    assert(inventory_catalog[1].name == "Model 101T");
    assert(inventory_catalog[1].category == "Light Pistol");
    assert(inventory_catalog[43].code == 44);
    assert(inventory_catalog[43].name == "Armor Vest");
    const auto cyberware_catalog = read_genesis_cyberware_catalog(rom);
    assert(cyberware_catalog.size() == 15);
    assert(cyberware_catalog[0] == "Datajack");
    assert(cyberware_catalog[1] == "CyberEyes");
    assert(cyberware_catalog[2] == "Hand Razors");
    assert(cyberware_catalog[14] == "Wired Reflexes(3)");
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
    const auto field_menu_font = decode_genesis_field_menu_font(rom);
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
    for (std::size_t index = 0; index < field_entity_animations.size();
         ++index) {
        const auto& animation = field_entity_animations[index];
        assert(animation.state_selector ==
               static_cast<std::uint16_t>(0x60U + index * 4U));
        assert(animation.state_marker == 0x8002);
        assert(animation.state_auxiliary_word == 0x0300);
        assert(animation.source_timing_value == 0x76F0);
        assert(animation.timing_multiplier == 0x0010);
        assert(animation.frames.size() == 2);
        for (const auto& frame : animation.frames) {
            assert(frame.width != 0 && frame.height != 0);
            assert(frame.pixels.size() ==
                   static_cast<std::size_t>(frame.width) * frame.height);
        }
        bool walking_frames_differ = false;
        for (std::size_t pixel = 0;
             pixel < animation.frames[0].pixels.size() &&
             pixel < animation.frames[1].pixels.size(); ++pixel) {
            const auto& left = animation.frames[0].pixels[pixel];
            const auto& right = animation.frames[1].pixels[pixel];
            if (left.red != right.red || left.green != right.green ||
                left.blue != right.blue || left.alpha != right.alpha) {
                walking_frames_differ = true;
                break;
            }
        }
        assert(walking_frames_differ);
    }

    auto native_assets = decode_genesis_native_assets(rom);
    const auto opaque_pixel_count = [](const auto& image) {
        return std::count_if(
            image.pixels.begin(), image.pixels.end(),
            [](const auto& pixel) { return pixel.alpha != 0U; });
    };
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
    const auto notebook_labels = read_genesis_notebook_labels(rom);
    assert((notebook_labels == std::array<std::string, 6>{
        "  Notebook", "Current Run", "Tips & Clues", "Contacts",
        "Mr. Johnsons", "Shadowrunners"}));
    assert((read_genesis_notebook_no_run_text(rom) ==
            std::array<std::string, 3>{
                " You are not currently", "engaged in a shadowrun.",
                "     Get a job!!"}));
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
            std::array<std::uint32_t, 3>{0U, 0U, 0U}));
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

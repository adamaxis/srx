#include "shadowrun/graphics.hpp"

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
    const auto inventory_catalog = read_genesis_inventory_catalog(rom);
    assert(inventory_catalog.size() == 50);
    assert(inventory_catalog[1].code == 2);
    assert(inventory_catalog[1].name == "Model 101T");
    assert(inventory_catalog[1].category == "Light Pistol");
    assert(inventory_catalog[43].code == 44);
    assert(inventory_catalog[43].name == "Armor Vest");
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
    assert(clear_probe.status == 0 && !clear_probe.blocked &&
           !clear_probe.out_of_bounds);
    const auto blocked_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(864U * 16U),
        static_cast<std::uint16_t>(536U * 16U));
    assert(blocked_probe.status == 0x7FU && blocked_probe.blocked &&
           !blocked_probe.out_of_bounds);
    const auto rectangle_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(600U * 16U),
        static_cast<std::uint16_t>(1250U * 16U));
    assert(rectangle_probe.status == 0x7FU && rectangle_probe.blocked);
    const auto bounds_probe = shadowrun::terrain::probe(
        terrain, static_cast<std::uint16_t>(43U * 16U),
        static_cast<std::uint16_t>(552U * 16U));
    assert(bounds_probe.status == 0x7FU && bounds_probe.blocked &&
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
    for (const auto& pixel : entity_frame.pixels) {
        entity_has_transparency |= pixel.alpha == 0;
        entity_has_color |= pixel.alpha != 0;
    }
    assert(entity_has_transparency && entity_has_color);

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

    const auto native_assets = decode_genesis_native_assets(rom);
    const auto pack_path = std::filesystem::temp_directory_path() /
                           "shadowrun_native_port_assets_test.srna";
    write_native_asset_pack(pack_path.string(), native_assets);
    const auto loaded_assets = read_native_asset_pack(pack_path.string());
    assert(loaded_assets.title_scene.width == native_assets.title_scene.width);
    assert(loaded_assets.title_scene.height == native_assets.title_scene.height);
    assert(loaded_assets.title_scene.pixels.size() ==
           native_assets.title_scene.pixels.size());
    assert(loaded_assets.field_map.width == native_assets.field_map.width);
    assert(loaded_assets.field_map.height == native_assets.field_map.height);
    assert(loaded_assets.field_map.pixels.size() ==
           native_assets.field_map.pixels.size());
    assert(loaded_assets.entity_frames.size() ==
           native_assets.entity_frames.size());
    assert(loaded_assets.field_terrain.statuses ==
           native_assets.field_terrain.statuses);
    assert(loaded_assets.initial_camera_x_fixed ==
           native_assets.initial_camera_x_fixed);
    assert(loaded_assets.initial_camera_y_fixed ==
           native_assets.initial_camera_y_fixed);
    assert(loaded_assets.field_direction_table ==
           native_assets.field_direction_table);
    assert(loaded_assets.entity_template.kind ==
           native_assets.entity_template.kind);
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
    assert(loaded_assets.inventory_items.size() ==
           native_assets.inventory_items.size());
    assert(loaded_assets.inventory_items[1].name ==
           native_assets.inventory_items[1].name);
    assert(loaded_assets.inventory_items[43].category ==
           native_assets.inventory_items[43].category);
    std::filesystem::remove(pack_path);
    return 0;
}

#include "shadowrun/graphics.hpp"
#include "shadowrun/rom_layout.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace shadowrun::graphics {

namespace {

void require_bytes(std::size_t available, std::size_t required,
                   const char* what) {
    if (required > available) {
        throw std::out_of_range(what);
    }
}

std::uint16_t read_rom_u16(const std::vector<std::uint8_t>& rom,
                           std::size_t offset, const char* what) {
    if (offset > rom.size() || rom.size() - offset < 2U) {
        throw std::out_of_range(what);
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(rom[offset]) << 8U) |
        rom[offset + 1U]);
}

std::uint32_t read_rom_u32(const std::vector<std::uint8_t>& rom,
                           std::size_t offset, const char* what) {
    if (offset > rom.size() || rom.size() - offset < 4U) {
        throw std::out_of_range(what);
    }
    return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
           (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(rom[offset + 3U]);
}

void write_u16(std::ostream& output, std::uint16_t value) {
    output.put(static_cast<char>(value & 0xFFU));
    output.put(static_cast<char>((value >> 8U) & 0xFFU));
}

void write_u32(std::ostream& output, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        output.put(static_cast<char>((value >> shift) & 0xFFU));
    }
}

void write_native_image(std::ostream& output,
                        const native::NativeImage& image) {
    const auto expected = static_cast<std::size_t>(image.width) * image.height;
    if (image.width == 0 || image.height == 0 || image.pixels.size() != expected) {
        throw std::invalid_argument("invalid native asset image");
    }
    write_u16(output, image.width);
    write_u16(output, image.height);
    write_u32(output, static_cast<std::uint32_t>(image.pixels.size()));
    for (const auto& pixel : image.pixels) {
        output.put(static_cast<char>(pixel.red));
        output.put(static_cast<char>(pixel.green));
        output.put(static_cast<char>(pixel.blue));
        output.put(static_cast<char>(pixel.alpha));
    }
}

std::uint8_t read_pack_u8(std::istream& input, const char* what) {
    const auto value = input.get();
    if (value == std::char_traits<char>::eof()) {
        throw std::out_of_range(what);
    }
    return static_cast<std::uint8_t>(value);
}

std::uint16_t read_pack_u16(std::istream& input, const char* what) {
    const auto low = read_pack_u8(input, what);
    const auto high = read_pack_u8(input, what);
    return static_cast<std::uint16_t>(low | (high << 8U));
}

std::uint32_t read_pack_u32(std::istream& input, const char* what) {
    const auto b0 = read_pack_u8(input, what);
    const auto b1 = read_pack_u8(input, what);
    const auto b2 = read_pack_u8(input, what);
    const auto b3 = read_pack_u8(input, what);
    return static_cast<std::uint32_t>(b0) |
           (static_cast<std::uint32_t>(b1) << 8U) |
           (static_cast<std::uint32_t>(b2) << 16U) |
           (static_cast<std::uint32_t>(b3) << 24U);
}

void read_pack_bytes(std::istream& input, std::uint8_t* destination,
                     std::size_t size, const char* what) {
    if (size > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::out_of_range(what);
    }
    input.read(reinterpret_cast<char*>(destination),
               static_cast<std::streamsize>(size));
    if (!input) {
        throw std::out_of_range(what);
    }
}

native::NativeImage read_native_image(std::istream& input) {
    const auto width = read_pack_u16(input, "native asset image header is truncated");
    const auto height = read_pack_u16(input, "native asset image header is truncated");
    const auto pixel_count = static_cast<std::size_t>(read_pack_u32(
        input, "native asset image size is truncated"));
    if (width == 0 || height == 0 || width > 8192 || height > 8192 ||
        pixel_count != static_cast<std::size_t>(width) * height) {
        throw std::runtime_error("invalid native asset image dimensions");
    }
    native::NativeImage image{
        width, height,
        std::vector<native::Rgba8>(pixel_count),
    };
    for (auto& pixel : image.pixels) {
        pixel.red = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.green = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.blue = read_pack_u8(input, "native asset RGBA data is truncated");
        pixel.alpha = read_pack_u8(input, "native asset RGBA data is truncated");
    }
    return image;
}

} // namespace

GenesisFieldEntityTemplate read_genesis_field_entity_template(
    const std::vector<std::uint8_t>& rom, std::size_t record_offset) {
    if (record_offset > rom.size() || rom.size() - record_offset < 0x58U) {
        throw std::out_of_range(
            "Genesis field entity template display state is truncated");
    }
    const auto flags = read_rom_u16(
        rom, record_offset + 0x10U,
        "Genesis field entity template flags are truncated");
    return {
        record_offset,
        read_rom_u16(
            rom, record_offset + 0x08U,
            "Genesis field entity template projection is truncated"),
        read_rom_u16(
            rom, record_offset + 0x0EU,
            "Genesis field entity template kind is truncated"),
        static_cast<std::uint8_t>(flags >> 8U),
        static_cast<std::uint8_t>(flags),
        read_rom_u32(
            rom, record_offset + 0x12U,
            "Genesis field entity template state pointer is truncated"),
        read_rom_u16(
            rom, record_offset + 0x16U,
            "Genesis field entity template selector is truncated"),
        read_rom_u16(
            rom, record_offset + 0x18U,
            "Genesis field entity template multiplier is truncated"),
        read_rom_u16(
            rom, record_offset + 0x1AU,
            "Genesis field entity template accumulator is truncated"),
        read_rom_u16(
            rom, record_offset + 0x1CU,
            "Genesis field entity template threshold is truncated"),
        read_rom_u16(
            rom, record_offset + 0x1EU,
            "Genesis field entity template state index is truncated"),
        static_cast<std::int8_t>(rom[record_offset + 0x56U]),
        static_cast<std::int8_t>(rom[record_offset + 0x57U]),
    };
}

native::NativePortAssets decode_genesis_native_assets(
    const std::vector<std::uint8_t>& rom) {
    native::NativePortAssets assets;
    auto title = decode_genesis_startup_screen(rom);
    assets.title_scene = {
        title.width, title.height, std::move(title.pixels)};

    const auto entity_frames = decode_genesis_field_entity_animation(rom);
    assets.entity_frames.reserve(entity_frames.size());
    for (const auto& frame : entity_frames) {
        assets.entity_frames.push_back(
            {frame.width, frame.height, frame.pixels});
    }

    const auto resources = read_genesis_mode2_field_resources(rom);
    auto field_map = decode_genesis_mode2_field_map(rom, resources);
    assets.field_map = {
        field_map.width, field_map.height, std::move(field_map.pixels)};
    assets.field_terrain = decode_genesis_mode2_terrain_map(rom, resources);
    for (std::size_t index = 0; index < assets.initial_field_records.size();
         ++index) {
        assets.initial_field_records[index] = {
            resources.initial_field_records[index].world_x_fixed,
            resources.initial_field_records[index].world_y_fixed,
        };
    }
    assets.initial_camera_x_fixed = resources.initial_camera_x_fixed;
    assets.initial_camera_y_fixed = resources.initial_camera_y_fixed;

    const auto entity_template = read_genesis_field_entity_template(rom);
    assets.entity_template.projection_word_08 =
        entity_template.projection_word_08;
    assets.entity_template.kind = entity_template.kind;
    assets.entity_template.flags_10 = entity_template.flags_10;
    assets.entity_template.flags_11 = entity_template.flags_11;
    assets.entity_template.state_pointer = entity_template.state_pointer;
    assets.entity_template.state_selector = entity_template.state_selector;
    assets.entity_template.animation_multiplier =
        entity_template.timing_multiplier;
    assets.entity_template.animation_accumulator =
        entity_template.timing_accumulator;
    assets.entity_template.animation_threshold =
        entity_template.timing_threshold;
    assets.entity_template.state_index = entity_template.state_index;
    assets.entity_template.display_state = entity_template.display_state;
    assets.entity_template.display_substate = entity_template.display_substate;

    for (std::size_t index = 0;
         index < assets.field_direction_table.size(); ++index) {
        const auto table_offset = rom::kEntityMovementDirectionTable + index * 4U;
        assets.field_direction_table[index] = {
            static_cast<std::int16_t>(read_rom_u16(
                rom, table_offset,
                "Genesis entity movement direction table is truncated")),
            static_cast<std::int16_t>(read_rom_u16(
                rom, table_offset + 2U,
                "Genesis entity movement direction table is truncated")),
        };
    }
    return assets;
}

void write_native_asset_pack(const std::string& path,
                             const native::NativePortAssets& assets) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot create native asset pack: " + path);
    }
    output.write("SRNA", 4);
    write_u16(output, 2);
    write_u16(output, 0);
    write_native_image(output, assets.title_scene);
    write_native_image(output, assets.field_map);
    if (assets.entity_frames.size() > 256) {
        throw std::invalid_argument("too many native entity frames");
    }
    write_u32(output, static_cast<std::uint32_t>(assets.entity_frames.size()));
    for (const auto& frame : assets.entity_frames) {
        write_native_image(output, {frame.width, frame.height, frame.pixels});
    }

    const auto& terrain = assets.field_terrain;
    if (terrain.regions.size() != terrain.region_count) {
        throw std::invalid_argument("native terrain region count is inconsistent");
    }
    write_u16(output, terrain.x_exclusive_upper);
    write_u16(output, terrain.y_exclusive_upper);
    write_u16(output, terrain.x_inclusive_lower);
    write_u16(output, terrain.y_inclusive_lower);
    write_u32(output, static_cast<std::uint32_t>(terrain.regions.size()));
    for (const auto& region : terrain.regions) {
        write_u16(output, region.x_start);
        write_u16(output, region.x_end);
        write_u16(output, region.y_start);
        write_u16(output, region.y_end);
        output.put(static_cast<char>(region.status));
    }
    if (terrain.statuses.size() > 0xFFFFFFFFU) {
        throw std::invalid_argument("native terrain status grid is too large");
    }
    write_u32(output, static_cast<std::uint32_t>(terrain.statuses.size()));
    output.write(reinterpret_cast<const char*>(terrain.statuses.data()),
                 static_cast<std::streamsize>(terrain.statuses.size()));

    write_u16(output, assets.initial_camera_x_fixed);
    write_u16(output, assets.initial_camera_y_fixed);
    for (const auto& position : assets.initial_field_records) {
        write_u16(output, position.world_x_fixed);
        write_u16(output, position.world_y_fixed);
    }

    const auto& entity = assets.entity_template;
    write_u16(output, entity.projection_word_08);
    write_u16(output, entity.kind);
    output.put(static_cast<char>(entity.flags_10));
    output.put(static_cast<char>(entity.flags_11));
    write_u32(output, entity.state_pointer);
    write_u16(output, entity.state_selector);
    write_u16(output, entity.animation_multiplier);
    write_u16(output, entity.animation_accumulator);
    write_u16(output, entity.animation_threshold);
    write_u16(output, entity.state_index);
    output.put(static_cast<char>(
        static_cast<std::uint8_t>(entity.display_state)));
    output.put(static_cast<char>(
        static_cast<std::uint8_t>(entity.display_substate)));
    for (const auto& direction : assets.field_direction_table) {
        write_u16(output, static_cast<std::uint16_t>(direction[0]));
        write_u16(output, static_cast<std::uint16_t>(direction[1]));
    }
    if (!output) {
        throw std::runtime_error("failed writing native asset pack: " + path);
    }
}

native::NativePortAssets read_native_asset_pack(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open native asset pack: " + path);
    }
    char magic[4]{};
    input.read(magic, sizeof(magic));
    if (!input || std::string(magic, sizeof(magic)) != "SRNA" ||
        read_pack_u16(input, "native asset pack version is truncated") != 2U) {
        throw std::runtime_error("invalid native asset pack header");
    }
    (void)read_pack_u16(input, "native asset pack header is truncated");

    native::NativePortAssets assets;
    assets.title_scene = read_native_image(input);
    assets.field_map = read_native_image(input);
    const auto entity_count = static_cast<std::size_t>(read_pack_u32(
        input, "native entity frame count is truncated"));
    if (entity_count > 256) {
        throw std::runtime_error("native asset pack has too many entity frames");
    }
    assets.entity_frames.reserve(entity_count);
    for (std::size_t index = 0; index < entity_count; ++index) {
        const auto image = read_native_image(input);
        assets.entity_frames.push_back(
            {image.width, image.height, image.pixels});
    }

    auto& terrain = assets.field_terrain;
    terrain.x_exclusive_upper = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.y_exclusive_upper = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.x_inclusive_lower = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.y_inclusive_lower = read_pack_u16(
        input, "native terrain bounds are truncated");
    terrain.region_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain region count is truncated"));
    if (terrain.region_count > 65536) {
        throw std::runtime_error("native asset pack has too many terrain regions");
    }
    terrain.regions.resize(terrain.region_count);
    for (auto& region : terrain.regions) {
        region.x_start = read_pack_u16(input, "native terrain region is truncated");
        region.x_end = read_pack_u16(input, "native terrain region is truncated");
        region.y_start = read_pack_u16(input, "native terrain region is truncated");
        region.y_end = read_pack_u16(input, "native terrain region is truncated");
        region.status = read_pack_u8(input, "native terrain region is truncated");
    }
    const auto status_count = static_cast<std::size_t>(read_pack_u32(
        input, "native terrain status count is truncated"));
    if (status_count != terrain::Map::kWidth * terrain::Map::kHeight) {
        throw std::runtime_error("native asset pack has an invalid terrain grid");
    }
    terrain.statuses.resize(status_count);
    read_pack_bytes(input, terrain.statuses.data(), status_count,
                    "native terrain status grid is truncated");

    assets.initial_camera_x_fixed = read_pack_u16(
        input, "native field camera is truncated");
    assets.initial_camera_y_fixed = read_pack_u16(
        input, "native field camera is truncated");
    for (auto& position : assets.initial_field_records) {
        position.world_x_fixed = read_pack_u16(
            input, "native field record position is truncated");
        position.world_y_fixed = read_pack_u16(
            input, "native field record position is truncated");
    }

    auto& entity = assets.entity_template;
    entity.projection_word_08 = read_pack_u16(
        input, "native entity template is truncated");
    entity.kind = read_pack_u16(input, "native entity template is truncated");
    entity.flags_10 = read_pack_u8(input, "native entity template is truncated");
    entity.flags_11 = read_pack_u8(input, "native entity template is truncated");
    entity.state_pointer = read_pack_u32(
        input, "native entity template is truncated");
    entity.state_selector = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_multiplier = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_accumulator = read_pack_u16(
        input, "native entity template is truncated");
    entity.animation_threshold = read_pack_u16(
        input, "native entity template is truncated");
    entity.state_index = read_pack_u16(
        input, "native entity template is truncated");
    entity.display_state = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity display state is truncated"));
    entity.display_substate = static_cast<std::int8_t>(read_pack_u8(
        input, "native entity display substate is truncated"));
    for (auto& direction : assets.field_direction_table) {
        direction[0] = static_cast<std::int16_t>(read_pack_u16(
            input, "native direction table is truncated"));
        direction[1] = static_cast<std::int16_t>(read_pack_u16(
            input, "native direction table is truncated"));
    }
    return assets;
}

native::Rgba8 decode_genesis_cram_word(std::uint16_t cram_word,
                                       bool transparent) {
    const auto expand = [](std::uint8_t value) {
        return static_cast<std::uint8_t>((value << 5U) |
                                         (value << 2U) |
                                         (value >> 1U));
    };
    return {
        expand(static_cast<std::uint8_t>((cram_word >> 1U) & 0x07U)),
        expand(static_cast<std::uint8_t>((cram_word >> 5U) & 0x07U)),
        expand(static_cast<std::uint8_t>((cram_word >> 9U) & 0x07U)),
        static_cast<std::uint8_t>(transparent ? 0 : 255),
    };
}

IndexedTile decode_genesis_tile(const std::uint8_t* source,
                                std::size_t source_size) {
    require_bytes(source_size, 32, "Genesis tile is truncated");
    IndexedTile tile;
    for (std::size_t row = 0; row < IndexedTile::kHeight; ++row) {
        for (std::size_t byte = 0; byte < 4U; ++byte) {
            const auto packed_pixels = source[row * 4U + byte];
            const auto pixel = row * IndexedTile::kWidth + byte * 2U;
            tile.pixels[pixel] = static_cast<std::uint8_t>(
                packed_pixels >> 4U);
            tile.pixels[pixel + 1U] = static_cast<std::uint8_t>(
                packed_pixels & 0x0FU);
        }
    }
    return tile;
}

std::vector<std::uint8_t> decode_genesis_resource_stream(
    const std::vector<std::uint8_t>& rom, std::size_t source_offset) {
    if (source_offset > rom.size() || rom.size() - source_offset < 2U) {
        throw std::out_of_range("Genesis resource stream is truncated");
    }
    const auto initial_counter = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(rom[source_offset]) << 8U) |
        rom[source_offset + 1U]);
    auto source = source_offset + 2U;
    auto remaining = initial_counter;
    std::array<std::uint8_t, 0x800> ring{};
    std::uint16_t ring_index = 0;
    std::uint8_t control = 0;
    std::uint16_t control_bits_remaining = 1;
    std::vector<std::uint8_t> output;
    output.reserve(static_cast<std::size_t>(initial_counter) + 1U);

    const auto read_byte = [&]() {
        if (source >= rom.size()) {
            throw std::out_of_range("Genesis resource token is truncated");
        }
        return rom[source++];
    };
    const auto emit = [&](std::uint8_t value) {
        ring[ring_index] = value;
        ring_index = static_cast<std::uint16_t>((ring_index + 1U) & 0x07FFU);
        output.push_back(value);
        --remaining;
    };

    for (;;) {
        --control_bits_remaining;
        if (control_bits_remaining == 0) {
            control = read_byte();
            control_bits_remaining = 8;
        }
        const auto compressed = (control & 0x80U) != 0;
        control = static_cast<std::uint8_t>(control << 1U);

        if (!compressed) {
            emit(read_byte());
        } else {
            const auto descriptor = read_byte();
            if ((descriptor & 0x80U) == 0) {
                const auto run_count = static_cast<unsigned>(
                    (descriptor & 0x7FU) + 2U);
                const auto value = read_byte();
                for (unsigned index = 0; index <= run_count; ++index) {
                    emit(value);
                }
            } else {
                const auto offset_low = read_byte();
                const auto offset = static_cast<std::uint16_t>(
                    ((static_cast<std::uint16_t>(offset_low) << 3U) |
                     ((descriptor >> 4U) & 0x0007U)) & 0x07FFU);
                const auto run_count = static_cast<unsigned>(
                    (descriptor & 0x0FU) + 2U);
                auto copy_index = static_cast<std::uint16_t>(
                    (ring_index - offset) & 0x07FFU);
                for (unsigned index = 0; index <= run_count; ++index) {
                    const auto value = ring[copy_index];
                    copy_index = static_cast<std::uint16_t>(
                        (copy_index + 1U) & 0x07FFU);
                    emit(value);
                }
            }
        }
        if (remaining == 0xFFFFU) {
            break;
        }
    }
    return output;
}

std::array<native::Rgba8, 16> read_genesis_palette(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset) {
    require_bytes(rom.size() - std::min(byte_offset, rom.size()), 32,
                  "Genesis palette is truncated");
    std::array<native::Rgba8, 16> palette{};
    for (std::size_t index = 0; index < palette.size(); ++index) {
        const auto offset = byte_offset + index * 2U;
        const auto word = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
        palette[index] = decode_genesis_cram_word(word, index == 0);
    }
    return palette;
}

std::array<native::Rgba8, 64> read_genesis_palette_set(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset) {
    require_bytes(rom.size() - std::min(byte_offset, rom.size()), 128,
                  "Genesis palette set is truncated");
    std::array<native::Rgba8, 64> palette{};
    for (std::size_t index = 0; index < palette.size(); ++index) {
        const auto offset = byte_offset + index * 2U;
        const auto word = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
        palette[index] = decode_genesis_cram_word(word, index % 16U == 0);
    }
    return palette;
}

GenesisMode2FieldResources read_genesis_mode2_field_resources(
    const std::vector<std::uint8_t>& rom, std::size_t record_offset) {
    if (record_offset > rom.size() || rom.size() - record_offset < 0x3EU) {
        throw std::out_of_range("Genesis mode-2 field record is truncated");
    }

    const auto setup_block = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x34U,
        "Genesis mode-2 setup pointer is truncated"));
    if (setup_block > rom.size() || rom.size() - setup_block < 0x20U) {
        throw std::out_of_range("Genesis mode-2 setup block is truncated");
    }

    const auto width_bytes = static_cast<std::size_t>(read_rom_u16(
        rom, record_offset + 0x24U,
        "Genesis mode-2 tilemap stride is truncated"));
    if (width_bytes == 0 || (width_bytes & 1U) != 0) {
        throw std::runtime_error(
            "Genesis mode-2 tilemap stride is not word-aligned");
    }
    const auto tilemap_width = width_bytes / 2U;

    const auto primary_tilemap = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x14U,
        "Genesis mode-2 primary tilemap pointer is truncated"));
    const auto secondary_tilemap = static_cast<std::size_t>(read_rom_u32(
        rom, record_offset + 0x1CU,
        "Genesis mode-2 secondary tilemap pointer is truncated"));
    if (primary_tilemap > rom.size() || secondary_tilemap > rom.size()) {
        throw std::out_of_range("Genesis mode-2 tilemaps are outside the ROM");
    }
    if (secondary_tilemap <= primary_tilemap ||
        (secondary_tilemap - primary_tilemap) % 2U != 0) {
        throw std::runtime_error(
            "Genesis mode-2 tilemap pointers do not describe adjacent maps");
    }
    const auto tilemap_entries = (secondary_tilemap - primary_tilemap) / 2U;
    if (tilemap_entries == 0 || tilemap_entries % tilemap_width != 0) {
        throw std::runtime_error(
            "Genesis mode-2 tilemap size does not match its stride");
    }

    return {
        record_offset,
        read_rom_u16(rom, record_offset,
                     "Genesis mode-2 camera X is truncated"),
        read_rom_u16(rom, record_offset + 2U,
                     "Genesis mode-2 camera Y is truncated"),
        {{
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x42U,
                             "Genesis mode-2 first field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x44U,
                             "Genesis mode-2 first field-record Y is truncated")},
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x46U,
                             "Genesis mode-2 second field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x48U,
                             "Genesis mode-2 second field-record Y is truncated")},
            GenesisFieldRecordPosition{
                read_rom_u16(rom, record_offset + 0x4AU,
                             "Genesis mode-2 third field-record X is truncated"),
                read_rom_u16(rom, record_offset + 0x4CU,
                             "Genesis mode-2 third field-record Y is truncated")}
        }},
        read_rom_u16(rom, record_offset + 0x0CU,
                     "Genesis mode-2 terrain X bound is truncated"),
        read_rom_u16(rom, record_offset + 0x0EU,
                     "Genesis mode-2 terrain Y bound is truncated"),
        read_rom_u16(rom, record_offset + 0x10U,
                     "Genesis mode-2 terrain X minimum is truncated"),
        read_rom_u16(rom, record_offset + 0x12U,
                     "Genesis mode-2 terrain Y minimum is truncated"),
        static_cast<std::size_t>(read_rom_u32(
            rom, record_offset + 0x26U,
            "Genesis mode-2 terrain record pointer is truncated")),
        setup_block,
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 4U,
            "Genesis mode-2 palette pointer is truncated")),
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 0x10U,
            "Genesis mode-2 pattern pointer is truncated")) & ~std::size_t{1},
        static_cast<std::size_t>(read_rom_u32(
            rom, setup_block + 0x18U,
            "Genesis mode-2 pattern length is truncated")),
        static_cast<std::size_t>(read_rom_u32(
            rom, record_offset + 0x18U,
            "Genesis mode-2 metatile pointer is truncated")) & ~std::size_t{1},
        primary_tilemap,
        secondary_tilemap,
        tilemap_width,
        tilemap_entries / tilemap_width,
    };
}

GenesisTerrainMap decode_genesis_mode2_terrain_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources) {
    GenesisTerrainMap terrain{};
    terrain.x_exclusive_upper = resources.terrain_x_exclusive_upper;
    terrain.y_exclusive_upper = resources.terrain_y_exclusive_upper;
    terrain.x_inclusive_lower = resources.terrain_x_inclusive_lower;
    terrain.y_inclusive_lower = resources.terrain_y_inclusive_lower;
    terrain.statuses.assign(GenesisTerrainMap::kWidth *
                                GenesisTerrainMap::kHeight,
                            0);
    auto cursor = resources.terrain_record_offset;
    for (;;) {
        const auto x_start = read_rom_u16(
            rom, cursor, "Genesis mode-2 terrain record is truncated");
        if ((x_start & 0x8000U) != 0) {
            return terrain;
        }
        const auto x_end = read_rom_u16(
            rom, cursor + 2U,
            "Genesis mode-2 terrain X range is truncated");
        const auto y_start = read_rom_u16(
            rom, cursor + 4U,
            "Genesis mode-2 terrain Y range is truncated");
        const auto y_end = read_rom_u16(
            rom, cursor + 6U,
            "Genesis mode-2 terrain Y range is truncated");
        const auto status = read_rom_u16(
            rom, cursor + 8U,
            "Genesis mode-2 terrain status is truncated");
        if (x_end < x_start || y_end < (y_start & 0xFFE0U)) {
            throw std::runtime_error(
                "Genesis mode-2 terrain rectangle is inverted");
        }
        terrain.regions.push_back({
            x_start, x_end, y_start, y_end,
            static_cast<std::uint8_t>(status)});

        auto y_value = static_cast<std::uint32_t>(y_start & 0xFFE0U);
        for (;;) {
            const auto row = y_value / 0x20U;
            if (row >= GenesisTerrainMap::kHeight) {
                throw std::out_of_range(
                    "Genesis mode-2 terrain row is outside the grid");
            }
            auto x_value = static_cast<std::uint32_t>(x_start);
            auto column = static_cast<std::size_t>(x_start >> 5U);
            for (;;) {
                if (column >= GenesisTerrainMap::kWidth) {
                    throw std::out_of_range(
                        "Genesis mode-2 terrain column is outside the grid");
                }
                terrain.statuses[row * GenesisTerrainMap::kWidth + column] =
                    static_cast<std::uint8_t>(status);
                if (x_value + 0x20U > x_end) {
                    break;
                }
                x_value += 0x20U;
                ++column;
            }
            if (y_value + 0x20U > y_end) {
                break;
            }
            y_value += 0x20U;
        }
        ++terrain.region_count;
        cursor += 10U;
    }
}

RgbaImage decode_tile_atlas(
    const std::vector<std::uint8_t>& rom,
    std::size_t tile_byte_offset,
    std::size_t tile_count,
    const std::array<native::Rgba8, 16>& palette,
    std::size_t columns,
    int transparent_index) {
    if (columns == 0 || columns > std::numeric_limits<std::uint16_t>::max()) {
        throw std::invalid_argument("tile atlas column count is invalid");
    }
    if (tile_count > std::numeric_limits<std::uint16_t>::max() / 8U) {
        throw std::invalid_argument("tile atlas is too large");
    }
    const auto rows = (tile_count + columns - 1U) / columns;
    if (rows > std::numeric_limits<std::uint16_t>::max() / 8U) {
        throw std::invalid_argument("tile atlas is too large");
    }
    if (tile_byte_offset > rom.size() ||
        tile_count > (rom.size() - tile_byte_offset) / 32U) {
        throw std::out_of_range("tile atlas is truncated");
    }

    RgbaImage image{
        static_cast<std::uint16_t>(columns * 8U),
        static_cast<std::uint16_t>(rows * 8U),
        std::vector<native::Rgba8>(columns * rows * 64U),
    };
    const native::Rgba8 transparent{0, 0, 0, 0};
    for (std::size_t tile_index = 0; tile_index < tile_count; ++tile_index) {
        const auto tile = decode_genesis_tile(
            rom.data() + tile_byte_offset + tile_index * 32U, 32U);
        const auto tile_x = (tile_index % columns) * 8U;
        const auto tile_y = (tile_index / columns) * 8U;
        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                const auto index = tile.pixels[pixel_y * 8U + pixel_x];
                image.pixels[(tile_y + pixel_y) * image.width +
                             tile_x + pixel_x] =
                    index == transparent_index ? transparent : palette[index];
            }
        }
    }
    return image;
}

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    const GenesisMode2FieldResources& resources) {
    return decode_genesis_mode2_field_map(
        rom, resources.pattern_data_offset, resources.pattern_byte_count,
        resources.metatile_stream_offset, resources.primary_tilemap_offset,
        resources.secondary_tilemap_offset, resources.tilemap_width,
        resources.tilemap_height, resources.palette_offset);
}

RgbaImage decode_genesis_mode2_field_map(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_data_offset,
    std::size_t pattern_byte_count,
    std::size_t metatile_stream_offset,
    std::size_t primary_tilemap_offset,
    std::size_t secondary_tilemap_offset,
    std::size_t tilemap_width,
    std::size_t tilemap_height,
    std::size_t palette_offset) {
    if (tilemap_width == 0 || tilemap_height == 0) {
        throw std::invalid_argument("Genesis field map dimensions are invalid");
    }
    if (tilemap_width > std::numeric_limits<std::size_t>::max() / 32U ||
        tilemap_height > std::numeric_limits<std::size_t>::max() / 32U) {
        throw std::invalid_argument("Genesis field map dimensions overflow");
    }
    const auto output_width = tilemap_width * 32U;
    const auto output_height = tilemap_height * 32U;
    if (output_width > std::numeric_limits<std::uint16_t>::max() ||
        output_height > std::numeric_limits<std::uint16_t>::max()) {
        throw std::invalid_argument("Genesis field map is too large");
    }
    if (tilemap_width > std::numeric_limits<std::size_t>::max() /
                           tilemap_height) {
        throw std::invalid_argument("Genesis field map dimensions overflow");
    }
    const auto map_entries = tilemap_width * tilemap_height;
    const auto require_map = [&](std::size_t offset, const char* what) {
        if (offset > rom.size() ||
            map_entries > (rom.size() - offset) / 2U) {
            throw std::out_of_range(what);
        }
    };
    require_map(primary_tilemap_offset, "Genesis primary field tilemap is truncated");
    require_map(secondary_tilemap_offset,
                "Genesis secondary field tilemap is truncated");

    if (pattern_data_offset > rom.size() ||
        pattern_byte_count > rom.size() - pattern_data_offset ||
        pattern_byte_count < 32U || pattern_byte_count % 32U != 0) {
        throw std::runtime_error(
            "Genesis field pattern upload is not tile-aligned");
    }
    const auto metatile_resource = decode_genesis_resource_stream(
        rom, metatile_stream_offset);
    if (metatile_resource.empty() || metatile_resource.size() % 32U != 0) {
        throw std::runtime_error("Genesis field metatiles are not stamp-aligned");
    }
    const auto tile_count = pattern_byte_count / 32U;
    const auto metatile_count = metatile_resource.size() / 32U;
    std::vector<IndexedTile> tiles;
    tiles.reserve(tile_count);
    for (std::size_t tile_index = 0; tile_index < tile_count; ++tile_index) {
        tiles.push_back(decode_genesis_tile(
            rom.data() + pattern_data_offset + tile_index * 32U, 32U));
    }

    const auto palette = read_genesis_palette_set(rom, palette_offset);
    const auto read_map_word = [&](std::size_t offset,
                                   std::size_t entry_index) {
        const auto word_offset = offset + entry_index * 2U;
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[word_offset]) << 8U) |
            rom[word_offset + 1U]);
    };
    const auto read_metatile_word = [&](std::size_t metatile_index,
                                        std::size_t word_index) {
        const auto word_offset = metatile_index * 32U + word_index * 2U;
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(metatile_resource[word_offset]) << 8U) |
            metatile_resource[word_offset + 1U]);
    };
    RgbaImage image{
        static_cast<std::uint16_t>(output_width),
        static_cast<std::uint16_t>(output_height),
        std::vector<native::Rgba8>(output_width * output_height),
    };

    const auto render_layer = [&](std::size_t tilemap_offset,
                                  bool transparent_zero) {
        for (std::size_t stamp_y = 0; stamp_y < tilemap_height; ++stamp_y) {
            for (std::size_t stamp_x = 0; stamp_x < tilemap_width; ++stamp_x) {
                const auto metatile_index = static_cast<std::size_t>(
                    read_map_word(tilemap_offset,
                                  stamp_y * tilemap_width + stamp_x));
                if (metatile_index >= metatile_count) {
                    throw std::out_of_range(
                        "Genesis field metatile index is truncated (index " +
                        std::to_string(metatile_index) + ", count " +
                        std::to_string(metatile_count) + ")");
                }
                for (std::size_t tile_y = 0; tile_y < 4U; ++tile_y) {
                    for (std::size_t tile_x = 0; tile_x < 4U; ++tile_x) {
                        const auto name_word = read_metatile_word(
                            metatile_index, tile_y * 4U + tile_x);
                        const auto pattern_word =
                            static_cast<std::int32_t>(name_word & 0x07FFU) -
                            0x0201;
                        if (pattern_word < 0 ||
                            static_cast<std::size_t>(pattern_word) >= tile_count) {
                            throw std::out_of_range(
                                "Genesis field pattern index is truncated (name " +
                                std::to_string(name_word) + ", relative " +
                                std::to_string(pattern_word) + ", tiles " +
                                std::to_string(tile_count) + ")");
                        }
                        const auto& tile = tiles[
                            static_cast<std::size_t>(pattern_word)];
                        const auto palette_line = static_cast<std::size_t>(
                            (name_word >> 13U) & 0x03U);
                        const auto hflip = (name_word & 0x0800U) != 0;
                        const auto vflip = (name_word & 0x1000U) != 0;
                        const auto origin_x = stamp_x * 32U + tile_x * 8U;
                        const auto origin_y = stamp_y * 32U + tile_y * 8U;
                        for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                            for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                                const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                                const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                                const auto color_index = tile.pixels[
                                    source_y * 8U + source_x];
                                if (transparent_zero && color_index == 0) {
                                    continue;
                                }
                                auto color = palette[palette_line * 16U +
                                                      color_index];
                                color.alpha = 255;
                                image.pixels[(origin_y + pixel_y) * image.width +
                                             origin_x + pixel_x] = color;
                            }
                        }
                    }
                }
            }
        }
    };
    render_layer(primary_tilemap_offset, false);
    render_layer(secondary_tilemap_offset, true);
    return image;
}

RgbaImage decode_genesis_entity_frame(
    const std::vector<std::uint8_t>& rom,
    std::size_t descriptor_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset) {
    require_bytes(rom.size() - std::min(descriptor_offset, rom.size()), 0x10U,
                  "Genesis entity frame descriptor is truncated");
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    if (read16(descriptor_offset, "Genesis entity frame marker is truncated") !=
        0x8002U) {
        throw std::runtime_error(
            "Genesis entity descriptor is not an 0x8002 frame");
    }
    const auto source = static_cast<std::size_t>(read32(
        descriptor_offset + 2U, "Genesis entity frame source is truncated"));
    const auto length = static_cast<std::size_t>(read16(
        descriptor_offset + 6U, "Genesis entity frame length is truncated"));
    const auto dimensions = read16(
        descriptor_offset + 0x0CU,
        "Genesis entity frame dimensions are truncated");
    const auto columns = static_cast<std::size_t>((dimensions >> 2U) + 1U);
    const auto rows = static_cast<std::size_t>((dimensions & 3U) + 1U);
    const auto expected_length = columns * rows * 32U;
    if (length != expected_length) {
        throw std::runtime_error(
            "Genesis entity frame length does not match its dimensions");
    }
    if (source > rom.size() || length > rom.size() - source) {
        throw std::out_of_range("Genesis entity frame source is truncated");
    }
    require_bytes(rom.size() - std::min(nibble_table_offset, rom.size()),
                  16U, "Genesis entity nibble table is truncated");

    const auto palette = read_genesis_palette(rom, palette_offset);
    std::vector<std::uint8_t> decoded(length);
    for (std::size_t index = 0; index < length; ++index) {
        const auto packed = rom[source + index];
        const auto low = rom[nibble_table_offset + (packed & 0x0FU)];
        const auto high = rom[nibble_table_offset + (packed >> 4U)];
        decoded[index] = static_cast<std::uint8_t>(low | (high << 4U));
    }

    RgbaImage image{
        static_cast<std::uint16_t>(columns * 8U),
        static_cast<std::uint16_t>(rows * 8U),
        std::vector<native::Rgba8>(columns * rows * 64U,
                                   native::Rgba8{0, 0, 0, 0}),
    };
    // 0x3EFE consumes the generated tile buffer column first, then row.
    for (std::size_t column = 0; column < columns; ++column) {
        for (std::size_t row = 0; row < rows; ++row) {
            const auto tile_index = column * rows + row;
            const auto tile = decode_genesis_tile(
                decoded.data() + tile_index * 32U, 32U);
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto color_index =
                        tile.pixels[pixel_y * 8U + pixel_x];
                    auto color = palette[color_index];
                    if (color_index == 0) {
                        color.alpha = 0;
                    }
                    image.pixels[(row * 8U + pixel_y) * image.width +
                                 column * 8U + pixel_x] = color;
                }
            }
        }
    }
    return image;
}

std::vector<RgbaImage> decode_genesis_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t state_block_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset) {
    require_bytes(rom.size() - std::min(state_block_offset, rom.size()), 4U,
                  "Genesis entity animation state block is truncated");
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    const auto state_marker = read16(
        state_block_offset, "Genesis entity animation marker is truncated");
    if ((state_marker & 0x8000U) == 0) {
        throw std::runtime_error(
            "Genesis entity animation state block has no terminal marker");
    }
    const auto frame_count = static_cast<std::size_t>(state_marker & 0x7FFFU);
    if (frame_count == 0 || frame_count >
        (rom.size() - std::min(state_block_offset, rom.size()) - 4U) / 4U) {
        throw std::runtime_error(
            "Genesis entity animation frame table is truncated");
    }

    std::vector<RgbaImage> frames;
    frames.reserve(frame_count);
    for (std::size_t index = 0; index < frame_count; ++index) {
        const auto descriptor = static_cast<std::size_t>(read32(
            state_block_offset + 4U + index * 4U,
            "Genesis entity animation descriptor is truncated"));
        frames.push_back(decode_genesis_entity_frame(
            rom, descriptor, nibble_table_offset, palette_offset));
    }
    return frames;
}

std::vector<RgbaImage> decode_genesis_field_entity_animation(
    const std::vector<std::uint8_t>& rom,
    std::size_t entity_record_offset,
    std::size_t entity_state_pointer_table_offset,
    std::size_t nibble_table_offset,
    std::size_t palette_offset,
    std::size_t state_count) {
    const auto read16 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2U, what);
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };
    const auto read32 = [&](std::size_t offset, const char* what) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 4U, what);
        return (static_cast<std::uint32_t>(rom[offset]) << 24U) |
               (static_cast<std::uint32_t>(rom[offset + 1U]) << 16U) |
               (static_cast<std::uint32_t>(rom[offset + 2U]) << 8U) |
               static_cast<std::uint32_t>(rom[offset + 3U]);
    };

    const auto entity_kind = read16(
        entity_record_offset + 0x0EU,
        "Genesis field entity kind is truncated");
    const auto first_pointer = static_cast<std::size_t>(read32(
        entity_state_pointer_table_offset +
            static_cast<std::size_t>(entity_kind) * 4U,
        "Genesis field entity state pointer is truncated"));
    const auto state_selector = static_cast<std::size_t>(read16(
        entity_record_offset + 0x16U,
        "Genesis field entity state selector is truncated") & 0x7FFFU);
    const auto selected_state_pointer = static_cast<std::size_t>(read32(
        first_pointer + state_selector,
        "Genesis field entity state list is truncated"));
    const auto record_state_index = read16(
        entity_record_offset + 0x1EU,
        "Genesis field entity state index is truncated");
    const auto selected_state_marker = read16(
        selected_state_pointer,
        "Genesis field entity state marker is truncated");
    if (static_cast<std::int16_t>(selected_state_marker) < 0) {
        throw std::runtime_error(
            "Genesis field entity starts in an unsupported terminal state");
    }

    if (state_count == 0 || state_count > 256) {
        throw std::runtime_error(
            "Genesis field entity state count is outside the supported range");
    }

    std::vector<RgbaImage> frames;
    frames.reserve(state_count);
    for (std::size_t index = 0; index < state_count; ++index) {
        // The outer table is a longword table at the address selected by the
        // kind. Its entries point to independent state lists; they are not
        // eight adjacent frames in one list. ROM 0x600E uses the record's
        // +0x1E index, resetting it to zero when it reaches a list's marker.
        const auto state_list = static_cast<std::size_t>(read32(
            first_pointer + index * 4U,
            "Genesis field entity state-pointer table is truncated"));
        const auto state_marker = read16(
            state_list, "Genesis field entity state-list marker is truncated");
        if (static_cast<std::int16_t>(state_marker) < 0) {
            throw std::runtime_error(
                "Genesis field entity state-pointer table terminates early");
        }
        const auto frame_index = record_state_index >= state_marker
            ? 0U : static_cast<std::size_t>(record_state_index);
        const auto descriptor = static_cast<std::size_t>(read32(
            state_list + 4U + frame_index * 4U,
            "Genesis field entity frame descriptor is truncated"));
        if (read16(descriptor,
                   "Genesis field entity descriptor is truncated") !=
            0x8002U) {
            throw std::runtime_error(
                "Genesis field entity does not resolve to an 0x8002 frame");
        }
        frames.push_back(decode_genesis_entity_frame(
            rom, descriptor, nibble_table_offset, palette_offset));
    }
    return frames;
}

RgbaImage decode_genesis_startup_screen(
    const std::vector<std::uint8_t>& rom,
    std::size_t pattern_stream_offset,
    std::size_t tilemap_stream_offset,
    std::size_t palette_offset) {
    constexpr std::size_t kColumns = 40;
    constexpr std::size_t kRows = 28;
    constexpr std::size_t kMapRows = 32;
    constexpr std::uint16_t kWordsPerRow = 0x27;
    constexpr std::uint16_t kRowCounter = 0x1F;
    constexpr std::uint16_t kD5WordCount = 0x28;
    constexpr std::uint16_t kWordMask = 0x4000;
    constexpr std::size_t kPatternVramSize = 0x10000;
    constexpr std::size_t kPatternVramOffset = 0x20;
    constexpr std::size_t kPatternStream2Offset = 0x67CB0;
    constexpr std::size_t kPatternStream2VramOffset = 0x4B20;
    constexpr std::size_t kPatternStream3Offset = 0x6875C;
    constexpr std::size_t kPatternStream3VramOffset = 0x5020;

    const auto pattern_stream1 = decode_genesis_resource_stream(
        rom, pattern_stream_offset);
    const auto pattern_stream2 = decode_genesis_resource_stream(
        rom, kPatternStream2Offset);
    const auto pattern_stream3 = decode_genesis_resource_stream(
        rom, kPatternStream3Offset);
    std::vector<std::uint8_t> pattern_vram(kPatternVramSize, 0);
    const auto copy_pattern_stream = [&](const auto& bytes, std::size_t target) {
        if (target > pattern_vram.size() ||
            bytes.size() > pattern_vram.size() - target) {
            throw std::out_of_range("Genesis startup pattern upload is truncated");
        }
        std::copy(bytes.begin(), bytes.end(), pattern_vram.begin() + target);
    };
    copy_pattern_stream(pattern_stream1, kPatternVramOffset);
    copy_pattern_stream(pattern_stream2, kPatternStream2VramOffset);
    copy_pattern_stream(pattern_stream3, kPatternStream3VramOffset);
    const auto palette = read_genesis_palette_set(rom, palette_offset);
    require_bytes(rom.size() - std::min(tilemap_stream_offset, rom.size()), 2,
                  "Genesis startup tilemap is truncated");

    const auto read16 = [&](std::size_t offset) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 2,
                      "Genesis startup tilemap word is truncated");
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(rom[offset]) << 8U) |
            rom[offset + 1U]);
    };

    const auto read8 = [&](std::size_t offset) {
        require_bytes(rom.size() - std::min(offset, rom.size()), 1,
                      "Genesis startup tilemap byte is truncated");
        return rom[offset];
    };
    auto source = tilemap_stream_offset;
    const auto base_word = read16(source);
    source += 2U;
    std::vector<std::uint16_t> map;
    map.reserve(kColumns * kMapRows);
    std::uint16_t output_index = 0;
    std::uint16_t words_remaining = kWordsPerRow;
    auto rows_remaining = kRowCounter;
    const auto row_compare = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(kWordsPerRow - kD5WordCount));

    const auto emit = [&](std::uint16_t value) {
        ++output_index;
        if (static_cast<std::int16_t>(
                static_cast<std::uint16_t>(0U - output_index)) >= 0) {
            return false;
        }
        if (static_cast<std::int16_t>(words_remaining) > row_compare) {
            value = static_cast<std::uint16_t>(value | kWordMask);
        }
        map.push_back(value);
        --words_remaining;
        if (words_remaining != 0xFFFFU) {
            return false;
        }
        --rows_remaining;
        if (rows_remaining == 0xFFFFU) {
            return true;
        }
        words_remaining = kWordsPerRow;
        return false;
    };

    while (rows_remaining != 0xFFFFU) {
        const auto token = read8(source++);
        if (token == 0) {
            break;
        }
        if (token > 0xC0U) {
            const auto count = static_cast<unsigned>(token & 0x1FU);
            const auto iterations = count == 0 ? 1U : count;
            const auto wide = (token & 0x20U) != 0;
            for (unsigned index = 0; index < iterations; ++index) {
                std::uint16_t value = 0;
                if (wide) {
                    value = read16(source);
                    source += 2U;
                } else {
                    value = read8(source++);
                }
                value = static_cast<std::uint16_t>(value + base_word);
                if (emit(value)) {
                    rows_remaining = 0xFFFFU;
                    break;
                }
            }
            continue;
        }

        const auto delta = token > 0x80U ? 1 : token > 0x40U ? 0 : -1;
        const auto count = static_cast<unsigned>(token & 0x1FU);
        const auto wide = (token & 0x20U) != 0;
        std::uint16_t value = 0;
        if (wide) {
            value = static_cast<std::uint16_t>(read8(source++)) << 8U;
        }
        value = static_cast<std::uint16_t>(value | read8(source++));
        value = static_cast<std::uint16_t>(value + base_word);
        for (unsigned index = 0; index <= count; ++index) {
            if (emit(value)) {
                rows_remaining = 0xFFFFU;
                break;
            }
            value = static_cast<std::uint16_t>(value + delta);
        }
    }

    if (map.empty()) {
        throw std::runtime_error("Genesis startup tilemap is empty");
    }

    RgbaImage image{
        static_cast<std::uint16_t>(kColumns * 8U),
        static_cast<std::uint16_t>(kRows * 8U),
        std::vector<native::Rgba8>(
            kColumns * kRows * 64U, native::Rgba8{0, 0, 0, 255}),
    };
    for (std::size_t map_y = 0; map_y < kRows; ++map_y) {
        for (std::size_t map_x = 0; map_x < kColumns; ++map_x) {
            const auto map_index = map_y * kColumns + map_x;
            if (map_index >= map.size()) {
                continue;
            }
            const auto entry = map[map_index];
            if (entry == 0) {
                continue;
            }
            const auto tile_index = static_cast<std::size_t>(entry & 0x07FFU);
            if (tile_index * 32U + 32U > pattern_vram.size()) {
                throw std::out_of_range("Genesis startup tile index is truncated");
            }
            const auto tile = decode_genesis_tile(
                pattern_vram.data() + tile_index * 32U, 32U);
            const auto palette_line = static_cast<std::size_t>(
                (entry >> 13U) & 0x03U);
            const auto hflip = (entry & 0x0800U) != 0;
            const auto vflip = (entry & 0x1000U) != 0;
            for (std::size_t pixel_y = 0; pixel_y < 8U; ++pixel_y) {
                for (std::size_t pixel_x = 0; pixel_x < 8U; ++pixel_x) {
                    const auto source_x = hflip ? 7U - pixel_x : pixel_x;
                    const auto source_y = vflip ? 7U - pixel_y : pixel_y;
                    const auto color_index = tile.pixels[source_y * 8U +
                                                         source_x];
                    image.pixels[(map_y * 8U + pixel_y) * image.width +
                                 map_x * 8U + pixel_x] =
                        palette[palette_line * 16U + color_index];
                }
            }
        }
    }
    return image;
}

void write_bmp32(const std::string& path, const RgbaImage& image) {
    if (image.width == 0 || image.height == 0 ||
        image.pixels.size() != static_cast<std::size_t>(image.width) *
                                    image.height) {
        throw std::invalid_argument("invalid RGBA image dimensions");
    }
    const auto row_bytes = static_cast<std::uint32_t>(image.width) * 4U;
    const auto pixel_bytes = row_bytes * image.height;
    const auto file_offset = 14U + 40U;
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("cannot open BMP output");
    }
    output.put('B');
    output.put('M');
    write_u32(output, file_offset + pixel_bytes);
    write_u16(output, 0);
    write_u16(output, 0);
    write_u32(output, file_offset);
    write_u32(output, 40);
    write_u32(output, image.width);
    write_u32(output, image.height);
    write_u16(output, 1);
    write_u16(output, 32);
    write_u32(output, 0);
    write_u32(output, pixel_bytes);
    write_u32(output, 2835);
    write_u32(output, 2835);
    write_u32(output, 0);
    write_u32(output, 0);

    for (std::size_t row = image.height; row-- > 0;) {
        for (std::size_t column = 0; column < image.width; ++column) {
            const auto pixel = image.pixels[row * image.width + column];
            output.put(static_cast<char>(pixel.blue));
            output.put(static_cast<char>(pixel.green));
            output.put(static_cast<char>(pixel.red));
            output.put(static_cast<char>(pixel.alpha));
        }
    }
    if (!output) {
        throw std::runtime_error("cannot write BMP output");
    }
}

} // namespace shadowrun::graphics

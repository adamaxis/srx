#include "shadowrun/graphics.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace shadowrun::graphics {

namespace {

void require_bytes(std::size_t available, std::size_t required,
                   const char* what) {
    if (required > available) {
        throw std::out_of_range(what);
    }
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

} // namespace

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
        const auto plane0 = source[row * 4U];
        const auto plane1 = source[row * 4U + 1U];
        const auto plane2 = source[row * 4U + 2U];
        const auto plane3 = source[row * 4U + 3U];
        for (std::size_t column = 0; column < IndexedTile::kWidth; ++column) {
            const auto bit = static_cast<unsigned>(7U - column);
            tile.pixels[row * IndexedTile::kWidth + column] =
                static_cast<std::uint8_t>(
                    ((plane0 >> bit) & 1U) |
                    (((plane1 >> bit) & 1U) << 1U) |
                    (((plane2 >> bit) & 1U) << 2U) |
                    (((plane3 >> bit) & 1U) << 3U));
        }
    }
    return tile;
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

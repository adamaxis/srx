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

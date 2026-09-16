#pragma once

#include "shadowrun/native_port.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace shadowrun::graphics {

struct IndexedTile {
    static constexpr std::size_t kWidth = 8;
    static constexpr std::size_t kHeight = 8;
    std::array<std::uint8_t, kWidth * kHeight> pixels{};
};

struct RgbaImage {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::vector<native::Rgba8> pixels;
};

// Genesis CRAM stores three 3-bit channels in the spaced bit positions of a
// 16-bit word. This expands each channel to the normal 0..255 host range.
native::Rgba8 decode_genesis_cram_word(std::uint16_t cram_word,
                                       bool transparent = false);

// Decode one Genesis 4-bit planar tile. Each row consists of four plane bytes,
// rather than eight packed pixel nibbles.
IndexedTile decode_genesis_tile(const std::uint8_t* source,
                                std::size_t source_size);

std::array<native::Rgba8, 16> read_genesis_palette(
    const std::vector<std::uint8_t>& rom, std::size_t byte_offset);

RgbaImage decode_tile_atlas(
    const std::vector<std::uint8_t>& rom,
    std::size_t tile_byte_offset,
    std::size_t tile_count,
    const std::array<native::Rgba8, 16>& palette,
    std::size_t columns = 16,
    int transparent_index = -1);

// Write a 32-bit uncompressed BMP. This is intentionally dependency-free and
// can be opened directly by Windows or used as an intermediate asset format.
void write_bmp32(const std::string& path, const RgbaImage& image);

} // namespace shadowrun::graphics

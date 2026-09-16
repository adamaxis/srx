#include "shadowrun/graphics.hpp"

#include <array>
#include <cassert>
#include <cstdint>
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
    tile[0] = 0x80;
    tile[1] = 0x40;
    tile[2] = 0x20;
    tile[3] = 0x10;
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
    const auto metatile_resource = decode_genesis_resource_stream(rom, 0x170102);
    assert(metatile_resource.size() == 0x4F40);
    const auto field_map = decode_genesis_mode2_field_map(rom);
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
    return 0;
}

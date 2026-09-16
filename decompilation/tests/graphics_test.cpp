#include "shadowrun/graphics.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

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
    return 0;
}

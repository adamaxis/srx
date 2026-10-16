#include "../field_mode_profile_import.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

void write_tile(std::vector<std::uint8_t>& vram, const std::size_t tile,
                const std::uint8_t color_index) {
    const auto packed = static_cast<std::uint8_t>(
        (color_index << 4U) | color_index);
    const auto offset = tile * 32U;
    for (std::size_t index = 0U; index < 32U; ++index) {
        vram[offset + index] = packed;
    }
}

bool color_is(const shadowrun::graphics::RgbaImage& image,
              const std::size_t tile_x, const std::size_t tile_y,
              const std::uint8_t red) {
    const auto& pixel = image.pixels[(tile_y * 8U) * image.width +
                                     tile_x * 8U];
    return pixel.red == red && pixel.alpha == 255U;
}

} // namespace

int main() {
    std::vector<std::uint8_t> pattern_vram(0x10000U, 0U);
    std::array<shadowrun::native::Rgba8, 16> palette{};
    for (std::size_t index = 0U; index < palette.size(); ++index) {
        palette[index] = shadowrun::native::Rgba8{
            static_cast<std::uint8_t>(index * 10U), 0U, 0U,
            static_cast<std::uint8_t>(index == 0U ? 0U : 255U)};
    }
    constexpr std::uint16_t kTileBase = 0x0100U;
    for (std::size_t index = 0U; index < 40U; ++index) {
        write_tile(pattern_vram, kTileBase + index,
                   static_cast<std::uint8_t>((index % 15U) + 1U));
    }

    // 0xE2 emits two explicit word deltas. The high 0x20 attribute bits in
    // the first word are retained by the source but do not change its tile
    // index. 0x41/0x40/0x81 then exercise the three 0x1728 step bands.
    std::vector<std::uint8_t> rom{
        0x01U, 0x00U, 0xE2U, 0x20U, 0x00U, 0x00U, 0x05U,
        0x41U, 0x00U, 0x40U, 0x00U, 0x81U, 0x00U, 0U,
    };
    const auto image =
        shadowrun::ghidra_conversion::rom_research::decode_1728_sprite_stream(
            pattern_vram, palette, rom, 0U);
    if (image.width != 56U || image.height != 72U ||
        !color_is(image, 0U, 0U, 10U) ||
        !color_is(image, 1U, 0U, 60U) ||
        !color_is(image, 2U, 0U, 10U) ||
        !color_is(image, 3U, 0U, 10U) ||
        !color_is(image, 4U, 0U, 10U) ||
        !color_is(image, 5U, 0U, 10U) ||
        !color_is(image, 6U, 0U, 20U) ||
        image.pixels[(1U * 8U) * image.width].alpha != 0U) {
        std::cerr << "0x1728 tile stream was not decoded in source order\n";
        return 1;
    }
    return 0;
}

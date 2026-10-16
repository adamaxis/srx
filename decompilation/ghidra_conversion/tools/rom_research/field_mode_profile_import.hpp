#pragma once

#include "shadowrun/rendering/graphics.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion::rom_research {

// Decode the field mode portrait shape used by ROM 0x1F292. The compressed
// pattern upload and the 0x1728 row stream are interpreted only by the
// offline importer; the host runtime consumes the resulting RGBA image.
graphics::RgbaImage decode_mode_profile_sprite(
    const std::vector<std::uint8_t>& rom, std::size_t profile_offset);

// Decode a 0x1728 row stream against already expanded pattern bytes. Kept
// separate so the ROM's row grammar can be checked with small synthetic
// inputs independently of the image assets.
graphics::RgbaImage decode_1728_sprite_stream(
    const std::vector<std::uint8_t>& pattern_vram,
    const std::array<native::Rgba8, 16>& palette,
    const std::vector<std::uint8_t>& rom, std::size_t sprite_stream_offset);

} // namespace shadowrun::ghidra_conversion::rom_research

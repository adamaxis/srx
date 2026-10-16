#include "field_mode_profile_import.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace shadowrun::ghidra_conversion::rom_research {

namespace {

std::uint16_t read_word(const std::vector<std::uint8_t>& bytes,
                        const std::size_t offset, const char* message) {
    if (offset > bytes.size() || bytes.size() - offset < 2U) {
        throw std::out_of_range(message);
    }
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        bytes[offset + 1U]);
}

std::uint32_t read_long(const std::vector<std::uint8_t>& bytes,
                        const std::size_t offset, const char* message) {
    if (offset > bytes.size() || bytes.size() - offset < 4U) {
        throw std::out_of_range(message);
    }
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3U]);
}

std::uint8_t read_byte(const std::vector<std::uint8_t>& bytes,
                       std::size_t& cursor) {
    if (cursor >= bytes.size()) {
        throw std::out_of_range("ROM 0x1728 sprite stream is truncated");
    }
    return bytes[cursor++];
}

std::uint16_t read_delta(const std::vector<std::uint8_t>& bytes,
                         std::size_t& cursor, const bool word_sized) {
    if (!word_sized) {
        return read_byte(bytes, cursor);
    }
    const auto high = read_byte(bytes, cursor);
    const auto low = read_byte(bytes, cursor);
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8U) | low);
}

} // namespace

graphics::RgbaImage decode_1728_sprite_stream(
    const std::vector<std::uint8_t>& pattern_vram,
    const std::array<native::Rgba8, 16>& palette,
    const std::vector<std::uint8_t>& rom,
    const std::size_t sprite_stream_offset) {
    constexpr std::size_t kVramBytes = 0x10000U;
    constexpr std::size_t kColumns = 7U;
    constexpr std::size_t kRows = 9U;
    constexpr std::size_t kTileBytes = 32U;
    constexpr std::uint16_t kPatternIndexMask = 0x07FFU;
    constexpr std::uint16_t kHorizontalFlip = 0x0800U;
    constexpr std::uint16_t kVerticalFlip = 0x1000U;

    if (pattern_vram.size() != kVramBytes) {
        throw std::invalid_argument("mode portrait VRAM has invalid size");
    }
    auto cursor = sprite_stream_offset;
    const auto tile_base = read_word(
        rom, cursor, "ROM 0x1728 sprite header is truncated");
    cursor += 2U;

    std::array<std::uint16_t, kColumns * kRows> tile_words{};
    std::array<bool, kColumns * kRows> written{};
    std::size_t output_tile_count = 0U;
    const auto emit_tile = [&](const std::uint16_t word) {
        if (output_tile_count >= tile_words.size()) {
            return;
        }
        tile_words[output_tile_count] = word;
        written[output_tile_count] = true;
        ++output_tile_count;
    };

    while (output_tile_count < tile_words.size()) {
        const auto descriptor = read_byte(rom, cursor);
        if (descriptor == 0U) {
            break;
        }

        const auto word_sized_delta = (descriptor & 0x20U) != 0U;
        if (descriptor > 0xC0U) {
            // 0x1728's upper branch reads one delta for every tile. Its BGT
            // loop produces max(1, low-five-bits) explicit tile values.
            const auto tile_count = std::max<std::size_t>(
                1U, static_cast<std::size_t>(descriptor & 0x1FU));
            for (std::size_t tile = 0U;
                 tile < tile_count && output_tile_count < tile_words.size();
                 ++tile) {
                emit_tile(static_cast<std::uint16_t>(
                    tile_base + read_delta(rom, cursor, word_sized_delta)));
            }
            continue;
        }

        // The lower branch selects a horizontal tile-index step from the
        // descriptor's upper two bits. Its BGE loop produces
        // low-five-bits + 1 sequential tile values. D0 wraps the resulting
        // stream every seven writes to the next portrait row.
        const auto step = descriptor <= 0x40U
                              ? -1
                              : descriptor <= 0x80U ? 0 : 1;
        const auto tile_count =
            static_cast<std::size_t>(descriptor & 0x1FU) + 1U;
        auto tile_word = static_cast<std::uint16_t>(
            tile_base + read_delta(rom, cursor, word_sized_delta));
        for (std::size_t tile = 0U;
             tile < tile_count && output_tile_count < tile_words.size();
             ++tile) {
            emit_tile(tile_word);
            tile_word = static_cast<std::uint16_t>(tile_word + step);
        }
    }

    graphics::RgbaImage image{
        static_cast<std::uint16_t>(kColumns * 8U),
        static_cast<std::uint16_t>(kRows * 8U),
        std::vector<native::Rgba8>(kColumns * kRows * 64U,
                                   native::Rgba8{0U, 0U, 0U, 0U}),
    };
    for (std::size_t cell = 0U; cell < tile_words.size(); ++cell) {
        if (!written[cell]) {
            continue;
        }
        const auto word = tile_words[cell];
        const auto tile_index = static_cast<std::size_t>(
            word & kPatternIndexMask);
        const auto pattern_offset = tile_index * kTileBytes;
        if (pattern_offset > pattern_vram.size() ||
            pattern_vram.size() - pattern_offset < kTileBytes) {
            throw std::out_of_range(
                "ROM 0x1728 tile is outside the expanded pattern memory");
        }
        const bool flip_x = (word & kHorizontalFlip) != 0U;
        const bool flip_y = (word & kVerticalFlip) != 0U;
        const auto tile_x = (cell % kColumns) * 8U;
        const auto tile_y = (cell / kColumns) * 8U;
        for (std::size_t y = 0U; y < 8U; ++y) {
            for (std::size_t x = 0U; x < 8U; ++x) {
                const auto source_x = flip_x ? 7U - x : x;
                const auto source_y = flip_y ? 7U - y : y;
                const auto packed = pattern_vram[
                    pattern_offset + source_y * 4U + source_x / 2U];
                const auto color_index = (source_x & 1U) == 0U
                                             ? packed >> 4U
                                             : packed & 0x0FU;
                auto color = palette[color_index];
                color.alpha = color_index == 0U ? 0U : 255U;
                image.pixels[(tile_y + y) * image.width + tile_x + x] =
                    color;
            }
        }
    }
    return image;
}

graphics::RgbaImage decode_mode_profile_sprite(
    const std::vector<std::uint8_t>& rom, const std::size_t profile_offset) {
    constexpr std::size_t kProfileBytes = 12U;
    constexpr std::size_t kPatternVramOffset = 0x57A0U;
    if (profile_offset > rom.size() ||
        rom.size() - profile_offset < kProfileBytes) {
        throw std::out_of_range("mode profile record is truncated");
    }
    const auto palette_offset = static_cast<std::size_t>(read_long(
        rom, profile_offset, "mode profile palette pointer is truncated"));
    const auto pattern_stream_offset = static_cast<std::size_t>(read_long(
        rom, profile_offset + 4U,
        "mode profile pattern pointer is truncated"));
    const auto sprite_stream_offset = static_cast<std::size_t>(read_long(
        rom, profile_offset + 8U,
        "mode profile sprite pointer is truncated"));

    std::array<native::Rgba8, 16> palette{};
    for (std::size_t index = 0U; index < palette.size(); ++index) {
        palette[index] = graphics::decode_genesis_cram_word(
            read_word(rom, palette_offset + index * 2U,
                      "mode profile palette is truncated"),
            index == 0U);
    }
    const auto pattern_vram = graphics::decode_genesis_1530_vram(
        rom, pattern_stream_offset, kPatternVramOffset);
    return decode_1728_sprite_stream(
        pattern_vram, palette, rom, sprite_stream_offset);
}

} // namespace shadowrun::ghidra_conversion::rom_research

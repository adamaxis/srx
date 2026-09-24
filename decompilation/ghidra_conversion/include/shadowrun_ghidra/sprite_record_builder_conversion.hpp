#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct SpriteSourceRecord {
    std::int16_t relative_y = 0;
    std::int16_t relative_x = 0;
    std::uint8_t size = 0;
    std::uint16_t attributes = 0;
};

struct BuiltSpriteRecord {
    std::int16_t y = 0;
    std::int16_t x = 0;
    std::uint8_t size = 0;
    std::uint8_t sequence = 0;
    std::uint16_t attributes = 0;
};

// ROM 0x1378: build runtime sprite records until the source y word is the
// 0x8000 sentinel.
std::vector<BuiltSpriteRecord> build_title_sprite_records(
    const std::vector<SpriteSourceRecord>& source,
    std::int16_t y_offset,
    std::int16_t x_offset,
    std::uint8_t first_sequence,
    std::uint16_t attribute_flags);

// ROM 0x13BE: same record expansion, with an additional signed attribute
// offset applied to each source attribute word.
std::vector<BuiltSpriteRecord> build_title_sprite_records_with_attribute_offset(
    const std::vector<SpriteSourceRecord>& source,
    std::int16_t y_offset,
    std::int16_t x_offset,
    std::uint8_t first_sequence,
    std::uint16_t attribute_flags,
    std::int16_t attribute_offset);

} // namespace shadowrun::ghidra_conversion

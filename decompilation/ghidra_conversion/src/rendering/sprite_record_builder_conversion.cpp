#include "shadowrun_ghidra/rendering/sprite_record_builder_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

std::vector<BuiltSpriteRecord> build_records(
    const std::vector<SpriteSourceRecord>& source,
    const std::int16_t y_offset,
    const std::int16_t x_offset,
    const std::uint8_t first_sequence,
    const std::uint16_t attribute_flags,
    const std::int16_t attribute_offset) {
    std::vector<BuiltSpriteRecord> result;
    std::uint8_t sequence = first_sequence;
    for (const auto& source_record : source) {
        if (source_record.relative_y == static_cast<std::int16_t>(-0x8000)) {
            break;
        }
        result.push_back({
            static_cast<std::int16_t>(y_offset + source_record.relative_y),
            static_cast<std::int16_t>(x_offset + source_record.relative_x),
            source_record.size,
            sequence,
            static_cast<std::uint16_t>(
                attribute_flags + source_record.attributes + attribute_offset),
        });
        sequence = static_cast<std::uint8_t>(sequence + 1U);
    }
    return result;
}

} // namespace

std::vector<BuiltSpriteRecord> build_title_sprite_records(
    const std::vector<SpriteSourceRecord>& source,
    const std::int16_t y_offset,
    const std::int16_t x_offset,
    const std::uint8_t first_sequence,
    const std::uint16_t attribute_flags) {
    return build_records(source, y_offset, x_offset, first_sequence,
                         attribute_flags, 0);
}

std::vector<BuiltSpriteRecord> build_title_sprite_records_with_attribute_offset(
    const std::vector<SpriteSourceRecord>& source,
    const std::int16_t y_offset,
    const std::int16_t x_offset,
    const std::uint8_t first_sequence,
    const std::uint16_t attribute_flags,
    const std::int16_t attribute_offset) {
    return build_records(source, y_offset, x_offset, first_sequence,
                         attribute_flags, attribute_offset);
}

} // namespace shadowrun::ghidra_conversion

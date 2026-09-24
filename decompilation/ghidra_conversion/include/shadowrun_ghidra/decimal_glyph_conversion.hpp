#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::uint8_t kDecimalGroupSeparatorGlyph = 0xFFU;

// ROM 0xC216: produce the source's least-significant-first decimal glyph
// stream, inserting its special separator glyph after every three digits.
std::vector<std::uint8_t> build_grouped_decimal_glyphs(
    std::uint32_t value);

} // namespace shadowrun::ghidra_conversion

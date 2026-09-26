#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::uint8_t kDecimalGroupSeparatorGlyph = 0xFFU;

// ROM 0xC216: produce the source's least-significant-first decimal glyph
// stream, inserting its special separator glyph after every three digits.
std::vector<std::uint8_t> build_grouped_decimal_glyphs(
    std::uint32_t value);

struct NumericMenuBlockCallbacks {
    std::function<void()> write_menu_grid;
    std::function<void()> build_decimal_glyphs;
};

// ROM 0xC406: preserve the source wrapper's grid-then-number preparation
// order without importing either presentation implementation.
void prepare_numeric_menu_block(
    const NumericMenuBlockCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

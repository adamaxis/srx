#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// 0xC216 addresses the source glyph table in words.  Decimal digits occupy
// table indices 0x10..0x19 (source byte offsets 0x20..0x32), while the group
// separator is the table entry at byte offset 0x18.
constexpr std::uint16_t kDecimalDigitGlyphTableBaseIndex = 0x10U;
constexpr std::uint16_t kDecimalGroupSeparatorGlyphTableIndex = 0x0CU;

// ROM 0xC216: produce the source's least-significant-first glyph-table index
// stream, inserting the separator table entry after every three digits.  The
// source decrements A2 by one word after each write; vector order is the order
// of those source writes, so a caller can apply its own destination layout.
std::vector<std::uint16_t> build_grouped_decimal_glyphs(
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

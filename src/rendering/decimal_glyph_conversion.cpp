#include "shadowrun_ghidra/rendering/decimal_glyph_conversion.hpp"

#include <string>

namespace shadowrun::ghidra_conversion {

std::string NumericVisual::bitmap_font_text() const {
    return std::string(glyphs.begin(), glyphs.end());
}

std::size_t NumericVisual::pixel_width(
    const std::size_t glyph_width) const noexcept {
    return glyphs.size() * glyph_width;
}

NumericVisual build_numeric_visual(const std::uint32_t value,
                                   const bool group_thousands) {
    auto digits = std::to_string(value);
    if (group_thousands) {
        for (auto position = digits.size(); position > 3U;
             position -= 3U) {
            digits.insert(position - 3U, ",");
        }
    }
    NumericVisual visual{};
    visual.glyphs.assign(digits.begin(), digits.end());
    return visual;
}

void prepare_numeric_menu_block(
    const NumericMenuBlockCallbacks& callbacks) {
    if (callbacks.write_menu_grid) {
        callbacks.write_menu_grid();
    }
    if (callbacks.build_decimal_glyphs) {
        callbacks.build_decimal_glyphs();
    }
}

std::vector<std::uint16_t> build_grouped_decimal_glyphs(
    std::uint32_t value) {
    std::vector<std::uint16_t> glyphs;
    const auto visual = build_numeric_visual(value, true);
    glyphs.reserve(visual.glyphs.size());
    for (auto glyph = visual.glyphs.rbegin();
         glyph != visual.glyphs.rend(); ++glyph) {
        glyphs.push_back(*glyph == ','
                             ? kDecimalGroupSeparatorGlyphTableIndex
                             : static_cast<std::uint16_t>(
                                   kDecimalDigitGlyphTableBaseIndex +
                                   static_cast<std::uint16_t>(*glyph - '0')));
    }
    return glyphs;
}

} // namespace shadowrun::ghidra_conversion

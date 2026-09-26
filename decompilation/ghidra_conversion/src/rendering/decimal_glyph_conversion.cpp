#include "shadowrun_ghidra/rendering/decimal_glyph_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void prepare_numeric_menu_block(
    const NumericMenuBlockCallbacks& callbacks) {
    if (callbacks.write_menu_grid) {
        callbacks.write_menu_grid();
    }
    if (callbacks.build_decimal_glyphs) {
        callbacks.build_decimal_glyphs();
    }
}

std::vector<std::uint8_t> build_grouped_decimal_glyphs(
    std::uint32_t value) {
    std::vector<std::uint8_t> glyphs;
    std::uint8_t digits_since_separator = 0U;
    do {
        if (digits_since_separator == 3U) {
            glyphs.push_back(kDecimalGroupSeparatorGlyph);
            digits_since_separator = 0U;
            continue;
        }

        glyphs.push_back(static_cast<std::uint8_t>(value % 10U));
        value /= 10U;
        ++digits_since_separator;
    } while (value != 0U);
    return glyphs;
}

} // namespace shadowrun::ghidra_conversion

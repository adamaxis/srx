#include "shadowrun_ghidra/rendering/decimal_glyph_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    prepare_numeric_menu_block(
        {[&order] { order.push_back(1); },
         [&order] { order.push_back(2); }});
    if (order != std::vector<int>{1, 2}) {
        return 1;
    }

    if (build_grouped_decimal_glyphs(0U) !=
        std::vector<std::uint16_t>{0x10U}) {
        return 2;
    }

    if (build_grouped_decimal_glyphs(1234U) !=
        std::vector<std::uint16_t>{0x14U, 0x13U, 0x12U,
                                   kDecimalGroupSeparatorGlyphTableIndex,
                                   0x11U}) {
        return 3;
    }

    if (build_grouped_decimal_glyphs(1234567U) !=
        std::vector<std::uint16_t>{0x17U, 0x16U, 0x15U,
                                   kDecimalGroupSeparatorGlyphTableIndex,
                                   0x14U, 0x13U, 0x12U,
                                   kDecimalGroupSeparatorGlyphTableIndex,
                                   0x11U}) {
        return 4;
    }

    return 0;
}

#include "shadowrun_ghidra/decimal_glyph_conversion.hpp"

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

    if (build_grouped_decimal_glyphs(0U) != std::vector<std::uint8_t>{0U}) {
        return 2;
    }

    if (build_grouped_decimal_glyphs(1234U) !=
        std::vector<std::uint8_t>{4U, 3U, 2U,
                                  kDecimalGroupSeparatorGlyph, 1U}) {
        return 3;
    }

    if (build_grouped_decimal_glyphs(1234567U) !=
        std::vector<std::uint8_t>{7U, 6U, 5U,
                                  kDecimalGroupSeparatorGlyph, 4U, 3U, 2U,
                                  kDecimalGroupSeparatorGlyph, 1U}) {
        return 4;
    }

    return 0;
}

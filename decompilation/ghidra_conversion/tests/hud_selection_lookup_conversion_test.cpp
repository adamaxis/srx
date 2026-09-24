#include "shadowrun_ghidra/hud_selection_lookup_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::uint8_t, 5> entries{0x10U, 0x22U, 0x34U, 0x46U,
                                               0x58U};
    if (find_hud_selection_index(entries, 0x10U) != 0U ||
        find_hud_selection_index(entries, 0x46U) != 3U ||
        find_hud_selection_index(entries, 0x58U) != 4U) {
        return 1;
    }
    if (find_hud_selection_index(entries, 0x99U).has_value()) {
        return 2;
    }

    return 0;
}

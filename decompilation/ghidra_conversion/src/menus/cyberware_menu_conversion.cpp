#include "shadowrun_ghidra/menus/cyberware_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {

CyberwareMenuModel build_cyberware_menu(const std::uint16_t cyberware_flags) {
    CyberwareMenuModel result{};
    if (cyberware_flags == 0U) {
        result.shows_no_cyberware_message = true;
        return result;
    }

    for (std::uint8_t bit = 0; bit < 15U; ++bit) {
        if ((cyberware_flags & (std::uint16_t{1U} << bit)) != 0U) {
            result.installed_row_indices.push_back(bit);
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion

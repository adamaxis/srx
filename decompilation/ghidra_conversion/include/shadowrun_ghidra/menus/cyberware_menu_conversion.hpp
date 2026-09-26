#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct CyberwareMenuModel {
    // ROM 0x11950 takes the no-cyberware text branch only when the complete
    // source bitfield is zero. A nonzero bit 15 is therefore distinguishable
    // from the empty branch even though bit 15 has no visible row.
    bool shows_no_cyberware_message = false;
    // Source bits 0..14 are visited in ascending order for the visible rows.
    std::vector<std::uint8_t> installed_row_indices;
};

// ROM 0x11950: construct the Cyberware screen's row order from record +0x90.
// Text resources and the common input/menu-row service remain presentation
// boundaries; this function owns only the source-derived menu model.
CyberwareMenuModel build_cyberware_menu(std::uint16_t cyberware_flags);

} // namespace shadowrun::ghidra_conversion

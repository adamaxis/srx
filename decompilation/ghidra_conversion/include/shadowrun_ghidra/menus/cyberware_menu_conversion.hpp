#pragma once

#include <array>
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

struct CyberwareTextPlan {
    std::uint32_t source = 0;
    std::uint32_t destination = 0;
    std::uint16_t palette = 0x6000;
};

constexpr std::uint32_t kCyberwareTitleSource = 0x000D2054U;
constexpr std::uint32_t kCyberwareTitleDestination = 0x0000C1A2U;
constexpr std::uint32_t kCyberwareEmptySource = 0x000D206BU;
constexpr std::uint32_t kCyberwareEmptyDestination = 0x0000C328U;
constexpr std::uint32_t kCyberwareEntryDestinationBase = 0x0000C31CU;
constexpr std::uint16_t kCyberwareEntryDestinationStride = 0x0100U;
constexpr std::array<std::uint32_t, 15> kCyberwareEntrySources{{
    0x000D20AEU, 0x000D20BBU, 0x000D20C9U, 0x000D20D8U, 0x000D20E4U,
    0x000D20F2U, 0x000D2104U, 0x000D2116U, 0x000D2128U, 0x000D213AU,
    0x000D214CU, 0x000D215EU, 0x000D2170U, 0x000D2182U, 0x000D2194U,
}};

// ROM 0x11950: construct the Cyberware screen's row order from record +0x90.
// Text resources and the common input/menu-row service remain presentation
// boundaries; this function owns only the source-derived menu model.
CyberwareMenuModel build_cyberware_menu(std::uint16_t cyberware_flags);

// ROM 0x11950 0xC0DA writes: title at C1A2, then NONE at C328 when flags==0,
// else one entry per bits 0..14 at C31C + n*0x100.
std::vector<CyberwareTextPlan> build_cyberware_text_plans(
    std::uint16_t cyberware_flags);

} // namespace shadowrun::ghidra_conversion

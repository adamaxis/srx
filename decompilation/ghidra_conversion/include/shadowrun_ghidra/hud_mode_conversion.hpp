#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct HudModeRecordFields {
    // The source record stride is 0x14 bytes. These are the two fields read
    // by 0x19B40/0x19BAE; the remaining bytes stay outside this view.
    std::uint8_t eligibility = 0;
    std::int8_t mode_code = -1;
};

struct HudModeSelection {
    // 0x45 is the source sentinel when the selected record has no usable
    // mode. Native callers can use `available` instead of interpreting it.
    std::uint8_t value = 0x45;
    bool available = false;
    std::uint8_t table_index = 0;
};

// ROM 0x19BAE: select the mode-table byte for record +0x13. In alternate
// mode, codes 7..16 use the interleaved second table entry.
HudModeSelection select_hud_mode(
    const HudModeRecordFields& record, bool alternate_mode,
    const std::array<std::uint8_t, 64>& mode_table);

// ROM 0x19B40's confirmed record gate: +0x0F must be positive and +0x13
// must be nonnegative before the mode-table lookup is usable.
bool hud_mode_record_is_eligible(const HudModeRecordFields& record);

} // namespace shadowrun::ghidra_conversion

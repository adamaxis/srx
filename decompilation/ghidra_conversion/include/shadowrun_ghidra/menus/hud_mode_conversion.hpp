#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

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

struct HudModeRunCallbacks {
    // Source 0x17C3C reads the indexed FFFB30 eligibility/mode byte and
    // returns its sign-extended, four-bit logical shift result when FFDF84
    // is not negative. The caller supplies that typed lookup boundary.
    std::function<int()> resolve_active_run_mode;
};

struct HudModeAuxiliaryRecord {
    // Source record offsets +0x13 and +0x11 used by 0x19BF6.
    std::int8_t mode_code = -1;
    std::uint8_t auxiliary_value = 0;
};

struct HudModeAuxiliarySelection {
    bool valid = false;
    std::uint32_t preserved_value = 0;
    std::uint16_t adjusted_auxiliary_value = 0;
    std::size_t source_index = 0;
};

// ROM 0x19BF6: preserve the incoming D0 value while selecting the +0x11
// auxiliary field from the current/next record and applying FFF0A0's
// signed variant adjustment.
HudModeAuxiliarySelection select_hud_mode_auxiliary_value(
    std::uint32_t preserved_value,
    const std::vector<HudModeAuxiliaryRecord>& records,
    std::size_t record_index,
    std::int8_t mode_variant);

// ROM 0x19B40: select the mode-table value for the current run record. A
// negative FFDF84 byte uses the record's +0x13 mode directly; otherwise the
// source resolver must return a positive replacement mode or the selection is
// rejected.
HudModeSelection select_run_hud_mode(
    const HudModeRecordFields& record,
    bool source_gate_is_negative,
    bool alternate_mode,
    const std::array<std::uint8_t, 64>& mode_table,
    const HudModeRunCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

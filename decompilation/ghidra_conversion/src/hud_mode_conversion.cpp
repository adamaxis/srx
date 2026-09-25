#include "shadowrun_ghidra/hud_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool hud_mode_record_is_eligible(const HudModeRecordFields& record) {
    return record.eligibility > 0U && record.mode_code >= 0;
}

HudModeSelection select_hud_mode(
    const HudModeRecordFields& record, const bool alternate_mode,
    const std::array<std::uint8_t, 64>& mode_table) {
    if (!hud_mode_record_is_eligible(record)) {
        return {};
    }

    auto table_index = static_cast<std::uint8_t>(record.mode_code * 2);
    if (alternate_mode && record.mode_code >= 7 && record.mode_code < 17) {
        ++table_index;
    }
    return HudModeSelection{mode_table[table_index], true, table_index};
}

HudModeSelection select_run_hud_mode(
    const HudModeRecordFields& record,
    const bool source_gate_is_negative,
    const bool alternate_mode,
    const std::array<std::uint8_t, 64>& mode_table,
    const HudModeRunCallbacks& callbacks) {
    if (!hud_mode_record_is_eligible(record)) {
        return {};
    }

    auto mode_code = static_cast<int>(record.mode_code);
    if (!source_gate_is_negative) {
        if (!callbacks.resolve_active_run_mode) {
            return {};
        }
        mode_code = callbacks.resolve_active_run_mode();
        if (mode_code <= 0 || mode_code > 127) {
            return {};
        }
    }

    return select_hud_mode(
        {1U, static_cast<std::int8_t>(mode_code)}, alternate_mode,
        mode_table);
}

} // namespace shadowrun::ghidra_conversion

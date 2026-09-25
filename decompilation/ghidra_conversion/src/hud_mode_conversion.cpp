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

} // namespace shadowrun::ghidra_conversion

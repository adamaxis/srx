#include "shadowrun_ghidra/hud_mode_conversion.hpp"

#include <cstddef>

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

HudModeAuxiliarySelection select_hud_mode_auxiliary_value(
    const std::uint32_t preserved_value,
    const std::vector<HudModeAuxiliaryRecord>& records,
    const std::size_t record_index,
    const std::int8_t mode_variant) {
    if (record_index >= records.size()) {
        return {false, preserved_value, 0, record_index};
    }

    const auto& current = records[record_index];
    auto selected_index = record_index;
    if (mode_variant > 0 && current.mode_code >= 7 &&
        current.mode_code < 17) {
        ++selected_index;
        if (selected_index >= records.size()) {
            return {false, preserved_value, 0, selected_index};
        }
    }

    auto adjusted = static_cast<std::uint16_t>(
        records[selected_index].auxiliary_value);
    if (mode_variant == 0) {
        adjusted = static_cast<std::uint16_t>(adjusted + 1U);
    } else if (mode_variant > 0) {
        adjusted = static_cast<std::uint16_t>(adjusted + 2U);
    }
    return {true, preserved_value, adjusted, selected_index};
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<FieldActionDetailTextSelection>
select_field_action_detail_text(const std::uint8_t action_code) {
    std::size_t table_index = 0;
    if (action_code < 0x19U) {
        table_index = action_code;
    }
    else if (action_code < 0x25U) {
        table_index = 0x18U;
    }
    else {
        const auto adjusted = static_cast<int>(action_code) - 0x0DU;
        if (adjusted < 0x1BU) {
            table_index = static_cast<std::size_t>(adjusted);
        }
        else if (adjusted < 0x1EU) {
            // The source returns from 0x587B8 without rendering for codes
            // 0x28..0x2A.
            return std::nullopt;
        }
        else {
            table_index = static_cast<std::size_t>(adjusted - 3);
        }
    }

    if (table_index >= kFieldActionDetailTextEntryCount) {
        return std::nullopt;
    }
    return FieldActionDetailTextSelection{table_index};
}

} // namespace shadowrun::ghidra_conversion

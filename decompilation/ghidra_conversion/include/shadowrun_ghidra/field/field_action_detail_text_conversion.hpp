#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kFieldActionDetailTextEntryCount = 32U;

struct FieldActionDetailTextSelection {
    std::size_t table_index = 0;
};

// ROM 0x5878E. Convert the source action code into an index in the
// 0x1AC7FC detail-text pointer table. A missing value represents the source's
// deliberate no-text range (codes 0x28..0x2A) or an unavailable extracted
// table entry; callers do not receive a fabricated fallback.
std::optional<FieldActionDetailTextSelection>
select_field_action_detail_text(std::uint8_t action_code);

} // namespace shadowrun::ghidra_conversion

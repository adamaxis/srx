#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x164F2: the source scans ten entity records and sets the carry flag
// only when every leading occupancy word is nonzero. A native caller needs
// the semantic result rather than a condition-code side effect.
bool entity_table_has_free_slot(
    const std::array<std::uint16_t, 10>& occupancy_words);

} // namespace shadowrun::ghidra_conversion

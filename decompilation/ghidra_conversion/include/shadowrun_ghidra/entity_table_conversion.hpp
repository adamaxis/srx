#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x164F2: the source scans ten entity records and sets the carry flag
// only when every leading occupancy word is nonzero. A native caller needs
// the semantic result rather than a condition-code side effect.
bool entity_table_has_free_slot(
    const std::array<std::uint16_t, 10>& occupancy_words);

struct EntityReadinessState {
    bool active = false;
    bool ready = false;
};

// ROM 0x2FF0: find the first of thirteen entity records whose active bit and
// readiness bit are both set. The source stops when none exists; the host
// reports that condition as an empty optional.
std::optional<std::size_t> find_ready_entity_slot(
    const std::array<EntityReadinessState, 13>& entities);

} // namespace shadowrun::ghidra_conversion

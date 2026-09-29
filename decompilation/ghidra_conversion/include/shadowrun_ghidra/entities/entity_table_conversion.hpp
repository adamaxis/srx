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

constexpr std::size_t kEntityServiceRecordCount = 10;

struct EntityServiceRecord {
    std::uint32_t pair_a2 = 0; // +0xA2
    std::uint32_t pair_ac = 0; // +0xAC
    std::uint8_t byte_43 = 0; // +0x43
    std::uint8_t route_flags = 0; // +0xB0
    std::uint8_t animation_flags = 0; // +0x11
};

// ROM 0x1DBF0: CLR FFDF84, then ten records from FF0100 stride 0x100.
// CLR.L +0xA2/+0xAC, CLR.B +0x43, ANDI.B #0xBF +0xB0, ANDI.B #0xEF +0x11.
void reset_entity_service_records(
    std::int16_t& timeout_marker,
    std::array<EntityServiceRecord, kEntityServiceRecordCount>& records);

} // namespace shadowrun::ghidra_conversion

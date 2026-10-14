#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kEntityActionProfileCount = 5U;
constexpr std::size_t kEntityActionProfileRowSize = 0x22U;
constexpr std::size_t kEntityActionProfileEventSlotCount = 16U;

struct EntityActionProfileEvent {
    std::uint8_t code = 0;
    std::uint8_t value = 0;
};

struct EntityActionProfileModifier {
    std::uint8_t roll_count = 0;
    std::uint8_t difficulty = 0;
};

// One row copied by ROM 0x566A2. The final two event slots are also the
// mutable modifier bytes consumed by ROM 0x55786 (+0x1E..+0x21). Keeping the
// complete 0x22-byte shape makes that alias explicit without carrying a ROM
// pointer or a Genesis work-RAM address into the host conversion.
struct EntityActionProfileRow {
    std::uint16_t event_count_last_index = 0; // source row +0x00
    std::array<EntityActionProfileEvent, kEntityActionProfileEventSlotCount>
        events{}; // source row +0x02..+0x21

    EntityActionProfileModifier primary_modifier() const;
    EntityActionProfileModifier secondary_modifier() const;
    void set_primary_modifier(const EntityActionProfileModifier& modifier);
    void set_secondary_modifier(const EntityActionProfileModifier& modifier);
};

struct EntityActionProfileTable {
    std::array<EntityActionProfileRow, kEntityActionProfileCount> rows{};
};

// ROM 0x566A2: copy five 0x22-byte rows from the source table at 0x166AD8
// into the working table at FFF9FA. The host caller supplies the extracted
// bytes; this routine only decodes them into typed rows.
bool copy_action_selection_profile_table(
    const std::vector<std::uint8_t>& source,
    EntityActionProfileTable& destination);

// ROM 0x55828: follow the linked record's +0xA0 profile index and select its
// row from the copied table. An out-of-range index is an unresolved source
// boundary, not permission to invent a fallback profile.
const EntityActionProfileRow* select_entity_action_profile_row(
    const EntityActionProfileTable& table, std::uint16_t profile_index);

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/entities/entity_action_profile_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t read_big_endian_word(
    const std::vector<std::uint8_t>& source,
    const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(source[offset]) << 8U) |
        static_cast<std::uint16_t>(source[offset + 1U]));
}

} // namespace

EntityActionProfileModifier EntityActionProfileRow::primary_modifier() const {
    return {events[14].code, events[14].value};
}

EntityActionProfileModifier EntityActionProfileRow::secondary_modifier() const {
    return {events[15].code, events[15].value};
}

void EntityActionProfileRow::set_primary_modifier(
    const EntityActionProfileModifier& modifier) {
    events[14] = {modifier.roll_count, modifier.difficulty};
}

void EntityActionProfileRow::set_secondary_modifier(
    const EntityActionProfileModifier& modifier) {
    events[15] = {modifier.roll_count, modifier.difficulty};
}

bool copy_action_selection_profile_table(
    const std::vector<std::uint8_t>& source,
    EntityActionProfileTable& destination) {
    constexpr auto source_size =
        kEntityActionProfileCount * kEntityActionProfileRowSize;
    if (source.size() < source_size) {
        return false;
    }

    for (std::size_t row_index = 0; row_index < kEntityActionProfileCount;
         ++row_index) {
        const auto source_offset = row_index * kEntityActionProfileRowSize;
        auto& row = destination.rows[row_index];
        row.event_count_last_index = read_big_endian_word(source, source_offset);
        for (std::size_t event_index = 0;
             event_index < kEntityActionProfileEventSlotCount; ++event_index) {
            const auto event_offset = source_offset + 2U + event_index * 2U;
            row.events[event_index] = {source[event_offset],
                                       source[event_offset + 1U]};
        }
    }
    return true;
}

const EntityActionProfileRow* select_entity_action_profile_row(
    const EntityActionProfileTable& table,
    const std::uint16_t profile_index) {
    if (profile_index >= table.rows.size()) {
        return nullptr;
    }
    return &table.rows[profile_index];
}

} // namespace shadowrun::ghidra_conversion

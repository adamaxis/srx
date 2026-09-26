#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationState {
    std::uint16_t frame = 0;
    std::int16_t accumulator = 0;
    std::int16_t increment = 0;
    std::int16_t period = 0;
    bool redraw_requested = false;
};

enum class EntityAnimationTableBank {
    Standard,
    Alternate,
};

struct EntityAnimationTableSelection {
    EntityAnimationTableBank bank = EntityAnimationTableBank::Standard;
    std::uint16_t row_byte_offset = 0;
};

// ROM 0x5FDC: add one animation increment, advance the frame once when the
// period is reached, and request a redraw for that frame transition.
void advance_entity_animation_accumulator(EntityAnimationState& state);

// ROM 0x14D6A: select the standard or alternate 0x30-byte animation table
// from the signed entity variant and convert the one-based animation id into
// the source row byte offset.
EntityAnimationTableSelection select_entity_animation_table_row(
    std::int8_t entity_variant,
    std::uint8_t animation_id);

} // namespace shadowrun::ghidra_conversion

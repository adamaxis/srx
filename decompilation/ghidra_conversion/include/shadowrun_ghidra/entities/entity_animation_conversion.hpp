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

enum class EntityMotionActionTable {
    EquipmentItem,
    MagicSpell,
};

struct EntityMotionActionRowSelection {
    // 0x14D6A selects D4540 for ordinary equipment actions and D4EA0 for
    // the signed alternate/magic action state before 0x13D0C reads +0x27.
    EntityMotionActionTable table = EntityMotionActionTable::EquipmentItem;
    std::uint8_t one_based_action_code = 0;
    std::uint16_t row_byte_offset = 0;
};

// ROM 0x5FDC: add one animation increment, advance the frame once when the
// period is reached, and request a redraw for that frame transition.
void advance_entity_animation_accumulator(EntityAnimationState& state);

// ROM 0x14D6A: select the active equipment-item or magic-spell 0x30-byte
// action row for the entity-motion distance profile. The source chooses the
// table from +0x56's sign and converts +0x57's one-based code into the row
// byte offset; 0x13D0C then consumes that row's +0x27 byte as its threshold
// profile index.
EntityMotionActionRowSelection select_entity_motion_action_row(
    std::int8_t active_action_state,
    std::uint8_t one_based_action_code);

} // namespace shadowrun::ghidra_conversion

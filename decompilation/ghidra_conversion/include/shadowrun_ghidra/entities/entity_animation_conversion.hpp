#pragma once

#include <array>
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

// ROM 0xC052 idle and 0xC072 walk. Direction is the 0-7 heading slot.
constexpr std::array<std::uint16_t, 8> kFieldIdleAnimationSelectors{{
    0x0000U, 0x0004U, 0x0008U, 0x000CU, 0x0010U, 0x0014U, 0x0018U, 0x001CU}};
constexpr std::array<std::uint16_t, 8> kFieldWalkAnimationSelectors{{
    0x0060U, 0x0064U, 0x0068U, 0x006CU, 0x0070U, 0x0074U, 0x0078U, 0x007CU}};
// ROM 0x600E copies the walking list's auxiliary word (0x0300 at the 0x8002
// lists) into record +0x18. Template +0x1C is 0x1000.
constexpr std::int16_t kFieldWalkAnimationIncrement = 0x0300;
constexpr std::int16_t kFieldEntityAnimationPeriod = 0x1000;
constexpr std::uint16_t kFieldWalkAnimationFrameCount = 2U;

std::uint16_t select_field_entity_animation_selector(std::uint8_t direction,
                                                     bool walking);

// ROM 0x600E negative-list bound: after the last pose, the frame stays on
// count-1 until 0xAF6C clears it.
std::uint16_t bound_field_walk_animation_frame(std::uint16_t frame);

// ROM 0x14D6A: select the active equipment-item or magic-spell 0x30-byte
// action row for the entity-motion distance profile. The source chooses the
// table from +0x56's sign and converts +0x57's one-based code into the row
// byte offset; 0x13D0C then consumes that row's +0x27 byte as its threshold
// profile index.
EntityMotionActionRowSelection select_entity_motion_action_row(
    std::int8_t active_action_state,
    std::uint8_t one_based_action_code);

} // namespace shadowrun::ghidra_conversion

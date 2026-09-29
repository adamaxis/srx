#pragma once

#include <array>
#include <cstdint>
#include <vector>

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

// ROM 0x62AC: kind word +0x0E indexes this long table. Kind 3 is 0x6378.
constexpr std::array<std::uint32_t, 16> kEntityKindStateTables{{
    0x0000630CU, 0x0000634CU, 0x00006364U, 0x00006378U, 0x00006FD8U,
    0x00006480U, 0x00006588U, 0x00006690U, 0x00006ED0U, 0x00006798U,
    0x000068A0U, 0x000069A8U, 0x00006AB0U, 0x00006BB8U, 0x00006CC0U,
    0x00006DC8U}};
constexpr std::uint16_t kFieldEntityKind = 0x0003U;

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
std::uint32_t entity_kind_state_table(std::uint16_t kind);

// ROM 0x600E negative-list bound: after the last pose, the frame stays on
// count-1 until 0xAF6C clears it.
std::uint16_t bound_field_walk_animation_frame(std::uint16_t frame);

struct EntitySpriteDescriptorList {
    std::int16_t marker = 0;
    std::uint16_t auxiliary = 0;
    std::vector<std::uint32_t> frame_pointers;
};

struct EntitySpriteDescriptorState {
    std::uint16_t kind = 0; // +0x0E
    std::uint16_t selector = 0; // +0x16
    std::uint8_t flags = 0; // +0x11
    std::uint16_t frame = 0; // +0x1E
    std::uint16_t increment = 0; // +0x18
    std::uint16_t timing_word = 0; // +0xCA
    std::uint32_t descriptor = 0; // +0x12
};

// ROM 0x600E: resolve the kind/+0x16 list, copy cadence into +0x18, select
// the frame pointer, and run 0x60FA when that descriptor is 0x8002 and +0x11
// bit 0 is set. 0x60F4 always sets carry, so the FFDD3E restore is dead.
bool advance_entity_sprite_descriptor(
    EntitySpriteDescriptorState& state,
    const EntitySpriteDescriptorList& list,
    std::uint16_t descriptor_word);

// ROM 0x14D6A: select the active equipment-item or magic-spell 0x30-byte
// action row for the entity-motion distance profile. The source chooses the
// table from +0x56's sign and converts +0x57's one-based code into the row
// byte offset; 0x13D0C then consumes that row's +0x27 byte as its threshold
// profile index.
EntityMotionActionRowSelection select_entity_motion_action_row(
    std::int8_t active_action_state,
    std::uint8_t one_based_action_code);

} // namespace shadowrun::ghidra_conversion

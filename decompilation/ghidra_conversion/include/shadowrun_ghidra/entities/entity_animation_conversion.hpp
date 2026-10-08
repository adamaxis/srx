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

// The kind-3 state block at 0x6378 is a pointer table addressed by the
// record's +0x16 byte offset. Normal accepted movement selects 0x20..0x3C
// from 0xC062; 0xB442 resets to the static 0x00..0x1C poses from 0xC052.
// Other routines select these separate families:
//   0x00..0x1C idle (positive-marker static lists)
//   0x40..0x5C attack pose (0xC042 table, weapon-timed one-frame hold)
//   0x60..0x7C hurt (0xC072 table, 0x8002 two-frame lists)
//   0x82..0x9E unarmed delayed-action pose (0xC082 table, one frame)
//   0xA0..0xBC thrown/effect (0xC092 table, 0x8003 descriptors)
//   0xE8..0x104 behavior transition (0xC0B2 table, finite 0x8008 lists)
constexpr std::array<std::uint16_t, 8> kFieldIdleAnimationSelectors{{
    0x0000U, 0x0004U, 0x0008U, 0x000CU, 0x0010U, 0x0014U, 0x0018U, 0x001CU}};
constexpr std::array<std::uint16_t, 8> kFieldWalkAnimationSelectors{{
    0x0020U, 0x0024U, 0x0028U, 0x002CU,
    0x0030U, 0x0034U, 0x0038U, 0x003CU}};
constexpr std::array<std::uint16_t, 8> kFieldAttackAnimationSelectors{{
    0x0040U, 0x0044U, 0x0048U, 0x004CU,
    0x0050U, 0x0054U, 0x0058U, 0x005CU}};
// 0xBABC writes C006 (0x0028) to +0xCA. For kind 3, 0x600E multiplies
// that value by 0x10 into +0x18; +0x1C is the 0x1000 period.
constexpr std::int16_t kFieldWalkAnimationTimingWord = 0x0028;
constexpr std::int16_t kFieldWalkAnimationIncrement = 0x0280;
constexpr std::int16_t kFieldEntityAnimationPeriod = 0x1000;
constexpr std::uint16_t kFieldWalkAnimationFrameCount = 4U;

std::uint16_t select_field_entity_animation_selector(std::uint8_t direction,
                                                     bool walking);
// The kind-3 C052/C062 records for directions 5..7 use the same source art
// as directions 3..1 with descriptor attribute bit 0x0800 set. The native
// presenter applies that facing transform to the loaded sprite image.
bool field_entity_animation_flips_horizontally(std::uint8_t direction);
std::uint32_t entity_kind_state_table(std::uint16_t kind);

// The ordinary movement list has a positive marker and loops at its count.
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

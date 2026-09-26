#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// These are the fields consumed by the field entity action calculation. They
// are deliberately typed record values rather than a pointer into Genesis
// RAM. The comments retain the source offsets needed to audit each value.
struct EntityActionRecord {
    std::uint16_t cyberware_flags = 0;  // +0x90
    std::uint8_t physical_meter = 0;    // +0x4C
    std::uint8_t posture = 0;            // +0x4D
    std::uint8_t item18_base = 0;        // +0x80
    std::uint8_t mental_meter = 0;       // +0x9C
    std::uint8_t race_or_state = 0;      // +0x9A
    std::uint8_t attribute_7c = 0;       // +0x7C
    std::uint8_t attribute_77 = 0;       // +0x77
    std::uint8_t flags_10 = 0;           // +0x10
    std::uint8_t class_code = 0;         // +0x9B
    std::array<std::uint8_t, 8> item_codes{};   // +0x66..+0x6D
    std::array<std::uint8_t, 8> item_phases{};  // +0x6E..+0x75
};

struct EntityActionTarget {
    std::uint8_t class_code = 0;  // +0x9B
    std::uint8_t flags = 0;       // +0x11
};

// The action record is a decoded row from the source 0xD4EA0 table. Its
// effect value has already been read through the source 0x166AD4 lookup.
// Keeping the row as data makes the native conversion independent of ROM
// addresses while preserving the source table's meaning.
struct EntityActionCatalogRecord {
    std::uint8_t difficulty_modifier = 0;  // +0x26
    std::int16_t base_effect = 0;           // +0x24 -> 0x166AD4, negated
};

struct EntityActionSpecialModifier {
    std::uint8_t roll_count = 0;
    std::uint8_t difficulty = 0;
};

// Typed inputs for ROM 0x55786. The two special records are the source
// +0x1E/+0x1F and +0x20/+0x21 pairs selected by actor +0xDF bits 0 and 1.
// `attribute_selector` is the signed +0x27 byte from the corresponding
// D4EA0 action row: negative selects +0x77, otherwise +0x7C.
struct EntityActionDynamicModifierContext {
    std::uint8_t actor_flags_df = 0;
    std::uint16_t actor_record_type = 0;
    std::uint8_t actor_attribute_77 = 0;
    std::uint8_t actor_attribute_7c = 0;
    std::array<EntityActionRecord, 3> ready_entities{};
    std::array<EntityActionSpecialModifier, 2> special_modifiers{};
    std::array<std::int8_t, 2> attribute_selectors{};
};

using EntityActionRollService =
    std::function<std::uint8_t(std::uint16_t roll_count,
                               std::uint16_t difficulty)>;

struct EntityActionDynamicModifierResult {
    bool selected = false;
    bool roll_service_available = false;
    std::uint8_t selected_action_table_index = 0;
    std::uint16_t ready_roll_count = 0;
    std::uint8_t special_successes = 0;
    std::uint16_t added_difficulty = 0;
};

// ROM 0x55F02. Start with +0x80 and, scanning possession slots from 6 down
// to 0, add the phase of the first slot whose item code is 0x18.
std::uint16_t calculate_item18_skill_value(const EntityActionRecord& entity);

// ROM 0x55F20. Apply posture's 25-percent-per-step reduction to the input.
// `select_reduced_component` is the source D7 sign: false selects the
// remaining value (D1-D2), true selects the reduction (D2).
std::uint16_t select_posture_adjusted_value(
    std::uint16_t input,
    std::uint8_t posture,
    bool select_reduced_component);

// ROM 0x104F6. Scan possession slots from 6 down to 0 for an exact item code
// and add that slot's phase to the running value. The first match terminates
// the scan; duplicate inventory cells are not summed.
std::uint16_t add_first_matching_item_phase(
    std::uint16_t value,
    const EntityActionRecord& entity,
    std::uint8_t item_code);

// ROM 0x1047E. For each of the three party records, active class codes 0 and
// 1 contribute floor(item18_value / 4), plus one only when the remainder is
// at least three. This is the source's ready-entity bonus, not a generic
// average of the party.
std::uint16_t calculate_ready_entity_action_bonus(
    const std::array<EntityActionRecord, 3>& ready_entities);

// ROM 0x0D5A. Add the physical/mental wound penalty to a roll threshold.
// Meter values 0..4 add two, 5..7 add one, and 8..10 add nothing.
std::uint16_t add_wound_difficulty_penalty(
    std::uint16_t difficulty,
    std::uint8_t physical_meter,
    std::uint8_t mental_meter);

// ROM 0x55740. Apply the source cyberware, target-class, race/state, and
// target-flag branches to the running threshold. This is a pure calculation;
// the caller owns any later RNG or animation/effect service.
std::uint16_t apply_target_condition_modifier(
    std::uint16_t difficulty,
    const EntityActionRecord& attacker,
    const EntityActionTarget& target);

// ROM 0x55786. Bit 0 has priority over bit 1. The selected special record is
// rolled with the ready-entity/attribute count; successful rolls reduce its
// remaining bonus, and bit 1 halves that remainder before it is added to the
// caller's difficulty.
EntityActionDynamicModifierResult apply_dynamic_status_modifier(
    std::uint16_t& difficulty,
    const EntityActionDynamicModifierContext& context,
    const EntityActionRollService& roll_service);

enum class EntityActionPhaseConsumptionResult {
    no_request,
    consumed_partially,
    consumed_exactly,
    inventory_exhausted,
};

struct EntityActionPhaseConsumption {
    EntityActionPhaseConsumptionResult result =
        EntityActionPhaseConsumptionResult::no_request;
    std::uint16_t requested = 0;
    std::uint16_t remaining = 0;
    std::uint8_t cleared_slots = 0;
};

// ROM 0x1033E's 0x1042A..0x10460 tail. Consume a positive residual from
// item-code 0x13 phases in slots 6 down to 0. A partially consumed phase is
// retained; a phase reached at zero is cleared with its item code.
EntityActionPhaseConsumption consume_item13_residual(
    EntityActionRecord& entity,
    std::uint16_t residual);

struct EntityActionResidual {
    bool valid = false;
    std::uint16_t effective_difficulty = 0;
    std::uint16_t adjusted_roll_count = 0;
    std::int16_t residual_effect = 0;
};

// ROM 0x10660. Convert the action roll count and threshold into the residual
// effect value used by the caller. The source halves/rounds the incoming
// threshold, adds the selected 0xD4EA0 row modifier, clamps at two, scales
// the roll count by the resulting 20-percent steps, rounds at a 50 remainder,
// halves it, and subtracts it from the decoded base effect.
EntityActionResidual calculate_action_residual(
    std::uint8_t action_code,
    std::uint16_t difficulty,
    std::uint16_t roll_count,
    const EntityActionCatalogRecord& action);

}  // namespace shadowrun::ghidra_conversion

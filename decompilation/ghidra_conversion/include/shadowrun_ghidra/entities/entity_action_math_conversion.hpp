#pragma once

#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_profile_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// These are the fields consumed by the field entity action calculation. They
// are deliberately typed record values rather than a pointer into Genesis
// RAM. The comments retain the source offsets needed to audit each value.
struct EntityActionRecord {
    std::uint16_t cyberware_flags = 0;  // +0x90
    std::uint8_t physical_meter = 0;    // +0x4C
    std::uint8_t posture = 0;            // +0x4D
    std::uint8_t item18_base = 0;        // +0x80
    std::uint8_t attribute_78 = 0;        // +0x78
    std::uint8_t attribute_7b = 0;        // +0x7B
    std::uint8_t mental_meter = 0;       // +0x9C
    std::uint8_t race_or_state = 0;      // +0x9A
    std::uint8_t attribute_7c = 0;       // +0x7C
    std::uint8_t attribute_77 = 0;       // +0x77
    std::uint8_t attribute_87 = 0;       // +0x87
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

// ROM 0x104F6's data-dependent tail. C8AE0 supplies a signed selector for
// the action index; the selected word is then read from the slot-7 item's
// D4540 row at +0x22, +0x24, or +0x26. The table is passed as decoded data,
// not accessed through a ROM address at runtime.
struct EntityActionSpecialItemModifierTable {
    std::array<std::int8_t, 0x18> action_offsets{};
    std::array<std::array<std::int16_t, 3>, 3> slot7_item_modifiers{};
};

struct EntityActionActorState {
    EntityActionRecord record{};
    std::uint16_t record_type = 0;       // +0x3E
    std::int16_t equipped_slot = -1;    // signed +0x56
    std::uint8_t equipped_action_code = 0;  // +0x57
    std::uint8_t first_item_code = 0;       // +0x66
    std::uint8_t roll_count_cap = 0;        // +0x7F
    std::uint8_t auxiliary_roll_bonus = 0;  // +0x9D
    std::uint8_t action_result = 0;         // +0x9F
    std::optional<EntityActionSpecialItemModifierTable>
        special_item_modifier_table;
};

// Typed inputs for ROM 0x55786. The two special records are the source
// +0x1E/+0x1F and +0x20/+0x21 pairs selected by actor +0xDF bits 0 and 1.
// ROM 0x55828 resolves the profile row from the linked record's +0xA0
// selector. `attribute_selector` is the signed +0x27 byte from the
// corresponding D4EA0 action row: negative selects +0x77, otherwise +0x7C.
struct EntityActionDynamicModifierContext {
    std::uint8_t actor_flags_df = 0;
    std::uint16_t actor_record_type = 0;
    std::uint8_t actor_attribute_77 = 0;
    std::uint8_t actor_attribute_7c = 0;
    std::array<EntityActionRecord, 3> ready_entities{};
    const EntityActionProfileTable* action_profile_table = nullptr;
    std::optional<std::uint16_t> linked_profile_index;
    std::array<std::int8_t, 2> attribute_selectors{};
};

using EntityActionRollService =
    std::function<std::uint8_t(std::uint16_t roll_count,
                               std::uint16_t difficulty)>;

using EntityActionBehaviorService =
    std::function<void(EntityActionActorState&, std::int16_t delta)>;

struct EntityActionDynamicModifierResult {
    bool selected = false;
    bool action_profile_available = false;
    bool roll_service_available = false;
    std::uint8_t selected_action_table_index = 0;
    std::uint16_t selected_profile_index = 0;
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

// ROM 0x55ED6. Add the source +0x78, +0x7B, and +0x7C command attributes
// with byte wrapping, shift the sum once, then add the phase of the first
// item-code-0x28 possession cell. This overload is the entity-record form
// used by the late effect/action routines; the menu module has a separate
// presentation-input overload with the same source behavior.
std::uint16_t calculate_equipment_command_average(
    const EntityActionRecord& entity);

// ROM 0x104F6. Scan possession slots from 6 down to 0 for an exact item code
// and add that slot's phase to the running value. The first match terminates
// the scan; duplicate inventory cells are not summed.
std::uint16_t add_first_matching_item_phase(
    std::uint16_t value,
    const EntityActionRecord& entity,
    std::uint8_t item_code);

struct EntityActionPhaseModifierResult {
    std::uint16_t value = 0;
    bool phase_found = false;
    bool special_item_applied = false;
};

// ROM 0x104F6 in full: apply the first matching phase, then apply the
// slot-7 0x29..0x2B modifier when the decoded C8AE0 selector is nonnegative.
// The source uses ADD.B for the phase and ADD.W for the special table word;
// nonpositive final words become one.
EntityActionPhaseModifierResult apply_action_item_phase_modifiers(
    std::uint16_t value,
    const EntityActionRecord& entity,
    std::uint8_t item_code,
    std::uint8_t action_index,
    const EntityActionSpecialItemModifierTable* table);

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

// This is the second 0x55F02/0x55F20 composition inside 0x10660: D7 is
// negative there, so it selects the remaining posture-adjusted component and
// then adds record +0x7C. It intentionally does not include +0x9D or an
// inventory phase; those belong only to the initial 0x1033E roll count.
std::uint16_t calculate_action_effect_roll_count(
    const EntityActionRecord& entity);

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

enum class EntityTargetActionOutcome {
    target_state_gate,
    roll_service_unavailable,
    roll_failed,
    residual_not_positive,
    residual_consumed_partially,
    residual_consumed_exactly,
    residual_inventory_exhausted,
    behavior_service_unavailable,
    behavior_meter_advanced,
    invalid_action_record,
};

struct EntityTargetActionServices {
    EntityActionRollService roll_service;
    std::function<void(EntityActionActorState&)> spawn_entity_effect;
    std::function<void(std::uint8_t successes,
                       std::uint8_t auxiliary_value)>
        publish_special_result;
    EntityActionBehaviorService advance_behavior_meter;
};

struct EntityTargetActionResolution {
    EntityTargetActionOutcome outcome =
        EntityTargetActionOutcome::roll_service_unavailable;
    std::uint16_t initial_roll_count = 0;
    std::uint16_t difficulty = 0;
    std::uint8_t successes = 0;
    std::int16_t residual_effect = 0;
    bool effect_service_called = false;
    bool special_result_published = false;
    EntityActionPhaseConsumption phase_consumption{};
};

// ROM 0x1033E as reached from the field entity target-heading/attack path.
// The old Ghidra label called this First Aid, but the source caller enters it
// when +0x56 has no equipped weapon. The host conversion keeps the roll,
// effect, special-result, phase, and behavior-meter services explicit.
EntityTargetActionResolution resolve_entity_target_action(
    EntityActionActorState& actor,
    const EntityActionTarget& target,
    const EntityActionCatalogRecord& action,
    const EntityActionDynamicModifierContext* dynamic_context,
    const EntityTargetActionServices& services);

struct EntityActionResidual {
    bool valid = false;
    std::uint16_t effective_difficulty = 0;
    std::uint16_t adjusted_roll_count = 0;
    std::int16_t residual_effect = 0;
};

// ROM 0x10660. Convert the action roll count and threshold into the residual
// effect value used by the caller. `action_row_index` is the source's
// zero-based D4EA0 row selector; zero is a valid first row. The source
// halves/rounds the incoming threshold, adds the selected row modifier,
// clamps at two, scales the roll count by the resulting 20-percent steps,
// rounds at a 50 remainder, halves it, and subtracts it from the decoded base
// effect.
EntityActionResidual calculate_action_residual(
    std::uint8_t action_row_index,
    std::uint16_t difficulty,
    std::uint16_t roll_count,
    const EntityActionCatalogRecord& action);

struct EffectTargetOffsetInput {
    // 0x55848 keeps A0 as the source entity while the A1 target record is
    // used by 0x55740. These are decoded records, never source RAM aliases.
    EntityActionRecord source{};
    EntityActionTarget target{};
    // When present, this is the linked profile/context consumed by 0x55786.
    // A missing context means the source bit-0/bit-1 dynamic branch is not
    // selected for this call.
    const EntityActionDynamicModifierContext* dynamic_modifier_context =
        nullptr;
};

struct EffectTargetOffsetServices {
    // 0x0D3A. The callback returns a value in [0, bound), matching the
    // source PRNG helper's bounded result.
    BoundedRandomRollBoundary sample_random_bound;
    // 0x55786's nested 0x0DBA roll service. The final 0x55848 roll count is
    // supplied through sample_random_bound, not this action-result service.
    EntityActionRollService dynamic_roll_service;
};

struct EffectTargetOffsetResult {
    std::uint16_t command_average = 0;
    std::uint16_t posture_reduction = 0;
    std::uint16_t combined_value = 0;
    std::uint8_t target_skill = 0;
    std::uint16_t initial_difficulty = 0;
    std::uint16_t effective_difficulty = 0;
    std::uint8_t thresholded_rolls = 0;
    std::uint16_t half_threshold = 0;
    std::int16_t first_difference = 0;
    std::int16_t magnitude = 0;
    std::int16_t offset = 0;
    bool random_service_available = false;
    bool dynamic_modifier_selected = false;
    bool dynamic_roll_service_available = false;
};

// ROM 0x55848. Build the source entity's command/posture target value, apply
// the target and dynamic difficulty branches, then choose the signed random
// effect-placement offset. The three nested random draws preserve the source
// sequence: a six-sided threshold sample, a three-way sign sample, and a
// three-way axis sample with a two-way tie break.
EffectTargetOffsetResult calculate_effect_target_offset(
    const EffectTargetOffsetInput& input,
    const EffectTargetOffsetServices& services);

}  // namespace shadowrun::ghidra_conversion

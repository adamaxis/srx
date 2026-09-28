#pragma once

#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// Typed replacements for the compact globals consumed by the 0x559BC tail.
// These are conversion-state values, not an emulated Genesis RAM block.
struct EntityActionResolutionState {
    std::int8_t armor_mode = 0;       // FFF112: -1 secondary, 0 primary, >0 none
    std::uint8_t alternate_meter = 0; // FFF0CC: mental/behavior branch selector
    std::uint8_t world_mode = 0;      // FFE186: mode-three meter gate
    bool story_counter_gate = false;  // FFFBEC
    std::uint16_t class_b_counter = 0; // FFFBF0
    std::uint8_t phase_counter = 0;    // FFF0CE
    std::array<std::uint8_t, 3> phase_roll_counters{}; // FF018D/FF028D/FF038D
    std::uint8_t class13_counter = 0; // FFFBFA
};

using EntityActionArmorModifier =
    std::function<std::int16_t(std::int16_t value,
                               const EntityActionTarget& target)>;
using EntityActionDefenseModifier =
    std::function<std::int16_t(std::int16_t value,
                               const EntityActionActorState& actor,
                               const EntityActionTarget& target)>;

struct EntityActionResolutionServices {
    // 0x0DBA -> 0x0D3A bounded six-sided source.
    BoundedRandomRollBoundary sample_random_bound;
    // 0x55E92/0x55EB4. The host supplies decoded armor-table state through
    // these typed leaf callbacks; no source-address table lookup is allowed.
    EntityActionArmorModifier subtract_primary_armor;
    EntityActionArmorModifier subtract_secondary_armor;
    // 0x55E62. This callback may be a direct typed implementation of the
    // action-flag/item-0x27 reduction.
    EntityActionDefenseModifier apply_defense_modifier;
    // 0x37EC and 0x3674 boundaries used after the target's action value is
    // exhausted by the resolution path.
    std::function<void(EntityActionTarget&, std::int16_t)>
        advance_behavior_meter;
    std::function<void(EntityActionTarget&)> advance_delay_meter;
};

enum class EntityActionResolutionOutcome {
    random_service_unavailable,
    action_value_nonpositive,
    armor_modifier_unavailable,
    armor_threshold_failed,
    defense_modifier_unavailable,
    defense_threshold_failed,
    world_meter_gate,
    behavior_service_unavailable,
    behavior_meter_advanced,
    delay_service_unavailable,
    delay_meter_advanced,
    target_meter_pending,
    target_meter_exhausted,
};

struct EntityActionResolutionResult {
    EntityActionResolutionOutcome outcome =
        EntityActionResolutionOutcome::random_service_unavailable;
    std::uint16_t difficulty = 0;
    std::uint16_t trial_count = 0;
    std::uint8_t successes = 0;
    std::uint8_t previous_action_result = 0;
    std::int16_t action_value_before_modifiers = 0;
    std::int16_t action_value_after_modifiers = 0;
    std::int16_t meter_delta = 0;
    bool used_secondary_armor = false;
    bool used_primary_armor = false;
    bool used_mental_meter = false;
    bool counters_advanced = false;
};

enum class EntityActionDefaultBonusOutcome {
    dynamic_modifier_data_unavailable,
    dynamic_modifier_roll_service_unavailable,
    random_service_unavailable,
    resolved,
};

struct EntityActionDefaultBonusServices {
    // 0x0DBA used for both the initial six-roll result and the 0x559BC
    // continuation.
    BoundedRandomRollBoundary sample_random_bound;
    // 0x55786's nested profile roll.
    EntityActionRollService dynamic_roll_service;
    // 0x55E62, reached by the 0x559BC continuation after armor mode one has
    // selected the no-armor branch.
    EntityActionDefenseModifier apply_defense_modifier;
    // 0x1047E is called for the target record when its type is below three.
    std::function<void(EntityActionTarget&)> refresh_target_ready_animations;
    // 0x37EC and 0x3674 are reached by the continuation after target meters
    // or behavior state are updated.
    std::function<void(EntityActionTarget&, std::int16_t)>
        advance_behavior_meter;
    std::function<void(EntityActionTarget&)> advance_delay_meter;
};

struct EntityActionDefaultBonusResult {
    EntityActionDefaultBonusOutcome outcome =
        EntityActionDefaultBonusOutcome::dynamic_modifier_data_unavailable;
    std::uint16_t initial_difficulty = 4U;
    std::uint16_t effective_difficulty = 0;
    std::uint16_t initial_trial_count = 6U;
    std::uint16_t continuation_trial_count = 0U;
    std::uint8_t initial_successes = 0U;
    bool dynamic_modifier_selected = false;
    bool dynamic_modifier_available = false;
    bool dynamic_roll_service_available = false;
    bool target_animation_refresh_called = false;
    EntityActionResolutionResult continuation{};
};

// ROM 0x55B16. Apply the default difficulty-four action path, roll six
// source trials, publish the first result and +0x9D six-roll bonus, refresh
// low-type target animations, then enter 0x559BC with difficulty six, the
// target +0x77 trial count, action value two, and armor mode one.
EntityActionDefaultBonusResult resolve_entity_action_with_default_bonus(
    EntityActionActorState& actor,
    EntityActionTarget& target,
    EntityActionResolutionState& state,
    const EntityActionDynamicModifierContext& dynamic_context,
    const EntityActionDefaultBonusServices& services);

enum class EquipmentActionStatusOutcome {
    source_item_missing,
    source_effect_missing,
    dynamic_modifier_data_unavailable,
    dynamic_modifier_roll_service_unavailable,
    resolution_service_unavailable,
    resolved,
};

struct EquipmentActionStatusServices {
    // 0x55786's nested 0x0DBA roll. This is separate from the random service
    // used by the 0x559BC continuation because the source performs the two
    // phases as distinct calls.
    EntityActionRollService dynamic_roll_service;
    // 0x559BC and its typed armor/defense/meter boundaries.
    EntityActionResolutionServices resolution;
};

struct EquipmentActionStatusResult {
    EquipmentActionStatusOutcome outcome =
        EquipmentActionStatusOutcome::source_item_missing;
    std::uint8_t active_action_code = 0;
    std::uint8_t effect_table_index = 0;
    std::uint8_t action_effect_value = 0;
    std::uint16_t target_command_average = 0;
    std::uint16_t target_posture_remaining = 0;
    std::uint16_t trial_count = 0;
    std::uint16_t initial_difficulty = 0;
    std::uint16_t effective_difficulty = 0;
    EntityActionDynamicModifierResult dynamic_modifier{};
    bool source_item_resolved = false;
    bool source_effect_resolved = false;
    bool continuation_invoked = false;
    EntityActionResolutionResult continuation{};
};

// ROM 0x55948. Resolve an ordinary equipment action against the opposing
// entity: select A0 +0x57's one-based D4540 record, decode its +0x25 effect
// index through the typed 0x166AD4 table, use +0x26 as the initial difficulty,
// derive the target's trial count from 0x55ED6/0x55F20/+0x77/cyberware bits,
// then enter the shared 0x559BC action-resolution continuation. The source
// catalogs and all service seams are explicit; a missing row never becomes a
// guessed action value or an address-shaped fallback.
EquipmentActionStatusResult resolve_equipment_action_status(
    EntityActionActorState& actor,
    EntityActionTarget& target,
    EntityActionResolutionState& state,
    const EntityActionDynamicModifierContext& target_dynamic_context,
    const UsItemCatalog& item_catalog,
    const EquipmentActionStatusServices& services);

// A full decoded participant used by ROM 0x55C76 after it swaps A0/A1. The
// record owns the command, posture, cyberware, item, and meter fields; the
// two fields below are the source record header values used by the reaction
// gate. This is a host model, not a Genesis memory image.
struct EntityActionReactionParticipant {
    EntityActionRecord record{};
    std::uint16_t record_type = 0; // +0x3E
    std::uint8_t equipped_action_code = 0; // +0x57
};

using EntityActionParticipantArmorModifier =
    std::function<std::int16_t(std::int16_t value,
                               const EntityActionRecord& target)>;
using EntityActionParticipantDefenseModifier =
    std::function<std::int16_t(std::int16_t value,
                               const EntityActionRecord& attacker,
                               const EntityActionRecord& target)>;

struct EntityActionReactionServices {
    // The three 0x0DBA passes in 0x55C76.
    BoundedRandomRollBoundary sample_random_bound;
    // 0x55EB4, applied after the first nonzero threshold result.
    EntityActionParticipantArmorModifier subtract_secondary_armor;
    // 0x55E62, applied after that armor subtraction.
    EntityActionParticipantDefenseModifier apply_defense_modifier;
    // Nested 0x55786 profile rolls for the actor pass and swapped peer pass.
    EntityActionRollService dynamic_roll_service;
    // 0x37EC and 0x3674 final meter/display service boundaries.
    std::function<void(EntityActionReactionParticipant&, std::int16_t)>
        advance_behavior_meter;
    std::function<void(EntityActionReactionParticipant&)> advance_delay_meter;
};

// Bind the verified US D4D50 rows to the source armor call sites. The
// returned service objects retain all caller-supplied callbacks and replace
// only the 0x55E92/0x55EB4 armor seams. `catalog` must outlive the returned
// services because the source data is referenced by the callbacks.
EntityActionResolutionServices bind_entity_action_armor_catalog(
    const UsArmorCatalog& catalog,
    EntityActionResolutionServices services);
EntityActionReactionServices bind_entity_action_armor_catalog(
    const UsArmorCatalog& catalog,
    EntityActionReactionServices services);

enum class EntityActionReactionOutcome {
    random_service_unavailable,
    first_roll_failed,
    secondary_armor_service_unavailable,
    first_armor_threshold_failed,
    defense_service_unavailable,
    first_defense_threshold_failed,
    dynamic_modifier_data_unavailable,
    dynamic_modifier_roll_service_unavailable,
    second_roll_failed,
    final_roll_failed,
    physical_penalty_failed,
    world_meter_gate,
    behavior_service_unavailable,
    delay_service_unavailable,
    physical_meter_pending,
    mental_meter_pending,
    meter_exhausted,
};

struct EntityActionReactionResult {
    EntityActionReactionOutcome outcome =
        EntityActionReactionOutcome::random_service_unavailable;
    std::uint16_t initial_command_average = 0;
    std::uint16_t initial_posture_reduction = 0;
    std::uint16_t initial_trial_count = 0;
    std::uint16_t initial_difficulty = 0;
    std::uint8_t first_successes = 0;
    std::int16_t value_after_first_modifiers = 0;
    std::uint16_t peer_command_average = 0;
    std::uint16_t peer_posture_reduction = 0;
    std::uint16_t peer_trial_count = 0;
    std::uint16_t peer_difficulty = 0;
    std::uint8_t second_successes = 0;
    std::uint16_t final_trial_count = 0;
    std::uint16_t final_difficulty = 0;
    std::uint8_t final_successes = 0;
    std::int16_t remaining_value = 0;
    std::int16_t meter_delta = 0;
    bool used_secondary_armor = false;
    bool used_physical_meter = false;
    bool used_mental_meter = false;
    bool counters_advanced = false;
};

// ROM 0x55C76. Convert the three-pass reaction sequence: attacker command
// and posture roll, secondary armor/defense reduction, swapped peer roll,
// cyberware-selected final roll, and the physical/mental target-meter tail.
EntityActionReactionResult advance_entity_action_resolution(
    EntityActionReactionParticipant& actor,
    EntityActionReactionParticipant& target,
    EntityActionResolutionState& state,
    const EntityActionDynamicModifierContext& actor_dynamic_context,
    const EntityActionDynamicModifierContext& target_dynamic_context,
    const EntityActionReactionServices& services);

// ROM 0x559BC. Convert the shared action-resolution continuation: thresholded
// result delta, armor/defense reductions, record-type behavior/delay branch,
// physical/mental meter mutation, and the three source counter updates.
// `action_value` is the incoming D5 seed and `trial_count` is incoming D6.
EntityActionResolutionResult resolve_dynamic_entity_action(
    EntityActionActorState& actor,
    EntityActionTarget& target,
    std::uint16_t difficulty,
    std::uint16_t trial_count,
    std::int16_t action_value,
    EntityActionResolutionState& state,
    const EntityActionResolutionServices& services);

} // namespace shadowrun::ghidra_conversion

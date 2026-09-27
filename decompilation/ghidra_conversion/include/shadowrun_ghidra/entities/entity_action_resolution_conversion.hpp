#pragma once

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

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

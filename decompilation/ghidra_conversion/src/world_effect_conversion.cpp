#include "shadowrun_ghidra/world_effect_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool schedule_world_activity_timeout(WorldActivityTimeoutState& state) {
    if (state.scene_id == 0x0036U && !state.scene_ready) {
        return false;
    }

    if (state.timer_marker == 0) {
        state.timer_marker = -1;
    }
    state.timeout = state.long_timeout_source ? 0x0258U : 0x0078U;
    return true;
}

void clear_entity_effect_pending_flags(
    std::array<std::uint8_t, 6>& effect_flags) {
    for (auto& flags : effect_flags) {
        flags = static_cast<std::uint8_t>(flags & 0xFEU);
    }
}

void promote_entity_effect_flags(std::array<std::uint8_t, 6>& effect_flags) {
    for (auto& flags : effect_flags) {
        if ((flags & 0x02U) != 0U) {
            flags = static_cast<std::uint8_t>(flags | 0x04U);
        }
    }
}

WorldEffectTransitionAction begin_world_effect_transition(
    WorldEffectTransitionState& state,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list) {
    if (state.scene_id == 0x0014U) {
        static_cast<void>(schedule_world_activity_timeout(timeout_state));
        return WorldEffectTransitionAction::timeout_scheduled;
    }

    if (state.world_mode != 2U) {
        return WorldEffectTransitionAction::no_action;
    }

    state.transition_timer = 0x0960U;
    if (state.transition_latched) {
        return WorldEffectTransitionAction::already_latched;
    }

    state.transition_latched = true;
    if (play_sound) {
        play_sound(0x0030U);
    }
    if (clear_pending_flags) {
        clear_pending_flags();
    }
    if (process_effect_list) {
        process_effect_list();
    }
    return WorldEffectTransitionAction::started;
}

WorldEffectTriggerAction advance_world_effect_trigger(
    WorldEffectTriggerState& state,
    std::array<std::uint8_t, 6>& effect_flags,
    const std::vector<WorldEffectRegion>& regions,
    const WorldRandomBoundary& next_random,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& finish_transition_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list) {
    if (state.transition.transition_latched) {
        --state.transition.transition_timer;
        if (state.transition.transition_timer < 0) {
            state.transition.transition_latched = false;
            promote_entity_effect_flags(effect_flags);
            if (finish_transition_sound) {
                finish_transition_sound();
            }
            return WorldEffectTriggerAction::transition_finished;
        }
    }

    if (state.trigger_suppressed) {
        return WorldEffectTriggerAction::trigger_suppressed;
    }

    if ((state.frame_counter & 0x003FU) != 0x000DU) {
        return WorldEffectTriggerAction::cadence_wait;
    }

    if (!next_random || next_random() > 0x7FFFU) {
        return WorldEffectTriggerAction::random_rejected;
    }

    const auto player_x = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.player_fixed_x >> 4U));
    const auto player_y = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.player_fixed_y >> 4U));
    bool matching_region = false;
    for (const auto& region : regions) {
        if (region.sentinel) {
            break;
        }
        if (player_x > region.minimum_x &&
            player_x <= region.maximum_x &&
            player_y > region.minimum_y &&
            player_y <= region.maximum_y) {
            matching_region = true;
            break;
        }
    }
    if (!matching_region) {
        return WorldEffectTriggerAction::no_matching_region;
    }

    const auto transition_action = begin_world_effect_transition(
        state.transition,
        timeout_state,
        play_sound,
        clear_pending_flags,
        process_effect_list);
    switch (transition_action) {
    case WorldEffectTransitionAction::timeout_scheduled:
        return WorldEffectTriggerAction::timeout_scheduled;
    case WorldEffectTransitionAction::already_latched:
        return WorldEffectTriggerAction::transition_already_latched;
    case WorldEffectTransitionAction::started:
        return WorldEffectTriggerAction::transition_started;
    case WorldEffectTransitionAction::no_action:
        return WorldEffectTriggerAction::no_action;
    }
    return WorldEffectTriggerAction::no_action;
}

} // namespace shadowrun::ghidra_conversion

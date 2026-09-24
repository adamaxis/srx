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

} // namespace shadowrun::ghidra_conversion

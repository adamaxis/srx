#include "shadowrun_ghidra/world/world_effect_trigger_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectTriggerState state{};
    WorldEffectTriggerCallbacks callbacks{};
    if (trigger_world_effect_sequence(state, callbacks) !=
        WorldEffectTriggerAction::not_due) {
        return 1;
    }

    state.frame_counter = 10U;
    state.world_effects_active = true;
    if (trigger_world_effect_sequence(state, callbacks) !=
        WorldEffectTriggerAction::blocked) {
        return 2;
    }
    state.world_effects_active = false;
    state.trigger_delay_active = true;
    if (trigger_world_effect_sequence(state, callbacks) !=
        WorldEffectTriggerAction::blocked) {
        return 3;
    }

    state = {};
    state.frame_counter = 10U;
    int forwarded = 0;
    int activity = 0;
    int spawned = 0;
    callbacks.sample_random = [] { return std::pair<std::uint16_t,
                                                     std::uint16_t>{2U, 3U}; };
    callbacks.forward_effect_sequence = [&] { ++forwarded; };
    callbacks.select_world_effect_sequence = [] { return std::int16_t{1}; };
    callbacks.begin_world_activity_if_idle = [&] { ++activity; };
    callbacks.spawn_world_effect_sequence = [&] { ++spawned; };
    state.sequence_mode = true;
    if (trigger_world_effect_sequence(state, callbacks) !=
            WorldEffectTriggerAction::forwarded ||
        forwarded != 1 || activity != 0 || spawned != 0) {
        return 4;
    }

    state.sequence_mode = false;
    if (trigger_world_effect_sequence(state, callbacks) !=
            WorldEffectTriggerAction::activity_started ||
        forwarded != 1 || activity != 1 || spawned != 0) {
        return 5;
    }

    callbacks.select_world_effect_sequence = [] { return std::int16_t{0}; };
    if (trigger_world_effect_sequence(state, callbacks) !=
            WorldEffectTriggerAction::sequence_spawned ||
        spawned != 1) {
        return 6;
    }

    callbacks.sample_random = [] { return std::pair<std::uint16_t,
                                                    std::uint16_t>{4U, 3U}; };
    if (trigger_world_effect_sequence(state, callbacks) !=
        WorldEffectTriggerAction::random_miss) {
        return 7;
    }
    return 0;
}

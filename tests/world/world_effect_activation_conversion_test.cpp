#include "shadowrun_ghidra/world/world_effect_activation_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectActivationState state{};
    state.control_flags = 0x40U;
    WorldEffectActivationCallbacks callbacks{};
    if (advance_world_effect_activation(state, callbacks) !=
            WorldEffectActivationAction::secondary_record_started ||
        !state.secondary.active || state.secondary.position_x != 0x1400 ||
        state.secondary.position_y != 0x1070 ||
        state.secondary.velocity_x != -8 ||
        state.secondary.velocity_y != -14 ||
        state.secondary.lifetime != 0x3CU) {
        return 1;
    }
    if (advance_world_effect_activation(state, callbacks) !=
        WorldEffectActivationAction::idle_secondary_record) {
        return 2;
    }

    state = {};
    state.setup_marker = -1;
    std::uint16_t audio_command = 0;
    callbacks.play_audio = [&](const std::uint16_t command) {
        audio_command = command;
    };
    if (advance_world_effect_activation(state, callbacks) !=
            WorldEffectActivationAction::special_effect_started ||
        state.special.position_x != 0x0134U ||
        state.special.position_y != 0x0102U ||
        state.special.behavior_token != 0x1C27CU ||
        state.special.completion_marker != -1 ||
        state.special.frame_countdown != 3U || audio_command != 0x16U) {
        return 3;
    }

    state = {};
    state.setup_marker = 1;
    int initialized = 0;
    callbacks = {};
    callbacks.initialize_world_effect_motion = [&] { ++initialized; };
    if (advance_world_effect_activation(state, callbacks) !=
            WorldEffectActivationAction::motion_initialization_requested ||
        initialized != 1) {
        return 4;
    }

    state = {};
    state.transition_pending = true;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
    std::int16_t source_word = 1;
    int spawned = 0;
    std::int16_t y_adjustment = 0;
    callbacks = {};
    callbacks.spawn_effect_record =
        [&](const std::int16_t x, const std::int16_t y,
            const std::int16_t source) {
        offset_x = x;
        offset_y = y;
        source_word = source;
        ++spawned;
        return true;
    };
    callbacks.adjust_spawn_record_y = [&](const std::int16_t delta) {
        y_adjustment = delta;
    };
    if (advance_world_effect_activation(state, callbacks) !=
            WorldEffectActivationAction::spawn_requested || spawned != 1 ||
        offset_x != 0x20 || offset_y != -0x28 || source_word != 0 ||
        y_adjustment != -6) {
        return 5;
    }
    return 0;
}

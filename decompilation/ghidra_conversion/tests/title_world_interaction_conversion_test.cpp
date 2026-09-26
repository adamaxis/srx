#include "shadowrun_ghidra/title_world_interaction_conversion.hpp"

#include <cstdint>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleWorldInteractionState completion{};
    completion.scene_complete = true;
    TitleWorldBehaviorPair installed{};
    int initialized = 0;
    TitleWorldInteractionCallbacks callbacks{};
    callbacks.initialize_behavior_record = [&] { ++initialized; };
    callbacks.install_behavior_pair =
        [&](const TitleWorldBehaviorPair& pair) { installed = pair; };
    if (!expect(advance_title_world_interaction_service(completion, callbacks) ==
                    TitleWorldInteractionAction::completion_behavior_installed &&
                initialized == 1 && installed.draw_descriptor == 0x001C2A8U &&
                installed.update_descriptor == 0x001C2AEU)) {
        return 1;
    }

    TitleWorldInteractionState blocked{};
    blocked.title_service_flags = 0x01U;
    if (!expect(advance_title_world_interaction_service(blocked, {}) ==
                TitleWorldInteractionAction::title_sweep_blocked)) {
        return 2;
    }

    TitleWorldInteractionState activation{};
    activation.primary_behavior_descriptor = 0x001C36AU;
    activation.primary_stage = 3U;
    activation.primary_phase = 3U;
    int activations = 0;
    callbacks = {};
    callbacks.advance_world_effect_activation = [&] { ++activations; };
    if (!expect(advance_title_world_interaction_service(activation, callbacks) ==
                    TitleWorldInteractionAction::world_effect_activation_advanced &&
                activations == 1)) {
        return 3;
    }

    TitleWorldInteractionState primary_waiting{};
    primary_waiting.primary_behavior_descriptor = 0x001C36AU;
    primary_waiting.primary_stage = 2U;
    primary_waiting.primary_phase = 3U;
    if (!expect(advance_title_world_interaction_service(primary_waiting, {}) ==
                TitleWorldInteractionAction::primary_behavior_waiting)) {
        return 4;
    }

    TitleWorldInteractionState interaction{};
    interaction.secondary_behavior_descriptor = 0x001C350U;
    interaction.secondary_stage = 3U;
    interaction.secondary_phase = 3U;
    int interactions = 0;
    callbacks = {};
    callbacks.update_interaction_state = [&] { ++interactions; };
    if (!expect(advance_title_world_interaction_service(interaction, callbacks) ==
                    TitleWorldInteractionAction::interaction_state_advanced &&
                interactions == 1)) {
        return 5;
    }

    TitleWorldInteractionState matrix{};
    matrix.primary_behavior_descriptor = 0x001C36AU;
    matrix.primary_stage = 5U;
    matrix.world_effect_mode = 2U;
    matrix.controller_events = 0x20U;
    int matrix_actions = 0;
    callbacks = {};
    callbacks.service_matrix_program_action = [&] { ++matrix_actions; };
    if (!expect(advance_title_world_interaction_service(matrix, callbacks) ==
                    TitleWorldInteractionAction::matrix_program_action_serviced &&
                matrix_actions == 1)) {
        return 6;
    }

    matrix.descriptor_transition_active = true;
    if (!expect(advance_title_world_interaction_service(matrix, callbacks) ==
                TitleWorldInteractionAction::no_action)) {
        return 7;
    }
    return 0;
}

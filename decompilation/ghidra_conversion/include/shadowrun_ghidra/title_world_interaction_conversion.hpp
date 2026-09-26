#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleWorldBehaviorPair {
    std::uint32_t draw_descriptor = 0;
    std::uint32_t update_descriptor = 0;
};

// Fields from the shared behavior record at FF1580 used by ROM 0x1C754.
// The names describe the two source behavior slots, not a byte-for-byte
// runtime record layout.
struct TitleWorldInteractionState {
    bool scene_complete = false; // FFE084
    std::uint8_t title_service_flags = 0; // FF1592 bit 0 blocks input

    std::uint32_t primary_behavior_descriptor = 0; // FF1584 (+4)
    std::uint32_t secondary_behavior_descriptor = 0; // FF1588 (+8)
    std::uint8_t primary_phase = 0; // FF1583 (+3)
    std::uint8_t primary_stage = 0; // FF158D (+0x0D)
    std::uint8_t secondary_phase = 0; // FF1582 (+2)
    std::uint8_t secondary_stage = 0; // FF158C (+0x0C)

    std::uint16_t world_effect_mode = 0; // FFE076
    std::uint8_t controller_events = 0; // FFDD22 low byte
    bool descriptor_transition_active = false; // FFE082
};

struct TitleWorldInteractionCallbacks {
    // ROM 0x1B7D0 followed by writes of 0x1C2A8 and 0x1C2AE.
    std::function<void()> initialize_behavior_record;
    std::function<void(const TitleWorldBehaviorPair&)> install_behavior_pair;

    std::function<void()> advance_world_effect_activation; // 0x1CB20
    std::function<void()> update_interaction_state; // 0x1CAD4
    std::function<void()> service_matrix_program_action; // 0x19976
};

enum class TitleWorldInteractionAction {
    completion_behavior_installed,
    title_sweep_blocked,
    world_effect_activation_advanced,
    primary_behavior_waiting,
    interaction_state_advanced,
    secondary_behavior_waiting,
    matrix_program_action_serviced,
    no_action,
    dependency_unavailable,
};

// ROM 0x1C754: dispatch the title/world interaction state machine.  It is a
// source behavior gate, not a renderer and not a hardware emulation routine.
TitleWorldInteractionAction advance_title_world_interaction_service(
    TitleWorldInteractionState& state,
    const TitleWorldInteractionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

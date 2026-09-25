#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The title/world-effect routines share the record beginning at FF163E. The
// source reuses FF1643 as either an animation phase or a descriptor-table
// index, so the host model keeps that overlap visible instead of pretending
// the two source paths have independent storage.
struct WorldEffectWorkRecord {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t frame_counter = 0;
    std::uint8_t phase_or_descriptor_index = 0;
    std::uint8_t descriptor_state = 0;
    std::uint8_t descriptor_phase = 0;
    std::uint32_t descriptor_pointer = 0;
};

enum class WorldEffectDescriptorTransitionAction {
    blocked_by_completion_gate,
    loading_started,
    descriptor_attached,
    descriptor_unresolved,
};

struct WorldEffectDescriptorTransitionState {
    bool completion_gate = false;       // FFE0D4
    std::uint16_t animation_mode = 0;   // FFE076
    bool transition_active = false;     // FFE082
    bool descriptor_loading = false;    // FFE0F2
    bool descriptor_stream_ready = false; // FFE0F4
    std::optional<std::uint32_t> behavior_token; // FFDF7E
    std::uint16_t behavior_position = 0;         // FFDF76
    WorldEffectWorkRecord work{};
};

struct WorldEffectDescriptorTransitionCallbacks {
    // ROM 0x1A818 follows FF163E+5 through the active descriptor table and
    // reads the longword at descriptor +2. The table remains a source-data
    // boundary until its records are independently named.
    std::function<std::optional<std::uint32_t>(
        std::uint8_t descriptor_index)>
        read_behavior_token;
    std::function<void()> attach_behavior_descriptor;
};

// ROM 0x1A818: start the descriptor transition, either staging the six-frame
// loading state or attaching the selected behavior descriptor.  0xD280 is
// the source word written to FFDF76 (the Ghidra unsigned rendering was stale).
WorldEffectDescriptorTransitionAction begin_world_effect_descriptor_transition(
    WorldEffectDescriptorTransitionState& state,
    const WorldEffectDescriptorTransitionCallbacks& callbacks);

struct WorldEffectModeDescriptor {
    std::uint8_t descriptor_state = 0;
    std::uint8_t descriptor_phase = 0;
    std::uint32_t descriptor_pointer = 0;
};

struct WorldEffectModeState {
    bool animation_active = false;       // FF164A
    bool exit_requested = false;         // FFE080
    bool mode_request_pending = false;   // FFE0DE
    std::int16_t requested_mode = 0;     // FFE0E0
    std::int16_t selected_mode = 0;      // FFE0BC
    bool mode_seven_started = false;     // FFE0B6
    std::uint16_t mode_seven_position = 0; // FFE0BE
    bool mode_six_started = false;       // FFE0B8
    std::uint16_t mode_six_position = 0; // FFE0BA
    WorldEffectWorkRecord work{};
};

enum class WorldEffectModeAction {
    initialized,
    exit_requested,
    descriptor_unresolved,
};

struct WorldEffectModeCallbacks {
    // ROM 0x19B40 is a separate mode-selection hill. It is invoked only when
    // FFE0DE is clear.
    std::function<std::int16_t()> select_run_hud_mode;
    // Positive mode values index the descriptor pointer table at ROM 0x1A9CE.
    std::function<std::optional<WorldEffectModeDescriptor>(
        std::int16_t mode)>
        resolve_mode_descriptor;
};

// ROM 0x1A908: select a world-effect mode, arm its one-shot mode-6/mode-7
// setup, handle the 0x45 exit sentinel, and initialize FF163E's descriptor
// work record.  Negative selections use the fixed descriptor at ROM 0x1AA10.
WorldEffectModeAction initialize_world_effect_mode(
    WorldEffectModeState& state,
    const WorldEffectModeCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

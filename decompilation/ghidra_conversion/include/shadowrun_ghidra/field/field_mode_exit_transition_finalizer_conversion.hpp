#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

enum class ExitEffectSlotGroup {
    mode_one,
    mode_two_or_four,
    mode_five,
};

enum class FieldModeExitServicePhase {
    mode_specific_cleanup,
    common_state_refresh,
};

enum class FieldModeProfile {
    profile_a,
    profile_b,
};

struct FieldModeExitTransitionState {
    // Typed state corresponding to the source fields consumed by ROM 0xC64C.
    std::uint8_t exit_mode = 0;                 // FFFBED
    std::uint8_t transition_flags = 0;          // FFFC1C
    std::uint32_t effect_service_flags = 0;     // FFFC22
    std::uint32_t world_mode_flags = 0;         // FFFC1E
    std::uint8_t effect_transition_marker = 0;  // FFE186
    std::uint8_t effect_animation_marker = 0;   // FFE182
    std::uint8_t mode_prompt_pending = 0;       // FFF10C
    std::uint8_t mode_five_effect_marker = 0;   // FFE183
    std::uint8_t primary_runner_transition = 0; // FF024C
    std::uint8_t secondary_runner_transition = 0; // FF034C
    std::uint8_t runner_flags = 0;              // FF0110
    std::uint8_t recruited_runner_flags = 0;    // FF0111
    std::uint32_t field_anchor = 0;             // FF013A
    std::uint8_t action_counter = 0;            // FF014C
    std::uint8_t resource_counter = 0;          // FF019C
    std::uint32_t resource_pool = 0;            // FFFBFE
};

struct FieldModeExitTransitionCallbacks {
    std::function<void(ExitEffectSlotGroup)> clear_high_effect_slots;
    std::function<void()> reset_world_effect_globals;
    std::function<void(FieldModeExitServicePhase)> run_mode_exit_services;
    std::function<void()> refresh_field_runner_state_and_assets;
    std::function<void()> run_entity_animation_triplet;
    std::function<void()> handle_mode_action_event;
    std::function<void()> initialize_field_mode_profile_a;
    std::function<void()> initialize_field_mode_profile_b;
};

struct FieldModeExitTransitionResult {
    std::optional<ExitEffectSlotGroup> cleared_effect_group;
    bool mode_specific_cleanup_ran = false;
    FieldModeProfile initialized_profile = FieldModeProfile::profile_b;
};

// ROM 0xC64C: finalize a field-mode transition.  The native conversion owns
// the typed flag/counter mutations; resource decoding, animation, action, and
// profile initialization remain explicit host callbacks.
FieldModeExitTransitionResult finalize_field_mode_exit_transition(
    FieldModeExitTransitionState& state,
    const FieldModeExitTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

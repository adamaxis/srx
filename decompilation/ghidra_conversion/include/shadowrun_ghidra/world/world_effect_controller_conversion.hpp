#pragma once

#include "shadowrun_ghidra/world/world_effect_mode_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// State consumed by ROM 0x1A370.  These names describe the world-effect
// controller's behavior rather than reproducing its RAM layout.  The source
// still supplies the address comments where a field has been proven.
struct WorldEffectControllerState {
    WorldEffectWorkRecord work{}; // FF163E
    bool work_record_active = false; // FF163E + 0x0C
    bool descriptor_transition_active = false; // FFE082
    bool descriptor_stream_loading = false; // FFE0F2
    bool world_effects_active = false; // FFDF84 != 0
    std::uint16_t animation_mode = 0; // FFE076
    std::int16_t effect_countdown = 0; // FFE074
    std::int16_t selected_effect_mode = 0; // FFE07E
    std::int16_t world_effect_mode = 0; // FFE0BC
    std::int8_t animation_divisor = 0; // FFF0E7

    // These are the values consumed by the later world-effect animation
    // passes.  The source publishes them here, then 0x1A5A0/0x1AB6E/etc.
    // consume them on later frames.
    std::uint16_t title_phase_handler_index = 0; // FFE04E
    std::uint16_t motion_step = 0; // FFE0C0
    std::uint16_t background_step = 0; // FFE0C4
    std::uint16_t background_started = 0; // FFE0C2
    std::uint16_t background_position = 0; // FFE0C6
    std::uint16_t secondary_started = 0; // FFE0B8
    std::uint16_t primary_started = 0; // FFE0B6
    bool exit_requested = false; // FFE080, published by 0x1A8F4
};

struct WorldEffectTransitionCallbacks {
    // 0x00BD2 and 0x00E9C are presentation boundaries.  The conversion
    // exposes the completed transition-noise frame instead of reproducing a
    // VDP command stream in the host runtime.
    std::function<void()> wait_for_vblank;
    std::function<void(std::uint8_t phase)> stream_transition_noise;
    std::function<void()> reset_effect_entity;
};

enum class WorldEffectTransitionAction {
    inactive,
    descriptor_stream_loading,
    noise_frame_streamed,
    effect_entity_reset,
};

// ROM 0x1A88E, reached from 0x1A370 while FFE082 is set.  It advances the
// byte-sized transition cadence at FF163E+4/+6, streams one transition-noise
// frame, and ends the effect after the six descriptor phases.
WorldEffectTransitionAction advance_world_effect_transition(
    WorldEffectControllerState& state,
    const WorldEffectTransitionCallbacks& callbacks);

struct WorldEffectControllerCallbacks {
    // ROM 0x1A538: selected compact runner record +0x0F.
    std::function<bool()> runner_has_active_effect;
    // ROM 0x1A818: starts descriptor loading or attaches the descriptor.
    std::function<void()> begin_descriptor_transition;
    // ROM 0x17E78: returns the frame/countdown divisor used by this effect.
    std::function<std::optional<std::uint16_t>()> calculate_effect_countdown;
    // ROM 0x561E8: selects the next world-effect mode/result.
    std::function<std::optional<std::int16_t>()> select_effect_mode;
    // ROM 0x1A54E: initializes the default effect placement/timing.
    std::function<void()> initialize_effect_defaults;
    WorldEffectTransitionCallbacks transition{};
};

enum class WorldEffectControllerAction {
    inactive,
    descriptor_transition_started,
    descriptor_stream_loading,
    transition_noise_streamed,
    effect_entity_reset,
    runner_effect_wait,
    world_effects_disabled,
    animation_already_active,
    countdown_wait,
    countdown_unresolved,
    effect_mode_unresolved,
    effect_mode_rejected,
    default_effect_initialized,
    background_effect_initialized,
    motion_effect_initialized,
};

// ROM 0x1A370: coordinate the world-effect descriptor transition and the
// per-frame mode-specific setup.  This is conversion/reference code; it
// keeps VDP/audio/ROM-table operations at explicit callback boundaries.
WorldEffectControllerAction advance_world_effect_controller(
    WorldEffectControllerState& state,
    const WorldEffectControllerCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

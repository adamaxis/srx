#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WorldActivityTimeoutState {
    std::uint16_t scene_id = 0;
    bool scene_ready = false;
    std::int16_t timer_marker = 0;
    std::uint16_t timeout = 0;
    bool long_timeout_source = false;
};

// ROM 0x1DBAA: suppress timeout scheduling for the guarded scene, otherwise
// arm the source marker and select the short or long timeout value.
bool schedule_world_activity_timeout(WorldActivityTimeoutState& state);

// ROM 0x1DF00: clear bit 0 in the six world-record effect flag bytes.
void clear_entity_effect_pending_flags(
    std::array<std::uint8_t, 6>& effect_flags);

// ROM 0x1E0B8: promote every record carrying bit 1 to the bit-2 effect state.
void promote_entity_effect_flags(std::array<std::uint8_t, 6>& effect_flags);

enum class WorldEffectTransitionAction {
    no_action,
    timeout_scheduled,
    already_latched,
    started,
};

struct WorldEffectTransitionState {
    std::uint16_t scene_id = 0;
    std::uint16_t world_mode = 0;
    std::int16_t transition_timer = 0;
    bool transition_latched = false;
};

using WorldEffectBoundary = std::function<void()>;
using WorldSoundBoundary = std::function<void(std::uint16_t sound_id)>;

// ROM 0x1E04A: schedule the scene-14 timeout or start the mode-2 world
// effect transition. The sound, pending-flag clear, and effect-list walk are
// explicit host boundaries because their callees are separate source hills.
WorldEffectTransitionAction begin_world_effect_transition(
    WorldEffectTransitionState& state,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list);

struct WorldEffectRegion {
    std::int16_t minimum_x = 0;
    std::int16_t maximum_x = 0;
    std::int16_t minimum_y = 0;
    std::int16_t maximum_y = 0;
    bool sentinel = false;
};

struct WorldEffectTriggerState {
    WorldEffectTransitionState transition{};
    bool trigger_suppressed = false;
    std::uint16_t frame_counter = 0;
    std::uint16_t player_fixed_x = 0;
    std::uint16_t player_fixed_y = 0;
};

enum class WorldEffectTriggerAction {
    no_action,
    transition_finished,
    trigger_suppressed,
    cadence_wait,
    random_rejected,
    no_matching_region,
    timeout_scheduled,
    transition_already_latched,
    transition_started,
};

using WorldRandomBoundary = std::function<std::uint16_t()>;

// ROM 0x1DFC4: finish an expired transition or, on the source cadence,
// sample the PRNG and scan the bounded effect-region list before delegating
// to the 0x1E04A transition policy.
WorldEffectTriggerAction advance_world_effect_trigger(
    WorldEffectTriggerState& state,
    std::array<std::uint8_t, 6>& effect_flags,
    const std::vector<WorldEffectRegion>& regions,
    const WorldRandomBoundary& next_random,
    WorldActivityTimeoutState& timeout_state,
    const WorldSoundBoundary& play_sound,
    const WorldEffectBoundary& finish_transition_sound,
    const WorldEffectBoundary& clear_pending_flags,
    const WorldEffectBoundary& process_effect_list);

} // namespace shadowrun::ghidra_conversion

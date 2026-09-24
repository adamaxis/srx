#pragma once

#include <array>
#include <cstdint>
#include <functional>

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
    std::uint16_t transition_timer = 0;
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

} // namespace shadowrun::ghidra_conversion

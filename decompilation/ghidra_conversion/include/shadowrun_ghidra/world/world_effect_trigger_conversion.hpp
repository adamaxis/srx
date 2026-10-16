#pragma once

#include <cstdint>
#include <functional>
#include <utility>

namespace shadowrun::ghidra_conversion {

struct WorldEffectTriggerState {
    bool trigger_delay_active = false;    // FFD FE8 != 0
    bool current_entity_protected = false; // entity +0x11 bit 7
    bool activity_lock = false;            // FFE18B != 0
    bool sequence_mode = false;            // FFE16F != 0
    bool world_effects_active = false;     // FFDF84 != 0
    bool placement_pending = false;        // FFE177 != 0
    std::uint16_t frame_counter = 0;       // FFDF88
};

struct WorldEffectTriggerCallbacks {
    // Returns the random value in first and the source bound in second.
    std::function<std::pair<std::uint16_t, std::uint16_t>()> sample_random;
    std::function<void()> forward_effect_sequence;
    std::function<std::int16_t()> select_world_effect_sequence;
    std::function<void()> begin_world_activity_if_idle;
    std::function<void()> spawn_world_effect_sequence;
};

enum class WorldEffectTriggerAction {
    blocked,
    not_due,
    random_miss,
    forwarded,
    activity_started,
    sequence_spawned,
};

// ROM 0x1E166: gate random world-event attempts, then forward, begin activity,
// or spawn a sequence according to the current world state and selector.
WorldEffectTriggerAction trigger_world_effect_sequence(
    const WorldEffectTriggerState& state,
    const WorldEffectTriggerCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

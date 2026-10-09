#pragma once

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldActorAnimationPhase : std::uint8_t {
    initial_idle,
    entry_transition,
};

// Native state for the initial behavior of a profile-listed field actor.
// It intentionally contains no ROM/RAM addresses or graphics objects.
struct FieldActorBehaviorState {
    std::uint16_t entity_kind = 0U;
    std::uint16_t heading = 0U;
    std::uint8_t direction = 0U;
    std::uint8_t behavior_timer = 0U;
    FieldActorAnimationPhase animation_phase =
        FieldActorAnimationPhase::initial_idle;
    std::uint8_t animation_frame = 0U;
    std::int16_t animation_accumulator = 0;
    bool transition_started = false;
    bool transition_frame_list_finished = false;
    bool transition_complete = false;
};

struct FieldActorBehaviorUpdate {
    bool transition_started = false;
    bool transition_complete = false;
    std::optional<std::uint16_t> audio_event;
};

FieldActorBehaviorState make_field_actor_behavior_state(
    std::uint16_t entity_kind,
    std::uint16_t heading,
    std::uint8_t initial_delay);

std::uint8_t field_actor_direction_from_heading(std::uint16_t heading);
std::uint16_t field_actor_idle_animation_selector(std::uint8_t direction);
std::uint16_t field_actor_entry_animation_selector(std::uint8_t direction);

FieldActorBehaviorUpdate advance_field_actor_behavior(
    FieldActorBehaviorState& state, bool start_entry_animation,
    std::optional<std::uint16_t> entry_audio_event);

} // namespace shadowrun::ghidra_conversion::qt_runtime

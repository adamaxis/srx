#include "field_actor_runtime.hpp"

#include <cstdint>

using shadowrun::ghidra_conversion::qt_runtime::FieldActorAnimationPhase;
using shadowrun::ghidra_conversion::qt_runtime::advance_field_actor_behavior;
using shadowrun::ghidra_conversion::qt_runtime::field_actor_direction_from_heading;
using shadowrun::ghidra_conversion::qt_runtime::field_actor_entry_animation_selector;
using shadowrun::ghidra_conversion::qt_runtime::field_actor_idle_animation_selector;
using shadowrun::ghidra_conversion::qt_runtime::make_field_actor_behavior_state;

int main() {
    if (field_actor_direction_from_heading(0x0000U) != 0U ||
        field_actor_direction_from_heading(0x4000U) != 2U ||
        field_actor_direction_from_heading(0xC000U) != 6U ||
        field_actor_idle_animation_selector(0U) != 0x0000U ||
        field_actor_idle_animation_selector(2U) != 0x0008U ||
        field_actor_idle_animation_selector(6U) != 0x0018U ||
        field_actor_entry_animation_selector(0U) != 0x00E8U ||
        field_actor_entry_animation_selector(2U) != 0x00F0U ||
        field_actor_entry_animation_selector(6U) != 0x0100U) {
        return 1;
    }

    auto guard = make_field_actor_behavior_state(5U, 0x4000U, 0U);
    const auto started = advance_field_actor_behavior(guard);
    if (!started.transition_started || started.transition_complete ||
        started.audio_event != 0x002EU ||
        guard.animation_phase != FieldActorAnimationPhase::entry_transition ||
        guard.direction != 2U || guard.animation_frame != 0U ||
        guard.behavior_timer != 20U) {
        return 2;
    }
    for (int frame = 0; frame < 8; ++frame) {
        const auto update = advance_field_actor_behavior(guard);
        if (update.transition_started || update.audio_event.has_value() ||
            guard.animation_frame != 0U) {
            return 3;
        }
    }
    (void)advance_field_actor_behavior(guard);
    if (guard.animation_frame != 1U ||
        guard.transition_frame_list_finished) {
        return 4;
    }
    for (int frame = 0; frame < 7; ++frame) {
        (void)advance_field_actor_behavior(guard);
        if (guard.transition_frame_list_finished ||
            guard.animation_frame != 1U) {
            return 5;
        }
    }
    (void)advance_field_actor_behavior(guard);
    if (!guard.transition_frame_list_finished || guard.animation_frame != 1U) {
        return 6;
    }
    for (int frame = 0; frame < 3; ++frame) {
        const auto update = advance_field_actor_behavior(guard);
        if (update.transition_complete) {
            return 7;
        }
    }
    const auto completed = advance_field_actor_behavior(guard);
    if (!completed.transition_complete || guard.animation_frame != 1U ||
        !guard.transition_complete) {
        return 8;
    }
    for (int frame = 0; frame < 30; ++frame) {
        const auto update = advance_field_actor_behavior(guard);
        if (update.transition_started || update.transition_complete ||
            update.audio_event.has_value() || guard.animation_frame != 1U) {
            return 9;
        }
    }

    auto pedestrian = make_field_actor_behavior_state(11U, 0xC000U, 15U);
    for (int frame = 0; frame < 15; ++frame) {
        const auto update = advance_field_actor_behavior(pedestrian);
        if (update.transition_started || update.audio_event.has_value()) {
            return 10;
        }
    }
    const auto pedestrian_started =
        advance_field_actor_behavior(pedestrian);
    if (!pedestrian_started.transition_started ||
        pedestrian_started.audio_event != 0x002FU ||
        pedestrian.direction != 6U || pedestrian.animation_frame != 0U) {
        return 11;
    }

    return 0;
}

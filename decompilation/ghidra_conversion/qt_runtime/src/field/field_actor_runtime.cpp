#include "field_actor_runtime.hpp"

#include <array>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::array<std::uint16_t, 8> kIdleAnimationSelectors{
    0x0000U, 0x0004U, 0x0008U, 0x000CU,
    0x0010U, 0x0014U, 0x0018U, 0x001CU};
constexpr std::array<std::uint16_t, 8> kEntryAnimationSelectors{
    0x00E8U, 0x00ECU, 0x00F0U, 0x00F4U,
    0x00F8U, 0x00FCU, 0x0100U, 0x0104U};
constexpr std::int32_t kEntryAnimationPeriod = 0x1000;
constexpr std::int32_t kEntryAnimationStep = 0x0200;
constexpr std::uint8_t kEntryAnimationFrameCount = 2U;
constexpr std::uint8_t kBehaviorTimerReload = 20U;

} // namespace

FieldActorBehaviorState make_field_actor_behavior_state(
    const std::uint16_t entity_kind,
    const std::uint16_t heading,
    const std::uint8_t initial_delay) {
    FieldActorBehaviorState state;
    state.entity_kind = entity_kind;
    state.heading = heading;
    state.direction = field_actor_direction_from_heading(heading);
    state.behavior_timer = static_cast<std::uint8_t>(initial_delay & 0x0FU);
    return state;
}

std::uint8_t field_actor_direction_from_heading(const std::uint16_t heading) {
    // The B45C continuation rotates the heading three bits, normalizes a
    // negative result, then takes the low three bits for C052.
    auto rotated = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(heading << 3U) | (heading >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        rotated = static_cast<std::uint16_t>(rotated + 1U);
    }
    return static_cast<std::uint8_t>(rotated & 0x0007U);
}

std::uint16_t field_actor_idle_animation_selector(
    const std::uint8_t direction) {
    return kIdleAnimationSelectors[direction & 0x07U];
}

std::uint16_t field_actor_entry_animation_selector(
    const std::uint8_t direction) {
    return kEntryAnimationSelectors[direction & 0x07U];
}

FieldActorBehaviorUpdate advance_field_actor_behavior(
    FieldActorBehaviorState& state, const bool start_entry_animation,
    const std::optional<std::uint16_t> entry_audio_event) {
    FieldActorBehaviorUpdate update;
    bool just_started = false;

    if (!state.transition_started) {
        // The field service subtracts the +0xB6 timer before testing its
        // sign, so an initial value of zero activates on the first update.
        if (state.behavior_timer == 0U && start_entry_animation) {
            state.transition_started = true;
            state.animation_phase = FieldActorAnimationPhase::entry_transition;
            state.behavior_timer = kBehaviorTimerReload;
            state.animation_frame = 0U;
            // B330 resets the accumulator; the same frame's descriptor pass
            // applies the previous 0xFFF0 idle increment before 600E installs
            // the transition list's 0x0200 increment.
            state.animation_accumulator = -16;
            update.transition_started = true;
            update.audio_event = entry_audio_event;
            just_started = true;
        } else {
            --state.behavior_timer;
        }
    } else if (!state.transition_complete) {
        if (state.behavior_timer == 0U) {
            state.behavior_timer = kBehaviorTimerReload;
            if (state.transition_frame_list_finished) {
                state.transition_complete = true;
                update.transition_complete = true;
            }
        } else {
            --state.behavior_timer;
        }
    }

    if (state.animation_phase == FieldActorAnimationPhase::entry_transition &&
        !just_started && !state.transition_frame_list_finished) {
        const auto accumulated =
            static_cast<std::int32_t>(state.animation_accumulator) +
            kEntryAnimationStep;
        if (accumulated >= kEntryAnimationPeriod) {
            state.animation_accumulator = static_cast<std::int16_t>(
                accumulated - kEntryAnimationPeriod);
            if (state.animation_frame + 1U < kEntryAnimationFrameCount) {
                ++state.animation_frame;
            } else {
                // The source sets the selector's high bit and holds the final
                // frame until the next 21-update behavior-service gate.
                state.animation_frame = kEntryAnimationFrameCount - 1U;
                state.transition_frame_list_finished = true;
            }
        } else {
            state.animation_accumulator =
                static_cast<std::int16_t>(accumulated);
        }
    }

    return update;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

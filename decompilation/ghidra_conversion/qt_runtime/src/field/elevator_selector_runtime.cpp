#include "elevator_selector_runtime.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

bool button_pressed(const std::uint16_t controller_events,
                    const ElevatorSelectorButton button) noexcept {
    return (controller_events & elevator_selector_button_mask(button)) != 0U;
}

} // namespace

bool ElevatorSelectorRuntime::open(
    const std::uint8_t floor_count,
    const std::uint8_t entry_floor) noexcept {
    if (floor_count < 2U || floor_count > kElevatorSelectorMaximumFloors ||
        entry_floor >= floor_count) {
        return false;
    }
    floor_count_ = floor_count;
    entry_floor_ = entry_floor;
    current_floor_ = entry_floor;
    selected_floor_ = entry_floor;
    animation_frame_ = 0U;
    animation_frame_ticks_ = 0U;
    animation_sequence_index_ = 0U;
    navigation_feedback_frames_remaining_ = 0U;
    navigation_feedback_ = ElevatorSelectorNavigationFeedback::none;
    animation_direction_ = 0;
    animating_ = false;
    open_ = true;
    return true;
}

void ElevatorSelectorRuntime::begin_floor_step() noexcept {
    animation_direction_ = selected_floor_ > current_floor_ ? 1 : -1;
    animation_sequence_index_ = 0U;
    animation_frame_ = animation_direction_ > 0
                           ? static_cast<std::uint8_t>(
                                 kElevatorSelectorAnimationFrameCount - 1U)
                           : 0U;
    animation_frame_ticks_ = 0U;
    animating_ = true;
}

ElevatorSelectorUpdate ElevatorSelectorRuntime::advance(
    const std::uint16_t controller_events) noexcept {
    ElevatorSelectorUpdate update{};
    if (!open_) {
        return update;
    }

    if (navigation_feedback_frames_remaining_ != 0U) {
        --navigation_feedback_frames_remaining_;
        if (navigation_feedback_frames_remaining_ == 0U) {
            navigation_feedback_ =
                navigation_feedback_ ==
                        ElevatorSelectorNavigationFeedback::previous_floor_active
                    ? ElevatorSelectorNavigationFeedback::previous_floor_rest
                    : ElevatorSelectorNavigationFeedback::next_floor_rest;
            update.navigation_feedback_finished = true;
        }
        return update;
    }

    if (animating_) {
        if (animation_frame_ticks_ + 1U <
            kElevatorSelectorAnimationFrameDelay) {
            ++animation_frame_ticks_;
            return update;
        }
        animation_frame_ticks_ = 0U;
        ++animation_sequence_index_;
        if (animation_sequence_index_ <
            kElevatorSelectorAnimationFrameCount) {
            animation_frame_ = animation_direction_ > 0
                                   ? static_cast<std::uint8_t>(
                                         kElevatorSelectorAnimationFrameCount -
                                         1U - animation_sequence_index_)
                                   : animation_sequence_index_;
            update.animation_frame_changed = true;
            return update;
        }

        current_floor_ = static_cast<std::uint8_t>(
            static_cast<std::int16_t>(current_floor_) +
            animation_direction_);
        if (current_floor_ == selected_floor_) {
            animation_frame_ = 0U;
            animation_direction_ = 0;
            animating_ = false;
            update.animation_finished = true;
        } else {
            begin_floor_step();
            update.animation_frame_changed = true;
        }
        return update;
    }

    if (button_pressed(controller_events,
                       ElevatorSelectorButton::previous_floor)) {
        if (selected_floor_ > 0U) {
            --selected_floor_;
            update.selection_moved = true;
        }
        navigation_feedback_ =
            ElevatorSelectorNavigationFeedback::previous_floor_active;
        navigation_feedback_frames_remaining_ =
            kElevatorSelectorNavigationFeedbackFrameCount;
        update.navigation_feedback_started = true;
        return update;
    }
    if (button_pressed(controller_events,
                       ElevatorSelectorButton::next_floor)) {
        if (selected_floor_ + 1U < floor_count_) {
            ++selected_floor_;
            update.selection_moved = true;
        }
        navigation_feedback_ =
            ElevatorSelectorNavigationFeedback::next_floor_active;
        navigation_feedback_frames_remaining_ =
            kElevatorSelectorNavigationFeedbackFrameCount;
        update.navigation_feedback_started = true;
        return update;
    }
    if (button_pressed(controller_events, ElevatorSelectorButton::cancel)) {
        update.sound_effect_requested = true;
    }
    if (button_pressed(controller_events, ElevatorSelectorButton::confirm) &&
        selected_floor_ != current_floor_) {
        begin_floor_step();
        update.animation_started = true;
        return update;
    }
    if (button_pressed(controller_events, ElevatorSelectorButton::start)) {
        update.confirmed = true;
    }
    return update;
}

bool ElevatorSceneTransitionRuntime::begin_fade_out() noexcept {
    if (phase_ != ElevatorSceneTransitionPhase::idle) {
        return false;
    }
    phase_ = ElevatorSceneTransitionPhase::fading_to_black;
    palette_step_ = 0U;
    vblanks_in_palette_step_ = 0U;
    return true;
}

bool ElevatorSceneTransitionRuntime::begin_fade_in() noexcept {
    if (phase_ != ElevatorSceneTransitionPhase::destination_handoff) {
        return false;
    }
    phase_ = ElevatorSceneTransitionPhase::fading_from_black;
    palette_step_ = 0U;
    vblanks_in_palette_step_ = 0U;
    return true;
}

void ElevatorSceneTransitionRuntime::cancel() noexcept {
    phase_ = ElevatorSceneTransitionPhase::idle;
    palette_step_ = 0U;
    vblanks_in_palette_step_ = 0U;
}

ElevatorSceneTransitionUpdate ElevatorSceneTransitionRuntime::advance()
    noexcept {
    ElevatorSceneTransitionUpdate update{};
    if (phase_ != ElevatorSceneTransitionPhase::fading_to_black &&
        phase_ != ElevatorSceneTransitionPhase::fading_from_black) {
        return update;
    }
    ++vblanks_in_palette_step_;
    if (vblanks_in_palette_step_ <
        kElevatorSceneTransitionPaletteStepVblanks) {
        return update;
    }
    vblanks_in_palette_step_ = 0U;
    ++palette_step_;
    update.palette_step_changed = true;
    if (palette_step_ < kElevatorSceneTransitionPaletteStepCount) {
        return update;
    }
    if (phase_ == ElevatorSceneTransitionPhase::fading_to_black) {
        phase_ = ElevatorSceneTransitionPhase::destination_handoff;
        update.destination_load_requested = true;
    } else {
        phase_ = ElevatorSceneTransitionPhase::idle;
        update.completed = true;
    }
    return update;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

#include "elevator_selector_runtime.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kPreviousFloor = 0x0200U;
constexpr std::uint16_t kNextFloor = 0x0100U;
constexpr std::uint16_t kB = 0x2000U;
constexpr std::uint16_t kA = 0x4000U;
constexpr std::uint16_t kStart = 0x8000U;

} // namespace

bool ElevatorSelectorRuntime::open(
    const std::array<std::uint16_t, kElevatorSelectorMaximumFloors>& scene_ids,
    const std::uint8_t floor_count,
    const std::uint8_t entry_floor) noexcept {
    if (floor_count < 2U || floor_count > scene_ids.size() ||
        entry_floor >= floor_count) {
        return false;
    }
    scene_ids_ = scene_ids;
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

    if ((controller_events & kPreviousFloor) != 0U) {
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
    if ((controller_events & kNextFloor) != 0U) {
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
    if ((controller_events & kB) != 0U) {
        update.sound_effect_requested = true;
    }
    if ((controller_events & kA) != 0U &&
        selected_floor_ != current_floor_) {
        begin_floor_step();
        update.animation_started = true;
        return update;
    }
    if ((controller_events & kStart) != 0U) {
        update.confirmed = true;
    }
    return update;
}

std::uint16_t ElevatorSelectorRuntime::selected_scene_id() const noexcept {
    if (floor_count_ == 0U || selected_floor_ >= floor_count_) {
        return 0U;
    }
    return scene_ids_[selected_floor_];
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

std::uint8_t
ElevatorSceneTransitionRuntime::black_overlay_alpha() const noexcept {
    switch (phase_) {
    case ElevatorSceneTransitionPhase::fading_to_black:
        return static_cast<std::uint8_t>(
            (static_cast<unsigned>(palette_step_) * 255U) /
            kElevatorSceneTransitionPaletteStepCount);
    case ElevatorSceneTransitionPhase::destination_handoff:
        return 255U;
    case ElevatorSceneTransitionPhase::fading_from_black:
        return static_cast<std::uint8_t>(
            255U - (static_cast<unsigned>(palette_step_) * 255U) /
                      kElevatorSceneTransitionPaletteStepCount);
    case ElevatorSceneTransitionPhase::idle:
        return 0U;
    }
    return 0U;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

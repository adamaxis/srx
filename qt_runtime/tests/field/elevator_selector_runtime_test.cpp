#include "elevator_selector_runtime.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    ElevatorSelectorRuntime selector;
    if (!selector.open(3U, 1U) ||
        selector.entry_floor() != 1U || selector.current_floor() != 1U ||
        selector.selected_floor() != 1U) {
        return 1;
    }

    auto update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::next_floor));
    if (!update.selection_moved || selector.selected_floor() != 2U ||
        !update.navigation_feedback_started ||
        selector.navigation_feedback() !=
            ElevatorSelectorNavigationFeedback::next_floor_active ||
        selector.navigation_feedback_frames_remaining() !=
            kElevatorSelectorNavigationFeedbackFrameCount) {
        return 2;
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::confirm));
    if (update.animation_started || selector.is_animating() ||
        selector.navigation_feedback() !=
            ElevatorSelectorNavigationFeedback::next_floor_active) {
        return 3;
    }
    for (std::uint8_t tick = 0U; tick < 6U; ++tick) {
        update = selector.advance(0U);
        if (update.navigation_feedback_finished ||
            selector.navigation_feedback() !=
                ElevatorSelectorNavigationFeedback::next_floor_active) {
            return 4;
        }
    }
    update = selector.advance(0U);
    if (!update.navigation_feedback_finished ||
        selector.navigation_feedback() !=
            ElevatorSelectorNavigationFeedback::next_floor_rest) {
        return 5;
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::start));
    if (!update.confirmed || selector.current_floor() != 1U ||
        selector.selected_floor() != 2U || selector.destination_changed()) {
        return 23;
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::confirm));
    if (!update.animation_started || !selector.is_animating() ||
        selector.animation_frame() != 16U) {
        return 6;
    }
    for (std::uint8_t tick = 0U; tick < 7U; ++tick) {
        update = selector.advance(0U);
        if (update.animation_frame_changed ||
            selector.animation_frame() != 16U) {
            return 7;
        }
    }
    update = selector.advance(0U);
    if (!update.animation_frame_changed || selector.animation_frame() != 15U) {
        return 8;
    }
    for (std::size_t tick = 8U;
         tick < kElevatorSelectorAnimationFrameCount *
                   kElevatorSelectorAnimationFrameDelay;
         ++tick) {
        update = selector.advance(0U);
    }
    if (!update.animation_finished || selector.is_animating() ||
        selector.current_floor() != 2U || selector.selected_floor() != 2U ||
        !selector.destination_changed()) {
        return 9;
    }

    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::previous_floor));
    if (!update.selection_moved || selector.selected_floor() != 1U ||
        selector.navigation_feedback() !=
            ElevatorSelectorNavigationFeedback::previous_floor_active) {
        return 10;
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::cancel));
    if (update.sound_effect_requested ||
        selector.navigation_feedback() !=
            ElevatorSelectorNavigationFeedback::previous_floor_active) {
        return 11;
    }
    for (std::uint8_t tick = 1U;
         tick < kElevatorSelectorNavigationFeedbackFrameCount; ++tick) {
        update = selector.advance(0U);
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::cancel));
    if (!update.sound_effect_requested || !selector.is_open()) {
        return 12;
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::next_floor));
    if (!update.selection_moved || selector.selected_floor() != 2U) {
        return 13;
    }
    for (std::uint8_t tick = 0U;
         tick < kElevatorSelectorNavigationFeedbackFrameCount; ++tick) {
        selector.advance(0U);
    }
    update = selector.advance(elevator_selector_button_mask(
        ElevatorSelectorButton::start));
    if (!update.confirmed || !selector.is_open() ||
        !selector.destination_changed()) {
        return 14;
    }
    selector.close();

    if (selector.open(1U, 0U) || selector.open(6U, 0U)) {
        return 15;
    }
    ElevatorSceneTransitionRuntime transition;
    if (transition.is_active() || transition.black_overlay_alpha() != 0U ||
        !transition.begin_fade_out() || !transition.is_active() ||
        transition.begin_fade_in()) {
        return 16;
    }
    for (std::uint8_t pass = 1U;
         pass <= kElevatorSceneTransitionPaletteStepCount; ++pass) {
        auto fade_update = transition.advance();
        if (fade_update.palette_step_changed ||
            transition.palette_step() != pass - 1U) {
            return 17;
        }
        fade_update = transition.advance();
        const auto expected_alpha = static_cast<std::uint8_t>(
            (static_cast<unsigned>(pass) * 255U) /
            kElevatorSceneTransitionPaletteStepCount);
        if (!fade_update.palette_step_changed ||
            fade_update.destination_load_requested !=
                (pass == kElevatorSceneTransitionPaletteStepCount) ||
            fade_update.completed || transition.palette_step() != pass ||
            transition.black_overlay_alpha() != expected_alpha) {
            return 18;
        }
    }
    if (transition.phase() !=
            ElevatorSceneTransitionPhase::destination_handoff ||
        !transition.begin_fade_in()) {
        return 19;
    }
    for (std::uint8_t pass = 1U;
         pass <= kElevatorSceneTransitionPaletteStepCount; ++pass) {
        auto fade_update = transition.advance();
        if (fade_update.palette_step_changed) {
            return 20;
        }
        fade_update = transition.advance();
        const auto expected_alpha = static_cast<std::uint8_t>(
            255U - (static_cast<unsigned>(pass) * 255U) /
                       kElevatorSceneTransitionPaletteStepCount);
        if (!fade_update.palette_step_changed ||
            fade_update.destination_load_requested ||
            fade_update.completed !=
                (pass == kElevatorSceneTransitionPaletteStepCount) ||
            transition.black_overlay_alpha() != expected_alpha) {
            return 21;
        }
    }
    if (transition.is_active() || transition.black_overlay_alpha() != 0U ||
        transition.begin_fade_in()) {
        return 22;
    }
    return 0;
}

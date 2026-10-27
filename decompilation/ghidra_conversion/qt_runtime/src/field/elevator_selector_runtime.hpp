#pragma once

#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

constexpr std::size_t kElevatorSelectorMaximumFloors = 6U;
constexpr std::size_t kElevatorSelectorAnimationFrameCount = 17U;
constexpr std::uint8_t kElevatorSelectorAnimationFrameDelay = 8U;
// ROM 0x58F14 sets D5 to 8 before 0xC24. That routine waits for VBlank and
// DBF-loops, so the active arrow interval is nine VBlanks; 0x58E5A waits once
// more before polling the controller again.
constexpr std::uint8_t kElevatorSelectorNavigationFeedbackFrameCount = 9U;
// ROM 0x131C / 0x12F6 loop d6 from 0x10 through zero: 17 calls to 0x1088 /
// 0x10D6, with each call spaced by two VBlanks at 0x1342 [C].
constexpr std::uint8_t kElevatorSceneTransitionPaletteStepCount = 17U;
constexpr std::uint8_t kElevatorSceneTransitionPaletteStepVblanks = 2U;

enum class ElevatorSelectorButton : std::uint16_t {
    previous_floor = 0x0200U,
    next_floor = 0x0100U,
    cancel = 0x2000U,
    confirm = 0x4000U,
    start = 0x8000U,
};

constexpr std::uint16_t elevator_selector_button_mask(
    const ElevatorSelectorButton button) noexcept {
    return static_cast<std::uint16_t>(button);
}

enum class ElevatorSelectorNavigationFeedback : std::uint8_t {
    none,
    previous_floor_active,
    previous_floor_rest,
    next_floor_active,
    next_floor_rest,
};

struct ElevatorSelectorUpdate {
    bool selection_moved = false;
    bool navigation_feedback_started = false;
    bool navigation_feedback_finished = false;
    bool animation_started = false;
    bool animation_frame_changed = false;
    bool current_floor_changed = false;
    bool animation_finished = false;
    bool sound_effect_requested = false;
    bool confirmed = false;
};

// Native presentation state for the elevator floor selector. Lua resolves
// the ROM-derived child scene list and destination; this class owns only
// menu selection and the source-timed lift animation.
class ElevatorSelectorRuntime final {
public:
    bool open(std::uint8_t floor_count, std::uint8_t entry_floor) noexcept;
    ElevatorSelectorUpdate advance(std::uint16_t controller_events) noexcept;

    bool is_open() const noexcept { return open_; }
    void close() noexcept { open_ = false; }
    bool is_animating() const noexcept { return animating_; }
    std::uint8_t floor_count() const noexcept { return floor_count_; }
    std::uint8_t entry_floor() const noexcept { return entry_floor_; }
    std::uint8_t current_floor() const noexcept { return current_floor_; }
    std::uint8_t selected_floor() const noexcept { return selected_floor_; }
    std::uint8_t animation_frame() const noexcept { return animation_frame_; }
    ElevatorSelectorNavigationFeedback navigation_feedback() const noexcept {
        return navigation_feedback_;
    }
    std::uint8_t navigation_feedback_frames_remaining() const noexcept {
        return navigation_feedback_frames_remaining_;
    }
    bool destination_changed() const noexcept {
        // ROM 0x58D94 compares the selector's saved floor with FFF0C8 after
        // 0x58DE0 returns. FFF0CA is only the highlighted choice; A must run
        // the lift animation before FFF0C8 changes and a destination is due.
        return current_floor_ != entry_floor_;
    }

private:
    void begin_floor_step() noexcept;

    std::uint8_t floor_count_ = 0U;
    std::uint8_t entry_floor_ = 0U;
    std::uint8_t current_floor_ = 0U;
    std::uint8_t selected_floor_ = 0U;
    std::uint8_t animation_frame_ = 0U;
    std::uint8_t animation_frame_ticks_ = 0U;
    std::uint8_t animation_sequence_index_ = 0U;
    std::uint8_t navigation_feedback_frames_remaining_ = 0U;
    ElevatorSelectorNavigationFeedback navigation_feedback_ =
        ElevatorSelectorNavigationFeedback::none;
    std::int8_t animation_direction_ = 0;
    bool animating_ = false;
    bool open_ = false;
};

enum class ElevatorSceneTransitionPhase : std::uint8_t {
    idle,
    fading_to_black,
    destination_handoff,
    fading_from_black,
};

struct ElevatorSceneTransitionUpdate {
    bool palette_step_changed = false;
    bool destination_load_requested = false;
    bool completed = false;
};

// The source scene handoff fades 17 palette passes toward black (0x131C),
// reloads the destination scene (0x2BD2/0x16A8E), then fades toward its target
// palette in 17 passes (0x12F6). Each pass waits two VBlanks via 0x1342.
class ElevatorSceneTransitionRuntime final {
public:
    bool begin_fade_out() noexcept;
    bool begin_fade_in() noexcept;
    void cancel() noexcept;
    ElevatorSceneTransitionUpdate advance() noexcept;

    ElevatorSceneTransitionPhase phase() const noexcept { return phase_; }
    std::uint8_t palette_step() const noexcept { return palette_step_; }
    std::uint8_t vblanks_in_palette_step() const noexcept {
        return vblanks_in_palette_step_;
    }
    bool is_active() const noexcept {
        return phase_ != ElevatorSceneTransitionPhase::idle;
    }

private:
    ElevatorSceneTransitionPhase phase_ = ElevatorSceneTransitionPhase::idle;
    std::uint8_t palette_step_ = 0U;
    std::uint8_t vblanks_in_palette_step_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

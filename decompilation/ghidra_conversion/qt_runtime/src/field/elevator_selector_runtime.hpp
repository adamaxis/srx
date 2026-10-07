#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

constexpr std::size_t kElevatorSelectorMaximumFloors = 5U;
constexpr std::size_t kElevatorSelectorAnimationFrameCount = 17U;
constexpr std::uint8_t kElevatorSelectorAnimationFrameDelay = 8U;
constexpr std::uint8_t kElevatorSelectorNavigationFeedbackFrameCount = 8U;
constexpr std::uint8_t kElevatorSceneTransitionPaletteStepCount = 17U;
constexpr std::uint8_t kElevatorSceneTransitionPaletteStepVblanks = 2U;

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
    bool animation_finished = false;
    bool sound_effect_requested = false;
    bool confirmed = false;
};

// Native state for the ROM's event-29 elevator screen. The ROM's child scene
// pointers are resolved into this list by offline extraction; this class
// owns only menu selection and the source-timed 17-frame lift animation.
class ElevatorSelectorRuntime final {
public:
    bool open(const std::array<std::uint16_t,
                               kElevatorSelectorMaximumFloors>& scene_ids,
              std::uint8_t floor_count, std::uint8_t entry_floor) noexcept;
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
    std::uint16_t selected_scene_id() const noexcept;
    bool destination_changed() const noexcept {
        return selected_floor_ != entry_floor_;
    }

private:
    void begin_floor_step() noexcept;

    std::array<std::uint16_t, kElevatorSelectorMaximumFloors> scene_ids_{};
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
    std::uint8_t black_overlay_alpha() const noexcept;
    bool is_active() const noexcept {
        return phase_ != ElevatorSceneTransitionPhase::idle;
    }

private:
    ElevatorSceneTransitionPhase phase_ = ElevatorSceneTransitionPhase::idle;
    std::uint8_t palette_step_ = 0U;
    std::uint8_t vblanks_in_palette_step_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

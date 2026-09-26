#pragma once

#include "shadowrun_ghidra/title_intro/title_intro_late_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::uint16_t kTitleIntroStageLoopLastFrame = 0x0528U;
constexpr std::uint16_t kTitleIntroStageFinalTextFrame = 0x01E0U;

struct TitleIntroStageLoopState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t loop_frame = 0;
    TitleIntroScrollState scroll{};
    TitleIntroTripletState triplet{};
    TitleIntroAnimationServiceState animation_service{};
    TitleIntroPulseState pulse{};
    TitleIntroLandingState landing{};
};

struct TitleIntroStageLoopCallbacks {
    std::function<void()> clear_display_buffer;
    std::function<void()> prepare_title_resources;
    std::function<void()> render_middle_text;
    std::function<void()> restore_scene_records;
    std::function<void()> sample_controller;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> render_final_text;
    std::function<void()> render_transition_text;
    std::function<void(const TitleIntroScrollFillRequest&)> fill_transition_tiles;
    std::function<void(const TitleIntroSpriteRequest&)> draw_sprite;
    std::function<bool()> stop_requested;
};

enum class TitleIntroStageLoopStopReason {
    source_complete,
    external_stop,
};

// ROM 0x5AD1A: native orchestration for the late title stage. The helper
// implementations own typed animation state; this function owns only their
// source order and frame boundaries.
TitleIntroStageLoopStopReason run_title_intro_stage_loop(
    TitleIntroStageLoopState& state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>&
        pulse_descriptors,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        landing_phases,
    std::uint32_t special_landing_descriptor,
    const TitleIntroStageLoopCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

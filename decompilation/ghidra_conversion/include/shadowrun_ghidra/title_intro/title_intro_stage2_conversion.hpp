#pragma once

#include "shadowrun_ghidra/title_intro/title_intro_schedule_conversion.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x59D7C starts D7 at 0x01F0 and tests for 0x0164 after each loop body.
// The equivalent zero-based elapsed index in the host's ascending state is
// therefore 0x01F0 - 0x0164, not the literal countdown value.
constexpr std::uint16_t kTitleIntroStage2StoryTextElapsedFrameIndex =
    0x01F0U - 0x0164U;

struct TitleIntroStage2State {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t first_loop_frame = 0;
    std::uint16_t second_loop_frame = 0;
    std::int16_t x_offset = -0x0040;
    std::int16_t y_offset = -0x0020;
    std::uint16_t x_phase = 0x0138;
    std::uint16_t y_phase = 0x00F1;
    std::uint8_t dirty_flags = 0;
};

struct TitleIntroStage2Callbacks {
    std::function<void()> load_title_font;
    std::function<void()> prepare_title_resources;
    std::function<void()> initialize_scene_mode;
    std::function<void()> render_scene_entities;
    std::function<void()> advance_palette_fade;
    std::function<void()> wait_for_vblank;
    std::function<void()> sample_controller;
    std::function<void()> latch_controller_events;
    std::function<void(std::uint32_t)> run_scheduled_action;
    std::function<void()> reset_overlay;
    std::function<void()> reset_sequence;
    std::function<void()> reset_persistent_sequence;
    std::function<void()> reset_motion;
    std::function<void()> reset_dual_sprites;
    std::function<void()> reset_timed_sprites;
    std::function<void()> reset_targeted_sweep;
    std::function<void()> reset_linear_sweep_a;
    std::function<void()> reset_linear_sweep_b;
    std::function<void()> reset_final_projection;
    std::function<void()> initialize_ambush;
    std::function<void()> advance_overlay;
    std::function<void()> advance_sequence;
    std::function<void()> advance_ambush;
    std::function<void()> render_scroll_text;
    std::function<void()> render_late_text;
    std::function<bool()> stop_requested;
};

enum class TitleIntroStage2StopReason {
    source_complete,
    external_stop,
};

// ROM 0x59D7C: run the stage-2 scene-entry transition and the subsequent
// layered intro loop. Schedule entries are host action IDs, not ROM pointers.
TitleIntroStage2StopReason run_title_intro_stage2(
    TitleIntroStage2State& state,
    TitleIntroScheduleState& schedule_state,
    const std::vector<TitleIntroScheduleEntry>& schedule_entries,
    const TitleIntroStage2Callbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct TitleIntroScheduleEntry {
    std::uint16_t delay_frames = 0;
    std::uint32_t action_id = 0;
};

struct TitleIntroScheduleState {
    std::int16_t x_offset = 0;
    std::int16_t x_velocity = 0;
    std::int16_t y_offset = 0;
    std::int16_t fade_counter = 0;
    std::uint8_t dirty_flags = 0;
    std::size_t current_entry = 0;
    std::int32_t countdown = 0;
};

// ROM 0x5AF2A: consume one schedule tick. An entry's callback is invoked only
// after its countdown crosses below zero; a zero delay in the newly loaded
// entry is the source terminal marker and is not invoked.
bool advance_title_intro_schedule(
    TitleIntroScheduleState& state,
    const std::vector<TitleIntroScheduleEntry>& entries,
    const std::function<void(std::uint32_t)>& run_action);

struct TitleIntroScheduleCallbacks {
    std::function<void()> clear_display_buffer;
    std::function<void()> prepare_title_resources;
    std::function<void()> load_title_font;
    std::function<void()> restore_scene_records;
    std::function<void()> sample_controller;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<bool()> stop_requested;
    std::function<void(std::uint32_t)> run_action;
};

enum class TitleIntroScheduleStopReason {
    schedule_complete,
    external_stop,
    frame_budget,
};

// ROM 0x5AF62: initialize the title-transition state, restore its working
// records, then run the source-ordered vblank/controller/schedule loop.
TitleIntroScheduleStopReason run_title_intro_schedule(
    TitleIntroScheduleState& state,
    const std::vector<TitleIntroScheduleEntry>& entries,
    std::uint16_t frame_budget,
    const TitleIntroScheduleCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

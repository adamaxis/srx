#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroStagesState {
    std::uint8_t controller_marker = 0;
    std::uint8_t completion_mode = 0;
    std::uint16_t stop_marker = 0;
};

struct TitleIntroStagesCallbacks {
    std::function<void()> sample_controller;
    std::function<void()> service_audio;
    std::function<void()> run_schedule;
    std::function<void()> prepare_mode_entry;
    std::function<void()> run_stage2;
    std::function<void()> run_orchestrator;
    std::function<void()> run_stage_loop;
    std::function<void()> step_palette_transition;
    std::function<void()> enter_post_intro_mode;
};

enum class TitleIntroStagesStopReason {
    source_complete,
    interrupted,
};

// ROM 0x59C5E: coordinate the four title-intro stages. The callbacks own the
// typed implementations of each child; this wrapper owns only the source
// marker checks, preparation boundaries, and final transition order.
TitleIntroStagesStopReason run_title_intro_stages(
    TitleIntroStagesState& state,
    const TitleIntroStagesCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/title_intro/title_intro_stage_loop_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount> pulse{{
        0x1000U, 0x1001U, 0x1002U, 0x1003U, 0x1004U, 0x1005U}};
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>
        landing{{{0x2000U, 1U, 1U, 1U},
                 {0x2001U, 1U, 1U, 1U},
                 {0x2002U, 1U, 1U, 1U},
                 {0x2003U, 1U, 1U, 1U},
                 {0x2004U, 1U, 1U, 1U},
                 {0x2005U, 1U, 1U, 1U},
                 {0x2006U, 1U, 1U, 1U}}};

    TitleIntroStageLoopState state{};
    state.scroll.x_offset = -0x190;
    std::uint32_t frame_waits = 0;
    std::uint32_t final_text_calls = 0;
    std::uint16_t final_text_frame = 0xFFFFU;
    std::uint32_t middle_text_calls = 0;
    std::uint32_t fill_calls = 0;
    std::uint32_t transition_text_calls = 0;
    std::uint32_t sprite_calls = 0;
    TitleIntroStageLoopCallbacks callbacks{};
    callbacks.wait_for_vblank = [&] {
        ++frame_waits;
        ++state.current_frame;
    };
    callbacks.render_middle_text = [&] { ++middle_text_calls; };
    callbacks.render_final_text = [&] {
        final_text_frame = state.loop_frame;
        ++final_text_calls;
    };
    callbacks.fill_transition_tiles = [&](const auto&) { ++fill_calls; };
    callbacks.render_transition_text = [&] { ++transition_text_calls; };
    callbacks.draw_sprite = [&](const auto&) { ++sprite_calls; };
    const auto reason = run_title_intro_stage_loop(
        state, pulse, landing, 0x3000U, callbacks);
    if (reason != TitleIntroStageLoopStopReason::source_complete ||
        frame_waits != 0x529U || middle_text_calls != 1U ||
        final_text_calls != 1U || fill_calls != 1U ||
        transition_text_calls != 1U || sprite_calls == 0U ||
        final_text_frame != kTitleIntroStageFinalTextElapsedFrameIndex ||
        state.loop_frame != 0x529U) {
        return 1;
    }

    TitleIntroStageLoopState stopped{};
    std::uint32_t stopped_frames = 0;
    TitleIntroStageLoopCallbacks stop_callbacks{};
    stop_callbacks.wait_for_vblank = [&] { ++stopped_frames; };
    stop_callbacks.stop_requested = [&] { return stopped_frames == 2U; };
    if (run_title_intro_stage_loop(stopped, pulse, landing, 0x3000U,
                                   stop_callbacks) !=
            TitleIntroStageLoopStopReason::external_stop ||
        stopped_frames != 2U) {
        return 2;
    }
    return 0;
}

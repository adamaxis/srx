#pragma once

#include "shadowrun_ghidra/title_intro_overlay_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kTitleIntroSequencePhaseCount = 4;

struct TitleIntroSequenceState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t stage = 0;
};

struct TitleIntroSequencePhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t delay_frames = 0;
};

struct TitleIntroFinalOverlayRequest {
    std::uint16_t destination_word = 0;
    std::uint16_t columns = 0;
    std::uint16_t rows = 0;
};

struct TitleIntroSequenceCallbacks {
    std::function<void(const TitleIntroSpriteStreamRequest&)>
        render_sprite_stream;
    std::function<void(std::uint16_t, std::uint16_t)> run_stage_four_transition;
    std::function<void()> run_repeated_stage;
    std::function<void(std::uint16_t, std::uint16_t)> set_stage_nine_anchor;
    std::function<void(const TitleIntroFinalOverlayRequest&)>
        draw_final_overlay;
    std::function<void(std::uint16_t)> play_audio_event;
};

enum class TitleIntroSequenceAdvanceResult {
    no_op,
    phase_rendered,
    transition_rendered,
    repeated_stage_rendered,
    final_overlay_rendered,
};

// ROM 0x5A0A4: clear the next-event frame and stage index for the second
// title-intro sequence.
void reset_title_intro_sequence(TitleIntroSequenceState& state);

// ROM 0x5A0B2: advance the four-entry sprite lead-in, the stage-four
// transition, the repeated stage steps, and the final overlay. All source
// pointers and rendering leaves are replaced with host handles and callbacks.
TitleIntroSequenceAdvanceResult advance_title_intro_sequence(
    TitleIntroSequenceState& state,
    const std::array<TitleIntroSequencePhase, kTitleIntroSequencePhaseCount>&
        phases,
    const TitleIntroSequenceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kTitleIntroPersistentPhaseCount = 4;

struct TitleIntroPersistentSequenceState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t stage = 0;
    std::uint32_t current_sprite_descriptor = 0;
    std::uint16_t related_phase_marker = 0;
};

struct TitleIntroPersistentPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t delay_frames = 0;
};

struct TitleIntroPersistentSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct TitleIntroPersistentFinalOverlayRequest {
    std::uint16_t destination_word = 0;
    std::uint16_t columns = 0;
    std::uint16_t rows = 0;
};

struct TitleIntroPersistentCallbacks {
    std::function<void(std::uint16_t)> play_audio_event;
    std::function<void(std::uint16_t, std::uint16_t)>
        run_stage_four_transition;
    std::function<void()> run_repeated_stage;
    std::function<void(std::uint16_t, std::uint16_t)> set_stage_nine_anchor;
    std::function<void(const TitleIntroPersistentFinalOverlayRequest&)>
        draw_final_overlay;
    std::function<void(const TitleIntroPersistentSpriteRequest&)>
        draw_persistent_sprite;
};

enum class TitleIntroPersistentAdvanceResult {
    no_op,
    phase_advanced,
    transition_advanced,
    repeated_stage_advanced,
    final_overlay_advanced,
};

// ROM 0x5A244: reset the persistent intro-sequence deadline and stage.
void reset_title_intro_persistent_sequence(
    TitleIntroPersistentSequenceState& state);

// ROM 0x5A252: advance the four-entry persistent sequence, preserve its
// current sprite descriptor through the stage transitions, and redraw that
// descriptor at the source position on every active call.
TitleIntroPersistentAdvanceResult advance_title_intro_persistent_sequence(
    TitleIntroPersistentSequenceState& state,
    const std::array<TitleIntroPersistentPhase,
                     kTitleIntroPersistentPhaseCount>& phases,
    const TitleIntroPersistentCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

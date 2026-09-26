#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kTitleIntroOverlayPhaseCount = 6;

struct TitleIntroOverlaySchedule {
    std::uint16_t next_sprite_frame = 0;
    std::uint16_t phase_index = 0;
    std::uint16_t scroll_accumulator = 0;
};

struct TitleIntroOverlayState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t vertical_scroll = 0;
    std::uint8_t dirty_flags = 0;
    TitleIntroOverlaySchedule schedule{};
};

struct TitleIntroOverlayPhase {
    // Host asset handle; the source table's ROM pointer is intentionally not
    // part of the native conversion model.
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t delay_frames = 0;
};

struct TitleIntroTransitionSpriteRequest {
    std::uint16_t destination_word = 0;
    std::uint16_t columns = 0;
    std::uint16_t rows = 0;
};

struct TitleIntroSpriteStreamRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t destination_word = 0;
    std::uint16_t columns = 0;
    std::uint16_t rows = 0;
    std::uint16_t tile_flags = 0;
    std::uint16_t row_delta = 0;
    std::uint16_t stream_stride = 0;
};

struct TitleIntroOverlayCallbacks {
    std::function<void(std::uint16_t)> play_audio_event;
    std::function<void()> write_transfer_header;
    std::function<void(const TitleIntroTransitionSpriteRequest&)>
        draw_transition_sprite;
    std::function<void(const TitleIntroSpriteStreamRequest&)>
        render_sprite_stream;
};

enum class TitleIntroOverlayAdvanceResult {
    no_op,
    phase_rendered,
    final_overlay_rendered,
};

// ROM 0x59F7A: reset the title-intro overlay's next-frame, phase, and scroll
// accumulators before the post-title overlay loop starts.
void reset_title_intro_overlay_schedule(TitleIntroOverlaySchedule& schedule);

// ROM 0x59F8E: advance the airport/title overlay choreography. The six phase
// descriptors and the final overlay descriptor are host asset handles; all
// presentation work remains explicit callbacks rather than VDP or ROM-pointer
// machinery.
TitleIntroOverlayAdvanceResult advance_title_intro_overlay(
    TitleIntroOverlayState& state,
    const std::array<TitleIntroOverlayPhase, kTitleIntroOverlayPhaseCount>&
        phases,
    std::uint32_t final_overlay_descriptor,
    const TitleIntroOverlayCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

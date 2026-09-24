#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroStageTextRequest {
    std::uint32_t text_asset = 0;
    std::uint16_t destination_word = 0;
    std::uint16_t palette_flags = 0;
};

constexpr std::size_t kTitleIntroStageTextCount = 3;

// ROM 0x5AA5A: submit the three secondary-intro text records in order.
void render_title_intro_stage_text_batch(
    const std::array<std::uint32_t, kTitleIntroStageTextCount>& text_assets,
    const std::function<void(const TitleIntroStageTextRequest&)>& render_text);

struct TitleIntroStageSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct TitleIntroStageProjectionState {
    std::uint16_t x_accumulator = 0;
    std::uint16_t y_accumulator = 0;
};

// ROM 0x5AC28: reset the moving-sprite accumulators.
void reset_title_intro_stage_projection(TitleIntroStageProjectionState& state);

// ROM 0x5AC0E: draw the fixed stage sprite at the source position and palette.
void draw_title_intro_stage_base_sprite(
    std::uint32_t sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite);

// ROM 0x5AC36: draw the moving stage sprite from the accumulator high bytes,
// then advance x by 0x40 and y by 0x18.
void advance_title_intro_stage_projection(
    TitleIntroStageProjectionState& state, std::uint32_t sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite);

struct TitleIntroStageTimedPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t delay_frames = 0;
};

constexpr std::size_t kTitleIntroStageTimedPhaseCount = 3;

struct TitleIntroStageTimedState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t current_sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct TitleIntroStageFrameState {
    std::uint16_t scene_mode = 0;
    bool entity_record_pending = false;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x5AC70: reset the three-entry timed-sprite sequence.
void reset_title_intro_stage_timed_sprites(TitleIntroStageTimedState& state);

// ROM 0x5AC7E: after the first 0x3C elapsed frames, select three descriptors
// at six-frame deadlines and continue drawing the current descriptor between
// deadlines with palette mask 0xE000.
void advance_title_intro_stage_timed_sprites(
    TitleIntroStageTimedState& state,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& phases,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite);

// ROM 0x5ABD2: publish stage mode, then draw timed/base/projection sprites in
// source order and mark the native entity record for presentation.
void service_title_intro_stage_frame(
    TitleIntroStageFrameState& frame_state,
    TitleIntroStageTimedState& timed_state,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& timed_phases,
    std::uint32_t base_sprite_descriptor,
    TitleIntroStageProjectionState& projection_state,
    std::uint32_t projection_sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite);

// ROM 0x5AB96: after frame 0xB4, on the source's masked cadence, invoke four
// ordered service steps. Returns whether the four-step batch ran.
bool advance_title_intro_stage_service_steps(
    std::uint16_t current_frame, std::uint16_t animation_start_frame,
    const std::function<void(std::uint16_t)>& service_step);

} // namespace shadowrun::ghidra_conversion

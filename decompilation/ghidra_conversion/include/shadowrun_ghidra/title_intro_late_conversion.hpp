#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroLateTextRenderRequest {
    std::uint32_t text_asset = 0;
    std::uint16_t destination_word = 0;
    std::uint16_t palette_flags = 0;
};

using TitleIntroLateTextRenderBoundary =
    std::function<void(const TitleIntroLateTextRenderRequest&)>;

constexpr std::size_t kTitleIntroThreeTextCount = 3;

// ROM 0x5A9E2: render the three middle-stage text assets.
void render_title_intro_middle_text_batch(
    const std::array<std::uint32_t, kTitleIntroThreeTextCount>& text_assets,
    const TitleIntroLateTextRenderBoundary& render_text);

// ROM 0x5AA1E: render one repeated asset into the three center rows.
void render_title_intro_center_text_batch(
    std::uint32_t text_asset,
    const TitleIntroLateTextRenderBoundary& render_text);

// ROM 0x5ADFE: render the three late-stage text assets.
void render_title_intro_late_text_batch(
    const std::array<std::uint32_t, kTitleIntroThreeTextCount>& text_assets,
    const TitleIntroLateTextRenderBoundary& render_text);

constexpr std::size_t kTitleIntroScrollTextCount = 5;

// ROM 0x5AE3A: render one repeated asset across five scroll rows.
void render_title_intro_scroll_text_batch(
    std::uint32_t text_asset,
    const TitleIntroLateTextRenderBoundary& render_text);

struct TitleIntroScrollFillRequest {
    std::uint16_t destination_word = 0;
    std::uint16_t words_per_row = 0;
    std::uint16_t row_count = 0;
    std::uint16_t row_stride_words = 0;
};

struct TitleIntroScrollState {
    std::int16_t x_offset = 0;
    bool transition_fill_started = false;
    std::uint16_t fade_counter = 0;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x5B002: clear the one-shot transition-fill latch.
void reset_title_intro_scroll_fill(TitleIntroScrollState& state);

// ROM 0x5B00A: move the title transition two words per frame and issue its
// one-shot eight-row fill when the source scroll reaches the threshold.
void advance_title_intro_scroll(
    TitleIntroScrollState& state,
    const std::function<void(const TitleIntroScrollFillRequest&)>& fill_tiles);

// ROM 0x5B056: start the transition text pass at x == -0x1B0, then advance
// its bounded fade counter while retaining the source dirty bit.
void advance_title_intro_scroll_fade(
    TitleIntroScrollState& state,
    const std::function<void()>& render_transition_text);

struct TitleIntroTripletState {
    std::array<std::uint16_t, 3> values{};
    bool dirty = false;
};

// ROM 0x5B08E: rotate the three-word stage triplet every eight source frames.
// The return value preserves the source's frame/rotated-value result.
std::uint16_t advance_title_intro_triplet(
    TitleIntroTripletState& state,
    std::uint16_t current_frame);

struct TitleIntroSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

struct TitleIntroPulseState {
    std::int16_t x_offset = 0;
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
};

constexpr std::size_t kTitleIntroPulsePhaseCount = 6;

// ROM 0x5B100: select a six-entry descriptor every six frames while the
// sprite is inside the transition window, then draw it at y == 0xEF.
void advance_title_intro_pulse(
    TitleIntroPulseState& state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>& descriptors,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite);

struct TitleIntroLandingPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x_delta = 0;
    std::uint16_t y_delta = 0;
    std::uint16_t delay_frames = 0;
};

constexpr std::size_t kTitleIntroLandingPhaseCount = 7;

struct TitleIntroLandingState {
    std::uint16_t current_frame = 0;
    std::uint16_t animation_start_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x_accumulator = 0x7C00U;
    std::uint16_t y_accumulator = 0xB700U;
};

// ROM 0x5B198: reset the late landing projection and phase schedule.
void reset_title_intro_landing(TitleIntroLandingState& state);

// ROM 0x5B1B6: wait for the late-intro lead-in, then advance seven movement
// phases and choose the special descriptor during the narrow landing window.
void advance_title_intro_landing(
    TitleIntroLandingState& state,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        phases,
    std::uint32_t special_landing_descriptor,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite);

struct TitleIntroAnimationServiceState {
    bool scene_animation_enabled = false;
    bool entity_record_pending = false;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x5B0BC: service the pulse and landing passes in source order and
// publish the small native scene-record state they update.
void service_title_intro_animation(
    TitleIntroAnimationServiceState& service_state,
    TitleIntroPulseState& pulse_state,
    const std::array<std::uint32_t, kTitleIntroPulsePhaseCount>&
        pulse_descriptors,
    TitleIntroLandingState& landing_state,
    const std::array<TitleIntroLandingPhase, kTitleIntroLandingPhaseCount>&
        landing_phases,
    std::uint32_t special_landing_descriptor,
    const std::function<void(const TitleIntroSpriteRequest&)>& draw_sprite);

struct TitleIntroEntityColorState {
    std::array<std::array<std::uint16_t, 15>, 4> words{};
    bool dirty = false;
};

// ROM 0x5B2C2: fill one entity's fifteen color words with a source value.
void set_title_intro_entity_color_words(TitleIntroEntityColorState& state,
                                        std::size_t entity_index,
                                        std::uint16_t value);

// ROM 0x5B2E2: lower each active color component by one source step.
void step_title_intro_entity_color_words(TitleIntroEntityColorState& state,
                                         std::size_t entity_index);

} // namespace shadowrun::ghidra_conversion

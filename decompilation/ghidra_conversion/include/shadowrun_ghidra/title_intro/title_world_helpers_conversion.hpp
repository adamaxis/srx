#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MatrixProgramTransitionState {
    std::uint16_t frame_countdown = 0; // FFE0DA
    std::uint16_t stage = 0;            // FFE0DC
    std::uint16_t selected_slot = 0;    // FFE0E4
    std::uint16_t resource_token = 0;   // FFE0D8
    std::uint16_t requested_mode = 0;   // FFE0E0
    bool mode_request_pending = false;  // FFE0DE
};

struct MatrixTransitionSpriteBuild {
    std::uint32_t source_table_token = 0x0001DB1CU;
    std::uint16_t stage = 0;
    std::uint16_t resource_token = 0;
    std::uint16_t position_y = 0x0147U;
    std::uint16_t build_flags = 0x2000U;
    std::uint32_t sprite_data_token = 0;
};

struct MatrixProgramTransitionCallbacks {
    std::function<void(std::uint16_t slot)> clear_program_slot;
    std::function<void()> refresh_program_grid;
    std::function<void()> refresh_mode_glyph;
    std::function<void()> render_menu_rows;
    std::function<void()> initialize_world_effect_mode;
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t stage)>
        resolve_sprite_data;
    std::function<void(const MatrixTransitionSpriteBuild&)> build_sprite;
};

// Typed extraction of the twelve pointer entries at ROM 0x1DB1C.  The
// transition reaches this table only for stages 0..0x0B.
std::optional<std::uint32_t> resolve_matrix_transition_sprite_data(
    std::uint16_t stage);

enum class MatrixProgramTransitionAction {
    rendered,
    finalized,
    sprite_data_unresolved,
};

// ROM 0x1B282: advance the Matrix transition every 0x0C frames, clear the
// selected slot at stage 6, and hand the completed transition back to mode
// initialization at stage 0x0C.
MatrixProgramTransitionAction advance_matrix_program_transition(
    MatrixProgramTransitionState& state,
    const MatrixProgramTransitionCallbacks& callbacks);

struct TitleFigureAnimationState {
    std::uint16_t frame_countdown = 0; // FFE0B0
    std::uint16_t phase = 0;            // FFE0AE
};

struct TitleFigureSpriteBuild {
    std::uint32_t source_table_token = 0;
    std::uint16_t sprite_key = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0;
    std::uint32_t sprite_data_token = 0;
};

struct TitleFigureAnimationCallbacks {
    std::function<void()> advance_support_animation;
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    std::function<void(const TitleFigureSpriteBuild&)> build_sprite;
};

// Typed extraction of the four five-entry pointer tables used by 0x1B364.
// The source advances each selected pointer by eight bytes before building.
std::optional<std::uint32_t> resolve_title_figure_sprite_data(
    std::uint32_t source_table_token, std::uint16_t phase);

enum class TitleFigureAnimationAction {
    rendered,
    sprite_data_unresolved,
};

// ROM 0x1B364: update the support animation, wrap its five-phase counter, and
// emit the two paired figure effects in source order.
TitleFigureAnimationAction advance_title_figure_animation(
    TitleFigureAnimationState& state,
    const TitleFigureAnimationCallbacks& callbacks);

struct TitleBurstAnimationState {
    std::uint16_t frame_countdown = 0; // FFE0B0
    std::uint16_t phase = 0;            // FFE0AE
    std::uint16_t position_x = 0;       // FFE04A
    std::uint16_t position_y = 0;       // FFE04C
};

struct TitleBurstSpriteBuild {
    std::uint32_t source_table_token = 0x0001D970U;
    std::uint16_t phase = 0;
    std::uint16_t sprite_key = 0x0780U;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x6000U;
    std::uint32_t sprite_data_token = 0;
};

struct TitleBurstAnimationCallbacks {
    // The source indexes the five-entry pointer table at ROM 0x1D970 and
    // passes the selected entry plus eight bytes to the sprite builder.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    std::function<void(const TitleBurstSpriteBuild&)> build_sprite;
    std::function<void()> clear_completion_marker;
};

enum class TitleBurstAnimationAction {
    rendered,
    completed,
    sprite_data_unresolved,
};

// ROM 0x1BA6E: run the four-frame title burst, appending sprite key 0x780
// at the shared title position. Phase four is the terminal path: it clears
// the source completion marker and emits no sprite for that tick.
TitleBurstAnimationAction advance_title_burst_animation(
    TitleBurstAnimationState& state,
    const TitleBurstAnimationCallbacks& callbacks);

struct TitleNoiseState {
    std::uint16_t frame_countdown = 0; // FFE0E8
    std::uint16_t frame_reload = 0;    // FFE0E6
    std::uint8_t remaining_frames = 0; // FF16A
    std::uint8_t record_flags = 0;     // behavior record +0x12
    std::uint8_t dirty_flags = 0;      // FFEF14
    std::vector<std::int16_t> cb42_wave;
    std::vector<std::int16_t> cb40_wave;
};

struct TitleNoiseCallbacks {
    std::function<std::optional<std::uint16_t>()> advance_source_prng;
    std::function<void()> finalize_menu_slot_code_six;
    std::function<void()> reset_word_stream_area;
    std::function<void()> write_word_stream;
};

enum class TitleNoiseAction {
    updated,
    finalized,
    prng_unresolved,
    waveform_unresolved,
};

// ROM 0x1B45C: update the FF CB42 and FF CB40 two-word-stride waves using the
// source PRNG's rotated sign bits and the ROM 0x1B592 limit table, then submit
// the stream and set FFEF14 bits 2 and 3.
TitleNoiseAction advance_title_noise_stream(
    TitleNoiseState& state,
    const TitleNoiseCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

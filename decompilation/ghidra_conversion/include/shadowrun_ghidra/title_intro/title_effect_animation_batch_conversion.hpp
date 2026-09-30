#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include "shadowrun_ghidra/title_intro/title_world_helpers_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void refresh_menu_selection_after_clear(
    bool has_pending_value,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text);

struct MenuSelectionRefreshState {
    // Signed source byte FFF0AA.
    std::int8_t pending_marker = 0;
};

struct MenuSelectionRefreshExactCallbacks {
    std::function<void()> clear_selected_menu_record;
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void()> prepare_menu_offset;
};

// ROM 0x1A09C: D0=9, clear the selected menu record only for a positive
// marker, and publish D1=6 or D1=0x0A before the text follow-up.
void refresh_menu_selection_after_clear(
    const MenuSelectionRefreshState& state,
    const MenuSelectionRefreshExactCallbacks& callbacks);

void refresh_menu_selection_or_prepare(
    bool has_pending_value,
    const std::function<void()>& prepare_offset,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text);

// ROM 0x1A106: D0=0x0A, prepare the offset for a non-positive marker or
// clear the selected menu record for a positive marker, then publish D1=0x0A
// or D1=6 and render the text follow-up.
void refresh_menu_selection_or_prepare(
    const MenuSelectionRefreshState& state,
    const MenuSelectionRefreshExactCallbacks& callbacks);

struct AudioFrameState {
    std::uint16_t field_a = 0;
    std::uint16_t field_b = 0;
    std::uint16_t field_c = 0;
};

void initialize_audio_frame(AudioFrameState& state,
                            const std::function<void()>& service_audio);

struct TitleAnimationCycleState {
    std::int16_t phase = 0;
    std::int16_t countdown = 0;
    bool active = true;
};

bool advance_title_animation_cycle(
    TitleAnimationCycleState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites);

// ROM 0x1A570 passes two different source sprite lists to the common
// 0x1378 builder.  Keep the source table and fixed-point origin visible at
// the conversion boundary instead of collapsing both calls into a counter.
struct TitleSpriteBatchBuildRequest {
    std::uint32_t source_table_token = 0;
    std::uint16_t base_x = 0;
    std::uint16_t base_y = 0;
    std::uint16_t build_flags = 0;
};

void build_title_sprites_twice(
    const std::function<void(const TitleSpriteBatchBuildRequest&)>&
        build_sprites);

struct TitleVerticalBuildState {
    std::uint16_t x = 0; // FFE078
    std::uint16_t y = 0; // FFE07A
    std::uint16_t render_flags = 0; // FFDF88
};

// ROM 0x1A7BE selects one of the four +8 title/world sprite lists, appends
// sprite record 0x6E0, and submits it with the attribute-offset builder.  The
// host resolves the source table from these typed fields; it does not read a
// ROM address at runtime.
struct TitleVerticalSpriteBuildRequest {
    std::uint32_t source_table_token = 0x000F1AEEU;
    std::uint8_t table_index = 0;
    std::uint16_t sprite_key = 0x06E0U;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0xA000U;
};

bool append_title_vertical_sprite(
    TitleVerticalBuildState& state,
    const std::function<void(std::uint16_t sprite_key)>& append_sprite,
    const std::function<void(const TitleVerticalSpriteBuildRequest&)>&
        build_sprites);

enum class MatrixProgramTransitionInitializationAction {
    initialized,
    resource_unresolved,
};

struct MatrixProgramTransitionInitializationCallbacks {
    // ROM 0x1B278 contains the five typed resource positions selected by D2.
    // The conversion resolves the selected program through host data rather
    // than indexing the ROM at runtime.
    std::function<std::optional<std::uint16_t>(std::uint16_t selected_slot)>
        resolve_resource_token;
    std::function<void()> initialize_world_effect_mode;
};

// Typed extraction of the five words at ROM 0x1B278.
std::optional<std::uint16_t> resolve_matrix_program_resource_position(
    std::uint16_t selected_slot);

// ROM 0x1B1C4: initialize the Matrix-program transition record from D1/D2,
// publish the selected resource position, and tail-call 0x1A908.  The
// original function was exported with the misleading generic name
// initialize_world_effect_seed; its writes are the FFE0D8..FFE0E4 Matrix
// transition fields, not a particle/effect seed.
MatrixProgramTransitionInitializationAction
initialize_matrix_program_transition(
    MatrixProgramTransitionState& state,
    std::uint16_t selected_slot,
    std::uint16_t requested_mode,
    const MatrixProgramTransitionInitializationCallbacks& callbacks);

struct TitleTransitionState {
    std::uint16_t world_effect_mode = 0; // FFE076
    bool title_phase_active = false;     // FFE0F6
    std::uint16_t phase_countdown = 0;   // FFE0FA
    std::uint16_t phase = 0;
};

void reset_title_transition(TitleTransitionState& state);

struct TitlePhaseCycleState {
    std::int16_t phase = 0;
    std::int16_t countdown = 0;
};

struct TitlePhaseSpriteBuildRequest {
    std::uint32_t source_table_token = 0x0001DB92U;
    std::uint16_t phase = 0;
    std::uint32_t sprite_data_token = 0;
    std::uint16_t sprite_key = 0x06E0U;
    std::uint16_t x = 0x010DU;
    std::uint16_t y = 0x012EU;
    std::uint16_t build_flags = 0x2000U;
};

struct TitlePhaseCycleCallbacks {
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    // ROM 0x1DB92 is a six-entry pointer table. The callback returns the
    // selected entry plus eight bytes, which is what 0x013BE receives.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(const TitlePhaseSpriteBuildRequest&)> build_sprite;
};

enum class TitlePhaseCycleAction {
    rendered,
    sprite_data_unresolved,
};

TitlePhaseCycleAction advance_title_phase_cycle(
    TitlePhaseCycleState& state,
    const TitlePhaseCycleCallbacks& callbacks);

struct WorldEffectMotionSeedState {
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
    std::int16_t horizontal_velocity = 0;
    std::int16_t lifetime = 0;
    std::int16_t palette = 0;
    std::int16_t variant = 0;
    bool active = false;
};

struct TitleJitterState {
    std::uint16_t frame = 0;            // FFE086
    std::uint16_t scene_complete = 0;   // FFE084
    std::array<std::int16_t, 4> bounds{};
    std::uint8_t dirty_flags = 0;       // FFEF14
};

enum class TitleJitterAction {
    updated,
    completed,
    prng_unresolved,
};

// ROM 0x1B302: run the 0x3C-frame title jitter stream, clear FFE084 at the
// terminal frame, and leave the four FFCB40 words to the word-stream reset
// boundary rather than inventing a second clear here.
TitleJitterAction advance_title_jitter(
    TitleJitterState& state,
    const std::function<std::uint16_t()>& next_random_word,
    const std::function<void()>& write_word_stream,
    const std::function<void()>& reset_word_stream);

bool dispatch_title_phase_handler(
    std::size_t phase,
    const std::vector<std::function<void()>>& handlers,
    const std::function<void()>& build_sprites);

void copy_title_profile(const std::array<std::uint16_t, 7>& source,
                        std::array<std::uint16_t, 7>& destination,
                        std::uint32_t& dirty_flags);

struct AlternatingProfileState {
    std::int16_t countdown = 0;
    std::array<std::uint16_t, 7> even_profile{};
    std::array<std::uint16_t, 7> odd_profile{};
    std::array<std::uint16_t, 7> destination{};
    std::uint32_t dirty_flags = 0;
};

bool advance_alternating_profile(
    AlternatingProfileState& state,
    const std::function<void()>& final_service);

struct AudioMenuCountdownState {
    std::int16_t frame = 0;
    std::int16_t reload = 0;
    std::uint8_t remaining = 0;
    std::uint8_t entity_flags = 0;
};

bool advance_audio_menu_countdown(
    AudioMenuCountdownState& state,
    const std::function<void()>& service_audio,
    const std::function<void()>& clear_ready_menu_slot);

void initialize_effect_motion_seed(
    WorldEffectMotionSeedState& state,
    bool alternate_direction,
    const std::function<void()>& service_audio);

struct InteractionState {
    std::uint8_t source_flags = 0;
    std::uint8_t latch = 0;
    std::uint8_t phase = 0;
    std::uint16_t countdown = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t velocity_x = 0;
    std::int16_t velocity_y = 0;
    bool motion_active = false;
};

struct InteractionCallbacks {
    std::function<void()> initialize_secondary;
    std::function<void()> initialize_primary;
    std::function<void()> advance_motion;
    std::function<void()> service_audio;
};

void update_interaction_state(InteractionState& state,
                              bool source_bit_40,
                              bool global_ready,
                              std::int8_t mode_marker,
                              const InteractionCallbacks& callbacks);

void update_persistent_title_effect(
    bool source_active,
    std::uint8_t slot_flags,
    std::uint16_t& transition_phase,
    const std::function<void()>& prepare_effect,
    const std::function<void()>& advance_effect,
    const std::function<void()>& build_sprites,
    const std::function<void()>& reset_transition,
    const std::function<void()>& refresh_menu);

void advance_title_effect_position(
    std::array<std::int16_t, 4>& positions,
    const std::function<void()>& build_sprites);

void build_title_sprites(const std::function<void()>& build_sprites);

} // namespace shadowrun::ghidra_conversion

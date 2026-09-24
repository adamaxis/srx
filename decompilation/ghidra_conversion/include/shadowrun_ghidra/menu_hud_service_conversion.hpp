#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MenuModeResourceState {
    std::int8_t mode_variant = 0;
    std::uint8_t transfer_marker = 0;
    std::uint8_t prompt_marker = 0;
    bool transition_pending = false;
};

// ROM 0x173F2: prepare the menu-mode resource state, run the intermediate
// service, clear its temporary markers, then prepare title resources.
void prepare_menu_mode_resources(
    MenuModeResourceState& state,
    bool source_mode_flag,
    std::uint8_t selected_mode,
    const std::function<void()>& prepare_intermediate,
    const std::function<void()>& prepare_title_resources);

// ROM 0x177CC: submit the source stream twice in source order.
void upload_menu_resource_pair(const std::function<void()>& upload_stream);

struct MenuHudSequenceCallbacks {
    std::function<void()> write_grid;
    std::function<void()> refresh_number_or_grid;
    std::function<void()> service_after_number;
    std::function<void()> service_menu;
    std::function<void()> service_footer;
};

// ROM 0x1789C: write two grid batches, then execute the four menu/HUD service
// boundaries in their recovered order.
void prepare_menu_hud_sequence(const MenuHudSequenceCallbacks& callbacks);

// ROM 0x178C8: write five consecutive HUD grids.
void write_menu_hud_grid_batch(const std::function<void()>& write_grid);

struct MenuHudRowsState {
    bool menu_upload_pending = false;
    std::array<std::int8_t, 5> row_status{};
};

struct MenuHudRowsCallbacks {
    std::function<void(std::int16_t row, std::uint32_t source_offset)>
        prepare_row;
    std::function<void()> service_row;
    std::function<void(std::int16_t row, std::uint32_t source_offset)>
        write_grid;
};

// ROM 0x1792E: visit rows 4..0, optionally prepare/service rows 4..1 when
// the upload marker and signed row status permit it, and always write a grid.
void render_menu_hud_rows(MenuHudRowsState& state,
                          const MenuHudRowsCallbacks& callbacks);

struct MenuNumberOrGridState {
    std::uint16_t selected_row = 0;
    std::vector<std::int8_t> row_status;
};

struct MenuNumberOrGridCallbacks {
    std::function<void()> render_title_text;
    std::function<void()> build_decimal_glyphs;
    std::function<void()> render_text_followup;
    std::function<void()> write_repeated_grid;
};

// ROM 0x17BC0: choose the text/number branch from the selected row's signed
// status, or submit the fallback repeated grid. Returns true for the text
// branch and false for the fallback.
bool refresh_menu_number_or_grid(
    const MenuNumberOrGridState& state,
    const MenuNumberOrGridCallbacks& callbacks);

// ROM 0x17C52: render one title-text source through the host text boundary.
void render_menu_text_followup(const std::function<void()>& render_title_text);

struct HudSpriteAudioPulseState {
    std::int8_t countdown = 0;
    std::uint8_t phase = 0;
};

// ROM 0x17CC8: decrement the pulse countdown; on expiry advance the four
// phase sprite cadence and service audio only at phase zero. Returns whether
// the cadence expired on this call.
bool advance_hud_sprite_audio_pulse(
    HudSpriteAudioPulseState& state,
    const std::function<void()>& render_hud_sprites,
    const std::function<void()>& advance_audio);

struct HudOffsetState {
    std::int8_t offset = 0;
};

// ROM 0x17D90 / 0x17DB8: apply the signed delta, retain the source's
// overflow-to-zero behavior, clamp positive offsets to ten, and refresh the
// grid/row service in order.
void advance_hud_panel_offset(HudOffsetState& state,
                              std::int8_t delta,
                              const std::function<void()>& write_grid,
                              const std::function<void()>& service_rows);
void advance_hud_footer_offset(HudOffsetState& state,
                               std::int8_t delta,
                               const std::function<void()>& write_grid,
                               const std::function<void()>& service_rows);

// ROM 0x17E04: evaluate the source text-state flags and write three grids only
// when the host predicate reports the recovered draw condition.
bool refresh_hud_grid_if_ready(const std::function<bool()>& ready_to_draw,
                               const std::function<void()>& write_grid);

// ROM 0x17E78: calculate the row's source-derived byte offset. A missing
// table row is reported as an empty optional instead of reading host memory.
std::optional<std::uint32_t> calculate_menu_row_offset(
    const std::vector<std::uint8_t>& row_metrics,
    std::size_t row_index,
    std::int8_t mode_variant);

struct MenuDelayOffsetState {
    std::int8_t offset = 0;
};

using AverageEntityDelayBoundary =
    std::function<std::array<std::int8_t, 2>()>;

// ROM 0x17EB4: publish 0x1D minus the two signed components returned by the
// average-delay service.
void calculate_menu_delay_offset(MenuDelayOffsetState& state,
                                 const AverageEntityDelayBoundary& average);

// ROM 0x17F80 / 0x17FB4 / 0x1897C / 0x18998: explicit host service wrappers.
void refresh_mode_metadata_wrapper(const std::function<void()>& refresh);
void write_hud_repeated_grid(const std::function<void()>& write_grid);
void write_hud_repeated_grid_variant_a(const std::function<void()>& write_grid);
void write_hud_repeated_grid_variant_b(const std::function<void()>& write_grid);

struct HudFadeServiceState {
    bool dirty = false;
};

// ROM 0x18776: perform five two-vblank palette fade passes and publish the
// dirty marker after each pass.
void advance_hud_fade_service(
    HudFadeServiceState& state,
    const std::function<void()>& wait_for_vblank,
    const std::function<void()>& latch_controller_events,
    const std::function<void()>& step_palette_toward_black);

struct MenuHudTickState {
    std::uint16_t frames_completed = 0;
    std::uint8_t dirty_flags = 0;
    bool controller_tick_ready = false;
    bool world_tick_ready = false;
    std::uint8_t selected_row = 0;
};

struct MenuHudTickCallbacks {
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> advance_sprite_audio_pulse;
    std::function<void()> advance_random_effect;
    std::function<void()> service_world_tick;
    std::function<void()> service_optional_effect;
    std::function<void()> finalize_world;
    std::function<void()> finalize_input;
    std::function<void()> render_rows;
};

// ROM 0x177FC: execute the sixty-frame HUD/menu service cadence and then run
// the recovered post-loop cleanup in order.
void run_menu_hud_tick_loop(MenuHudTickState& state,
                            const MenuHudTickCallbacks& callbacks);

struct MenuTextProgressionCallbacks {
    std::function<void()> render_title_text;
    std::function<void()> build_decimal_glyphs;
};

// ROM 0x17998: reproduce the count-dependent title-text and decimal-glyph
// progression without carrying the source condition-code frame.
void render_menu_text_progression(
    std::int16_t available_count,
    bool service_blocked,
    const MenuTextProgressionCallbacks& callbacks);

struct MenuOverlayTimerState {
    std::uint8_t overlay_phase = 0;
    std::int8_t countdown = 0;
    std::uint8_t service_marker = 0;
    std::int8_t reload_count = 0;
};

// ROM 0x17EEE: advance the overlay timer and write two grids when its
// countdown expires.
bool advance_menu_overlay_timer(
    MenuOverlayTimerState& state,
    const std::function<void()>& write_grid);

struct MenuRandomGlitchState {
    std::int16_t phase = -1;
    std::int16_t random_offset = 0;
};

struct MenuRandomGlitchCallbacks {
    std::function<std::int16_t()> sample_random_bound;
    std::function<void()> write_repeated_grid;
    std::function<void()> write_grid;
    std::function<void()> write_linear_grid;
};

// ROM 0x17FDA: advance the short random visual-effect sequence and decrement
// its phase after submitting the selected grid/linear-grid operations.
void advance_menu_random_glitch(
    MenuRandomGlitchState& state,
    const MenuRandomGlitchCallbacks& callbacks);

struct FieldHudAssetState {
    std::uint8_t selected_row = 0;
};

struct FieldHudAssetCallbacks {
    std::function<void()> render_field_hud;
    std::function<void()> refresh_selection_rows;
    std::function<void()> upload_source_stream;
    std::function<void()> prepare_field_effects;
    std::function<void()> render_hud_sprites;
};

// ROM 0x186FA: refresh the field HUD, reset its selected row, and submit the
// source stream/effect/sprite services in source order.
void refresh_field_hud_assets(FieldHudAssetState& state,
                              const FieldHudAssetCallbacks& callbacks);

struct HudIndicatorGridState {
    std::array<std::uint8_t, 12> active_flags{};
};

// ROM 0x187AA: write active indicator grids followed by four unconditional
// footer grids.
void render_hud_indicator_grids(const HudIndicatorGridState& state,
                                const std::function<void()>& write_grid);

void write_hud_repeated_grid_variant_c(const std::function<void()>& write_grid);
void write_two_hud_grids(const std::function<void()>& write_grid);

struct MenuDirectionalInputState {
    std::uint8_t input_bits = 0;
    std::size_t selected_row = 0;
    std::array<std::int8_t, 5> row_status{};
    std::array<std::int8_t, 5> row_timers{};
};

// ROM 0x19432: on directional input, publish a selection only for eligible
// rows, then refresh the shared menu state and number/grid branch.
void handle_menu_directional_input(
    const MenuDirectionalInputState& state,
    const std::function<void()>& set_selection,
    const std::function<void()>& refresh_menu_state,
    const std::function<void()>& refresh_number_or_grid);

struct HudSelectionRowState {
    std::int8_t selected_row = 0;
};

// ROM 0x194AC: apply the signed selection delta with 0..4 wrapping, then
// submit the panel grid and row renderer.
void advance_hud_selection_row(
    HudSelectionRowState& state,
    std::int8_t delta,
    const std::function<void()>& write_grid,
    const std::function<void()>& render_rows);

void write_hud_panel_grid(const std::function<void()>& write_grid);

void reset_menu_visual_slots(const std::function<void()>& reset_five,
                             const std::function<void()>& reset_twelve);
void clear_ready_menu_visual_slots(
    const std::array<std::int8_t, 5>& row_status,
    const std::function<void(std::size_t row)>& clear_slot);
void refresh_menu_slot_pair(const std::function<void()>& refresh_pair,
                            const std::function<void()>& clear_visual);
void clear_menu_slot_visual(const std::function<void()>& clear_visual);

struct HudHistoryState {
    std::int8_t countdown = 0;
    std::array<std::uint16_t, 4> values{};
    bool dirty = false;
};

// ROM 0x196F2: rotate the four-word HUD history when its countdown expires.
void rotate_hud_history(HudHistoryState& state);

} // namespace shadowrun::ghidra_conversion

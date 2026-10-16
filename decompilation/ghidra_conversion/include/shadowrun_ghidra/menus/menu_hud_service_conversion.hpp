#pragma once

#include "shadowrun_ghidra/core/word_transfer_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"

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

struct MenuEntryRenderState {
    std::int8_t entry_marker = 0;
    std::int8_t entry_status = 0;
    std::int16_t available_count = 0;
    std::uint8_t pending_flags = 0;
};

struct MenuEntryRenderCallbacks {
    std::function<void()> merge_pending_flags;
    std::function<void()> refresh_hud_grid;
    std::function<void()> advance_entry_state;
    std::function<void()> render_title_text;
    std::function<void()> build_grouped_decimal_glyphs;
};

enum class MenuEntryRenderAction {
    pending,
    text_only,
    text_and_number,
};

// ROM 0x17A5E: process one menu entry's signed marker/status pair and submit
// the recovered title-text/decimal-glyph sequence. The negative marker path
// merges pending flags and sets bit 6; the positive path may refresh the HUD.
MenuEntryRenderAction render_menu_entry(
    MenuEntryRenderState& state,
    bool hud_grid_dirty,
    bool text_variant_available,
    bool number_variant_available,
    const MenuEntryRenderCallbacks& callbacks);

// ROM 0x17C3C: index the source FFFB30 byte table with FFF102, sign-extend
// the selected byte to a word, and logically shift that word right by four.
// The native boundary supplies the source table explicitly instead of
// reaching into emulated RAM. An out-of-range host index is invalid rather
// than an accidental read past the extracted table.
std::optional<std::uint16_t> read_menu_entry_profile_eligibility(
    std::uint16_t table_index,
    const std::vector<std::uint8_t>& eligibility_table);

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

struct MenuDelayOffsetInput {
    // ROM 0x17EB8 reads FFFBBE+0x07 and multiplies it by four.
    std::uint8_t profile_delay_component = 0;
    // ROM 0xD81C reads these two bytes from the active runner record.
    std::uint8_t active_record_attribute_a = 0;
    std::uint8_t active_record_attribute_b = 0;
};

// ROM 0x17EB4: add the fixed six, four times the profile byte, and the
// byte-wrapped floor average from 0xD81C, then publish the low byte of
// 0x1D minus that total at FFF0A7.
void calculate_menu_delay_offset(MenuDelayOffsetState& state,
                                 const MenuDelayOffsetInput& input);

// ROM 0x17F80 / 0x17FB4 / 0x1897C / 0x18998: explicit host service wrappers.
void refresh_mode_metadata_wrapper(const std::function<void()>& refresh);
void write_hud_repeated_grid(const std::function<void()>& write_grid);
void write_hud_repeated_grid_variant_a(const std::function<void()>& write_grid);
void write_hud_repeated_grid_variant_b(const std::function<void()>& write_grid);

constexpr std::array<std::uint16_t, 19> kHudRepeatedGridWideRow{
    0x0070U, 0x0070U, 0x0070U, 0x0076U, 0x007AU, 0x007EU, 0x0077U, 0x007BU,
    0x007FU, 0x0078U, 0x007CU, 0x0080U, 0x0079U, 0x007DU, 0x0081U, 0x00B6U,
    0x0098U, 0x08B6U, 0x00B0U};

void write_hud_repeated_grid_variant_a(const WordTransferCallbacks& callbacks);
void write_hud_repeated_grid_variant_b(const WordTransferCallbacks& callbacks);
void write_hud_repeated_grid_variant_c(const WordTransferCallbacks& callbacks);

constexpr std::array<std::array<std::array<std::uint16_t, 9>, 2>, 4>
    kTwoHudGridTiles{{{{{{0x041AU, 0x041BU, 0x045CU, 0x045DU, 0x045EU, 0x045FU,
                          0x0460U, 0x0C1BU, 0x0421U}},
                       {{0x041AU, 0x041BU, 0x041CU, 0x041DU, 0x041EU, 0x041FU,
                         0x0420U, 0x0C1BU, 0x0421U}}}},
                     {{{{0x041AU, 0x041BU, 0x044DU, 0x044EU, 0x044FU, 0x0450U,
                         0x0451U, 0x0452U, 0x0453U}},
                       {{0x041AU, 0x041BU, 0x046CU, 0x046DU, 0x046EU, 0x046FU,
                         0x0470U, 0x0471U, 0x0453U}}}},
                     {{{{0x0476U, 0x0477U, 0x0478U, 0x0479U, 0x047AU, 0x0451U,
                         0x047BU, 0x047CU, 0x047DU}},
                       {{0x0482U, 0x0483U, 0x0484U, 0x0485U, 0x0486U, 0x0470U,
                         0x0487U, 0x0488U, 0x0489U}}}},
                     {{{{0x041AU, 0x048FU, 0x0490U, 0x0491U, 0x0491U, 0x0492U,
                         0x0493U, 0x0494U, 0x0453U}},
                       {{0x041AU, 0x0499U, 0x049AU, 0x049BU, 0x049CU, 0x049DU,
                         0x049EU, 0x049FU, 0x0453U}}}}}};

// ROM 0x189D8: two 9-wide grids from 0xD9FA8+FFF0D1*0x24 at 0xE08A+idx*0x280.
// FFDD22 bit 8 decrements the second index, wrapping 0..3.
void write_two_hud_grids(
    std::uint8_t index,
    bool wrap_decrement,
    const WordTransferCallbacks& callbacks);

constexpr std::array<std::uint16_t, 3> kHudPanelGridRow{
    0x00B0U, 0x00B0U, 0x00B0U};
constexpr std::uint16_t kHudPanelGridBase = 0xEC94U;

// ROM 0x19504: dest FFF0D4*0xC-0x136C, 3x1 mask 0x6000 from 0xD8478.
void write_hud_panel_grid(
    std::uint8_t selector,
    const WordTransferCallbacks& callbacks);

constexpr std::uint16_t kHudRepeatedGridBase = 0xC300U;
constexpr std::uint16_t kHudRepeatedGridMask = 0x6000U;
constexpr std::array<std::uint16_t, 3> kHudRepeatedGridRow{
    0x0070U, 0x0070U, 0x0070U};

// ROM 0x17FB4: dest 0xC300+FFF09A, 3x8, mask 0x6000, row 0xD8454.
void write_hud_repeated_grid(
    std::uint16_t scroll_word,
    const WordTransferCallbacks& callbacks);

constexpr std::array<std::array<std::uint32_t, 8>, 3> kHudFooterPaletteBanks{{
    {{0x00000000U, 0x02220444U, 0x08880EEEU, 0x08AC046AU, 0x022A048CU,
      0x0640000EU, 0x02440486U, 0x0CE00226U}},
    {{0x00000000U, 0x02220444U, 0x08880EEEU, 0x0CA80864U, 0x04AC0068U,
      0x02440846U, 0x04860264U, 0x08A00642U}},
    {{0x00000000U, 0x02220444U, 0x08880EEEU, 0x068C0248U, 0x000C0A20U,
      0x046802C2U, 0x02460042U, 0x0C8C0228U}},
}};

// ROM 0x17F8E: (record+0x2C)/0x2000-1 indexes table 0x18ACB6, then 0x566EC.
void draw_hud_footer(
    std::uint16_t palette_word,
    std::array<std::uint32_t, 8>& destination,
    std::uint8_t& display_dirty_flags);

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

struct HudHistoryState {
    std::int8_t countdown = 0;
    std::array<std::uint16_t, 4> values{};
    bool dirty = false;
};

// ROM 0x196F2: rotate the four-word HUD history when its countdown expires.
void rotate_hud_history(HudHistoryState& state);

} // namespace shadowrun::ghidra_conversion

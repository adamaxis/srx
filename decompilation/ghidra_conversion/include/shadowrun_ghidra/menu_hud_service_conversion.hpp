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

} // namespace shadowrun::ghidra_conversion

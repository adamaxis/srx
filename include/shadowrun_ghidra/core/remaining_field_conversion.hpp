#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x0D5A: signed windows on +0x4C then +0x9C. Value <5 adds 2 to D4,
// 5..7 adds 1, >=8 skips that meter.
std::int16_t guard_entity_phase_window(
    std::int8_t delay_meter,
    std::int8_t mental_meter,
    std::int16_t score);

struct TitleResourceTransfer {
    bool compressed = false;
    std::uint16_t source_id = 0;
};

struct TitleResourcePreparation {
    std::vector<TitleResourceTransfer> transfers;
    std::vector<std::uint16_t> sprite_streams;
    std::uint16_t header_a = 0;
    std::uint16_t header_b = 0;
    std::uint16_t header_c = 0;
    std::uint16_t header_d = 0;
    bool display_dirty = false;
};

// ROM 0x01406: translate the title's terminated resource descriptors into a
// host-owned transfer plan.  The callbacks are presentation boundaries; no
// VDP address or ROM pointer escapes this conversion module.
void prepare_title_resources(
    TitleResourcePreparation& state,
    std::uint16_t header_a,
    std::uint16_t header_b,
    std::uint16_t header_c,
    std::uint16_t header_d,
    const std::vector<std::uint16_t>& resource_descriptors,
    const std::vector<std::uint16_t>& sprite_descriptors,
    const std::function<void(const TitleResourceTransfer&)>& upload,
    const std::function<void(std::uint16_t)>& draw_sprite_stream);

struct HudSpriteWord {
    std::uint16_t tile = 0;
    std::uint16_t value = 0;
};

struct HudSpriteStreamCallbacks {
    std::function<void()> begin_transfer;
    std::function<void(std::uint16_t)> write_word;
    std::function<void()> wrap_transfer;
};

// ROM 0x01728: decode the compact sprite word stream used by the HUD and
// title overlays.  The stream is bounded by its host vector and terminates at
// a zero byte, matching the source sentinel without exposing source memory.
std::size_t render_hud_sprite_stream(
    const std::vector<std::uint8_t>& encoded,
    std::uint16_t initial_tile,
    std::uint16_t row_word_limit,
    std::uint16_t palette_or_flip,
    const HudSpriteStreamCallbacks& callbacks);

struct FieldModeTransitionState {
    std::uint8_t mode = 0;
    std::uint8_t scene = 0;
    std::uint16_t display_resource = 0;
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    bool world_activity_started = false;
    bool timeout_scheduled = false;
};

void advance_field_mode_transition(
    FieldModeTransitionState& state,
    const std::vector<std::int8_t>& mode_table,
    const std::vector<std::uint16_t>& resource_table,
    const std::vector<std::uint16_t>& coordinate_table,
    const std::function<void()>& begin_world_activity,
    const std::function<void()>& schedule_timeout);

struct FieldVariantState {
    std::uint8_t scene = 0;
    std::uint16_t display_resource = 0;
    std::uint16_t camera_x = 0;
    std::uint8_t variant = 0;
    bool pending_position = false;
};

void toggle_field_scene_variant(
    FieldVariantState& state,
    const std::function<void(std::uint8_t, std::uint16_t)>& select_position);

struct HudPanelSelection {
    std::uint16_t asset = 0;
    std::uint16_t routine = 0;
};

HudPanelSelection select_hud_panel_variant(bool zero_flag, std::int16_t mode);
void refresh_hud_panel_variant(
    bool zero_flag,
    std::int16_t mode,
    const std::function<void(HudPanelSelection)>& draw_panel,
    const std::function<void()>& post_hook);

struct EntityMeters {
    std::uint8_t delay = 0;
    std::uint8_t asset = 0;
    std::uint8_t behavior = 0;
    std::uint8_t state_flags = 0;
    std::uint8_t animation_mode = 0;
    std::uint8_t slot_value = 0;
};

void advance_entity_delay_meter(EntityMeters& state, std::int16_t delta,
                                const std::function<void()>& write_grid);
void advance_entity_asset_meter(EntityMeters& state, std::int16_t delta,
                                const std::vector<std::uint8_t>& cap_table,
                                const std::function<void()>& reload_asset,
                                const std::function<void()>& write_grid);
void advance_entity_behavior_meter(EntityMeters& state, std::int16_t delta,
                                   const std::function<void()>& write_grid);

bool accept_camera_delta(std::uint16_t current, std::uint16_t target,
                         std::int8_t direction);

struct FieldCameraState {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t min_x = 0;
    std::int16_t max_x = 0;
    std::int16_t min_y = 0;
    std::int16_t max_y = 0;
    std::uint16_t aligned_x = 0xFFFFU;
    std::uint16_t aligned_y = 0xFFFFU;
    std::uint8_t scene_state = 0;
};

void update_field_camera(
    FieldCameraState& state,
    std::int16_t delta_x,
    std::int16_t delta_y,
    const std::function<void(std::uint16_t)>& render_horizontal_edge,
    const std::function<void(std::uint16_t)>& render_vertical_edge,
    const std::function<void()>& reset_camera);

struct FieldTileSample {
    std::uint32_t source_offset = 0;
    std::vector<std::uint8_t> masked_words;
};

FieldTileSample decode_field_tile_sample(
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    std::int16_t entity_x,
    std::int16_t entity_y,
    std::uint16_t packed_region,
    const std::vector<std::uint8_t>& source,
    const std::function<std::uint32_t(std::uint16_t, std::int16_t)>&
        calculate_source_offset);

void render_field_tile_edge(
    std::uint16_t tile_key,
    std::size_t word_count,
    std::uint16_t palette_or_flip,
    const std::function<std::uint16_t(std::uint16_t, std::size_t)>& read_word,
    const std::function<void(std::uint16_t)>& write_word,
    const std::function<void()>& begin_transfer);

} // namespace shadowrun::ghidra_conversion

#pragma once

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kFieldTerrainLayerWordCount = 0xE10U;

struct DisplayServiceCallbacks {
    std::function<void()> service_pending_upload;
    std::function<void()> upload_startup_palette;
    std::function<void()> service_idle;
    std::function<void()> write_transfer_header;
    std::function<void()> write_fixed_transfer_pair;
    std::function<void()> render_normal_viewport;
    std::function<void()> render_alternate_viewport;
    std::function<void()> flush_primary_transfers;
    std::function<void()> flush_secondary_transfers;
};

struct DisplayServiceState {
    std::uint32_t frame_counter = 0;
    std::uint8_t dirty_flags = 0;
    std::uint8_t service_marker = 0;
    bool display_ready = false;
    bool render_normal = false;
    bool render_alternate = false;
    bool flush_primary = false;
    bool flush_secondary = false;
};

// ROM 0x01872: service the display dirty bits and field-view/transfer queues
// in source order. The callbacks are host-owned presentation boundaries.
void service_display_frame(DisplayServiceState& state,
                           const DisplayServiceCallbacks& callbacks);

// ROM 0x01070: display service wrapper whose register return values are not
// part of the native contract.
void service_display_frame_wrapper(DisplayServiceState& state,
                                    const DisplayServiceCallbacks& callbacks);

// ROM 0x01080: submit one alternating word-pair transfer.
void write_alternating_word_pair_wrapper(
    const std::function<void()>& write_pair);

struct FieldSceneBuffers {
    bool initialized = false;
    std::uint8_t scene_mode = 0; // selected profile: 0 normal, 1 alternate
    std::uint8_t field_resource_mode = 0; // ROM FFFBF8: low byte of scene id
    std::uint16_t scene_reference = 0; // ROM FFDDC4 / 0x03032 input
    std::array<std::uint16_t, 48> profile{};
    std::int32_t camera_x = 0;
    std::int32_t camera_y = 0;
    std::uint32_t terrain_stream_token = 0; // ROM FFDE22 <- FFDDDE
    std::array<SceneRunnerPosition, 3> pending_field_targets{};
    bool pending_position_consumed = false;
    bool pending_position_cleared = false;
    std::vector<std::uint16_t> layer_a;
    std::vector<std::uint16_t> layer_b;
};

struct FieldSceneBufferInput {
    std::array<std::uint16_t, 48> normal_profile{};
    std::array<std::uint16_t, 48> alternate_profile{};
    std::uint16_t scene_reference = 0;
    std::uint32_t terrain_stream_token = 0;
    std::vector<std::uint16_t> layer_a;
    std::vector<std::uint16_t> layer_b;
    // ROM FFFC0E: profile-specific pending-position selector.
    bool pending_position_available = false;
    // ROM FFE10E: a nonzero value suppresses the pending-position consume.
    bool pending_position_in_use = false;
    // ROM FFE10A/FFE10C: the source pair copied to all three target pairs.
    SceneRunnerPosition pending_position{};
};

struct FieldScenePreparationCallbacks {
    std::function<void()> initialize_scene_profile;
    std::function<bool()> scene_uses_alternate_profile;
};

// ROM 0x03074 -> 0x03084 -> 0x0313A: prepare the field-scene runtime when the
// source is entering a new scene, select/copy the active 48-word profile,
// publish the low-byte field resource mode and terrain-stream token, consume
// the pending position into three target pairs with the source's saturated
// camera recenter, and copy both fixed 0xE10-word terrain layers.
bool prepare_field_scene_buffers(
    FieldSceneBuffers& state,
    const FieldSceneBufferInput& input,
    const FieldScenePreparationCallbacks& callbacks);

struct HudPanelTransferCallbacks {
    // Each optional result represents one source transfer block and contains
    // the number of words emitted by that block.
    std::function<std::optional<std::size_t>()> begin_transfer;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x03648: emit incrementing HUD panel words for each transfer block.
void write_hud_panel_transfer(std::uint16_t initial_word,
                              const HudPanelTransferCallbacks& callbacks);

struct FieldTileColumnInput {
    std::uint16_t source_coordinate = 0;
    std::uint16_t tile_key = 0;
    std::uint16_t palette_or_flip = 0;
    std::size_t source_word_offset = 0;
};

struct FieldTileColumnCallbacks {
    std::function<void()> begin_transfer;
    std::function<std::uint16_t(std::uint16_t tile_key,
                                std::size_t word_offset)>
        read_word;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x03D4A/0x03D80: emit the visible portion of a tile column. The source
// writes a partial pre-alignment segment when needed, then continues through
// the shared 0x03D80 transfer body after aligning the destination; each body
// entry changes tile rows every four words and applies the palette/flip
// transform.
std::size_t render_field_tile_column(
    const FieldTileColumnInput& input,
    const FieldTileColumnCallbacks& callbacks);

struct FieldTileRowCallbacks {
    std::function<void()> fold_coordinate;
    std::function<std::int16_t()> column_start;
    std::function<void(std::int16_t row, std::uint16_t column_key,
                       std::int16_t start)> render_column;
};

// ROM 0x03D18: fold the field coordinate and submit the thirty-one columns
// in the source's descending row order.
void render_field_tile_rows(std::uint16_t initial_column_key,
                            std::int16_t initial_row_sum,
                            const FieldTileRowCallbacks& callbacks);

// ROM 0x03F9E: resolve a map tile through the active row-offset table.
std::optional<std::int32_t> calculate_field_tile_source_offset(
    const std::vector<std::int16_t>& row_offsets,
    std::size_t row_index,
    std::int32_t base_offset,
    const std::vector<std::uint16_t>& map_words);

struct ControllerDelayState {
    std::uint8_t controller_flags = 0;
    std::array<std::uint8_t, 2> entity_flags{};
    std::uint8_t delay = 0;
};

// ROM 0x0ABB6: arm the 0x3C-frame controller delay when all three source
// flag predicates permit activity.
bool arm_controller_activity_delay(ControllerDelayState& state);

struct EntityBehaviorDispatchCallbacks {
    std::function<void()> behavior_without_collision_flag;
    std::function<void()> behavior_with_collision_flag;
    std::function<void()> schedule_collision_recheck;
};

// ROM 0x0AC84: dispatch the entity behavior branch and optionally schedule a
// collision recheck when the global interaction marker is active.
void dispatch_entity_collision_behavior(
    std::uint8_t entity_flags,
    bool interaction_marker,
    const EntityBehaviorDispatchCallbacks& callbacks);

struct TitleSpriteWaveState {
    std::uint16_t timer = 0;
    std::uint16_t base_x = 0;
    std::uint16_t base_y = 0;
    std::uint16_t palette_selector = 0;
    bool active = true;
};

struct TitleSpriteWaveCallbacks {
    std::function<void(std::uint16_t x, std::uint16_t y,
                       std::uint16_t offset, std::uint32_t palette)> build;
};

// ROM 0x0AEBA: advance the title sprite wave by eight ticks and emit its
// thirty-two independently positioned sprite records while it is active.
bool advance_title_sprite_wave(
    TitleSpriteWaveState& state,
    const std::array<std::uint32_t, 4>& palettes,
    const TitleSpriteWaveCallbacks& callbacks);

void load_title_font(const std::function<void()>& decode_and_upload);
void write_menu_grid_with_metadata(const std::function<void()>& write_grid);
void render_menu_grid_number_pair(const std::function<void()>& write_grid,
                                  const std::function<void()>& build_glyphs);
// ROM 0xCFFE: prepare the Hotel Options presentation.  This source helper is
// reached only by the Hotel Options flow at 0xCF38; the native name records
// that context instead of preserving Ghidra's generic title-mode label.
void prepare_hotel_options_display(
    const std::function<void()>& refresh_mode,
    const std::function<void()>& refresh_palette,
    const std::function<void()>& prepare_display,
    const std::function<void()>& load_font);
// ROM 0xDA4A: the Attributes/Skills renderer's fixed heading helper. It
// ignores caller register values, renders the encoded resource at 0xC1B3B,
// and submits it to the host text boundary.
void render_attributes_skills_heading(
    const std::function<void()>& render_text);
void advance_random_grid_effect(const std::function<std::int16_t()>& sample,
                                const std::function<void()>& write_grid);
void write_menu_footer_grid(const std::function<void()>& write_grid);
void write_menu_grid_quadruple(const std::function<void()>& write_grid);

} // namespace shadowrun::ghidra_conversion

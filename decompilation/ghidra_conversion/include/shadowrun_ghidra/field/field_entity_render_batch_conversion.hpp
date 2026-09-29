#pragma once

#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityAnchor {
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct EntityAnchorSnapshotState {
    std::array<EntityAnchor, 3> anchors{};
    std::optional<EntityAnchor> pending_anchor;
    std::array<std::uint16_t, 6> pending_profile{};
    bool pending_profile_active = false;
};

void snapshot_entity_anchors(
    EntityAnchorSnapshotState& state,
    const std::array<EntityAnchor, 3>& source_anchors,
    const std::optional<EntityAnchor>& ready_anchor,
    const std::array<std::uint16_t, 8>& pending_source,
    const std::function<void(const EntityAnchor&)>& publish_ready_anchor);

struct TileWordStreamCallbacks {
    std::function<void()> begin_transfer;
    std::function<std::uint16_t(std::uint16_t tile_key,
                                std::size_t word_offset)>
        read_word;
    std::function<void(std::uint16_t)> write_word;
};

std::size_t emit_field_tile_column_words(
    std::size_t word_count,
    std::uint16_t tile_key,
    std::size_t word_offset,
    std::uint16_t palette_or_flip,
    const TileWordStreamCallbacks& callbacks);
std::size_t emit_field_tile_row_words(
    std::size_t word_count,
    std::uint16_t tile_key,
    std::size_t word_offset,
    std::uint16_t palette_or_flip,
    const TileWordStreamCallbacks& callbacks);

struct FieldTilePlaneRenderPlan {
    std::size_t first_word_count = 0;
    std::size_t second_word_count = 0;
    std::uint16_t wrapped_destination = 0;
};

// ROM 0x3D4A: visible column words from D4 bit 0-6, cap 0x2C, wrap D4 & 0xFF80.
FieldTilePlaneRenderPlan plan_field_tile_column(std::uint16_t destination_word);
// ROM 0x3DDE: visible row words from D4 bits 7-11, cap 0x20, wrap D4 & 0xF07F.
FieldTilePlaneRenderPlan plan_field_tile_row(std::uint16_t destination_word);

struct FieldTileRowUploadState {
    std::uint16_t word_offset = 0; // D1
    std::int16_t remaining = 0; // D7, DBF so words = remaining + 1
    std::int16_t palette_word = 0; // FFDF58
    std::uint16_t row_table_byte_offset = 0; // D3
    std::uint16_t row_stride_bytes = 0; // FFDDEA
};

struct FieldTileRowUploadCallbacks {
    std::function<void()> begin_transfer;
    std::function<void()> end_transfer;
    std::function<std::uint16_t(std::uint16_t table_offset)> read_tile_key;
    std::function<std::uint16_t(std::uint16_t tile_key,
                                std::uint16_t word_offset)>
        read_word;
    std::function<void(std::uint16_t)> write_word;
};

// ROM 0x3E14: VDP autoincrement 0x80, emit D7+1 nametable words stepping D1
// by 8, wrap at signed 0x1F with D1 -= 0x20 and D3 += FFDDEA.
std::size_t upload_field_tile_row(
    FieldTileRowUploadState& state,
    const FieldTileRowUploadCallbacks& callbacks);

struct FieldTileRowEdgeState {
    std::int16_t horizontal_direction = 0; // 0x40E2 D0 input
    std::uint16_t camera_x = 0; // FFDDC6
    std::int16_t camera_x_limit = 0; // FFDDCE
    std::uint16_t camera_y = 0; // FFDD0A
    std::uint16_t field_y = 0; // FFDDC8
    std::uint16_t base_destination = 0; // FFDD08
    std::uint16_t plane_a_offset = 0; // FFDD14
    std::uint16_t plane_b_offset = 0; // FFDD16
    std::uint16_t row_stride_bytes = 0; // FFDDEA
};

struct FieldTileRowEdgeRequest {
    std::int16_t x_coordinate = 0;
    std::int16_t row_coordinate = 0;
    std::int32_t destination_offset = 0;
    std::uint16_t plane_offset = 0;
    std::uint16_t palette_or_flip = 0;
    FieldTileSourceLayout source_layout{};
};

struct FieldTileRowEdgeCallbacks {
    std::function<void(const FieldTileRowEdgeRequest&)> render_row;
};

enum class FieldTileRowEdgeAction {
    outside_visible_window,
    rows_rendered,
    boundary_unresolved,
};

struct FieldTileRowEdgeResult {
    FieldTileRowEdgeAction action =
        FieldTileRowEdgeAction::boundary_unresolved;
    std::uint8_t rows_rendered = 0;
    std::optional<FieldTileRowEdgeRequest> first_request;
    std::optional<FieldTileRowEdgeRequest> second_request;
};

// ROM 0x040E2: select a vertical camera edge, prepare its source/destination
// coordinates, and submit the ordinary and XOR-flipped map-plane row passes.
FieldTileRowEdgeResult render_field_tile_row_edge(
    const FieldTileRowEdgeState& state,
    const FieldTileRowEdgeCallbacks& callbacks);

struct FieldTileColumnEdgeState {
    std::int16_t vertical_direction = 0; // 0x4018 D0 input
    std::uint16_t field_y = 0; // FFDDC8
    std::int16_t field_y_limit = 0; // FFDDD0, already in >>4 units
    std::uint16_t camera_x = 0; // FFDDC6, 0x4216
    std::uint16_t camera_y = 0; // FFDD0A destination base
    std::uint16_t vdp_data_word = 0; // FFDD08, 0x4216 D3
    std::uint16_t plane_a_offset = 0; // FFDD14
    std::uint16_t plane_b_offset = 0; // FFDD16
    std::uint16_t row_stride_bytes = 0; // FFDDEA
};

struct FieldTileColumnEdgeRequest {
    std::int16_t x_coordinate = 0;
    std::int16_t y_coordinate = 0;
    std::int32_t destination_offset = 0;
    std::uint16_t plane_offset = 0;
    std::uint16_t palette_or_flip = 0;
    FieldTileSourceLayout source_layout{};
};

struct FieldTileColumnEdgeCallbacks {
    std::function<void(const FieldTileColumnEdgeRequest&)> render_column;
};

enum class FieldTileColumnEdgeAction {
    outside_visible_window,
    columns_rendered,
    boundary_unresolved,
};

struct FieldTileColumnEdgeResult {
    FieldTileColumnEdgeAction action =
        FieldTileColumnEdgeAction::boundary_unresolved;
    std::uint8_t columns_rendered = 0;
    std::optional<FieldTileColumnEdgeRequest> first_request;
    std::optional<FieldTileColumnEdgeRequest> second_request;
};

// ROM 0x04018: select a horizontal camera edge, prepare 0x4216/0x4000
// coordinates, and submit the ordinary and XOR-flipped map-plane column
// passes. Positive and negative D0 each emit one edge; the source RTS.
FieldTileColumnEdgeResult render_field_tile_column_edge(
    const FieldTileColumnEdgeState& state,
    const FieldTileColumnEdgeCallbacks& callbacks);

struct RenderEntityState {
    bool active = false;
    std::uint32_t id = 0;
};

struct EntityRenderPassCallbacks {
    std::function<void(RenderEntityState&)> project_screen_position;
    std::function<void(RenderEntityState&)> advance_animation;
    std::function<void(RenderEntityState&)> finish_entity;
    std::function<void()> finish_pass;
};

// ROM 0x04618: run the sixteen-slot active-entity projection/animation pass.
void update_entity_render_pass(
    std::array<RenderEntityState, 16>& entities,
    const EntityRenderPassCallbacks& callbacks);

struct EffectRecordState {
    std::uint16_t coordinate = 0;
    std::uint8_t flags = 0;
    std::array<std::uint8_t, 5> tile_bytes{};
};

void reset_effect_record(EffectRecordState& state,
                         const std::function<void()>& clear_slot);

struct EntitySpawnRecord {
    std::uint8_t mode = 0;
    std::uint8_t variant = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint32_t script = 0;
};

void clone_entity_spawn_variant_a(EntitySpawnRecord& destination,
                                  const EntitySpawnRecord& source);
void clone_entity_spawn_variant_b(EntitySpawnRecord& destination,
                                  const EntitySpawnRecord& source);

struct TitleSpriteTimerState {
    std::int8_t countdown = 0;
    std::uint8_t phase = 0;
};

void advance_title_sprite_timer(
    TitleSpriteTimerState& state,
    const std::function<void(std::uint8_t phase)>& build_sprite_records);
void emit_title_sprite_burst(
    std::uint16_t sprite_type,
    std::int16_t progress,
    const std::function<void()>& build_sprite_records);

struct SpriteExpansionState {
    std::uint8_t flags = 0;
    std::uint16_t source_count = 0;
    std::uint32_t sprite_value = 0;
    std::uint16_t sprite_attribute = 0;
    bool appended = false;
};

struct SpriteExpansionCallbacks {
    std::function<void(SpriteExpansionState&)> expand_packed_nibbles;
    std::function<void(SpriteExpansionState&)> append_sprite;
};

bool append_entity_render_sprite(SpriteExpansionState& state,
                                 const SpriteExpansionCallbacks& callbacks);

struct EntityRuntimeState {
    bool entity_table_ready = false;
    std::uint32_t entity_table_token = 0;
    std::uint32_t sprite_cursor = 0;
    std::uint16_t sprite_count = 0;
    std::uint32_t secondary_cursor = 0;
};

void initialize_entity_runtime(EntityRuntimeState& state,
                               const std::function<void()>& initialize_order);

bool dispatch_player_action_if_ready(
    std::uint8_t auxiliary_marker,
    std::uint8_t action_code,
    std::int16_t status,
    const std::function<void()>& dispatch_action);

bool clear_entity_effect_if_forced(
    std::uint8_t& entity_flags,
    std::int16_t global_effect_state,
    const std::function<void()>& handle_world_effect);

struct TargetHeadingState {
    std::uint8_t mode = 0;
    bool has_target = false;
    std::uint16_t target_heading = 0;
    bool relation_rejected = false;
};

struct TargetHeadingCallbacks {
    std::function<std::uint16_t()> measure_target_heading;
    std::function<bool()> relation_is_rejected;
    std::function<void()> reset_heading;
    std::function<void()> use_fallback_heading;
};

void update_entity_target_heading(TargetHeadingState& state,
                                  const TargetHeadingCallbacks& callbacks);

std::size_t render_encoded_title_text(
    const std::vector<std::uint8_t>& encoded,
    const std::function<void(std::uint16_t glyph_index)>& write_glyph);

struct MenuValueState {
    std::uint16_t primary = 0;
    std::uint16_t selected = 0;
};

void select_menu_value(MenuValueState& state,
                       const std::vector<std::uint8_t>& row_table,
                       std::size_t row_index,
                       bool special_entity_state,
                       std::uint8_t special_value);

struct MenuTextDisplayCallbacks {
    std::function<void()> prepare_grid;
    std::function<void()> prepare_text;
    std::function<void()> render_text;
    std::function<void()> build_decimal_glyphs;
    std::function<void(std::uint32_t offset)> select_asset_offset;
};

void run_menu_text_display_sequence(std::uint16_t asset_index,
                                    const MenuTextDisplayCallbacks& callbacks);

void refresh_ready_entity_animations(
    std::vector<std::uint8_t>& entity_states,
    const std::function<void(std::size_t)>& refresh_entity);

void write_repeated_asset_batch(
    std::size_t batches,
    std::size_t blocks_per_batch,
    const std::function<void()>& write_block,
    std::uint8_t& transfer_marker);

std::size_t transfer_menu_glyph_triple(
    const std::function<std::optional<std::array<std::uint16_t, 3>>()>&
        read_source_triple,
    std::uint16_t tile_mask,
    const std::function<void(std::uint16_t)>& write_word);

} // namespace shadowrun::ghidra_conversion

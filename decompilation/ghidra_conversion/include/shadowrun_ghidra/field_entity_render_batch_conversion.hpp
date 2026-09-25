#pragma once

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

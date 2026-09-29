#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include <algorithm>
#include <utility>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

std::size_t emit_words(std::size_t word_count,
                       std::uint16_t tile_key,
                       std::size_t word_offset,
                       const std::uint16_t palette_or_flip,
                       const TileWordStreamCallbacks& callbacks,
                       const std::size_t tile_stride) {
    if (!callbacks.read_word || !callbacks.write_word) {
        return 0;
    }
    call_once(callbacks.begin_transfer);
    for (std::size_t index = 0; index < word_count; ++index) {
        auto word = callbacks.read_word(tile_key, word_offset);
        if ((palette_or_flip & 0x8000U) != 0U) {
            word = static_cast<std::uint16_t>(word ^ 0x8000U);
        } else {
            word = static_cast<std::uint16_t>(word | (palette_or_flip & 0x7FFFU));
        }
        callbacks.write_word(word);
        word_offset += 2U;
        if ((word_offset & (tile_stride - 1U)) == 0U) {
            word_offset -= tile_stride;
            tile_key = static_cast<std::uint16_t>(tile_key + 2U);
        }
    }
    return word_count;
}

} // namespace

void snapshot_entity_anchors(
    EntityAnchorSnapshotState& state,
    const std::array<EntityAnchor, 3>& source_anchors,
    const std::optional<std::size_t>& ready_slot,
    const std::function<void(std::size_t slot, EntityAnchor)>&
        publish_ready_anchor) {
    state.anchors = source_anchors;
    if (state.companion_restore && ready_slot.has_value() &&
        publish_ready_anchor) {
        publish_ready_anchor(
            *ready_slot,
            EntityAnchor{static_cast<std::int16_t>(state.anchors[0].x + 0x80),
                         state.anchors[0].y});
    }
    if (!state.pending_profile.has_value()) {
        return;
    }
    SceneProfileState profile{};
    SceneProfileDefaults unused{};
    apply_scene_profile_selection(profile, state.pending_profile, unused);
    state.camera_x = profile.camera_x;
    state.camera_y = profile.camera_y;
    state.camera_transition_x = profile.camera_transition_x;
    state.camera_transition_y = profile.camera_transition_y;
    for (std::size_t index = 0; index < state.anchors.size(); ++index) {
        state.anchors[index] = {
            static_cast<std::int16_t>(profile.runner_positions[index].x),
            static_cast<std::int16_t>(profile.runner_positions[index].y)};
    }
}

std::size_t emit_field_tile_column_words(
    const std::size_t word_count,
    const std::uint16_t tile_key,
    const std::size_t word_offset,
    const std::uint16_t palette_or_flip,
    const TileWordStreamCallbacks& callbacks) {
    return emit_words(word_count, tile_key, word_offset, palette_or_flip,
                      callbacks, 8U);
}

std::size_t emit_field_tile_row_words(
    const std::size_t word_count,
    const std::uint16_t tile_key,
    const std::size_t word_offset,
    const std::uint16_t palette_or_flip,
    const TileWordStreamCallbacks& callbacks) {
    return emit_words(word_count, tile_key, word_offset, palette_or_flip,
                      callbacks, 0x20U);
}

namespace {

std::uint16_t negate_word(const std::uint16_t value) {
    return static_cast<std::uint16_t>(0U - value);
}

} // namespace

FieldTilePlaneRenderPlan plan_field_tile_column(
    const std::uint16_t destination_word) {
    const auto folded = static_cast<std::uint16_t>(destination_word & 0x007FU);
    const auto visible = static_cast<std::uint16_t>(
        negate_word(static_cast<std::uint16_t>(folded - 0x0080U)) >> 1U);
    const auto remaining = static_cast<std::int16_t>(visible - 1U);
    if (remaining >= 0x002B) {
        return {0x002CU, 0U, destination_word};
    }
    return {static_cast<std::size_t>(remaining + 1),
            static_cast<std::size_t>(0x002A - remaining + 1),
            static_cast<std::uint16_t>(destination_word & 0xFF80U)};
}

FieldTilePlaneRenderPlan plan_field_tile_row(
    const std::uint16_t destination_word) {
    const auto folded = static_cast<std::uint16_t>(destination_word & 0x0F80U);
    const auto visible = static_cast<std::uint16_t>(
        negate_word(static_cast<std::uint16_t>(folded - 0x1000U)) >> 7U);
    const auto remaining = static_cast<std::int16_t>(visible - 1U);
    if (remaining >= 0x001F) {
        return {0x0020U, 0U, destination_word};
    }
    return {static_cast<std::size_t>(remaining + 1),
            static_cast<std::size_t>(0x001E - remaining + 1),
            static_cast<std::uint16_t>(destination_word & 0xF07FU)};
}

std::size_t upload_field_tile_row(
    FieldTileRowUploadState& state,
    const FieldTileRowUploadCallbacks& callbacks) {
    if (!callbacks.read_tile_key || !callbacks.read_word ||
        !callbacks.write_word) {
        return 0;
    }
    call_once(callbacks.begin_transfer);
    auto tile_key = callbacks.read_tile_key(state.row_table_byte_offset);
    std::size_t written = 0;
    auto remaining = state.remaining;
    do {
        auto word = callbacks.read_word(tile_key, state.word_offset);
        if (state.palette_word < 0) {
            word = static_cast<std::uint16_t>(word ^ 0x8000U);
        } else {
            word = static_cast<std::uint16_t>(
                word | static_cast<std::uint16_t>(state.palette_word));
        }
        callbacks.write_word(word);
        ++written;
        state.word_offset = static_cast<std::uint16_t>(state.word_offset + 8U);
        if (static_cast<std::int16_t>(state.word_offset) >= 0x001F) {
            state.word_offset =
                static_cast<std::uint16_t>(state.word_offset - 0x0020U);
            state.row_table_byte_offset = static_cast<std::uint16_t>(
                state.row_table_byte_offset + state.row_stride_bytes);
            tile_key = callbacks.read_tile_key(state.row_table_byte_offset);
        }
        remaining = static_cast<std::int16_t>(remaining - 1);
    } while (remaining != -1);
    call_once(callbacks.end_transfer);
    state.remaining = remaining;
    return written;
}

std::size_t render_field_tile_plane_rows(
    FieldTilePlaneFillState& state,
    const FieldTilePlaneFillCallbacks& callbacks) {
    call_once(callbacks.prepare_origin);
    if (callbacks.compose_destination) {
        state.destination = callbacks.compose_destination();
    }
    state.destination =
        static_cast<std::uint16_t>(state.destination + state.plane_offset);
    std::size_t rows = 0;
    auto remaining = static_cast<std::int16_t>(0x001E);
    do {
        if (callbacks.render_column) {
            callbacks.render_column(state.destination, state.y);
        }
        ++rows;
        const auto high =
            static_cast<std::uint16_t>(state.destination & 0xF000U);
        state.destination = static_cast<std::uint16_t>(
            high | static_cast<std::uint16_t>(
                       (state.destination + 0x0080U) & 0x0FFFU));
        state.y = static_cast<std::uint16_t>(state.y + 8U);
        remaining = static_cast<std::int16_t>(remaining - 1);
    } while (remaining != -1);
    return rows;
}

FieldTileRowEdgeResult render_field_tile_row_edge(
    const FieldTileRowEdgeState& state,
    const FieldTileRowEdgeCallbacks& callbacks) {
    FieldTileRowEdgeResult result{};
    const bool rendering_negative_edge = state.horizontal_direction < 0;
    const auto x_coordinate = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.camera_x >> 4U) +
        (rendering_negative_edge ? 8 : -8));

    if ((!rendering_negative_edge && x_coordinate < 0) ||
        (rendering_negative_edge && x_coordinate > state.camera_x_limit)) {
        result.action = FieldTileRowEdgeAction::outside_visible_window;
        return result;
    }

    const auto vertical_origin = prepare_field_vertical_origin(
        state.camera_y, state.field_y);
    const auto destination_row = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.base_destination) +
        (rendering_negative_edge ? static_cast<std::int16_t>(-0x0148)
                                 : static_cast<std::int16_t>(8)));
    const auto base_destination = compose_field_tile_destination_offset(
        destination_row, vertical_origin.camera_y);
    const auto source_layout = calculate_field_tile_source_layout(
        static_cast<std::uint16_t>(x_coordinate),
        static_cast<std::uint16_t>(vertical_origin.row),
        state.row_stride_bytes);

    const auto make_request =
        [&](const std::uint16_t plane_offset,
            const std::uint16_t palette_or_flip) {
            const auto destination_word = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(base_destination) + plane_offset);
            return FieldTileRowEdgeRequest{
                x_coordinate,
                vertical_origin.row,
                static_cast<std::int32_t>(static_cast<std::int16_t>(
                    destination_word)),
                plane_offset,
                palette_or_flip,
                source_layout};
        };

    result.first_request = make_request(state.plane_a_offset, 0U);
    result.second_request = make_request(state.plane_b_offset, 0x8000U);
    if (!callbacks.render_row) {
        result.action = FieldTileRowEdgeAction::boundary_unresolved;
        return result;
    }

    callbacks.render_row(*result.first_request);
    callbacks.render_row(*result.second_request);
    result.rows_rendered = 2U;
    result.action = FieldTileRowEdgeAction::rows_rendered;
    return result;
}

FieldTileColumnEdgeResult render_field_tile_column_edge(
    const FieldTileColumnEdgeState& state,
    const FieldTileColumnEdgeCallbacks& callbacks) {
    FieldTileColumnEdgeResult result{};
    const bool rendering_negative_edge = state.vertical_direction < 0;
    auto y_coordinate = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.field_y >> 4U) +
        (rendering_negative_edge ? -8 : 8));

    if ((rendering_negative_edge && y_coordinate < 0) ||
        (!rendering_negative_edge && y_coordinate > state.field_y_limit)) {
        result.action = FieldTileColumnEdgeAction::outside_visible_window;
        return result;
    }

    const auto horizontal_origin = prepare_field_horizontal_origin(
        state.camera_x, state.vdp_data_word);
    if (!rendering_negative_edge) {
        y_coordinate = static_cast<std::int16_t>(y_coordinate + 0x00E0);
    }
    const auto destination_row = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.camera_y) +
        (rendering_negative_edge ? static_cast<std::int16_t>(-8)
                                 : static_cast<std::int16_t>(0x00E8)));
    const auto base_destination = compose_field_tile_destination_offset(
        destination_row, horizontal_origin.vdp_data);
    const auto source_layout = calculate_field_tile_source_layout(
        static_cast<std::uint16_t>(horizontal_origin.column),
        static_cast<std::uint16_t>(y_coordinate),
        state.row_stride_bytes);

    const auto make_request =
        [&](const std::uint16_t plane_offset,
            const std::uint16_t palette_or_flip) {
            const auto destination_word = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(base_destination) + plane_offset);
            return FieldTileColumnEdgeRequest{
                horizontal_origin.column,
                y_coordinate,
                static_cast<std::int32_t>(static_cast<std::int16_t>(
                    destination_word)),
                plane_offset,
                palette_or_flip,
                source_layout};
        };

    result.first_request = make_request(state.plane_a_offset, 0U);
    result.second_request = make_request(state.plane_b_offset, 0x8000U);
    if (!callbacks.render_column) {
        result.action = FieldTileColumnEdgeAction::boundary_unresolved;
        return result;
    }

    callbacks.render_column(*result.first_request);
    callbacks.render_column(*result.second_request);
    result.columns_rendered = 2U;
    result.action = FieldTileColumnEdgeAction::columns_rendered;
    return result;
}

void update_entity_render_pass(
    std::array<RenderEntityState, 16>& entities,
    const EntityRenderPassCallbacks& callbacks) {
    for (auto& entity : entities) {
        if (!entity.active) {
            continue;
        }
        if (callbacks.project_screen_position) {
            callbacks.project_screen_position(entity);
        }
        if (callbacks.advance_animation) {
            callbacks.advance_animation(entity);
        }
        if (callbacks.advance_sprite_descriptor) {
            callbacks.advance_sprite_descriptor(entity);
        }
        if (callbacks.finish_entity) {
            callbacks.finish_entity(entity);
        }
    }
    call_once(callbacks.finish_pass);
}

void reset_effect_record(EffectRecordState& state,
                         const std::function<void()>& clear_slot) {
    state.coordinate = 0;
    state.flags = static_cast<std::uint8_t>(state.flags & ~0x01U);
    state.tile_bytes.fill(0);
    call_once(clear_slot);
}

void reset_effect_record(EffectRecordState& state,
                         std::array<std::uint8_t, 18>& slots,
                         std::vector<std::uint8_t>& collision_map) {
    state.coordinate = 0;
    clear_effect_slot(slots, state.slot);
    if (state.heading >= kEffectCollisionRestoreOffsets.size()) {
        return;
    }
    const auto& offsets = kEffectCollisionRestoreOffsets[state.heading];
    for (std::size_t index = 0; index < offsets.size(); ++index) {
        const auto map_index = static_cast<std::uint16_t>(
            state.collision_base + static_cast<std::uint16_t>(offsets[index]));
        if (map_index < collision_map.size()) {
            collision_map[map_index] = state.tile_bytes[index];
        }
    }
}

void clone_entity_spawn_variant_a(EntitySpawnRecord& destination,
                                  const EntitySpawnRecord& source) {
    destination = source;
    destination.mode = 5;
    destination.variant = 0x0D;
}

void clone_entity_spawn_variant_a(EntitySpawnCloneRecord& destination,
                                  const EntitySpawnCloneRecord& source,
                                  std::int16_t& phase_gate) {
    phase_gate = 4;
    destination.auxiliary = 0x0D;
    destination.slot = source.slot;
    destination.attribute = source.attribute;
    destination.countdown = 5;
    destination.phase = 0;
    destination.script = source.script;
}

void clone_entity_spawn_variant_b(EntitySpawnCloneRecord& destination,
                                  const EntitySpawnCloneRecord& source) {
    destination.auxiliary = 6;
    destination.slot = source.slot;
    destination.attribute = source.attribute;
    destination.countdown = 5;
    destination.phase = 0;
    destination.script = source.script;
}

void clone_entity_spawn_variant_b(EntitySpawnRecord& destination,
                                  const EntitySpawnRecord& source) {
    destination = source;
    destination.mode = 5;
    destination.variant = 6;
}

void advance_title_sprite_timer(
    TitleSpriteTimerState& state,
    const std::function<void(std::uint8_t phase)>& build_sprite_records) {
    --state.countdown;
    if (state.countdown < 0) {
        state.countdown = 3;
        state.phase = static_cast<std::uint8_t>((state.phase + 1U) & 3U);
    }
    if (build_sprite_records) {
        build_sprite_records(state.phase);
    }
}

void emit_title_sprite_burst(
    const std::uint16_t,
    const std::int16_t progress,
    const std::function<void()>& build_sprite_records) {
    if (progress < 8 || !build_sprite_records) {
        return;
    }
    for (int count = 0; count < 16; ++count) {
        build_sprite_records();
    }
}

std::vector<BuiltSpriteRecord> emit_title_sprite_burst(
    const TitleSpriteBurstState& state,
    const std::uint8_t first_sequence) {
    if (state.marker == 0x002AU) {
        return {};
    }
    const auto scale = static_cast<std::int16_t>(state.progress - 8);
    if (scale < 0) {
        return {};
    }
    const auto scale_word = static_cast<std::uint16_t>(scale);
    std::vector<BuiltSpriteRecord> records;
    auto sequence = first_sequence;
    const auto start = static_cast<std::size_t>(scale_word & 3U);
    for (std::size_t index = 0; index < kTitleBurstFactors.size(); ++index) {
        const auto x_delta = static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(
                static_cast<std::uint16_t>(kTitleBurstFactors[index][0])) *
            scale_word);
        const auto y_delta = static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(
                static_cast<std::uint16_t>(kTitleBurstFactors[index][1])) *
            scale_word);
        const auto built = build_title_sprite_records(
            {kTitleBurstSprites[(start + index) & 3U]},
            static_cast<std::int16_t>(
                static_cast<std::uint16_t>(state.screen_x + x_delta)),
            static_cast<std::int16_t>(
                static_cast<std::uint16_t>(state.screen_y + y_delta)),
            sequence, state.attributes);
        sequence = static_cast<std::uint8_t>(sequence + built.size());
        records.insert(records.end(), built.begin(), built.end());
    }
    return records;
}

namespace {

std::uint8_t entity_variant_selector(
    const EntityVariantSpriteState& state,
    const EntityVariantControlState& control) {
    if (control.vitality < 0) {
        return control.mode == 5U ? state.alternate : state.selector;
    }
    if (control.mode == 0U && (control.flags & 0x0CU) == 0U) {
        return state.alternate;
    }
    return state.selector;
}

std::size_t entity_variant_table(const std::uint8_t selector) {
    const auto signed_selector = static_cast<std::int8_t>(selector);
    if (signed_selector < 3) {
        return 0;
    }
    if (signed_selector < 6) {
        return 1;
    }
    if (signed_selector < 9) {
        return 2;
    }
    return 3;
}

std::vector<BuiltSpriteRecord> emit_entity_variant_list(
    const std::array<SpriteSourceRecord, 4>& sprites,
    const std::array<std::size_t, 4>& order,
    const std::int16_t y_offset,
    const std::int16_t x_offset,
    const std::uint8_t first_sequence,
    const std::uint16_t flags) {
    std::vector<BuiltSpriteRecord> records;
    auto sequence = first_sequence;
    for (const auto index : order) {
        auto built = build_title_sprite_records(
            {sprites[index]}, y_offset, x_offset, sequence, flags);
        sequence = static_cast<std::uint8_t>(sequence + built.size());
        records.insert(records.end(), built.begin(), built.end());
    }
    return records;
}

} // namespace

std::vector<BuiltSpriteRecord> emit_entity_variant_sprites(
    EntityVariantSpriteState& state,
    const EntityVariantControlState& control,
    const std::uint8_t first_sequence) {
    const auto table = entity_variant_table(
        entity_variant_selector(state, control));
    const auto flags = kEntityVariantFlags[table];
    const auto& sprites = kEntityVariantSprites[table];
    const auto y_base = static_cast<std::int16_t>(state.screen_x);
    auto x_base = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(state.screen_y - 0x0CU));
    if (state.marker == 0x000FU) {
        x_base = static_cast<std::int16_t>(
            static_cast<std::uint16_t>(x_base - 0x20));
    }
    const auto spread = static_cast<std::uint16_t>(state.spread);
    if (static_cast<std::int16_t>(spread) >= 0) {
        return emit_entity_variant_list(
            sprites, kEntityVariantSingleOrder, y_base, x_base,
            first_sequence, flags);
    }
    const auto decremented = static_cast<std::uint16_t>(spread - 1U);
    if (static_cast<std::int16_t>(decremented) >= 0) {
        state.spread = 0;
        return emit_entity_variant_list(
            sprites, kEntityVariantSingleOrder, y_base, x_base,
            first_sequence, flags);
    }
    state.spread = static_cast<std::int16_t>(decremented);
    const auto offset = static_cast<std::uint16_t>((decremented & 0x7FU) * 8U);
    const std::array<std::pair<std::int16_t, std::int16_t>, 4> corners{{
        {static_cast<std::int16_t>(
             static_cast<std::uint16_t>(y_base - offset)),
         static_cast<std::int16_t>(
             static_cast<std::uint16_t>(x_base - offset))},
        {static_cast<std::int16_t>(
             static_cast<std::uint16_t>(y_base + offset)),
         static_cast<std::int16_t>(
             static_cast<std::uint16_t>(x_base - offset))},
        {static_cast<std::int16_t>(
             static_cast<std::uint16_t>(y_base - offset)),
         static_cast<std::int16_t>(
             static_cast<std::uint16_t>(x_base + offset))},
        {static_cast<std::int16_t>(
             static_cast<std::uint16_t>(y_base + offset)),
         static_cast<std::int16_t>(
             static_cast<std::uint16_t>(x_base + offset))},
    }};
    std::vector<BuiltSpriteRecord> records;
    auto sequence = first_sequence;
    for (std::size_t index = 0; index < corners.size(); ++index) {
        auto built = build_title_sprite_records(
            {sprites[index]}, corners[index].first, corners[index].second,
            sequence, flags);
        sequence = static_cast<std::uint8_t>(sequence + built.size());
        records.insert(records.end(), built.begin(), built.end());
    }
    return records;
}

bool append_entity_render_sprite(SpriteExpansionState& state,
                                 const SpriteExpansionCallbacks& callbacks) {
    if ((state.flags & 0x80U) != 0U) {
        if (state.source_count == 0U) {
            return false;
        }
        if (callbacks.decode_field_tile_sample) {
            callbacks.decode_field_tile_sample(state);
        }
    }
    if ((state.flags & 0x20U) != 0U && callbacks.expand_packed_nibbles) {
        callbacks.expand_packed_nibbles(state);
    }
    if (callbacks.append_sprite) {
        callbacks.append_sprite(state);
    }
    state.appended = true;
    return true;
}

void initialize_entity_runtime(EntityRuntimeState& state,
                               const std::function<void()>& initialize_order) {
    state.entity_table_ready = true;
    state.entity_table_token = 0xFF0100U;
    state.sprite_cursor = 0xFF0000U;
    state.sprite_count = 0;
    state.secondary_cursor = 0xFF1480U;
    call_once(initialize_order);
}

bool dispatch_player_action_if_ready(
    const std::uint8_t auxiliary_marker,
    const std::uint8_t action_code,
    const std::int16_t status,
    const std::function<void()>& dispatch_action) {
    if (auxiliary_marker != 0U || action_code == 0U || status >= 0) {
        return false;
    }
    call_once(dispatch_action);
    return true;
}

bool clear_entity_effect_if_forced(
    const std::uint8_t behavior_flags,
    std::uint8_t& route_flags,
    const std::int16_t forced_effect_word,
    const std::function<void()>& advance_movement_and_mode_transition) {
    if ((behavior_flags & 0x08U) == 0U) {
        return false;
    }
    if (forced_effect_word == 0) {
        route_flags = static_cast<std::uint8_t>(route_flags & ~0x20U);
        return false;
    }
    if (forced_effect_word > 0) {
        return false;
    }
    call_once(advance_movement_and_mode_transition);
    return true;
}

void update_entity_target_heading(TargetHeadingState& state,
                                  const TargetHeadingCallbacks& callbacks) {
    if (state.mode == 3U) {
        state.relation_rejected = false;
        return;
    }
    if (state.has_target && callbacks.measure_target_heading) {
        state.target_heading = callbacks.measure_target_heading();
        state.relation_rejected = callbacks.relation_is_rejected &&
                                  callbacks.relation_is_rejected();
        if (state.relation_rejected) {
            call_once(callbacks.reset_heading);
            return;
        }
    }
    call_once(callbacks.use_fallback_heading);
}

std::size_t render_encoded_title_text(
    const std::vector<std::uint8_t>& encoded,
    const std::function<void(std::uint16_t glyph_index)>& write_glyph) {
    if (!write_glyph) {
        return 0;
    }
    std::size_t count = 0;
    for (const auto value : encoded) {
        if ((value & 0x80U) != 0U) {
            break;
        }
        auto glyph = static_cast<std::uint16_t>(value - 0x20U);
        if (glyph >= 0x3DU) {
            glyph = static_cast<std::uint16_t>(value - 0x24U);
        }
        write_glyph(glyph);
        ++count;
    }
    return count;
}

void select_menu_value(MenuValueState& state,
                       const std::vector<std::uint8_t>& table,
                       const MenuValueCursor cursor,
                       const MenuValueRecord& record) {
    const auto slot = static_cast<std::size_t>(
        (static_cast<std::uint16_t>(cursor.row) * 7U + cursor.column + 1U) *
        2U);
    if (slot + 1U >= table.size()) {
        state.primary = 0;
        state.selected = 0;
        return;
    }
    const auto spell_code = table[slot];
    state.primary = table[slot + 1U];
    if (record.inventory_slot < 0 &&
        static_cast<std::uint8_t>(spell_code + 1U) == record.item_code) {
        state.selected = record.preserved_force;
        return;
    }
    state.selected = state.primary;
}

void run_menu_text_display_sequence(
    const std::uint16_t asset_index,
    const MenuTextDisplayCallbacks& callbacks) {
    call_once(callbacks.prepare_grid);
    call_once(callbacks.prepare_text);
    call_once(callbacks.render_text);
    call_once(callbacks.render_text);
    call_once(callbacks.render_text);
    call_once(callbacks.render_text);
    if (callbacks.select_asset_offset) {
        callbacks.select_asset_offset(static_cast<std::uint32_t>(asset_index) *
                                      0x30U);
    }
    call_once(callbacks.build_decimal_glyphs);
}

void refresh_ready_entity_animations(
    std::vector<std::uint8_t>& entity_states,
    const std::function<void(std::size_t)>& refresh_entity) {
    for (std::size_t index = 0; index < entity_states.size(); ++index) {
        if ((entity_states[index] & 1U) != 0U && refresh_entity) {
            refresh_entity(index);
        }
    }
}

void write_repeated_asset_batch(
    const std::size_t batches,
    const std::size_t blocks_per_batch,
    const std::function<void()>& write_block,
    std::uint8_t& transfer_marker) {
    if (write_block) {
        for (std::size_t batch = 0; batch < batches; ++batch) {
            for (std::size_t block = 0; block < blocks_per_batch; ++block) {
                write_block();
            }
        }
    }
    transfer_marker = 0;
}

std::size_t transfer_menu_glyph_triple(
    const std::function<std::optional<std::array<std::uint16_t, 3>>()>&
        read_source_triple,
    const std::uint16_t tile_mask,
    const std::function<void(std::uint16_t)>& write_word) {
    if (!read_source_triple || !write_word) {
        return 0;
    }
    std::size_t count = 0;
    while (const auto triple = read_source_triple()) {
        for (const auto word : *triple) {
            write_word(static_cast<std::uint16_t>(tile_mask | word + 0x03E9U));
            ++count;
        }
    }
    return count;
}

} // namespace shadowrun::ghidra_conversion

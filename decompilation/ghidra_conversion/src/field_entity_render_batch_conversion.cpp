#include "shadowrun_ghidra/field_entity_render_batch_conversion.hpp"

#include <algorithm>

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
            word_offset -= tile_stride - 2U;
            tile_key = static_cast<std::uint16_t>(tile_key + 2U);
        }
    }
    return word_count;
}

} // namespace

void snapshot_entity_anchors(
    EntityAnchorSnapshotState& state,
    const std::array<EntityAnchor, 3>& source_anchors,
    const std::optional<EntityAnchor>& ready_anchor,
    const std::array<std::uint16_t, 8>& pending_source,
    const std::function<void(const EntityAnchor&)>& publish_ready_anchor) {
    state.anchors = source_anchors;
    state.pending_anchor = ready_anchor;
    if (ready_anchor) {
        if (publish_ready_anchor) {
            publish_ready_anchor(*ready_anchor);
        }
        state.pending_anchor = std::nullopt;
    }
    std::copy_n(pending_source.begin(), state.pending_profile.size(),
                state.pending_profile.begin());
    state.pending_profile_active = true;
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

void clone_entity_spawn_variant_a(EntitySpawnRecord& destination,
                                  const EntitySpawnRecord& source) {
    destination = source;
    destination.mode = 5;
    destination.variant = 0x0D;
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

bool append_entity_render_sprite(SpriteExpansionState& state,
                                 const SpriteExpansionCallbacks& callbacks) {
    if ((state.flags & 0x80U) != 0U && state.source_count == 0U) {
        return false;
    }
    if ((state.flags & 0x80U) != 0U && callbacks.expand_packed_nibbles) {
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
    std::uint8_t& entity_flags,
    const std::int16_t global_effect_state,
    const std::function<void()>& handle_world_effect) {
    if ((entity_flags & 0x08U) == 0U || global_effect_state >= 0) {
        return false;
    }
    entity_flags = static_cast<std::uint8_t>(entity_flags & ~0x20U);
    call_once(handle_world_effect);
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
                       const std::vector<std::uint8_t>& row_table,
                       const std::size_t row_index,
                       const bool special_entity_state,
                       const std::uint8_t special_value) {
    const auto offset = row_index * 2U;
    if (offset + 1U >= row_table.size()) {
        state.primary = 0;
        state.selected = 0;
        return;
    }
    state.primary = row_table[offset + 1U];
    state.selected = state.primary;
    if (special_entity_state && row_table[offset] + 1U == special_value) {
        state.selected = special_value;
    }
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

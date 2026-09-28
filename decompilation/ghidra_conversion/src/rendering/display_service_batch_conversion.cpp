#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"

#include <algorithm>
#include <cstddef>

namespace shadowrun::ghidra_conversion {

void service_display_frame(DisplayServiceState& state,
                           const DisplayServiceCallbacks& callbacks) {
    ++state.frame_counter;
    state.service_marker = 0;

    const auto service_dirty = [&](const std::uint8_t bit,
                                   const std::function<void()>& action) {
        if ((state.dirty_flags & bit) == 0U) {
            return;
        }
        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags &
                                                       ~bit);
        if (action) {
            action();
        }
    };

    if ((state.dirty_flags & 0x02U) != 0U) {
        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags &
                                                       ~0x02U);
        if (callbacks.service_pending_upload) {
            callbacks.service_pending_upload();
        }
        state.display_ready = true;
    }
    service_dirty(0x01U, callbacks.upload_startup_palette);
    if (!state.display_ready && callbacks.service_idle) {
        callbacks.service_idle();
    }
    service_dirty(0x04U, callbacks.write_transfer_header);
    service_dirty(0x08U, callbacks.write_fixed_transfer_pair);
    if (state.render_normal && callbacks.render_normal_viewport) {
        callbacks.render_normal_viewport();
    }
    if (state.render_alternate && callbacks.render_alternate_viewport) {
        callbacks.render_alternate_viewport();
    }
    if (state.flush_primary && callbacks.flush_primary_transfers) {
        callbacks.flush_primary_transfers();
    }
    if (state.flush_secondary && callbacks.flush_secondary_transfers) {
        callbacks.flush_secondary_transfers();
    }
}

void service_display_frame_wrapper(
    DisplayServiceState& state,
    const DisplayServiceCallbacks& callbacks) {
    service_display_frame(state, callbacks);
}

void write_alternating_word_pair_wrapper(
    const std::function<void()>& write_pair) {
    if (write_pair) {
        write_pair();
    }
}

bool prepare_field_scene_buffers(
    FieldSceneBuffers& state,
    const FieldSceneBufferInput& input,
    const FieldScenePreparationCallbacks& callbacks) {
    if (input.layer_a.size() < kFieldTerrainLayerWordCount ||
        input.layer_b.size() < kFieldTerrainLayerWordCount) {
        state = FieldSceneBuffers{};
        return false;
    }
    if (!state.initialized && callbacks.prepare_field_scene_runtime) {
        callbacks.prepare_field_scene_runtime();
    }
    if (!state.initialized && callbacks.initialize_scene_profile) {
        callbacks.initialize_scene_profile();
    }
    const bool alternate = callbacks.scene_uses_alternate_profile &&
                           callbacks.scene_uses_alternate_profile();
    state.scene_mode = alternate ? 1U : 0U;
    state.field_resource_mode = static_cast<std::uint8_t>(input.scene_reference);
    state.scene_reference = input.scene_reference;
    state.profile = alternate ? input.alternate_profile : input.normal_profile;
    state.camera_x = state.profile[0];
    state.camera_y = state.profile[1];
    state.terrain_stream = input.terrain_stream;
    state.pending_position_consumed = false;
    state.pending_position_cleared = false;
    state.pending_field_targets = {};
    if (input.pending_position_available && !input.pending_position_in_use) {
        state.pending_field_targets.fill(input.pending_position);
        state.camera_x = std::max<std::int32_t>(
            0, static_cast<std::int32_t>(input.pending_position.x) - 0x0A00);
        state.camera_y = std::max<std::int32_t>(
            0, static_cast<std::int32_t>(input.pending_position.y) - 0x0700);
        state.pending_position_consumed = true;
    }
    state.pending_position_cleared = true;
    state.layer_a.assign(input.layer_a.begin(),
                         input.layer_a.begin() + kFieldTerrainLayerWordCount);
    state.layer_b.assign(input.layer_b.begin(),
                         input.layer_b.begin() + kFieldTerrainLayerWordCount);
    state.source_metadata = input.source_metadata;
    state.initialized = true;
    return true;
}

void write_hud_panel_transfer(
    const std::uint16_t initial_word,
    const HudPanelTransferCallbacks& callbacks) {
    if (!callbacks.begin_transfer || !callbacks.write_word) {
        return;
    }
    auto next_word = initial_word;
    while (true) {
        const auto count = callbacks.begin_transfer();
        if (!count) {
            return;
        }
        for (std::size_t index = 0; index < *count; ++index) {
            callbacks.write_word(next_word++);
        }
    }
}

std::size_t render_field_tile_column(
    const FieldTileColumnInput& input,
    const FieldTileColumnCallbacks& callbacks) {
    const auto half_visible = static_cast<std::size_t>(
        (0x80U - (input.source_coordinate & 0x7FU)) >> 1U);
    if (!callbacks.read_word || !callbacks.write_word) {
        return 0;
    }

    // ROM 0x3D4A calculates D7 as the number of words before the next
    // 0x80-aligned destination, then calls the shared 0x3D80 body.  When the
    // source is not already at the clamp boundary, the body is entered once
    // for that partial segment and then again by linear continuation after
    // D4 is aligned.  The old conversion only modeled the second segment.
    struct Cursor {
        std::uint16_t tile_key;
        std::size_t word_offset;
    } cursor{input.tile_key, input.source_word_offset};

    const auto emit_segment = [&](const std::size_t word_count) {
        if (callbacks.begin_transfer) {
            callbacks.begin_transfer();
        }
        for (std::size_t index = 0; index < word_count; ++index) {
            auto word = callbacks.read_word(cursor.tile_key, cursor.word_offset);
            if ((input.palette_or_flip & 0x8000U) != 0U) {
                word = static_cast<std::uint16_t>(word ^ 0x8000U);
            } else {
                word = static_cast<std::uint16_t>(
                    word | (input.palette_or_flip & 0x7FFFU));
            }
            callbacks.write_word(word);
            cursor.word_offset += 2U;
            if ((cursor.word_offset & 7U) == 0U) {
                cursor.word_offset -= 6U;
                cursor.tile_key =
                    static_cast<std::uint16_t>(cursor.tile_key + 2U);
            }
        }
    };

    // The source clamps D7 to 0x2B, so the common tail always contains 44
    // words.  A zero half-visible value is an invalid source alignment for
    // the original routine; treating it as the already-aligned case keeps
    // the host boundary total and avoids exposing a 68000 DBF underflow.
    if (half_visible == 0U || half_visible >= 0x2CU) {
        emit_segment(0x2CU);
        return 0x2CU;
    }

    emit_segment(half_visible);
    emit_segment(0x2CU - half_visible);
    return 0x2CU;
}

void render_field_tile_rows(const std::uint16_t initial_column_key,
                            const std::int16_t initial_row_sum,
                            const FieldTileRowCallbacks& callbacks) {
    if (callbacks.fold_coordinate) {
        callbacks.fold_coordinate();
    }
    const auto start = callbacks.column_start ? callbacks.column_start() : 0;
    auto column_key = initial_column_key;
    for (std::int16_t row = 0x1E; row >= 0; --row) {
        if (callbacks.render_column) {
            callbacks.render_column(row,
                                    static_cast<std::uint16_t>(column_key),
                                    start);
        }
        column_key = static_cast<std::uint16_t>(initial_row_sum & 0xF000);
    }
}

std::optional<std::int32_t> calculate_field_tile_source_offset(
    const std::vector<std::int16_t>& row_offsets,
    const std::size_t row_index,
    const std::int32_t base_offset,
    const std::vector<std::uint16_t>& map_words) {
    if (row_index >= row_offsets.size()) {
        return std::nullopt;
    }
    const auto byte_offset = base_offset +
                             static_cast<std::int32_t>(row_offsets[row_index]) *
                                 0x20;
    if (byte_offset < 0 || (byte_offset % 2) != 0 ||
        static_cast<std::size_t>(byte_offset / 2) >= map_words.size()) {
        return std::nullopt;
    }
    const auto tile = static_cast<std::int32_t>(
        map_words[static_cast<std::size_t>(byte_offset / 2)] & 0x07FFU);
    return (tile - 0x0201) * 0x20;
}

bool arm_controller_activity_delay(ControllerDelayState& state) {
    const bool ready = (state.controller_flags & 0x40U) != 0U &&
                       (((state.entity_flags[0] & 0x01U) == 0U) ||
                        ((state.entity_flags[0] & 0x40U) != 0U)) &&
                       (((state.entity_flags[1] & 0x01U) == 0U) ||
                        ((state.entity_flags[1] & 0x40U) != 0U));
    if (ready) {
        state.delay = 0x3CU;
    }
    return ready;
}

void dispatch_entity_collision_behavior(
    const std::uint8_t entity_flags,
    const bool interaction_marker,
    const EntityBehaviorDispatchCallbacks& callbacks) {
    if ((entity_flags & 0x08U) == 0U) {
        if (callbacks.behavior_without_collision_flag) {
            callbacks.behavior_without_collision_flag();
        }
    } else if (callbacks.behavior_with_collision_flag) {
        callbacks.behavior_with_collision_flag();
    }
    if (interaction_marker && callbacks.schedule_collision_recheck) {
        callbacks.schedule_collision_recheck();
    }
}

bool advance_title_sprite_wave(
    TitleSpriteWaveState& state,
    const std::array<std::uint32_t, 4>& palettes,
    const TitleSpriteWaveCallbacks& callbacks) {
    state.timer = static_cast<std::uint16_t>(state.timer + 8U);
    if (state.timer >= 0xA0U) {
        state.active = false;
        return false;
    }
    if (!callbacks.build) {
        return true;
    }
    const auto palette = palettes[(state.palette_selector >> 2U) & 3U];
    for (std::uint16_t offset = 0; offset < 0x100U; offset += 8U) {
        callbacks.build(state.base_x, state.base_y, offset, palette);
    }
    return true;
}

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void load_title_font(const std::function<void()>& decode_and_upload) {
    call_once(decode_and_upload);
}

void write_menu_grid_with_metadata(const std::function<void()>& write_grid) {
    call_once(write_grid);
}

void render_menu_grid_number_pair(const std::function<void()>& write_grid,
                                  const std::function<void()>& build_glyphs) {
    call_once(write_grid);
    call_once(build_glyphs);
}

void prepare_hotel_options_display(
    const std::function<void()>& refresh_mode,
    const std::function<void()>& refresh_palette,
    const std::function<void()>& prepare_display,
    const std::function<void()>& load_font) {
    call_once(refresh_mode);
    call_once(refresh_palette);
    call_once(prepare_display);
    call_once(load_font);
}

void render_attributes_skills_heading(
    const std::function<void()>& render_text) {
    call_once(render_text);
}

void advance_random_grid_effect(
    RandomGridEffectState& state,
    const RandomGridEffectCallbacks& callbacks) {
    state.random_samples = 0U;

    const auto sample = [&](const std::uint16_t bound) {
        ++state.random_samples;
        if (!callbacks.sample_random || bound == 0U) {
            return static_cast<std::uint16_t>(0U);
        }
        return std::min<std::uint16_t>(
            callbacks.sample_random(bound), static_cast<std::uint16_t>(bound - 1U));
    };

    const std::uint16_t first = sample(2U);
    state.plan = {};
    state.plan.first_selection_nonzero = first != 0U;
    if (first == 0U) {
        state.plan.primary_source = RandomGridPrimarySource::branch_zero_table;
        state.plan.primary_entry_index =
            static_cast<std::uint8_t>(sample(8U));
        state.plan.variant_source =
            RandomGridVariantSource::fixed_three_variant_resources;
        state.plan.variant_entry_index =
            static_cast<std::uint8_t>(sample(3U));
    } else {
        state.plan.primary_source = RandomGridPrimarySource::branch_nonzero_table;
        state.plan.primary_entry_index =
            static_cast<std::uint8_t>(sample(4U));
        state.plan.variant_source =
            RandomGridVariantSource::indexed_four_variant_resources;
        state.plan.variant_entry_index =
            static_cast<std::uint8_t>(sample(4U));
    }

    state.grid_write_requested = true;
    if (callbacks.write_grid) {
        callbacks.write_grid(state.plan);
    }
}

void write_menu_footer_grid(const std::function<void()>& write_grid) {
    call_once(write_grid);
}

void write_menu_grid_quadruple(const std::function<void()>& write_grid) {
    if (!write_grid) {
        return;
    }
    for (int count = 0; count < 4; ++count) {
        write_grid();
    }
}

} // namespace shadowrun::ghidra_conversion

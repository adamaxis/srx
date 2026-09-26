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

void prepare_field_scene_buffers(
    FieldSceneBuffers& state,
    const std::array<std::uint16_t, 48>& normal_profile,
    const std::array<std::uint16_t, 48>& alternate_profile,
    const std::uint16_t scene_reference,
    const std::int32_t camera_x,
    const std::int32_t camera_y,
    const std::vector<std::uint16_t>& layer_a,
    const std::vector<std::uint16_t>& layer_b,
    const FieldScenePreparationCallbacks& callbacks) {
    if (!state.initialized && callbacks.initialize_scene_profile) {
        callbacks.initialize_scene_profile();
    }
    const bool alternate = callbacks.scene_uses_alternate_profile &&
                           callbacks.scene_uses_alternate_profile();
    state.scene_mode = alternate ? 1U : 0U;
    state.profile = alternate ? alternate_profile : normal_profile;
    state.scene_reference = scene_reference;
    state.camera_x = camera_x;
    state.camera_y = camera_y;
    if (state.recenter_pending) {
        state.camera_x = std::max<std::int32_t>(0, state.camera_x - 0x0A00);
        state.camera_y = std::max<std::int32_t>(0, state.camera_y - 0x0700);
    }
    state.recenter_pending = false;
    state.layer_a = layer_a;
    state.layer_b = layer_b;
    state.initialized = true;
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
    if (half_visible >= 0x2BU || !callbacks.read_word ||
        !callbacks.write_word) {
        return 0;
    }
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer();
    }
    const auto word_count = 0x2CU - half_visible;
    std::size_t word_offset = input.source_word_offset;
    std::uint16_t tile_key = input.tile_key;
    for (std::size_t index = 0; index < word_count; ++index) {
        auto word = callbacks.read_word(tile_key, word_offset);
        if ((input.palette_or_flip & 0x8000U) != 0U) {
            word = static_cast<std::uint16_t>(word ^ 0x8000U);
        } else {
            word = static_cast<std::uint16_t>(word |
                                              (input.palette_or_flip & 0x7FFFU));
        }
        callbacks.write_word(word);
        word_offset += 2U;
        if ((word_offset & 7U) == 0U) {
            word_offset -= 6U;
            tile_key = static_cast<std::uint16_t>(tile_key + 2U);
        }
    }
    return word_count;
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

void prepare_title_mode_display(const std::function<void()>& refresh_mode,
                                const std::function<void()>& refresh_palette,
                                const std::function<void()>& prepare_display,
                                const std::function<void()>& load_font) {
    call_once(refresh_mode);
    call_once(refresh_palette);
    call_once(prepare_display);
    call_once(load_font);
}

void render_title_text_wrapper(const std::function<void()>& render_text) {
    call_once(render_text);
}

void advance_random_grid_effect(const std::function<std::int16_t()>& sample,
                                const std::function<void()>& write_grid) {
    if (sample) {
        sample();
        sample();
        sample();
    }
    call_once(write_grid);
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

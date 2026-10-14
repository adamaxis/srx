#include "shadowrun_ghidra/core/remaining_field_conversion.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

std::uint8_t saturating_add(const std::uint8_t value, const std::int16_t delta) {
    const auto result = static_cast<int>(value) + delta;
    return static_cast<std::uint8_t>(std::clamp(result, 0, 10));
}

std::int16_t add_phase_meter_window(const std::int8_t meter,
                                    const std::int16_t score) {
    if (meter >= 8) {
        return score;
    }
    return static_cast<std::int16_t>(score + (meter < 5 ? 2 : 1));
}

} // namespace

std::int16_t guard_entity_phase_window(const std::int8_t delay_meter,
                                       const std::int8_t mental_meter,
                                       const std::int16_t score) {
    return add_phase_meter_window(
        mental_meter, add_phase_meter_window(delay_meter, score));
}

void prepare_title_resources(
    TitleResourcePreparation& state,
    const std::uint16_t header_a,
    const std::uint16_t header_b,
    const std::uint16_t header_c,
    const std::uint16_t header_d,
    const std::vector<std::uint16_t>& resource_descriptors,
    const std::vector<std::uint16_t>& sprite_descriptors,
    const std::function<void(const TitleResourceTransfer&)>& upload,
    const std::function<void(std::uint16_t)>& draw_sprite_stream) {
    state.header_a = header_a;
    state.header_b = header_b;
    state.header_c = header_c;
    state.header_d = header_d;
    state.transfers.clear();
    state.sprite_streams.clear();
    for (const auto descriptor : resource_descriptors) {
        if (descriptor == 0U) {
            break;
        }
        const TitleResourceTransfer transfer{
            (descriptor & 1U) != 0U,
            static_cast<std::uint16_t>(descriptor & 0xFFFEU)};
        state.transfers.push_back(transfer);
        if (upload) {
            upload(transfer);
        }
    }
    for (std::size_t index = 0; index + 6U < sprite_descriptors.size();
         index += 7U) {
        if (sprite_descriptors[index] == 0U) {
            break;
        }
        const auto stream = sprite_descriptors[index];
        state.sprite_streams.push_back(stream);
        if (draw_sprite_stream) {
            draw_sprite_stream(stream);
        }
    }
    state.display_dirty = true;
}

std::size_t render_hud_sprite_stream(
    const std::vector<std::uint8_t>& encoded,
    std::uint16_t initial_tile,
    const std::uint16_t row_word_limit,
    const std::uint16_t palette_or_flip,
    const HudSpriteStreamCallbacks& callbacks) {
    if (!callbacks.write_word || row_word_limit == 0U) {
        return 0U;
    }
    call_once(callbacks.begin_transfer);
    std::size_t cursor = 0;
    std::size_t emitted = 0;
    std::uint16_t tile = initial_tile;
    std::uint16_t row_words = 0;
    while (cursor < encoded.size()) {
        const auto code = encoded[cursor++];
        if (code == 0U) {
            break;
        }
        std::size_t count = 1U;
        std::int16_t direction = 1;
        std::uint16_t value = code;
        if (code >= 0xC1U) {
            count = static_cast<std::size_t>(code & 0x1FU);
            if (count == 0U) {
                count = 1U;
            }
            if ((code & 0x20U) != 0U && cursor + 1U < encoded.size()) {
                value = static_cast<std::uint16_t>(encoded[cursor] << 8U) |
                        encoded[cursor + 1U];
                cursor += 2U;
            } else if (cursor < encoded.size()) {
                value = encoded[cursor++];
            }
            direction = (code & 0x40U) != 0U ? -1 : 1;
        } else {
            if ((code & 0x20U) != 0U && cursor + 1U < encoded.size()) {
                value = static_cast<std::uint16_t>(encoded[cursor] << 8U) |
                        encoded[cursor + 1U];
                cursor += 2U;
            }
            direction = code < 0x41U ? 1 : -1;
        }
        for (std::size_t repeat = 0; repeat < count; ++repeat) {
            auto output = value;
            if ((palette_or_flip & 0x8000U) != 0U) {
                output = static_cast<std::uint16_t>(output ^ 0x8000U);
            } else {
                output = static_cast<std::uint16_t>(
                    output | (palette_or_flip & 0x7FFFU));
            }
            callbacks.write_word(output);
            ++emitted;
            tile = static_cast<std::uint16_t>(tile + direction);
            if (++row_words == row_word_limit) {
                row_words = 0U;
                call_once(callbacks.wrap_transfer);
            }
        }
    }
    return emitted;
}

void advance_field_mode_transition(
    FieldModeTransitionState& state,
    const std::vector<std::int8_t>& mode_table,
    const std::vector<std::uint16_t>& resource_table,
    const std::vector<std::uint16_t>& coordinate_table,
    const std::function<void()>& begin_world_activity,
    const std::function<void()>& schedule_timeout) {
    const auto mode_index = static_cast<std::size_t>(state.scene) * 4U +
                            static_cast<std::size_t>(state.mode);
    if (mode_index < mode_table.size()) {
        state.mode = static_cast<std::uint8_t>(mode_table[mode_index]);
    }
    if (state.mode == 0x13U && !state.world_activity_started) {
        state.world_activity_started = true;
        call_once(begin_world_activity);
        state.timeout_scheduled = true;
        call_once(schedule_timeout);
    }
    const auto resource_index = std::min<std::size_t>(state.mode,
                                                       resource_table.size());
    if (resource_index < resource_table.size()) {
        state.display_resource = resource_table[resource_index];
    }
    const auto coordinate_index = static_cast<std::size_t>(state.display_resource) * 2U;
    if (coordinate_index + 1U < coordinate_table.size()) {
        state.camera_x = coordinate_table[coordinate_index];
        state.camera_y = coordinate_table[coordinate_index + 1U];
    }
}

void toggle_field_scene_variant(
    FieldVariantState& state,
    const std::function<void(std::uint8_t, std::uint16_t)>& select_position) {
    if (state.scene != 0x10U) {
        return;
    }
    if (state.display_resource == 0U) {
        state.variant = state.camera_x < 0x1400U ? 3U : 5U;
        state.pending_position = true;
    } else {
        state.pending_position = true;
        state.variant = state.display_resource == 5U ? 0U : 0U;
    }
    if (select_position) {
        select_position(state.variant, state.camera_x);
    }
}

HudPanelSelection select_hud_panel_variant(const bool zero_flag,
                                           const std::int16_t mode) {
    if (zero_flag) {
        return {0x86A4U, 0xF448U};
    }
    if (mode == 1) {
        return {0x86BAU, 0xF848U};
    }
    return {0x86D0U, 0xC48U};
}

void refresh_hud_panel_variant(
    const bool zero_flag,
    const std::int16_t mode,
    const std::function<void(HudPanelSelection)>& draw_panel,
    const std::function<void()>& post_hook) {
    const auto selection = select_hud_panel_variant(zero_flag, mode);
    if (draw_panel) {
        draw_panel(selection);
    }
    call_once(post_hook);
    if (draw_panel) {
        draw_panel(selection);
    }
}

void advance_entity_delay_meter(EntityMeters& state, const std::int16_t delta,
                                const std::function<void()>& write_grid) {
    if ((state.state_flags & 0x02U) != 0U) {
        state.delay = saturating_add(state.delay, delta);
    } else {
        state.delay = 0U;
    }
    call_once(write_grid);
}

void advance_entity_asset_meter(
    EntityMeters& state,
    const std::int16_t delta,
    const std::vector<std::uint8_t>& cap_table,
    const std::function<void()>& reload_asset,
    const std::function<void()>& write_grid) {
    if ((state.state_flags & 0x01U) == 0U) {
        state.asset = 0U;
    } else if (state.animation_mode < 0x0DU) {
        const auto candidate = static_cast<int>(state.asset) + delta;
        const auto cap = state.animation_mode < cap_table.size()
                             ? cap_table[state.animation_mode]
                             : 0xFFU;
        state.asset = static_cast<std::uint8_t>(
            std::clamp(candidate, 0, static_cast<int>(cap)));
    } else if (state.animation_mode < 0x10U) {
        state.slot_value = static_cast<std::uint8_t>(state.slot_value + delta);
        if (state.slot_value == 0U) {
            state.asset = 0U;
            call_once(reload_asset);
        }
    }
    call_once(write_grid);
}

void advance_entity_behavior_meter(EntityMeters& state, const std::int16_t delta,
                                   const std::function<void()>& write_grid) {
    if ((state.state_flags & 0x01U) != 0U) {
        state.behavior = saturating_add(state.behavior, delta);
    } else {
        state.behavior = 0U;
    }
    call_once(write_grid);
}

bool accept_camera_delta(const std::uint16_t current,
                         const std::uint16_t target,
                         const std::int8_t direction) {
    const auto delta = target >= current ? target - current : current - target;
    if (delta > 0x2FFU) {
        return false;
    }
    if (delta > 0x17FU) {
        return direction == (target >= current ? 2 : -2);
    }
    return direction == (target >= current ? 1 : -1);
}

void update_field_camera(
    FieldCameraState& state,
    const std::int16_t delta_x,
    const std::int16_t delta_y,
    const std::function<void(std::uint16_t)>& render_horizontal_edge,
    const std::function<void(std::uint16_t)>& render_vertical_edge,
    const std::function<void()>& reset_camera) {
    if (state.scene_state == 3U) {
        state.x = 0;
        state.y = 0;
        state.aligned_x = 0x600U;
        call_once(reset_camera);
        return;
    }
    state.x = std::clamp<std::int16_t>(state.x + delta_x, state.min_x, state.max_x);
    state.y = std::clamp<std::int16_t>(state.y + delta_y, state.min_y, state.max_y);
    const auto aligned_x = static_cast<std::uint16_t>(state.x) & 0xFF80U;
    const auto aligned_y = static_cast<std::uint16_t>(state.y) & 0xFF80U;
    if (aligned_x != state.aligned_x) {
        state.aligned_x = aligned_x;
        if (render_horizontal_edge) {
            render_horizontal_edge(aligned_x);
        }
    }
    if (aligned_y != state.aligned_y) {
        state.aligned_y = aligned_y;
        if (render_vertical_edge) {
            render_vertical_edge(aligned_y);
        }
    }
}

FieldTileSample decode_field_tile_sample(
    const std::uint16_t camera_x,
    const std::uint16_t camera_y,
    const std::int16_t entity_x,
    const std::int16_t entity_y,
    const std::uint16_t packed_region,
    const std::vector<std::uint8_t>& source,
    const std::function<std::uint32_t(std::uint16_t, std::int16_t)>&
        calculate_source_offset) {
    FieldTileSample result{};
    const auto x = static_cast<std::uint16_t>((camera_x >> 4U) + entity_x - 0x80);
    const auto y = static_cast<std::int16_t>((camera_y >> 4U) + entity_y - 0x80);
    result.source_offset = calculate_source_offset
                               ? calculate_source_offset(x, y)
                               : 0U;
    const auto mask = static_cast<std::uint8_t>(
        packed_region == 0U ? 0x0FU : (packed_region & 0x0FU) == 0U ? 0xF0U : 0xFFU);
    for (std::size_t index = 0; index < 32U; ++index) {
        const auto source_index = static_cast<std::size_t>(result.source_offset) + index;
        result.masked_words.push_back(source_index < source.size()
                                          ? static_cast<std::uint8_t>(source[source_index] & mask)
                                          : 0U);
    }
    return result;
}

void render_field_tile_edge(
    const std::uint16_t tile_key,
    const std::size_t word_count,
    const std::uint16_t palette_or_flip,
    const std::function<std::uint16_t(std::uint16_t, std::size_t)>& read_word,
    const std::function<void(std::uint16_t)>& write_word,
    const std::function<void()>& begin_transfer) {
    if (!read_word || !write_word) {
        return;
    }
    call_once(begin_transfer);
    for (std::size_t index = 0; index < word_count; ++index) {
        auto value = read_word(tile_key, index * 2U);
        if ((palette_or_flip & 0x8000U) != 0U) {
            value = static_cast<std::uint16_t>(value ^ 0x8000U);
        } else {
            value = static_cast<std::uint16_t>(value | (palette_or_flip & 0x7FFFU));
        }
        write_word(value);
    }
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/menu_hud_service_conversion.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

void prepare_menu_mode_resources(
    MenuModeResourceState& state,
    const bool source_mode_flag,
    const std::uint8_t selected_mode,
    const std::function<void()>& prepare_intermediate,
    const std::function<void()>& prepare_title_resources) {
    state.mode_variant = source_mode_flag ? 1 : -1;
    state.transfer_marker = 0xFFU;
    if (prepare_intermediate) {
        prepare_intermediate();
    }
    state.prompt_marker = 0;
    state.transfer_marker = 0;
    if (prepare_title_resources) {
        prepare_title_resources();
    }
    if (selected_mode == 2U && state.mode_variant == 1) {
        state.transition_pending = true;
    }
}

void upload_menu_resource_pair(const std::function<void()>& upload_stream) {
    if (upload_stream) {
        upload_stream();
        upload_stream();
    }
}

void prepare_menu_hud_sequence(const MenuHudSequenceCallbacks& callbacks) {
    write_menu_hud_grid_batch(callbacks.write_grid);
    write_menu_hud_grid_batch(callbacks.write_grid);
    if (callbacks.refresh_number_or_grid) {
        callbacks.refresh_number_or_grid();
    }
    if (callbacks.service_after_number) {
        callbacks.service_after_number();
    }
    if (callbacks.service_menu) {
        callbacks.service_menu();
    }
    if (callbacks.service_footer) {
        callbacks.service_footer();
    }
}

void write_menu_hud_grid_batch(const std::function<void()>& write_grid) {
    if (write_grid) {
        for (int count = 0; count < 5; ++count) {
            write_grid();
        }
    }
}

void render_menu_hud_rows(MenuHudRowsState& state,
                          const MenuHudRowsCallbacks& callbacks) {
    constexpr std::uint32_t first_source_offset = 0xEBC0U;
    constexpr std::uint32_t source_stride = 0x0CU;
    for (std::int16_t row = 4; row >= 0; --row) {
        const auto source_offset = static_cast<std::uint32_t>(
            first_source_offset -
            static_cast<std::uint32_t>(4 - row) * source_stride);
        if (state.menu_upload_pending && row != 0 &&
            state.row_status[static_cast<std::size_t>(row)] >= 0) {
            if (callbacks.prepare_row) {
                callbacks.prepare_row(row, source_offset);
            }
            if (callbacks.service_row) {
                callbacks.service_row();
            }
        }
        if (callbacks.write_grid) {
            callbacks.write_grid(row, source_offset);
        }
    }
}

bool refresh_menu_number_or_grid(
    const MenuNumberOrGridState& state,
    const MenuNumberOrGridCallbacks& callbacks) {
    const auto row = static_cast<std::size_t>(state.selected_row);
    if (row < state.row_status.size() && state.row_status[row] >= 0) {
        if (callbacks.render_title_text) {
            callbacks.render_title_text();
        }
        if (callbacks.build_decimal_glyphs) {
            callbacks.build_decimal_glyphs();
        }
        if (callbacks.render_text_followup) {
            callbacks.render_text_followup();
        }
        return true;
    }
    if (callbacks.write_repeated_grid) {
        callbacks.write_repeated_grid();
    }
    return false;
}

void render_menu_text_followup(
    const std::function<void()>& render_title_text) {
    if (render_title_text) {
        render_title_text();
    }
}

bool advance_hud_sprite_audio_pulse(
    HudSpriteAudioPulseState& state,
    const std::function<void()>& render_hud_sprites,
    const std::function<void()>& advance_audio) {
    state.countdown = static_cast<std::int8_t>(state.countdown - 1);
    if (state.countdown >= 0) {
        return false;
    }

    state.countdown = 1;
    state.phase = static_cast<std::uint8_t>((state.phase + 1U) % 4U);
    if (render_hud_sprites) {
        render_hud_sprites();
    }
    if (state.phase == 0U && advance_audio) {
        advance_audio();
    }
    return true;
}

namespace {

void advance_hud_offset(HudOffsetState& state,
                        const std::int8_t delta,
                        const std::function<void()>& write_grid,
                        const std::function<void()>& service_rows) {
    const auto sum = static_cast<std::int16_t>(state.offset) + delta;
    const bool overflow = sum > 127 || sum < -128;
    const auto wrapped = static_cast<std::int8_t>(sum);
    if (wrapped == 0 || overflow != (wrapped < 0)) {
        state.offset = 0;
    } else if (wrapped > 10) {
        state.offset = 10;
    } else {
        state.offset = wrapped;
    }
    if (write_grid) {
        write_grid();
    }
    if (service_rows) {
        service_rows();
    }
}

} // namespace

void advance_hud_panel_offset(HudOffsetState& state,
                              const std::int8_t delta,
                              const std::function<void()>& write_grid,
                              const std::function<void()>& service_rows) {
    advance_hud_offset(state, delta, write_grid, service_rows);
}

void advance_hud_footer_offset(HudOffsetState& state,
                               const std::int8_t delta,
                               const std::function<void()>& write_grid,
                               const std::function<void()>& service_rows) {
    advance_hud_offset(state, delta, write_grid, service_rows);
}

bool refresh_hud_grid_if_ready(const std::function<bool()>& ready_to_draw,
                               const std::function<void()>& write_grid) {
    if (!ready_to_draw || !ready_to_draw()) {
        return false;
    }
    if (write_grid) {
        write_grid();
        write_grid();
        write_grid();
    }
    return true;
}

std::optional<std::uint32_t> calculate_menu_row_offset(
    const std::vector<std::uint8_t>& row_metrics,
    const std::size_t row_index,
    const std::int8_t mode_variant) {
    constexpr std::size_t row_stride = 0x14U;
    const auto table_offset = row_index * row_stride;
    if (table_offset >= row_metrics.size()) {
        return std::nullopt;
    }

    auto metric = static_cast<std::uint16_t>(row_metrics[table_offset]);
    if (mode_variant >= 0) {
        metric = static_cast<std::uint16_t>(
            metric + (mode_variant == 0 ? 1U : 2U));
    }
    const auto row_delta = static_cast<std::uint16_t>(0x1BU - metric);
    return static_cast<std::uint32_t>(row_delta) * 0x14U;
}

void calculate_menu_delay_offset(MenuDelayOffsetState& state,
                                 const AverageEntityDelayBoundary& average) {
    const auto components = average ? average() : std::array<std::int8_t, 2>{};
    const auto sum = static_cast<std::int16_t>(components[0]) + components[1];
    state.offset = static_cast<std::int8_t>(0x1D - sum);
}

void refresh_mode_metadata_wrapper(const std::function<void()>& refresh) {
    if (refresh) {
        refresh();
    }
}

void write_hud_repeated_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void write_hud_repeated_grid_variant_a(
    const std::function<void()>& write_grid) {
    write_hud_repeated_grid(write_grid);
}

void write_hud_repeated_grid_variant_b(
    const std::function<void()>& write_grid) {
    write_hud_repeated_grid(write_grid);
}

void advance_hud_fade_service(
    HudFadeServiceState& state,
    const std::function<void()>& wait_for_vblank,
    const std::function<void()>& latch_controller_events,
    const std::function<void()>& step_palette_toward_black) {
    for (int pass = 0; pass < 5; ++pass) {
        for (int wait = 0; wait < 2; ++wait) {
            if (wait_for_vblank) {
                wait_for_vblank();
            }
            if (latch_controller_events) {
                latch_controller_events();
            }
        }
        if (step_palette_toward_black) {
            step_palette_toward_black();
        }
        state.dirty = true;
    }
}

void run_menu_hud_tick_loop(MenuHudTickState& state,
                            const MenuHudTickCallbacks& callbacks) {
    for (std::uint16_t frame = 0; frame < 60U; ++frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (callbacks.advance_sprite_audio_pulse) {
            callbacks.advance_sprite_audio_pulse();
        }
        if (callbacks.advance_random_effect) {
            callbacks.advance_random_effect();
        }
        state.controller_tick_ready = true;
        if (callbacks.service_world_tick) {
            callbacks.service_world_tick();
        }
        if (callbacks.service_optional_effect) {
            callbacks.service_optional_effect();
        }
        state.world_tick_ready = true;
        state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 2U);
        ++state.frames_completed;
    }
    if (callbacks.finalize_world) {
        callbacks.finalize_world();
    }
    if (callbacks.finalize_input) {
        callbacks.finalize_input();
    }
    state.selected_row = 0;
    if (callbacks.render_rows) {
        callbacks.render_rows();
    }
}

void render_menu_text_progression(
    const std::int16_t available_count,
    const bool service_blocked,
    const MenuTextProgressionCallbacks& callbacks) {
    const auto render = [&callbacks] {
        if (callbacks.render_title_text) {
            callbacks.render_title_text();
        }
    };
    const auto build = [&callbacks] {
        if (callbacks.build_decimal_glyphs) {
            callbacks.build_decimal_glyphs();
        }
    };

    render();
    if (!service_blocked) {
        render();
        if (available_count >= 3) {
            render();
        }
        if (available_count >= 2) {
            build();
            if (available_count >= 3) {
                return;
            }
        }
        render();
    }
    render();
}

bool advance_menu_overlay_timer(
    MenuOverlayTimerState& state,
    const std::function<void()>& write_grid) {
    state.service_marker = 0;
    if (state.overlay_phase == 0x0CU) {
        state.service_marker = 0xFFU;
    }
    state.countdown = static_cast<std::int8_t>(state.countdown - 1);
    if (state.countdown >= 0 || state.overlay_phase == 0x0CU) {
        return false;
    }

    state.countdown = state.reload_count;
    ++state.overlay_phase;
    if (write_grid) {
        write_grid();
        write_grid();
    }
    return true;
}

void advance_menu_random_glitch(
    MenuRandomGlitchState& state,
    const MenuRandomGlitchCallbacks& callbacks) {
    if (state.phase < 0) {
        const auto sample = callbacks.sample_random_bound
                                ? callbacks.sample_random_bound()
                                : 1;
        if (sample != 0) {
            return;
        }
        state.phase = 7;
        state.random_offset = callbacks.sample_random_bound
                                  ? callbacks.sample_random_bound()
                                  : 0;
        const auto second_sample = callbacks.sample_random_bound
                                       ? callbacks.sample_random_bound()
                                       : 1;
        if (second_sample == 0) {
            state.random_offset = static_cast<std::int16_t>(
                state.random_offset + 0x1C);
        }
        state.random_offset = static_cast<std::int16_t>(
            state.random_offset * 2);
    } else if (state.phase < 1) {
        if (callbacks.write_repeated_grid) {
            callbacks.write_repeated_grid();
        }
        --state.phase;
        return;
    }

    if (callbacks.write_repeated_grid) {
        callbacks.write_repeated_grid();
    }
    if (callbacks.write_grid) {
        callbacks.write_grid();
    }
    if (state.phase != 2 && state.phase != 7) {
        if (callbacks.sample_random_bound) {
            callbacks.sample_random_bound();
        }
        if (callbacks.write_linear_grid) {
            callbacks.write_linear_grid();
        }
        if (state.phase == 4 || state.phase == 5) {
            if (callbacks.sample_random_bound) {
                callbacks.sample_random_bound();
            }
            if (callbacks.write_linear_grid) {
                callbacks.write_linear_grid();
            }
            if (state.phase == 5 && callbacks.write_grid) {
                callbacks.write_grid();
            }
        }
    }
    --state.phase;
}

void refresh_field_hud_assets(FieldHudAssetState& state,
                              const FieldHudAssetCallbacks& callbacks) {
    if (callbacks.render_field_hud) {
        callbacks.render_field_hud();
    }
    state.selected_row = 0;
    if (callbacks.refresh_selection_rows) {
        callbacks.refresh_selection_rows();
    }
    if (callbacks.upload_source_stream) {
        callbacks.upload_source_stream();
    }
    if (callbacks.prepare_field_effects) {
        callbacks.prepare_field_effects();
    }
    if (callbacks.render_hud_sprites) {
        callbacks.render_hud_sprites();
        callbacks.render_hud_sprites();
    }
}

void render_hud_indicator_grids(const HudIndicatorGridState& state,
                                const std::function<void()>& write_grid) {
    if (!write_grid) {
        return;
    }
    for (const auto active : state.active_flags) {
        if (active != 0U) {
            write_grid();
        }
    }
    for (int footer = 0; footer < 4; ++footer) {
        write_grid();
    }
}

void write_hud_repeated_grid_variant_c(
    const std::function<void()>& write_grid) {
    write_hud_repeated_grid(write_grid);
}

void write_two_hud_grids(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
        write_grid();
    }
}

void handle_menu_directional_input(
    const MenuDirectionalInputState& state,
    const std::function<void()>& set_selection,
    const std::function<void()>& refresh_menu_state,
    const std::function<void()>& refresh_number_or_grid) {
    if ((state.input_bits & 0x0CU) == 0U || state.selected_row >= 5U) {
        return;
    }
    const auto row = state.selected_row;
    const auto status = state.row_status[row];
    const auto timer = state.row_timers[row];
    const bool eligible = status != 3 && status != 5 && status != 6 &&
                          status != 7 && timer >= 6;
    if (!eligible) {
        return;
    }
    if (set_selection) {
        set_selection();
    }
    if (refresh_menu_state) {
        refresh_menu_state();
    }
    if (refresh_number_or_grid) {
        refresh_number_or_grid();
    }
}

void advance_hud_selection_row(
    HudSelectionRowState& state,
    const std::int8_t delta,
    const std::function<void()>& write_grid,
    const std::function<void()>& render_rows) {
    const auto next = static_cast<std::int16_t>(state.selected_row) + delta;
    const auto wrapped = static_cast<std::int16_t>(next % 5);
    state.selected_row = static_cast<std::int8_t>(wrapped < 0 ? wrapped + 5
                                                               : wrapped);
    if (write_grid) {
        write_grid();
    }
    if (render_rows) {
        render_rows();
    }
}

void write_hud_panel_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void reset_menu_visual_slots(const std::function<void()>& reset_five,
                             const std::function<void()>& reset_twelve) {
    if (reset_five) {
        for (int count = 0; count < 5; ++count) {
            reset_five();
        }
    }
    if (reset_twelve) {
        for (int count = 0; count < 12; ++count) {
            reset_twelve();
        }
    }
}

void clear_ready_menu_visual_slots(
    const std::array<std::int8_t, 5>& row_status,
    const std::function<void(std::size_t row)>& clear_slot) {
    if (!clear_slot) {
        return;
    }
    for (std::size_t row = row_status.size(); row-- > 0;) {
        if (row_status[row] >= 0) {
            clear_slot(row);
        }
    }
}

void refresh_menu_slot_pair(const std::function<void()>& refresh_pair,
                            const std::function<void()>& clear_visual) {
    if (refresh_pair) {
        refresh_pair();
    }
    if (clear_visual) {
        clear_visual();
    }
}

void clear_menu_slot_visual(const std::function<void()>& clear_visual) {
    if (clear_visual) {
        clear_visual();
    }
}

void rotate_hud_history(HudHistoryState& state) {
    state.countdown = static_cast<std::int8_t>(state.countdown - 1);
    if (state.countdown >= 0) {
        return;
    }
    state.countdown = 4;
    const auto first = state.values.front();
    for (std::size_t index = 0; index + 1U < state.values.size(); ++index) {
        state.values[index] = state.values[index + 1U];
    }
    state.values.back() = first;
    state.dirty = true;
}

} // namespace shadowrun::ghidra_conversion

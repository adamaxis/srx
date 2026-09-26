#include "shadowrun_ghidra/menus/menu_service_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void draw_menu_hud_panel_variant_a(const std::function<void()>& draw_panel) {
    if (draw_panel) {
        draw_panel();
    }
}

void draw_active_runner_hud_panel(
    const ActiveRunnerHudPanelInput& input,
    const std::function<void(const ActiveRunnerHudPanelPlan&)>& draw_panel) {
    if (draw_panel) {
        ActiveRunnerHudPanelPlan plan;
        plan.field_parameter = input.field_parameter;
        plan.tile_origin = input.panel_variant > 1
                               ? static_cast<std::int16_t>(-0x7940)
                               : input.panel_variant == 1
                                     ? static_cast<std::int16_t>(-0x7956)
                                     : static_cast<std::int16_t>(-0x796C);
        draw_panel(plan);
    }
}

std::uint16_t render_menu_tile_rows(
    const std::function<void()>& clear_row,
    const std::function<void()>& write_row) {
    std::uint16_t countdown = 7U;
    std::uint16_t last_countdown = 0U;
    do {
        if (clear_row) {
            clear_row();
        }
        if (write_row) {
            write_row();
        }
        last_countdown = countdown;
        countdown = static_cast<std::uint16_t>(countdown - 1U);
    } while (countdown != 0xFFFFU);
    return static_cast<std::uint16_t>(last_countdown >> 1U);
}

void reset_and_upload_menu_tile_block(
    const std::function<void()>& reset_upload_state,
    const std::function<void()>& upload_tile_block) {
    if (reset_upload_state) {
        reset_upload_state();
    }
    if (upload_tile_block) {
        upload_tile_block();
    }
}

void write_menu_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void write_menu_grid_quad(const std::function<void()>& write_grid,
                          const std::function<void()>& refresh_metadata) {
    for (int index = 0; index < 4; ++index) {
        if (write_grid) {
            write_grid();
        }
    }
    if (refresh_metadata) {
        refresh_metadata();
    }
}

void prepare_menu_row_grid(const std::function<void()>& prepare_row,
                           const std::function<void()>& write_repeated_grid) {
    if (prepare_row) {
        prepare_row();
    }
    if (write_repeated_grid) {
        write_repeated_grid();
    }
}

void render_menu_row(const std::function<void()>& write_grid,
                     const std::function<void()>& render_row) {
    if (write_grid) {
        write_grid();
    }
    if (render_row) {
        render_row();
    }
}

std::uint64_t prepare_menu_dialog(
    const std::function<std::uint64_t()>& prepare_dialog,
    const std::function<void()>& service_dialog) {
    const auto result = prepare_dialog ? prepare_dialog() : 0U;
    if (service_dialog) {
        service_dialog();
        service_dialog();
    }
    return result;
}

bool refresh_menu_text(
    MenuTextRefreshState& state,
    const std::function<void()>& prepare_text,
    const std::function<void()>& render_text) {
    if (state.pending_text_count >= 1) {
        return false;
    }
    if (prepare_text) {
        prepare_text();
    }
    if (render_text) {
        render_text();
        render_text();
    }
    state.pending_text_count = 0;
    return true;
}

void advance_menu_text_cursor(
    MenuTextCursorState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_cursor) {
    std::int16_t* selected_marker = nullptr;
    do {
        if (state.selector == 0) {
            state.selector = -1;
            selected_marker = &state.primary_marker;
        } else {
            state.selector = 0;
            selected_marker = &state.secondary_marker;
        }
    } while (*selected_marker < 0);

    if (render_text) {
        render_text();
    }
    if (service_cursor) {
        service_cursor();
    }
}

void render_menu_number(MenuNumberRenderState& state,
                        const std::int16_t delta,
                        const MenuNumberRenderCallbacks& callbacks) {
    if (callbacks.write_number_word) {
        callbacks.write_number_word(0x6000U);
    }

    const auto sum = static_cast<std::int32_t>(state.cursor) + delta;
    const bool overflow = sum > 0x7FFF || sum < -0x8000;
    state.cursor = static_cast<std::int16_t>(sum);
    if (state.cursor == 0 || overflow) {
        state.cursor = static_cast<std::int16_t>(state.available_count);
    }
    if (static_cast<std::int16_t>(state.available_count) < state.cursor) {
        state.cursor = 1;
    }
    if (callbacks.build_decimal_glyphs) {
        callbacks.build_decimal_glyphs(state.cursor);
    }
}

void clear_menu_slot(MenuSlotCleanupState& state,
                     const std::int8_t action,
                     const std::size_t slot_index,
                     const bool asset_load_pending,
                     const std::function<void()>& prepare_assets) {
    const auto index = slot_index % state.pending_actions.size();
    if (action < 0x2C) {
        if (asset_load_pending) {
            state.slot_type = 0x7FU;
            state.slot_subtype = 0U;
            state.action_marker = 0;
            if (prepare_assets) {
                prepare_assets();
            }
        }
    } else if (static_cast<std::int8_t>(action - 0x2B) ==
                   state.action_marker &&
               state.active_slots[index] != 0U) {
        state.action_marker = 0;
    }
    state.pending_actions[index] = 0U;
    state.active_slots[index] = 0U;
}

void refresh_menu_panel(MenuPanelRefreshState& state,
                        const std::function<void()>& upload_panel,
                        const std::function<void()>& update_layout,
                        const std::function<void()>& render_text) {
    if (upload_panel) {
        upload_panel();
    }
    state.toggle = static_cast<std::uint8_t>(state.toggle ^ 0xFFU);
    if (update_layout) {
        update_layout();
    }
    if (render_text) {
        render_text();
    }
}

void prepare_menu_grid_variant_a(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void prepare_menu_grid_variant_b(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void prepare_menu_grid_variant_c(
    const std::function<void()>& prepare_grid) {
    if (prepare_grid) {
        prepare_grid();
    }
}

void write_menu_repeated_grid(const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void write_menu_repeated_grid_with_metadata(
    const std::function<void()>& write_grid) {
    if (write_grid) {
        write_grid();
    }
}

void render_menu_text_asset(const std::function<void()>& render_text) {
    if (render_text) {
        render_text();
    }
}

std::uint16_t run_cyberware_menu(
    CyberwareMenuState& state,
    const CyberwareMenuCallbacks& callbacks) {
    if (callbacks.prepare_grid_variant_b) {
        callbacks.prepare_grid_variant_b();
    }
    if (callbacks.prepare_row_grid) {
        callbacks.prepare_row_grid();
    }

    state.rendered_entries = 0U;
    if (callbacks.render_title_text) {
        callbacks.render_title_text();
    }
    for (std::uint16_t bit = 0U; bit < 15U; ++bit) {
        if ((state.installed_flags & (std::uint16_t{1U} << bit)) == 0U) {
            continue;
        }
        if (callbacks.render_title_text) {
            callbacks.render_title_text();
        }
        ++state.rendered_entries;
    }
    if (callbacks.wait_for_controller_event) {
        callbacks.wait_for_controller_event();
    }
    if (callbacks.render_menu_row) {
        callbacks.render_menu_row();
    }
    return state.rendered_entries;
}

} // namespace shadowrun::ghidra_conversion

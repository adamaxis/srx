#include "shadowrun_ghidra/menu_service_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void draw_menu_hud_panel_variant_a(const std::function<void()>& draw_panel) {
    if (draw_panel) {
        draw_panel();
    }
}

void draw_menu_hud_panel_variant_b(const std::function<void()>& draw_panel) {
    if (draw_panel) {
        draw_panel();
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

} // namespace shadowrun::ghidra_conversion

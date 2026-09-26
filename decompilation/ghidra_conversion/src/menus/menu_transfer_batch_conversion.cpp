#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void write_repeated_menu_grid(const std::function<void()>& write_grid) {
    call_once(write_grid);
}

void prepare_menu_grid_then_service(
    const std::function<void()>& write_grid,
    const std::function<void()>& service_menu) {
    call_once(write_grid);
    call_once(service_menu);
}

void prepare_grid_then_write(const std::function<void()>& prepare,
                             const std::function<void()>& write_grid) {
    call_once(prepare);
    call_once(write_grid);
}

void write_three_grids_then_service(
    const std::function<void()>& write_grid,
    const std::function<void()>& service_once) {
    if (write_grid) {
        for (int index = 0; index < 3; ++index) {
            write_grid();
        }
    }
    call_once(service_once);
}

bool find_matching_menu_metric(
    const std::function<bool(std::int16_t slot)>& matches_slot) {
    if (!matches_slot) {
        return false;
    }
    for (std::int16_t slot = 6; slot >= 0; --slot) {
        if (matches_slot(slot)) {
            return true;
        }
    }
    return false;
}

void upload_resource_then_write_two_blocks(
    const std::function<void()>& upload_resource,
    const std::function<void()>& write_block) {
    call_once(upload_resource);
    if (write_block) {
        write_block();
        write_block();
    }
}

void upload_resource_stream(const std::function<void()>& upload_stream) {
    call_once(upload_stream);
}

void draw_hud_panel_wrapper(const std::function<void()>& draw_panel) {
    call_once(draw_panel);
}

void render_title_text_source_wrapper(
    const std::function<void()>& render_text) {
    call_once(render_text);
}

void render_title_text_source_leaf(const std::function<void()>& render_text) {
    call_once(render_text);
}

void render_title_text_source_variant(
    const std::function<void()>& render_text) {
    call_once(render_text);
}

void clear_entity_auxiliary_marker(EntityAuxiliaryState& state) {
    state.marker = 0;
}

void write_single_grid(const std::function<void()>& write_grid) {
    call_once(write_grid);
}

bool update_resource_variant(ResourceVariantState& state,
                             const std::int16_t selection,
                             const std::function<void()>& upload_stream) {
    const bool requested_alternate = selection >= 13;
    if (state.alternate == requested_alternate) {
        return false;
    }
    state.alternate = requested_alternate;
    call_once(upload_stream);
    return true;
}

bool passes_entity_action_threshold(
    const std::int8_t entity_phase,
    const std::int16_t requested_threshold,
    const std::uint8_t current_threshold,
    const std::function<void()>& sample_threshold) {
    if (entity_phase == 1) {
        return true;
    }
    call_once(sample_threshold);
    if (requested_threshold < 0) {
        return false;
    }
    return static_cast<std::uint8_t>(requested_threshold) >= current_threshold;
}

void advance_palette_fade(PaletteFadeState& state,
                          const PaletteFadeCallbacks& callbacks) {
    call_once(callbacks.advance_effects);
    if (!state.active) {
        return;
    }
    if ((state.remaining & 1) == 0) {
        call_once(callbacks.step_palette_toward_black);
        state.dirty = true;
    }
    --state.remaining;
    if (state.remaining < 0) {
        state.active = false;
        state.completed = true;
    }
}

std::int32_t project_entity_x_axis(const CoordinateProjectionInput& input,
                                   const std::int32_t row_stride) {
    const auto adjusted = input.orientation < 0x8000U
                              ? static_cast<std::int32_t>(input.first_word >> 4U) +
                                    0x10
                              : static_cast<std::int32_t>(input.first_word >> 4U) -
                                    0x10;
    return (static_cast<std::int32_t>(input.second_word >> 9U) - 1) *
               row_stride +
           (adjusted >> 5) * 2;
}

std::int32_t project_entity_y_axis(const CoordinateProjectionInput& input,
                                   const std::int32_t row_stride) {
    const bool lower_half = input.orientation < 0x4000U ||
                            input.orientation > 0xBFFFU;
    const auto adjusted = static_cast<std::int32_t>(input.second_word >> 4U) +
                          (lower_half ? -0x10 : 0x10);
    return (static_cast<std::int32_t>(adjusted >> 5) - 1) * row_stride +
           static_cast<std::int32_t>(input.first_word >> 9U) * 2;
}

void draw_hud_panel_forward(const std::function<void()>& draw_panel) {
    call_once(draw_panel);
}

void draw_hud_footer_forward(const std::function<void()>& draw_footer) {
    call_once(draw_footer);
}

} // namespace shadowrun::ghidra_conversion

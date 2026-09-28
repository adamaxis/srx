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

std::optional<MenuMetricLookupResult> resolve_menu_metric_action(
    const MenuMetricLookupInput& input) {
    // 0x104C8 performs MULU.W on D0 and then ADD.B for the column. Preserve
    // that byte-only addition before the later word shift; using a host-wide
    // row*6+column expression silently changes wrapped source indices.
    const auto row_product = static_cast<std::uint32_t>(input.menu_row) * 6U;
    const auto low_byte = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(row_product) + input.menu_column);
    const auto indexed_product = (row_product & 0xFFFFFF00U) | low_byte;
    const auto shifted_index = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(indexed_product) << 1U);
    const auto table_offset = static_cast<std::size_t>(
        static_cast<std::uint16_t>(shifted_index + 2U));
    if (table_offset >= input.source_action_indices.size()) {
        return std::nullopt;
    }

    const auto action_index = input.source_action_indices[table_offset];
    const auto matching_item_code = static_cast<std::uint8_t>(
        static_cast<std::uint16_t>(action_index) + 0x19U);
    EntityActionPhaseModifierResult modifiers{};
    const auto has_item_catalog = input.item_catalog != nullptr;
    const auto has_selector_table = input.selector_table != nullptr;
    if (has_item_catalog != has_selector_table) {
        // A source-bound call cannot silently fall back to the caller-owned
        // matrix when only half of C8AE0/D4540 is available. Preserve the
        // phase result, then expose the missing dependency if slot 7 would
        // enter the special branch.
        modifiers = apply_action_item_phase_modifiers(
            input.value, input.entity, matching_item_code, action_index,
            static_cast<const EntityActionSpecialItemModifierTable*>(nullptr));
        const auto slot7_item = input.entity.item_codes[7];
        if (slot7_item >= 0x29U && slot7_item <= 0x2BU) {
            modifiers.source_data_missing = true;
        }
    } else if (has_item_catalog) {
        modifiers = apply_action_item_phase_modifiers(
            input.value, input.entity, matching_item_code, action_index,
            *input.item_catalog, *input.selector_table);
    } else {
        modifiers = apply_action_item_phase_modifiers(
            input.value, input.entity, matching_item_code, action_index,
            input.special_item_table);
    }
    return MenuMetricLookupResult{action_index, matching_item_code, modifiers,
                                  table_offset};
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

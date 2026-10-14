#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> events;
    write_repeated_menu_grid([&] { events.push_back(1); });
    prepare_menu_grid_then_service([&] { events.push_back(2); },
                                   [&] { events.push_back(3); });
    prepare_grid_then_write([&] { events.push_back(4); },
                            [&] { events.push_back(5); });
    write_three_grids_then_service([&] { events.push_back(6); },
                                   [&] { events.push_back(7); });
    if (events != std::vector<int>{1, 2, 3, 4, 5, 6, 6, 6, 7}) {
        return 1;
    }

    MenuMetricLookupInput metric_input{};
    metric_input.menu_row = 1U;
    metric_input.menu_column = 2U;
    metric_input.source_action_indices.resize(20U, 0U);
    metric_input.source_action_indices[18U] = 2U;
    metric_input.value = 5U;
    metric_input.entity.item_codes[3] = 0x1BU;
    metric_input.entity.item_phases[3] = 4U;
    const auto metric = resolve_menu_metric_action(metric_input);
    if (!metric.has_value() || metric->source_table_offset != 18U ||
        metric->action_index != 2U || metric->matching_item_code != 0x1BU ||
        metric->modifiers.value != 9U || !metric->modifiers.phase_found ||
        resolve_menu_metric_action(MenuMetricLookupInput{}) .has_value()) {
        return 2;
    }

    // The same 0x104C8 selection can enter the source-bound 0x104F6 tail.
    // Action index 2 selects D4540 +0x24 for slot-7 item 0x29 (item 41),
    // whose aligned source word is 0xFFFF.
    auto source_metric = metric_input;
    source_metric.entity.item_codes[7] = 0x29U;
    source_metric.item_catalog = &us_item_catalog();
    source_metric.selector_table = &us_special_item_modifier_selector_table();
    const auto typed_metric = resolve_menu_metric_action(source_metric);
    if (!typed_metric.has_value() || typed_metric->modifiers.value != 8U ||
        !typed_metric->modifiers.special_item_applied ||
        typed_metric->modifiers.source_data_missing) {
        return 15;
    }

    auto incomplete_source_metric = source_metric;
    incomplete_source_metric.selector_table = nullptr;
    const auto incomplete_metric =
        resolve_menu_metric_action(incomplete_source_metric);
    if (!incomplete_metric.has_value() ||
        incomplete_metric->modifiers.value != 9U ||
        incomplete_metric->modifiers.special_item_applied ||
        !incomplete_metric->modifiers.source_data_missing) {
        return 16;
    }

    // Source 0x104C8 adds the column with ADD.B after MULU.W. For a wrapped
    // low byte this differs from a host-wide row*6+column calculation.
    MenuMetricLookupInput wrapped_metric{};
    wrapped_metric.menu_row = 0xFFU;
    wrapped_metric.menu_column = 0x0AU;
    wrapped_metric.source_action_indices.resize(0xA0BU, 0U);
    wrapped_metric.source_action_indices[0xA0AU] = 1U;
    const auto wrapped = resolve_menu_metric_action(wrapped_metric);
    if (!wrapped.has_value() || wrapped->source_table_offset != 0xA0AU ||
        wrapped->action_index != 1U ||
        wrapped->matching_item_code != 0x1AU) {
        return 14;
    }

    events.clear();
    upload_resource_then_write_two_blocks([&] { events.push_back(1); },
                                          [&] { events.push_back(2); });
    if (events != std::vector<int>{1, 2, 2}) {
        return 3;
    }
    upload_resource_stream([&] { events.push_back(3); });
    if (events != std::vector<int>{1, 2, 2, 3}) {
        return 4;
    }

    EntityAuxiliaryState auxiliary{9};
    clear_entity_auxiliary_marker(auxiliary);
    if (auxiliary.marker != 0) {
        return 5;
    }
    const auto scaled = scale_and_clear_entity_auxiliary(0U, 3U);
    const auto remainder = scale_and_clear_entity_auxiliary(0U, 1U);
    const auto wide = scale_and_clear_entity_auxiliary(0U, 18U);
    const auto zero_factor = scale_and_clear_entity_auxiliary(18U, 32U);
    if (scaled.scaled_value != 50U || scaled.auxiliary != 0U ||
        remainder.scaled_value != 17U || wide.scaled_value != 301U ||
        zero_factor.scaled_value != 3201U) {
        return 17;
    }
    write_single_grid([&] { events.push_back(8); });
    if (events.back() != 8) {
        return 6;
    }

    ResourceVariantState variant{};
    int uploads = 0;
    if (!update_resource_variant(variant, 13, [&] { ++uploads; }) ||
        !variant.alternate || uploads != 1 ||
        update_resource_variant(variant, 14, [&] { ++uploads; }) ||
        uploads != 1) {
        return 7;
    }
    if (passes_entity_action_threshold(1, -1, 10, {})) {
        // Phase one is the intentional bypass.
    } else {
        return 8;
    }
    if (passes_entity_action_threshold(2, 5, 10, {})) {
        return 9;
    }
    if (!passes_entity_action_threshold(2, 10, 10, {})) {
        return 10;
    }

    PaletteFadeState fade{true, 2, false, false};
    int effects = 0;
    int palette_steps = 0;
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    advance_palette_fade(fade, {[&] { ++effects; }, [&] { ++palette_steps; }});
    if (effects != 3 || palette_steps != 2 || !fade.dirty || fade.active) {
        return 11;
    }

    CoordinateProjectionInput coordinate{0x0100, 0x0400, 0};
    if (project_entity_x_axis(coordinate, 20) != 22 ||
        project_entity_y_axis(coordinate, 20) != 0) {
        return 12;
    }

    int forwards = 0;
    draw_hud_panel_forward([&] { ++forwards; });
    draw_hud_footer_forward([&] { ++forwards; });
    if (forwards != 2) {
        return 13;
    }
    return 0;
}

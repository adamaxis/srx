#include "shadowrun_ghidra/field/field_action_detail_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldActionDetailRenderResult render_field_action_detail(
    const std::vector<FieldActionCostRecord>& records,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    FieldActionDetailRenderState& state,
    const FieldActionBaseCostLookup& base_cost_lookup,
    const FieldActionDetailRenderCallbacks& callbacks) {
    FieldActionDetailRenderResult result{};
    if (state.primary_cursor < 0 ||
        static_cast<std::size_t>(state.primary_cursor) >= records.size()) {
        return result;
    }

    result.record = records[static_cast<std::size_t>(state.primary_cursor)];
    if (static_cast<std::int8_t>(result.record.type) < 0) {
        result.kind = FieldActionDetailRenderKind::unavailable;
        if (callbacks.show_unavailable) {
            callbacks.show_unavailable();
        }
        if (callbacks.render_tail_text) {
            callbacks.render_tail_text();
        }
        return result;
    }

    if (result.record.type == static_cast<std::uint8_t>('E')) {
        result.kind = FieldActionDetailRenderKind::special_action;
        if (callbacks.write_special_grid) {
            callbacks.write_special_grid();
        }
        if (callbacks.render_special_header) {
            callbacks.render_special_header();
        }
        result.cost = apply_field_action_cost_discounts(
            result.record.quantity, state.cost_context);
        if (callbacks.build_grouped_decimal) {
            callbacks.build_grouped_decimal();
        }
        if (callbacks.render_special_label) {
            callbacks.render_special_label();
        }
        if (callbacks.render_tail_text) {
            callbacks.render_tail_text();
        }
        return result;
    }

    result.kind = FieldActionDetailRenderKind::ordinary_action;
    result.detail_plan = build_field_action_detail_plan(
        FieldActionDetailInput{result.record.type, result.record.value},
        detail_resources, resource_variant, callbacks.upload_resource);
    if (callbacks.present_detail_plan) {
        callbacks.present_detail_plan(*result.detail_plan);
    }
    result.detail_text = select_field_action_detail_text(result.record.type);
    if (result.detail_text && callbacks.render_detail_text) {
        callbacks.render_detail_text(*result.detail_text);
    }
    if (callbacks.render_normal_header) {
        callbacks.render_normal_header();
    }

    if (!base_cost_lookup) {
        result.kind = FieldActionDetailRenderKind::missing_cost_data;
        return result;
    }
    result.cost = calculate_field_action_record_cost(
        result.record, state.cost_context, base_cost_lookup);
    if (callbacks.build_grouped_decimal) {
        callbacks.build_grouped_decimal();
    }
    if (callbacks.render_action_name) {
        callbacks.render_action_name();
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion

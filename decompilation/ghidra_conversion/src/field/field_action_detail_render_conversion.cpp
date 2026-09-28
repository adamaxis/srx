#include "shadowrun_ghidra/field/field_action_detail_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

using FieldActionDetailPlanBuilder = std::function<
    ActionDetailRenderPlan(const FieldActionDetailInput&,
                           ResourceVariantState&,
                           const std::function<void()>&)>;
using FieldActionCostCalculator = std::function<
    std::uint32_t(const FieldActionCostRecord&, const FieldActionCostContext&)>;

FieldActionDetailRenderResult render_field_action_detail_impl(
    const std::vector<FieldActionCostRecord>& records,
    ResourceVariantState& resource_variant,
    FieldActionDetailRenderState& state,
    const FieldActionDetailPlanBuilder& build_detail_plan,
    const FieldActionCostCalculator& calculate_cost,
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
    result.detail_plan = build_detail_plan(
        FieldActionDetailInput{result.record.type, result.record.value},
        resource_variant, callbacks.upload_resource);
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

    if (!calculate_cost) {
        result.kind = FieldActionDetailRenderKind::missing_cost_data;
        if (callbacks.render_action_name) {
            callbacks.render_action_name();
        }
        if (callbacks.render_tail_text) {
            callbacks.render_tail_text();
        }
        return result;
    }
    result.cost = calculate_cost(result.record, state.cost_context);
    if (callbacks.build_grouped_decimal) {
        callbacks.build_grouped_decimal();
    }
    if (callbacks.render_action_name) {
        callbacks.render_action_name();
    }
    if (callbacks.render_tail_text) {
        callbacks.render_tail_text();
    }
    return result;
}

} // namespace

FieldActionDetailRenderResult render_field_action_detail(
    const std::vector<FieldActionCostRecord>& records,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    FieldActionDetailRenderState& state,
    const FieldActionBaseCostLookup& base_cost_lookup,
    const FieldActionDetailRenderCallbacks& callbacks) {
    const FieldActionDetailPlanBuilder build_detail_plan =
        [&detail_resources](const FieldActionDetailInput& input,
                            ResourceVariantState& variant,
                            const std::function<void()>& upload_resource) {
            return build_field_action_detail_plan(
                input, detail_resources, variant, upload_resource);
        };
    FieldActionCostCalculator calculate_cost;
    if (base_cost_lookup) {
        calculate_cost =
            [&base_cost_lookup](const FieldActionCostRecord& record,
                                const FieldActionCostContext& context) {
                return calculate_field_action_record_cost(
                    record, context, base_cost_lookup);
            };
    }
    return render_field_action_detail_impl(
        records, resource_variant, state, build_detail_plan, calculate_cost,
        callbacks);
}

FieldActionDetailRenderResult render_field_action_detail(
    const std::vector<FieldActionCostRecord>& records,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    FieldActionDetailRenderState& state,
    const FieldActionDetailRenderCallbacks& callbacks) {
    if (state.primary_cursor >= 0 &&
        static_cast<std::size_t>(state.primary_cursor) < records.size()) {
        const auto& record =
            records[static_cast<std::size_t>(state.primary_cursor)];
        const auto unavailable = static_cast<std::int8_t>(record.type) < 0;
        const auto special_action =
            record.type == static_cast<std::uint8_t>('E');
        if (!unavailable && !special_action &&
            (record.type >= item_catalog.size() ||
             item_catalog[record.type].item_code !=
                 static_cast<std::uint8_t>(record.type + 1U))) {
            FieldActionDetailRenderResult missing{};
            missing.kind = FieldActionDetailRenderKind::missing_source_data;
            missing.record = record;
            return missing;
        }
    }

    const FieldActionDetailPlanBuilder build_detail_plan =
        [&item_catalog](const FieldActionDetailInput& input,
                        ResourceVariantState& variant,
                        const std::function<void()>& upload_resource) {
            return build_field_action_detail_plan(
                input, item_catalog, variant, upload_resource);
        };
    const FieldActionCostCalculator calculate_cost =
        [&item_catalog](const FieldActionCostRecord& record,
                        const FieldActionCostContext& context) {
            return calculate_field_action_record_cost(record, context,
                                                       item_catalog);
        };
    return render_field_action_detail_impl(
        records, resource_variant, state, build_detail_plan, calculate_cost,
        callbacks);
}

} // namespace shadowrun::ghidra_conversion

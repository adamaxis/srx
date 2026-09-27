#include "shadowrun_ghidra/field/field_inventory_action_detail_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& selected_action_code,
    const InventoryRunnerCostLookup& runner_cost_lookup,
    const FieldInventoryActionDetailRenderCallbacks& callbacks) {
    FieldInventoryActionDetailRenderResult result{};
    if (state.primary_cursor >= state.item_codes.size()) {
        result.kind = FieldInventoryActionDetailRenderKind::invalid_cursor;
        return result;
    }

    result.item_code = state.item_codes[state.primary_cursor];
    // 0x5855E subtracts one, then accepts item codes 1..0x2B. The source
    // routes zero and values above that range to the common unavailable text.
    if (result.item_code == 0U || result.item_code > 0x2BU) {
        result.kind = FieldInventoryActionDetailRenderKind::unavailable;
        if (callbacks.show_unavailable) {
            callbacks.show_unavailable();
        }
        return result;
    }

    const InventoryActionDetailInput input{
        result.item_code,
        state.item_phases[state.primary_cursor],
        selected_action_code,
    };
    result.detail_plan = build_inventory_action_detail_plan(
        input, detail_resources, resource_variant, callbacks.upload_resource);
    if (!result.detail_plan || !result.detail_plan->valid) {
        result.kind = FieldInventoryActionDetailRenderKind::missing_detail_data;
        return result;
    }
    if (callbacks.present_detail_plan) {
        callbacks.present_detail_plan(*result.detail_plan);
    }

    if (result.detail_plan->label == ActionDetailLabelVariant::action_dependent &&
        !result.detail_plan->action_code.has_value()) {
        result.kind = FieldInventoryActionDetailRenderKind::missing_action_code;
        return result;
    }

    result.resolved_action_code = result.detail_plan->action_code.value_or(
        result.detail_plan->source_code);
    result.detail_text =
        select_field_action_detail_text(result.resolved_action_code);
    if (result.detail_text && callbacks.render_detail_text) {
        callbacks.render_detail_text(*result.detail_text);
    }

    if (!state.suppress_runner_cost) {
        if (callbacks.render_runner_action_header) {
            callbacks.render_runner_action_header();
        }
        if (!runner_cost_lookup) {
            result.kind = FieldInventoryActionDetailRenderKind::missing_cost_data;
        } else {
            const auto base_cost = runner_cost_lookup(result.resolved_action_code);
            if (!base_cost) {
                result.kind = FieldInventoryActionDetailRenderKind::missing_cost_data;
            } else {
                // 0x585C4 reads the source word at +0x22 and halves it before
                // entering the surcharge routine.
                result.runner_cost = apply_runner_cost_surcharge(
                    static_cast<std::uint32_t>(*base_cost) >> 1U,
                    state.cost_context);
                if (callbacks.render_runner_cost) {
                    callbacks.render_runner_cost(result.runner_cost);
                }
            }
        }
    }

    if (callbacks.render_common_action_name) {
        callbacks.render_common_action_name(result.resolved_action_code);
    }
    // 0x585E0 clears FFF0B1 before the common action-name render. The source
    // branch was valid even when pricing data was unavailable on the host.
    result.kind = result.kind == FieldInventoryActionDetailRenderKind::missing_cost_data
                      ? result.kind
                      : FieldInventoryActionDetailRenderKind::presented;
    return result;
}

} // namespace shadowrun::ghidra_conversion

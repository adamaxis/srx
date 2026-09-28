#include "shadowrun_ghidra/field/field_inventory_action_detail_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

using InventoryDetailPlanBuilder = std::function<
    std::optional<ActionDetailRenderPlan>(
        const InventoryActionDetailInput&,
        ResourceVariantState&,
        const std::function<void()>&)>;

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail_impl(
    const FieldInventoryActionDetailState& state,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const InventoryDetailPlanBuilder& build_detail_plan,
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
        action_lookup_code,
    };
    result.detail_plan = build_detail_plan(
        input, resource_variant, callbacks.upload_resource);
    if (!result.detail_plan || !result.detail_plan->valid) {
        result.kind = FieldInventoryActionDetailRenderKind::missing_detail_data;
        return result;
    }
    if (callbacks.present_detail_plan) {
        callbacks.present_detail_plan(*result.detail_plan);
    }

    result.action_lookup_code = result.detail_plan->action_code;
    // 0x13228 restores the original inventory item code in D0 before
    // returning to 0x58540. Therefore 0x5878E receives the item code, not
    // the action-code lookup result.
    result.detail_text = select_field_action_detail_text(result.item_code);
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
            const auto base_cost = runner_cost_lookup(result.item_code);
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

    if (callbacks.render_item_name) {
        callbacks.render_item_name(result.item_code);
    }
    // 0x585E0 clears FFF0B1 before the selected item's encoded-name render.
    // The source branch was valid even when pricing data was unavailable.
    result.kind = result.kind == FieldInventoryActionDetailRenderKind::missing_cost_data
                      ? result.kind
                      : FieldInventoryActionDetailRenderKind::presented;
    return result;
}

} // namespace

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const ActionDetailResourceTable& detail_resources,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const InventoryRunnerCostLookup& runner_cost_lookup,
    const FieldInventoryActionDetailRenderCallbacks& callbacks) {
    const InventoryDetailPlanBuilder build_detail_plan =
        [&](const InventoryActionDetailInput& input,
            ResourceVariantState& variant,
            const std::function<void()>& upload_resource) {
            return build_inventory_action_detail_plan(
                input, detail_resources, variant, upload_resource);
        };
    return render_field_inventory_action_detail_impl(
        state, resource_variant, action_lookup_code, build_detail_plan,
        runner_cost_lookup, callbacks);
}

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const FieldInventoryActionDetailRenderCallbacks& callbacks) {
    const InventoryDetailPlanBuilder build_detail_plan =
        [&item_catalog](const InventoryActionDetailInput& input,
                        ResourceVariantState& variant,
                        const std::function<void()>& upload_resource) {
            return build_inventory_action_detail_plan(
                input, item_catalog, variant, upload_resource);
        };
    const InventoryRunnerCostLookup runner_cost_lookup =
        [&item_catalog](const std::uint8_t item_code)
        -> std::optional<std::uint16_t> {
        if (item_code == 0U || item_code > item_catalog.size()) {
            return std::nullopt;
        }
        return item_catalog[static_cast<std::size_t>(item_code - 1U)]
            .base_cost;
    };
    return render_field_inventory_action_detail_impl(
        state, resource_variant, action_lookup_code, build_detail_plan,
        runner_cost_lookup, callbacks);
}

} // namespace shadowrun::ghidra_conversion

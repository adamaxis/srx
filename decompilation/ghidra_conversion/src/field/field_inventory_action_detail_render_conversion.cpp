#include "shadowrun_ghidra/field/field_inventory_action_detail_render_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail_impl(
    const FieldInventoryActionDetailState& state,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
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
    result.detail_plan = build_inventory_action_detail_plan(
        input, item_catalog, resource_variant, callbacks.upload_resource);
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
        const auto base_cost = item_catalog[
            static_cast<std::size_t>(result.item_code - 1U)].base_cost;
        // 0x585C4 reads the source word at +0x22 and halves it before
        // entering the surcharge routine.
        result.runner_cost = apply_runner_cost_surcharge(
            static_cast<std::uint32_t>(base_cost) >> 1U,
            state.cost_context);
        if (callbacks.render_runner_cost) {
            callbacks.render_runner_cost(result.runner_cost);
        }
    }

    if (callbacks.render_item_name) {
        callbacks.render_item_name(result.item_code);
    }
    // 0x585E0 clears FFF0B1 before the selected item's encoded-name render.
    // The source branch was valid even when pricing data was unavailable.
    result.kind = FieldInventoryActionDetailRenderKind::presented;
    return result;
}

} // namespace

FieldInventoryActionDetailRenderResult render_field_inventory_action_detail(
    const FieldInventoryActionDetailState& state,
    const UsItemCatalog& item_catalog,
    ResourceVariantState& resource_variant,
    const std::optional<std::uint8_t>& action_lookup_code,
    const FieldInventoryActionDetailRenderCallbacks& callbacks) {
    return render_field_inventory_action_detail_impl(
        state, item_catalog, resource_variant, action_lookup_code, callbacks);
}

} // namespace shadowrun::ghidra_conversion

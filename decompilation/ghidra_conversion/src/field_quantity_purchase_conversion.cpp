#include "shadowrun_ghidra/field_quantity_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldQuantityPurchaseResult apply_field_quantity_purchase(
    FieldQuantityPurchaseState& state,
    const FieldQuantityPurchaseCallbacks& callbacks) {
    // 0x577BA rejects primary categories 5 and above before touching the
    // purchase marker.
    if (state.category >= state.purchased_quantities.size()) {
        return FieldQuantityPurchaseResult::invalid_category;
    }

    // The caller arms this path through FFF0B3. A negative marker bypasses
    // the panel/cost flow; the source clears it on the way out.
    if (state.availability_marker < 0) {
        state.availability_marker = 0;
        return FieldQuantityPurchaseResult::unavailable;
    }

    if (callbacks.refresh_detail_panel) {
        callbacks.refresh_detail_panel();
    }

    if (state.funds < state.preview_cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        state.availability_marker = 0;
        return FieldQuantityPurchaseResult::insufficient_funds;
    }

    const bool confirmed = callbacks.confirm_purchase
                               ? callbacks.confirm_purchase()
                               : true;
    if (!confirmed) {
        state.availability_marker = 0;
        return FieldQuantityPurchaseResult::cancelled;
    }

    state.funds -= state.preview_cost;
    state.purchased_quantities[state.category] = state.quantity;
    state.availability_marker = 0;
    return FieldQuantityPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion

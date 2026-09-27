#include "shadowrun_ghidra/field/field_quantity_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {

FieldQuantityPurchaseResult apply_field_quantity_purchase(
    FieldQuantityPurchaseState& state,
    const FieldQuantityPurchaseCallbacks& callbacks) {
    // 0x577BA uses a signed-byte BMI against five. Values outside the typed
    // five-slot model are still rejected here rather than turning a malformed
    // high-bit source byte into an unchecked host index.
    if (static_cast<std::int8_t>(state.category) >= 5 ||
        state.category >= state.purchased_quantities.size()) {
        state.availability_marker = -1;
        state.service_result = -1;
        return FieldQuantityPurchaseResult::invalid_category;
    }

    // The caller arms this path through FFF0B3. A negative marker bypasses
    // the panel/cost flow; the source clears it on the way out.
    if (state.availability_marker < 0) {
        state.availability_marker = 0;
        return FieldQuantityPurchaseResult::unavailable;
    }

    if (callbacks.write_quantity_purchase_grid) {
        callbacks.write_quantity_purchase_grid();
    }
    if (callbacks.render_purchase_title) {
        callbacks.render_purchase_title();
    }
    if (callbacks.render_quantity) {
        callbacks.render_quantity();
    }
    if (callbacks.render_preview_cost) {
        callbacks.render_preview_cost();
    }

    if (state.funds < state.preview_cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
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

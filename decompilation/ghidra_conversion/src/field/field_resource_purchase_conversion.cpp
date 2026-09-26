#include "shadowrun_ghidra/field/field_resource_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool category_is_satisfied(const FieldResourcePurchaseState& state) {
    switch (state.category) {
    case 0:
        return state.progress_category_0 >=
               static_cast<std::uint16_t>(state.requested_quantity) * 0x28U;
    case 1:
        return state.progress_category_1 >=
               static_cast<std::uint16_t>(state.requested_quantity) * 0x50U;
    case 2:
        return state.progress_category_2 >=
               static_cast<std::uint16_t>(state.requested_quantity) * 0x0AU;
    case 3:
        return state.progress_category_3 >= 3U;
    default:
        return false;
    }
}

void increment_category_progress(FieldResourcePurchaseState& state) {
    switch (state.category) {
    case 0:
        state.progress_category_0 =
            static_cast<std::uint16_t>(state.progress_category_0 + 10U);
        break;
    case 1:
        state.progress_category_1 =
            static_cast<std::uint16_t>(state.progress_category_1 + 0x19U);
        break;
    case 2:
        state.progress_category_2 =
            static_cast<std::uint8_t>(state.progress_category_2 + 5U);
        break;
    case 3:
        state.progress_category_3 =
            static_cast<std::uint8_t>(state.progress_category_3 + 1U);
        break;
    default:
        break;
    }
}

} // namespace

FieldResourcePurchaseResult apply_field_resource_purchase(
    FieldResourcePurchaseState& state,
    const std::array<std::uint16_t, 4>& category_costs,
    const FieldResourcePurchaseCallbacks& callbacks) {
    // 0x5766A rejects categories 4 and above before refreshing the detail
    // panel and signals the caller through its shared invalid-selection flag.
    if (state.category >= category_costs.size()) {
        return FieldResourcePurchaseResult::invalid_category;
    }

    if (callbacks.refresh_detail_panel) {
        callbacks.refresh_detail_panel();
    }

    if (category_is_satisfied(state)) {
        if (state.category == 3U) {
            if (callbacks.show_category_limit) {
                callbacks.show_category_limit();
            }
            return FieldResourcePurchaseResult::category_limit;
        }
        if (callbacks.show_category_satisfied) {
            callbacks.show_category_satisfied();
        }
        return FieldResourcePurchaseResult::category_satisfied;
    }

    const std::uint32_t cost = category_costs[state.category];
    if (state.funds < cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return FieldResourcePurchaseResult::insufficient_funds;
    }

    // The ROM publishes the cost to FFF0BE before entering the confirmation
    // prompt.  Keep that observable preview even if the user cancels.
    state.preview_cost = cost;
    const bool confirmed = callbacks.confirm_purchase
                               ? callbacks.confirm_purchase()
                               : true;
    if (!confirmed) {
        return FieldResourcePurchaseResult::cancelled;
    }

    state.funds -= cost;
    increment_category_progress(state);
    return FieldResourcePurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/field/field_resource_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool category_is_satisfied(const FieldResourcePurchaseState& state) {
    switch (state.category) {
    case 0:
        return static_cast<std::int16_t>(state.progress_category_0) >=
               static_cast<std::int16_t>(
                   static_cast<std::uint16_t>(state.requested_quantity) *
                   0x28U);
    case 1:
        return static_cast<std::int16_t>(state.progress_category_1) >=
               static_cast<std::int16_t>(
                   static_cast<std::uint16_t>(state.requested_quantity) *
                   0x50U);
    case 2:
        // The source multiplies in a long register but compares only the
        // low signed byte at FFFBC4.
        return static_cast<std::int8_t>(state.progress_category_2) >=
               static_cast<std::int8_t>(static_cast<std::uint8_t>(
                   static_cast<std::uint16_t>(state.requested_quantity) *
                   0x0AU));
    case 3:
        // 0x576CE uses CMP.B #3 followed by BMI. BMI observes only the
        // wrapped result's N bit; it is not a host signed-byte comparison.
        return (static_cast<std::uint8_t>(state.progress_category_3 - 3U) &
                0x80U) == 0U;
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
    // 0x5766A uses a signed-byte BMI against four. Keep the typed array bound
    // as a second guard so malformed high-bit bytes cannot become indexes.
    if (static_cast<std::int8_t>(state.category) >= 4 ||
        state.category >= category_costs.size()) {
        state.service_result = -1;
        return FieldResourcePurchaseResult::invalid_category;
    }

    if (callbacks.write_resource_purchase_grid) {
        callbacks.write_resource_purchase_grid();
    }

    if (category_is_satisfied(state)) {
        if (state.category == 3U) {
            if (callbacks.show_category_limit) {
                callbacks.show_category_limit();
            }
            if (callbacks.wait_for_controller_event) {
                callbacks.wait_for_controller_event();
            }
            return FieldResourcePurchaseResult::category_limit;
        }
        if (callbacks.show_category_satisfied) {
            callbacks.show_category_satisfied();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return FieldResourcePurchaseResult::category_satisfied;
    }

    const std::uint32_t cost = category_costs[state.category];
    if (state.funds < cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return FieldResourcePurchaseResult::insufficient_funds;
    }

    // The ROM publishes the cost to FFF0BE before entering the confirmation
    // prompt.  Keep that observable preview even if the user cancels.
    state.preview_cost = cost;
    if (callbacks.render_purchase_title) {
        callbacks.render_purchase_title();
    }
    if (callbacks.render_preview_cost) {
        callbacks.render_preview_cost();
    }
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

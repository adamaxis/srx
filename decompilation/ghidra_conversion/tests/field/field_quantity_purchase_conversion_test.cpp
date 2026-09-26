#include "shadowrun_ghidra/field/field_quantity_purchase_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    int refreshes = 0;
    int insufficient_messages = 0;
    const FieldQuantityPurchaseCallbacks callbacks{
        [&] { ++refreshes; },
        [&] { ++insufficient_messages; },
        [] { return true; },
    };

    FieldQuantityPurchaseState category_zero{};
    category_zero.quantity = 3U;
    category_zero.preview_cost = 50U;
    category_zero.funds = 100U;
    category_zero.availability_marker = 1;
    if (apply_field_quantity_purchase(category_zero, callbacks) !=
            FieldQuantityPurchaseResult::applied ||
        category_zero.purchased_quantities[0] != 3U ||
        category_zero.funds != 50U || category_zero.availability_marker != 0 ||
        refreshes != 1) {
        return 1;
    }

    FieldQuantityPurchaseState category_four{};
    category_four.category = 4U;
    category_four.quantity = 9U;
    category_four.preview_cost = 25U;
    category_four.funds = 25U;
    category_four.availability_marker = 1;
    if (apply_field_quantity_purchase(category_four, callbacks) !=
            FieldQuantityPurchaseResult::applied ||
        category_four.purchased_quantities[4] != 9U ||
        category_four.purchased_quantities[0] != 0U ||
        category_four.funds != 0U) {
        return 2;
    }

    FieldQuantityPurchaseState unavailable{};
    unavailable.quantity = 7U;
    unavailable.preview_cost = 10U;
    unavailable.funds = 10U;
    unavailable.availability_marker = -1;
    if (apply_field_quantity_purchase(unavailable, callbacks) !=
            FieldQuantityPurchaseResult::unavailable ||
        unavailable.availability_marker != 0 || unavailable.funds != 10U ||
        unavailable.purchased_quantities[0] != 0U || refreshes != 2) {
        return 3;
    }

    FieldQuantityPurchaseState poor{};
    poor.preview_cost = 10U;
    poor.funds = 9U;
    poor.availability_marker = 1;
    if (apply_field_quantity_purchase(poor, callbacks) !=
            FieldQuantityPurchaseResult::insufficient_funds ||
        poor.availability_marker != 0 || poor.funds != 9U ||
        poor.purchased_quantities[0] != 0U || insufficient_messages != 1) {
        return 4;
    }

    FieldQuantityPurchaseState cancelled{};
    cancelled.quantity = 2U;
    cancelled.preview_cost = 10U;
    cancelled.funds = 10U;
    cancelled.availability_marker = 1;
    const FieldQuantityPurchaseCallbacks cancel_callbacks{
        {}, {}, [] { return false; }};
    if (apply_field_quantity_purchase(cancelled, cancel_callbacks) !=
            FieldQuantityPurchaseResult::cancelled ||
        cancelled.availability_marker != 0 || cancelled.funds != 10U ||
        cancelled.purchased_quantities[0] != 0U) {
        return 5;
    }

    FieldQuantityPurchaseState invalid{};
    invalid.category = 5U;
    invalid.availability_marker = -1;
    if (apply_field_quantity_purchase(invalid, callbacks) !=
            FieldQuantityPurchaseResult::invalid_category ||
        invalid.availability_marker != -1 || refreshes != 3) {
        return 6;
    }

    return 0;
}

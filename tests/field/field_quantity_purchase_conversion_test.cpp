#include <string>
#include <vector>

#include "shadowrun_ghidra/field/field_quantity_purchase_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<std::string> events;
    int insufficient_messages = 0;
    int confirmations = 0;
    const FieldQuantityPurchaseCallbacks callbacks{
        [&] { events.emplace_back("grid"); },
        [&] { events.emplace_back("title"); },
        [&] { events.emplace_back("quantity"); },
        [&] { events.emplace_back("cost"); },
        [&] {
            ++insufficient_messages;
            events.emplace_back("insufficient");
        },
        [&] { events.emplace_back("wait"); },
        [&] {
            ++confirmations;
            events.emplace_back("confirm");
            return true;
        },
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
        confirmations != 1 || events !=
            std::vector<std::string>{"grid", "title", "quantity", "cost",
                                     "confirm"}) {
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
        category_four.funds != 0U || confirmations != 2 || events.size() != 10U ||
        events[5] != "grid" || events[6] != "title" ||
        events[7] != "quantity" || events[8] != "cost" ||
        events[9] != "confirm") {
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
        unavailable.purchased_quantities[0] != 0U || events.size() != 10U ||
        confirmations != 2) {
        return 3;
    }

    FieldQuantityPurchaseState poor{};
    poor.preview_cost = 10U;
    poor.funds = 9U;
    poor.availability_marker = 1;
    if (apply_field_quantity_purchase(poor, callbacks) !=
            FieldQuantityPurchaseResult::insufficient_funds ||
        poor.availability_marker != 0 || poor.funds != 9U ||
        poor.purchased_quantities[0] != 0U || insufficient_messages != 1 ||
        events.size() != 16U || events[10] != "grid" ||
        events[11] != "title" || events[12] != "quantity" ||
        events[13] != "cost" || events[14] != "insufficient" ||
        events[15] != "wait") {
        return 4;
    }

    FieldQuantityPurchaseState cancelled{};
    cancelled.quantity = 2U;
    cancelled.preview_cost = 10U;
    cancelled.funds = 10U;
    cancelled.availability_marker = 1;
    const FieldQuantityPurchaseCallbacks cancel_callbacks{
        {}, {}, {}, {}, {}, {}, [] { return false; }};
    if (apply_field_quantity_purchase(cancelled, cancel_callbacks) !=
            FieldQuantityPurchaseResult::cancelled ||
        cancelled.availability_marker != 0 || cancelled.funds != 10U ||
        cancelled.purchased_quantities[0] != 0U) {
        return 5;
    }

    FieldQuantityPurchaseState invalid{};
    invalid.category = 5U;
    invalid.availability_marker = 1;
    if (apply_field_quantity_purchase(invalid, callbacks) !=
            FieldQuantityPurchaseResult::invalid_category ||
        invalid.availability_marker != 1 || invalid.service_result != -1 ||
        confirmations != 2) {
        return 6;
    }

    FieldQuantityPurchaseState malformed_high_bit{};
    malformed_high_bit.category = 0xFFU;
    malformed_high_bit.availability_marker = 1;
    if (apply_field_quantity_purchase(malformed_high_bit, callbacks) !=
            FieldQuantityPurchaseResult::invalid_category ||
        malformed_high_bit.availability_marker != 1 ||
        malformed_high_bit.service_result != -1) {
        return 7;
    }

    return 0;
}

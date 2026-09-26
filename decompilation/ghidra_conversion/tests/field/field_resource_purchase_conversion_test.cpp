#include "shadowrun_ghidra/field/field_resource_purchase_conversion.hpp"

#include <array>

using namespace shadowrun::ghidra_conversion;

int main() {
    const std::array<std::uint16_t, 4> costs{25U, 40U, 60U, 80U};
    int refreshes = 0;
    int satisfied_messages = 0;
    int limit_messages = 0;
    int funds_messages = 0;
    const FieldResourcePurchaseCallbacks callbacks{
        [&] { ++refreshes; },
        [&] { ++satisfied_messages; },
        [&] { ++limit_messages; },
        [&] { ++funds_messages; },
        [] { return true; },
    };

    FieldResourcePurchaseState category_zero{};
    category_zero.requested_quantity = 2U;
    category_zero.progress_category_0 = 69U;
    category_zero.funds = 100U;
    if (apply_field_resource_purchase(category_zero, costs, callbacks) !=
            FieldResourcePurchaseResult::applied ||
        category_zero.progress_category_0 != 79U ||
        category_zero.funds != 75U || category_zero.preview_cost != 25U ||
        refreshes != 1) {
        return 1;
    }

    FieldResourcePurchaseState already_satisfied{};
    already_satisfied.category = 1U;
    already_satisfied.requested_quantity = 1U;
    already_satisfied.progress_category_1 = 0x50U;
    already_satisfied.funds = 100U;
    if (apply_field_resource_purchase(already_satisfied, costs, callbacks) !=
            FieldResourcePurchaseResult::category_satisfied ||
        already_satisfied.funds != 100U || satisfied_messages != 1) {
        return 2;
    }

    FieldResourcePurchaseState poor{};
    poor.category = 2U;
    poor.requested_quantity = 3U;
    poor.progress_category_2 = 1U;
    poor.funds = 59U;
    if (apply_field_resource_purchase(poor, costs, callbacks) !=
            FieldResourcePurchaseResult::insufficient_funds ||
        poor.progress_category_2 != 1U || poor.funds != 59U ||
        funds_messages != 1) {
        return 3;
    }

    FieldResourcePurchaseState category_three{};
    category_three.category = 3U;
    category_three.progress_category_3 = 2U;
    category_three.funds = 100U;
    if (apply_field_resource_purchase(category_three, costs, callbacks) !=
            FieldResourcePurchaseResult::applied ||
        category_three.progress_category_3 != 3U ||
        category_three.funds != 20U) {
        return 4;
    }

    FieldResourcePurchaseState category_three_limit{};
    category_three_limit.category = 3U;
    category_three_limit.progress_category_3 = 3U;
    category_three_limit.funds = 100U;
    if (apply_field_resource_purchase(
            category_three_limit, costs, callbacks) !=
            FieldResourcePurchaseResult::category_limit ||
        category_three_limit.funds != 100U || limit_messages != 1) {
        return 5;
    }

    FieldResourcePurchaseState cancelled{};
    cancelled.requested_quantity = 1U;
    cancelled.funds = 100U;
    const FieldResourcePurchaseCallbacks cancel_callbacks{
        {}, {}, {}, {}, [] { return false; }};
    if (apply_field_resource_purchase(cancelled, costs, cancel_callbacks) !=
            FieldResourcePurchaseResult::cancelled ||
        cancelled.preview_cost != 25U || cancelled.funds != 100U ||
        cancelled.progress_category_0 != 0U) {
        return 6;
    }

    FieldResourcePurchaseState invalid{};
    invalid.category = 4U;
    if (apply_field_resource_purchase(invalid, costs, callbacks) !=
            FieldResourcePurchaseResult::invalid_category ||
        refreshes != 5) {
        return 7;
    }

    return 0;
}

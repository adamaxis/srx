#include <array>
#include <string>
#include <vector>

#include "shadowrun_ghidra/field/field_resource_purchase_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    const std::array<std::uint16_t, 4> costs{25U, 40U, 60U, 80U};
    std::vector<std::string> events;
    const FieldResourcePurchaseCallbacks callbacks{
        [&] { events.emplace_back("grid"); },
        [&] { events.emplace_back("satisfied"); },
        [&] { events.emplace_back("limit"); },
        [&] { events.emplace_back("insufficient"); },
        [&] { events.emplace_back("title"); },
        [&] { events.emplace_back("cost"); },
        [&] { events.emplace_back("wait"); },
        [&] { events.emplace_back("confirm"); return true; },
    };

    FieldResourcePurchaseState category_zero{};
    category_zero.requested_quantity = 2U;
    category_zero.progress_category_0 = 69U;
    category_zero.funds = 100U;
    if (apply_field_resource_purchase(category_zero, costs, callbacks) !=
            FieldResourcePurchaseResult::applied ||
        category_zero.progress_category_0 != 79U ||
        category_zero.funds != 75U || category_zero.preview_cost != 25U ||
        category_zero.service_result != 0 ||
        events != std::vector<std::string>{"grid", "title", "cost",
                                           "confirm"}) {
        return 1;
    }

    FieldResourcePurchaseState already_satisfied{};
    already_satisfied.category = 1U;
    already_satisfied.requested_quantity = 1U;
    already_satisfied.progress_category_1 = 0x50U;
    already_satisfied.funds = 100U;
    if (apply_field_resource_purchase(already_satisfied, costs, callbacks) !=
            FieldResourcePurchaseResult::category_satisfied ||
        already_satisfied.funds != 100U || already_satisfied.preview_cost != 0U ||
        events.size() != 7U || events[4] != "grid" ||
        events[5] != "satisfied" || events[6] != "wait") {
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
        poor.preview_cost != 0U || events.size() != 10U ||
        events[7] != "grid" || events[8] != "insufficient" ||
        events[9] != "wait") {
        return 3;
    }

    FieldResourcePurchaseState category_three{};
    category_three.category = 3U;
    category_three.progress_category_3 = 2U;
    category_three.funds = 100U;
    if (apply_field_resource_purchase(category_three, costs, callbacks) !=
            FieldResourcePurchaseResult::applied ||
        category_three.progress_category_3 != 3U ||
        category_three.funds != 20U || events.size() != 14U ||
        events[10] != "grid" || events[11] != "title" ||
        events[12] != "cost" || events[13] != "confirm") {
        return 4;
    }

    FieldResourcePurchaseState category_three_limit{};
    category_three_limit.category = 3U;
    category_three_limit.progress_category_3 = 3U;
    category_three_limit.funds = 100U;
    if (apply_field_resource_purchase(
            category_three_limit, costs, callbacks) !=
            FieldResourcePurchaseResult::category_limit ||
        category_three_limit.funds != 100U || events.size() != 17U ||
        events[14] != "grid" || events[15] != "limit" ||
        events[16] != "wait") {
        return 5;
    }

    FieldResourcePurchaseState cancelled{};
    cancelled.requested_quantity = 1U;
    cancelled.funds = 100U;
    const FieldResourcePurchaseCallbacks cancel_callbacks{
        {}, {}, {}, {}, {}, {}, {}, [] { return false; }};
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
        invalid.service_result != -1 || events.size() != 17U) {
        return 7;
    }

    FieldResourcePurchaseState wrapped_category_two{};
    wrapped_category_two.category = 2U;
    wrapped_category_two.requested_quantity = 26U;
    wrapped_category_two.progress_category_2 = 4U;
    wrapped_category_two.funds = 100U;
    // 26 * 10 is compared as the source's low byte: 0x04 is already
    // satisfied, so no purchase is charged.
    if (apply_field_resource_purchase(
            wrapped_category_two, costs, callbacks) !=
            FieldResourcePurchaseResult::category_satisfied ||
        wrapped_category_two.funds != 100U || events.size() != 20U ||
        events[17] != "grid" || events[18] != "satisfied" ||
        events[19] != "wait") {
        return 8;
    }

    FieldResourcePurchaseState malformed_high_bit{};
    malformed_high_bit.category = 0xFFU;
    if (apply_field_resource_purchase(
            malformed_high_bit, costs, callbacks) !=
            FieldResourcePurchaseResult::invalid_category ||
        malformed_high_bit.service_result != -1 || events.size() != 20U) {
        return 9;
    }

    return 0;
}

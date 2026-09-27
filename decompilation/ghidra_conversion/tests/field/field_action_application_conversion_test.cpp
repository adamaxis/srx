#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionCostContext context{};
    FieldActionPurchaseState state{};
    state.funds = 1000U;
    std::vector<std::string> events;
    const FieldActionApplicationCallbacks callbacks{
        [&] { events.emplace_back("refresh"); },
        [&] { events.emplace_back("no-slot"); },
        [&] { events.emplace_back("insufficient"); },
        [&] { events.emplace_back("limit"); },
        [&] { events.emplace_back("wait"); },
        [&] { events.emplace_back("confirm"); return true; },
    };

    const FieldActionCostRecord ordinary{3U, 0x5AU, 0U};
    if (!expect(apply_selected_field_action(
                    ordinary, context, state,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::applied &&
                state.runner.action_codes[0] == 4U &&
                state.runner.companion_values[0] == 0x5AU &&
                state.funds == 900U &&
                events == std::vector<std::string>{"refresh", "confirm"})) {
        return 1;
    }

    FieldActionPurchaseState special{};
    special.funds = 1000U;
    const FieldActionCostRecord special_record{'E', 0U, 2U};
    if (!expect(apply_selected_field_action(
                    special_record, context, special, {}, callbacks) ==
                    FieldActionApplicationResult::applied &&
                special.runner.special_action_count == 1U &&
                special.funds == 998U && events.size() == 4U &&
                events[2] == "refresh" && events[3] == "confirm")) {
        return 2;
    }

    FieldActionPurchaseState full{};
    full.funds = 1000U;
    full.runner.action_codes.fill(1U);
    if (!expect(apply_selected_field_action(
                    ordinary, context, full,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::no_inventory_slot &&
                events.size() == 7U && events[4] == "refresh" &&
                events[5] == "no-slot" && events[6] == "wait")) {
        return 3;
    }

    FieldActionPurchaseState poor{};
    poor.funds = 50U;
    if (!expect(apply_selected_field_action(
                    ordinary, context, poor,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::insufficient_funds &&
                poor.runner.action_codes[0] == 0U &&
                events.size() == 10U && events[7] == "refresh" &&
                events[8] == "insufficient" && events[9] == "wait")) {
        return 4;
    }

    FieldActionPurchaseState cancelled{};
    cancelled.funds = 1000U;
    const FieldActionApplicationCallbacks cancel_callbacks{
        {}, {}, {}, {}, {},
        [] { return false; },
    };
    if (!expect(apply_selected_field_action(
                    ordinary, context, cancelled,
                    [](const std::uint8_t) { return 100U; },
                    cancel_callbacks) == FieldActionApplicationResult::cancelled &&
                cancelled.runner.action_codes[0] == 0U &&
                cancelled.funds == 1000U)) {
        return 5;
    }

    FieldActionPurchaseState capped{};
    capped.funds = 1000U;
    capped.runner.special_action_count = 0x14U;
    if (!expect(apply_selected_field_action(
                    special_record, context, capped, {}, callbacks) ==
                    FieldActionApplicationResult::special_action_limit &&
                capped.runner.special_action_count == 0x14U &&
                events.size() == 14U && events[10] == "refresh" &&
                events[11] == "confirm" && events[12] == "refresh" &&
                events[13] == "limit")) {
        return 6;
    }

    const FieldActionCostRecord invalid{0xFFU, 0U, 0U};
    if (!expect(apply_selected_field_action(
                    invalid, context, state,
                    [](const std::uint8_t) { return 0U; }, callbacks) ==
                FieldActionApplicationResult::invalid_record &&
                state.service_result == -1 && events.size() == 14U)) {
        return 7;
    }

    return 0;
}

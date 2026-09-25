#include "shadowrun_ghidra/field_action_application_conversion.hpp"

#include <cstdint>

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
    int detail_refreshes = 0;
    int no_slot_messages = 0;
    int insufficient_messages = 0;
    int limit_messages = 0;
    const FieldActionApplicationCallbacks callbacks{
        [&] { ++detail_refreshes; },
        [&] { ++no_slot_messages; },
        [&] { ++insufficient_messages; },
        [&] { ++limit_messages; },
        [&](const std::uint32_t preview) {
            return std::optional<std::uint32_t>(preview > 10U ? preview - 10U
                                                               : preview);
        },
    };

    const FieldActionCostRecord ordinary{3U, 0x5AU, 0U};
    if (!expect(apply_selected_field_action(
                    ordinary, context, state,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::applied &&
                state.runner.action_codes[0] == 4U &&
                state.runner.companion_values[0] == 0x5AU &&
                state.funds == 910U && detail_refreshes == 1)) {
        return 1;
    }

    FieldActionPurchaseState special{};
    special.funds = 1000U;
    const FieldActionCostRecord special_record{'E', 0U, 2U};
    if (!expect(apply_selected_field_action(
                    special_record, context, special, {}, callbacks) ==
                    FieldActionApplicationResult::applied &&
                special.runner.special_action_count == 1U &&
                special.funds == 998U)) {
        return 2;
    }

    FieldActionPurchaseState full{};
    full.funds = 1000U;
    full.runner.action_codes.fill(1U);
    if (!expect(apply_selected_field_action(
                    ordinary, context, full,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::no_inventory_slot &&
                no_slot_messages == 1)) {
        return 3;
    }

    FieldActionPurchaseState poor{};
    poor.funds = 50U;
    if (!expect(apply_selected_field_action(
                    ordinary, context, poor,
                    [](const std::uint8_t) { return 100U; }, callbacks) ==
                    FieldActionApplicationResult::insufficient_funds &&
                poor.runner.action_codes[0] == 0U &&
                insufficient_messages == 1)) {
        return 4;
    }

    FieldActionPurchaseState cancelled{};
    cancelled.funds = 1000U;
    const FieldActionApplicationCallbacks cancel_callbacks{
        {}, {}, {}, {},
        [](const std::uint32_t) -> std::optional<std::uint32_t> {
            return std::nullopt;
        },
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
                limit_messages == 1)) {
        return 6;
    }

    const FieldActionCostRecord invalid{0xFFU, 0U, 0U};
    if (!expect(apply_selected_field_action(
                    invalid, context, state,
                    [](const std::uint8_t) { return 0U; }, callbacks) ==
                FieldActionApplicationResult::invalid_record)) {
        return 7;
    }

    return 0;
}

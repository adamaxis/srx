#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <cstdint>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(calculate_field_action_quantity_cost(0, 2) == 200U &&
                calculate_field_action_quantity_cost(1, 2) == 1200U &&
                calculate_field_action_quantity_cost(3, 2) == 800U)) {
        return 1;
    }

    FieldActionCostContext context{};
    context.runner_cost_stat = 12U;
    context.runner_discount_threshold = 5U;
    if (!expect(calculate_runner_cost_adjustment(3200U, context) == 700U)) {
        return 2;
    }

    context.runner_cost_stat = 30U;
    if (!expect(calculate_runner_cost_adjustment(3200U, context) == 1000U)) {
        return 3;
    }

    context = {};
    context.mode = 4U;
    context.feature_flags = 0x40U;
    if (!expect(apply_field_action_cost_discounts(1000U, context) == 900U)) {
        return 4;
    }

    context.mode = 5U;
    context.feature_flags = 0x80U;
    if (!expect(apply_field_action_cost_discounts(1000U, context) == 750U)) {
        return 5;
    }

    context = {};
    context.mode = 2U;
    context.feature_flags = 0x04U;
    if (!expect(apply_field_action_cost_discounts(1001U, context) == 500U)) {
        return 6;
    }

    context = {};
    context.mode = 8U;
    context.feature_flags = 0x02U;
    if (!expect(apply_field_action_cost_discounts(1000U, context) == 500U &&
                apply_runner_cost_surcharge(1000U, context) == 1000U)) {
        return 7;
    }

    context = {};
    context.runner_cost_stat = 8U;
    context.runner_discount_threshold = 3U;
    const FieldActionCostRecord record{7U, 0U, 2U};
    if (!expect(calculate_field_action_record_cost(
                    record, context,
                    [](const std::uint8_t type) {
                        return type == 7U ? 1000U : 0U;
                    }) == 170U)) {
        return 8;
    }

    context.runner_cost_stat = 13U;
    context.runner_discount_threshold = 3U;
    if (!expect(apply_large_field_action_cost_discount(0x10000U, context) ==
                0x9C00U)) {
        return 9;
    }

    return 0;
}

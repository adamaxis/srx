#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {
namespace {

bool source_subtract_byte_is_nonpositive(const std::uint8_t minuend,
                                         const std::uint8_t subtrahend,
                                         const std::uint8_t result) {
    // 0x57B22 is SUB.B followed by BLE. Preserve the source N/V/Z flags;
    // a signed host cast loses the overflow case.
    const bool negative = (result & 0x80U) != 0U;
    const bool zero = result == 0U;
    const bool overflow =
        (((minuend ^ subtrahend) & 0x80U) != 0U) &&
        (((minuend ^ result) & 0x80U) != 0U);
    return zero || (negative != overflow);
}

std::uint32_t runner_margin(const FieldActionCostContext& context) {
    const auto wrapped_margin = static_cast<std::uint8_t>(
        context.runner_cost_stat - context.runner_discount_threshold);
    if (source_subtract_byte_is_nonpositive(
            context.runner_cost_stat, context.runner_discount_threshold,
            wrapped_margin)) {
        return 0U;
    }

    // The following CMP.W #0xB/BMI sees the cleared word containing the
    // wrapped byte. Values 1..10 retain their value; 11..255 clamp to 10.
    return std::min<std::uint32_t>(wrapped_margin, 10U);
}

std::uint32_t calculate_field_action_record_cost_from_base_cost(
    const std::uint16_t source_base_cost,
    const FieldActionCostRecord& record,
    const FieldActionCostContext& context) {
    auto base_cost = static_cast<std::uint32_t>(source_base_cost);
    if (record.quantity != 0U) {
        // 0x57A62 performs a 68000 word multiply/divide and then keeps the
        // resulting 32-bit value's low word. Host values remain wide until
        // this explicit source-visible truncation point.
        const auto scaled =
            (static_cast<std::uint64_t>(base_cost) *
             static_cast<std::uint32_t>(record.quantity)) /
            10U;
        base_cost = static_cast<std::uint32_t>(scaled & 0xFFFFU);
    }
    return apply_field_action_cost_discounts(base_cost, context);
}

} // namespace

std::uint32_t calculate_field_action_quantity_cost(
    const std::int8_t primary_cursor,
    const std::uint8_t quantity) {
    const auto q = static_cast<std::uint32_t>(quantity);
    const auto square = q * q;
    if (primary_cursor < 1) {
        return square * q * 0x19U;
    }

    const auto category_rate = primary_cursor < 3 ? 3U : 2U;
    return square * category_rate * 100U;
}

std::uint32_t calculate_runner_cost_adjustment(
    const std::uint32_t cost,
    const FieldActionCostContext& context) {
    return (cost >> 5U) * runner_margin(context);
}

std::uint32_t apply_field_action_cost_discounts(
    std::uint32_t base_cost,
    const FieldActionCostContext& context) {
    const auto percent_unit = base_cost / 100U;
    if (context.mode == 4U && (context.feature_flags & 0x40U) != 0U) {
        base_cost -= percent_unit * 10U;
    }
    if (context.mode == 5U && (context.feature_flags & 0x80U) != 0U) {
        base_cost -= percent_unit * 25U;
    }
    if (context.mode == 2U && (context.feature_flags & 0x04U) != 0U) {
        base_cost >>= 1U;
    }
    if (context.mode == 8U && (context.feature_flags & 0x02U) != 0U) {
        base_cost >>= 1U;
    }

    return base_cost - calculate_runner_cost_adjustment(base_cost, context);
}

std::uint32_t apply_runner_cost_surcharge(
    const std::uint32_t base_cost,
    const FieldActionCostContext& context) {
    return base_cost + calculate_runner_cost_adjustment(base_cost, context);
}

std::uint32_t calculate_field_action_record_cost(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& context,
    const FieldActionBaseCostLookup& base_cost_lookup) {
    if (!base_cost_lookup) {
        return 0U;
    }

    // The source loads a word from +0x22 before doing any quantity scaling.
    return calculate_field_action_record_cost_from_base_cost(
        static_cast<std::uint16_t>(base_cost_lookup(record.type) & 0xFFFFU),
        record, context);
}

std::uint32_t calculate_field_action_record_cost(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& context,
    const UsItemCatalog& catalog) {
    // 0x57A44 multiplies the source byte by 0x30 before reading D4540 +0x22.
    // This is deliberately not find_us_item_definition(), whose public item
    // code is one-based for ordinary inventory consumers.
    if (record.type >= catalog.size()) {
        return 0U;
    }
    return calculate_field_action_record_cost_from_base_cost(
        catalog[record.type].base_cost, record, context);
}

std::uint32_t apply_large_field_action_cost_discount(
    std::uint32_t base_cost,
    const FieldActionCostContext& context) {
    const auto margin = runner_margin(context);
    const auto adjustment = (base_cost >> 8U) * margin;
    for (int iteration = 0; iteration < 10; ++iteration) {
        base_cost -= adjustment;
    }
    return base_cost;
}

} // namespace shadowrun::ghidra_conversion

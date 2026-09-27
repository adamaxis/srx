#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// The three-byte records consumed by the field action menu are not runtime
// pointers in this model.  They are the record type, the record's secondary
// value, and an optional quantity.  The cost routines only inspect byte 0 and
// byte 2; byte 1 is consumed by the action application routine.
struct FieldActionCostRecord {
    std::uint8_t type = 0;
    std::uint8_t value = 0;
    std::uint8_t quantity = 0;
};

// These values are the typed equivalents of the source's FFF0B6, FF018F,
// active-runner +0x8C, and 1AAC6E[mode].  The ROM table is supplied by the
// caller because it is data evidence, not a native memory layout.
struct FieldActionCostContext {
    std::uint8_t mode = 0;
    std::uint8_t feature_flags = 0;
    std::uint8_t runner_cost_stat = 0;
    std::uint8_t runner_discount_threshold = 0;
};

using FieldActionBaseCostLookup =
    std::function<std::uint32_t(std::uint8_t record_type)>;

// ROM 0x5787C.  The field action quantity selector uses a cubic cost for the
// first category and quadratic costs for the remaining categories.
std::uint32_t calculate_field_action_quantity_cost(
    std::int8_t primary_cursor,
    std::uint8_t quantity);

// ROM 0x57B04.  The source subtracts the threshold as a signed byte, then
// caps a positive wrapped margin at ten points; each point removes cost >> 5
// from the amount being adjusted.
std::uint32_t calculate_runner_cost_adjustment(
    std::uint32_t cost,
    const FieldActionCostContext& context);

// ROM 0x57A6E.  Apply mode-specific feature discounts, then the runner-stat
// adjustment, and return the amount published to FFF0BE by the source.
std::uint32_t apply_field_action_cost_discounts(
    std::uint32_t base_cost,
    const FieldActionCostContext& context);

// ROM 0x57AF6.  This is the companion path used by the menu when the runner
// adjustment is added instead of subtracted.
std::uint32_t apply_runner_cost_surcharge(
    std::uint32_t base_cost,
    const FieldActionCostContext& context);

// ROM 0x57A44 -> 0x57A6E.  Look up the record's base rate at source table
// 0xD4540[type * 0x30 + 0x22], scale it by quantity/10 when quantity is
// present, then apply the normal field-action discounts.
std::uint32_t calculate_field_action_record_cost(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& context,
    const FieldActionBaseCostLookup& base_cost_lookup);

// ROM 0x57B42.  Large costs use a byte-scaled adjustment ten times instead of
// the normal cost >> 5 path.  The source selects this after the cost exceeds
// 0xFFFF, so the host function preserves the exact arithmetic without a
// Genesis-sized integer representation.
std::uint32_t apply_large_field_action_cost_discount(
    std::uint32_t base_cost,
    const FieldActionCostContext& context);

} // namespace shadowrun::ghidra_conversion

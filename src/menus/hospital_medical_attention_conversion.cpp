#include "shadowrun_ghidra/menus/hospital_medical_attention_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

// CMP.B followed by BPL tests the sign bit of the wrapped byte subtraction.
// For the actual runner class values (0 and 1 versus 2 and above), this is
// the source's class-under-two gate without widening the comparison into a
// host-specific signed/unsigned rule.
bool source_byte_subtraction_is_nonnegative(const std::uint8_t value,
                                            const std::uint8_t immediate) {
    return (static_cast<std::uint8_t>(value - immediate) & 0x80U) == 0U;
}

// 0xCDF6 performs DIVU #3, rounds a nonzero remainder upward, then subtracts
// one from the quotient. Keeping the intermediate signed matters for a zero
// attribute: zero becomes -1 before the source D4 subtraction.
std::int32_t source_thirds_minus_one(const std::uint8_t value) {
    const auto quotient = static_cast<std::int32_t>(value / 3U);
    const auto rounded_quotient =
        quotient + (value % 3U == 0U ? 0 : 1);
    return rounded_quotient - 1;
}

} // namespace

std::uint16_t calculate_hospital_medical_attention_cost(
    const HospitalMedicalAttentionCostInput& input) {
    auto service_level = static_cast<std::int32_t>(input.base_service_level);

    // CMP.B #2,(record+0x9B) / BPL / ADDQ.W #2,D4.
    if (!source_byte_subtraction_is_nonnegative(input.class_code, 2U)) {
        service_level += 2;
    }

    service_level -= source_thirds_minus_one(input.body_attribute);
    service_level -= source_thirds_minus_one(input.willpower_attribute);
    service_level = std::max<std::int32_t>(service_level, 2);

    // After the minimum-tier clamp, source D4 is reduced by seven. Negative
    // values choose an absolute-value * 20 factor; seven or above selects the
    // fixed factor ten.
    const auto tier_delta = service_level - 7;
    const auto percentage_factor =
        tier_delta >= 0 ? 10 : (-tier_delta * 20);

    // D6 starts as Body, is multiplied by the factor, divided by 100, then
    // rounded up when the remainder is at least 50. The source guarantees a
    // minimum returned charge of one.
    const auto scaled_body = static_cast<std::int32_t>(input.body_attribute) *
                             percentage_factor;
    auto cost = scaled_body / 100;
    if ((scaled_body % 100) >= 50) {
        ++cost;
    }
    return static_cast<std::uint16_t>(std::max<std::int32_t>(cost, 1));
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/entity_coordinate_conversion.hpp"

#include <array>

namespace {

// ROM 0x15E56. The companion table at 0x15E16 is the same 128-entry wave
// rotated by 96 entries; retaining that relationship avoids a second copy of
// the same extracted source data while preserving every table lookup.
constexpr std::array<std::uint16_t, 128> kHeadingWaveTable{
    0x0000U, 0x0647U, 0x0C8BU, 0x12C7U, 0x18F8U, 0x1F19U, 0x2527U,
    0x2B1EU, 0x30FBU, 0x36B9U, 0x3C56U, 0x41CDU, 0x471CU, 0x4C3FU,
    0x5133U, 0x55F4U, 0x5A81U, 0x5ED6U, 0x62F1U, 0x66CEU, 0x6A6CU,
    0x6DC9U, 0x70E1U, 0x73B5U, 0x7640U, 0x7883U, 0x7A7CU, 0x7C29U,
    0x7D89U, 0x7E9CU, 0x7F61U, 0x7FD7U, 0x7FFFU, 0x7FD7U, 0x7F61U,
    0x7E9CU, 0x7D89U, 0x7C29U, 0x7A7CU, 0x7883U, 0x7640U, 0x73B5U,
    0x70E1U, 0x6DC9U, 0x6A6CU, 0x66CEU, 0x62F1U, 0x5ED6U, 0x5A81U,
    0x55F4U, 0x5133U, 0x4C3FU, 0x471CU, 0x41CDU, 0x3C56U, 0x36B9U,
    0x30FBU, 0x2B1EU, 0x2527U, 0x1F19U, 0x18F8U, 0x12C7U, 0x0C8BU,
    0x0647U, 0x0000U, 0xF9B9U, 0xF375U, 0xED39U, 0xE708U, 0xE0E7U,
    0xDAD9U, 0xD4E2U, 0xCF05U, 0xC947U, 0xC3AAU, 0xBE33U, 0xB8E4U,
    0xB3C1U, 0xAECDU, 0xAA0CU, 0xA57FU, 0xA12AU, 0x9D0FU, 0x9932U,
    0x9594U, 0x9237U, 0x8F1FU, 0x8C4BU, 0x89C0U, 0x877DU, 0x8584U,
    0x83D7U, 0x8277U, 0x8164U, 0x809FU, 0x8029U, 0x8001U, 0x8029U,
    0x809FU, 0x8164U, 0x8277U, 0x83D7U, 0x8584U, 0x877DU, 0x89C0U,
    0x8C4BU, 0x8F1FU, 0x9237U, 0x9594U, 0x9932U, 0x9D0FU, 0xA1A2U,
    0xA57FU, 0xAA0CU, 0xAECDU, 0xB3C1U, 0xB8E4U, 0xBE33U, 0xC3AAU,
    0xC947U, 0xCF05U, 0xD4E2U, 0xDAD9U, 0xE0E7U, 0xE708U, 0xED39U,
    0xF375U, 0xF9B9U};

std::int16_t scale_heading_sample(
    const std::uint16_t raw_sample,
    const std::int16_t multiplier,
    const std::uint8_t arithmetic_shift) {
    const auto signed_sample = static_cast<std::int32_t>(
        static_cast<std::int16_t>(raw_sample));
    const auto scaled = signed_sample * static_cast<std::int32_t>(multiplier);
    const auto divisor = static_cast<std::int32_t>(1U << arithmetic_shift);
    const auto shifted = scaled >= 0
                             ? scaled / divisor
                             : -((-scaled + divisor - 1) / divisor);
    return static_cast<std::int16_t>(
        static_cast<std::uint16_t>(shifted));
}

} // namespace

namespace shadowrun::ghidra_conversion {

void approach_entity_fixed_point_target(EntityCoordinateTarget& state) {
    if (state.current == state.target) {
        return;
    }

    const auto delta = static_cast<std::uint16_t>(state.current - state.target);
    if (static_cast<std::int16_t>(delta) < 0) {
        if (delta < 0xF800U) {
            state.current = static_cast<std::uint16_t>(state.current + 0x800U);
            return;
        }
    } else if (delta > 0x7FFU) {
        state.current = static_cast<std::uint16_t>(state.current - 0x800U);
        return;
    }

    state.current = state.target;
}

void apply_mode_five_boundary_motion(FieldBoundaryMotionState& state,
                                     const std::uint8_t mode) {
    if (mode != 5U || state.y <= 0x11FFU || state.y >= 0x1F80U ||
        state.x <= 0x25FFU) {
        return;
    }

    if (state.x < 0x2800U) {
        state.y = static_cast<std::uint16_t>(state.y - 8U);
        return;
    }
    if (state.x > 0x3BFFU && state.x < 0x3E00U) {
        state.y = static_cast<std::uint16_t>(state.y + 8U);
    }
}

EntityWorldProbeResult probe_entity_world_position(
    const EntityFixedPointPosition& position,
    const EntityWorldProbe& probe) {
    const auto world_x = position.world_x +
        (static_cast<std::int32_t>(position.tile_offset_x) * 0x100);
    const auto world_y = position.world_y +
        (static_cast<std::int32_t>(position.tile_offset_y) * 0x100);
    return EntityWorldProbeResult{world_x, world_y, probe(world_x, world_y)};
}

EntityHeadingOffsets derive_entity_heading_offsets(
    const std::uint16_t heading) {
    return derive_scaled_entity_heading_offsets(heading, 0x000CU, 4U);
}

EntityHeadingOffsets derive_scaled_entity_heading_offsets(
    const std::uint16_t heading,
    const std::int16_t multiplier,
    const std::uint8_t arithmetic_shift) {
    const auto table_byte_index = static_cast<std::uint16_t>(
        (heading >> 8U) & 0x00FEU);
    const auto table_index = static_cast<std::size_t>(table_byte_index >> 1U);
    const auto vertical_index = (table_index + 96U) & 0x7FU;
    return EntityHeadingOffsets{
        scale_heading_sample(kHeadingWaveTable[table_index], multiplier,
                             arithmetic_shift),
        scale_heading_sample(kHeadingWaveTable[vertical_index], multiplier,
                             arithmetic_shift)};
}

namespace {

std::int16_t heading_product_high_word(
    const std::int16_t magnitude,
    const std::uint16_t raw_sample) {
    const auto sample = static_cast<std::int16_t>(raw_sample);
    const auto doubled_product =
        static_cast<std::int64_t>(magnitude) * sample * 2;

    // 0x15DE2/0x15DEA are 32-bit ADD.L operations. Keep the source's
    // high-word result explicit while avoiding a host signed-overflow path.
    const auto product_bits = static_cast<std::uint32_t>(doubled_product);
    return static_cast<std::int16_t>(product_bits >> 16U);
}

} // namespace

EntityHeadingVector transform_entity_heading_vector(
    const std::uint16_t heading,
    const std::int16_t magnitude) {
    if (magnitude == 0) {
        return {};
    }

    const auto table_byte_index = static_cast<std::uint16_t>(
        (heading >> 8U) & 0x00FEU);
    const auto table_index = static_cast<std::size_t>(table_byte_index >> 1U);
    const auto vertical_index = (table_index + 96U) & 0x7FU;
    return EntityHeadingVector{
        heading_product_high_word(magnitude, kHeadingWaveTable[table_index]),
        heading_product_high_word(magnitude,
                                  kHeadingWaveTable[vertical_index])};
}

} // namespace shadowrun::ghidra_conversion

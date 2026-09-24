#include "shadowrun_ghidra/movement_metric_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

std::uint32_t compute_diagonal_step_cost(
    const std::int32_t delta_x,
    const std::int32_t delta_y) {
    const auto x = static_cast<std::uint32_t>(
        delta_x < 0 ? -static_cast<std::int64_t>(delta_x) : delta_x);
    const auto y = static_cast<std::uint32_t>(
        delta_y < 0 ? -static_cast<std::int64_t>(delta_y) : delta_y);
    const auto major = std::max(x, y);
    const auto minor = std::min(x, y);
    return major + ((minor + 1U) / 2U);
}

RelativeHeadingMeasurement measure_heading_and_distance(
    const std::int32_t delta_x,
    const std::int32_t delta_y) {
    const auto x = static_cast<std::uint64_t>(
        delta_x < 0 ? -static_cast<std::int64_t>(delta_x) : delta_x);
    const auto y = static_cast<std::uint64_t>(
        delta_y < 0 ? -static_cast<std::int64_t>(delta_y) : delta_y);
    auto major = y;
    auto minor = x;
    const bool x_is_smaller = minor < major;
    if (!x_is_smaller) {
        std::swap(major, minor);
    }
    const auto distance = static_cast<std::uint32_t>(major + minor / 2U);

    while (major >= 0x20U) {
        major >>= 1U;
        minor >>= 1U;
    }
    if (!x_is_smaller) {
        std::swap(major, minor);
    }

    static constexpr std::uint16_t kHeadingBases[16] = {
        0x0040U, 0x0080U, 0x0080U, 0xFF00U,
        0x0040U, 0xFF00U, 0x00C0U, 0xFF00U,
        0x0000U, 0x0000U, 0x00C0U, 0xFF00U,
        0xFF00U, 0xFF00U, 0xFF00U, 0xFF00U,
    };
    static constexpr std::uint8_t kHeadingCorrections[1024] = {
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
        64U, 32U, 19U, 13U, 10U, 8U, 7U, 6U, 5U, 5U, 4U, 4U, 3U, 3U, 3U, 3U,
        3U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U, 1U, 1U, 1U, 1U,
        64U, 45U, 32U, 24U, 19U, 16U, 13U, 11U, 10U, 9U, 8U, 7U, 7U, 6U, 6U, 5U,
        5U, 5U, 5U, 4U, 4U, 4U, 4U, 4U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U,
        64U, 51U, 40U, 32U, 26U, 22U, 19U, 16U, 15U, 13U, 12U, 11U, 10U, 9U, 9U, 8U,
        8U, 7U, 7U, 6U, 6U, 6U, 6U, 5U, 5U, 5U, 5U, 5U, 4U, 4U, 4U, 4U,
        64U, 54U, 45U, 38U, 32U, 27U, 24U, 21U, 19U, 17U, 16U, 14U, 13U, 12U, 11U, 11U,
        10U, 9U, 9U, 8U, 8U, 8U, 7U, 7U, 7U, 6U, 6U, 6U, 6U, 6U, 5U, 5U,
        64U, 56U, 48U, 42U, 37U, 32U, 28U, 25U, 23U, 21U, 19U, 17U, 16U, 15U, 14U, 13U,
        12U, 12U, 11U, 10U, 10U, 10U, 9U, 9U, 8U, 8U, 8U, 7U, 7U, 7U, 7U, 7U,
        64U, 57U, 51U, 45U, 40U, 36U, 32U, 29U, 26U, 24U, 22U, 20U, 19U, 18U, 16U, 16U,
        15U, 14U, 13U, 12U, 12U, 11U, 11U, 10U, 10U, 10U, 9U, 9U, 9U, 8U, 8U, 8U,
        64U, 58U, 53U, 48U, 43U, 39U, 35U, 32U, 29U, 27U, 25U, 23U, 22U, 20U, 19U, 18U,
        17U, 16U, 15U, 14U, 14U, 13U, 13U, 12U, 12U, 11U, 11U, 10U, 10U, 10U, 9U, 9U,
        64U, 59U, 54U, 49U, 45U, 41U, 38U, 35U, 32U, 30U, 27U, 26U, 24U, 22U, 21U, 20U,
        19U, 18U, 17U, 16U, 16U, 15U, 14U, 14U, 13U, 13U, 12U, 12U, 11U, 11U, 11U, 10U,
        64U, 59U, 55U, 51U, 47U, 43U, 40U, 37U, 34U, 32U, 30U, 28U, 26U, 25U, 23U, 22U,
        21U, 20U, 19U, 18U, 17U, 16U, 16U, 15U, 15U, 14U, 14U, 13U, 13U, 12U, 12U, 12U,
        64U, 60U, 56U, 52U, 48U, 45U, 42U, 39U, 37U, 34U, 32U, 30U, 28U, 27U, 25U, 24U,
        23U, 22U, 21U, 20U, 19U, 18U, 17U, 17U, 16U, 16U, 15U, 14U, 14U, 14U, 13U, 13U,
        64U, 60U, 57U, 53U, 50U, 47U, 44U, 41U, 38U, 36U, 34U, 32U, 30U, 29U, 27U, 26U,
        25U, 23U, 22U, 21U, 20U, 20U, 19U, 18U, 18U, 17U, 16U, 16U, 15U, 15U, 14U, 14U,
        64U, 61U, 57U, 54U, 51U, 48U, 45U, 42U, 40U, 38U, 36U, 34U, 32U, 30U, 29U, 27U,
        26U, 25U, 24U, 23U, 22U, 21U, 20U, 20U, 19U, 18U, 18U, 17U, 16U, 16U, 16U, 15U,
        64U, 61U, 58U, 55U, 52U, 49U, 46U, 44U, 42U, 39U, 37U, 35U, 34U, 32U, 30U, 29U,
        28U, 27U, 25U, 24U, 23U, 23U, 22U, 21U, 20U, 20U, 19U, 18U, 18U, 17U, 17U, 16U,
        64U, 61U, 58U, 55U, 53U, 50U, 48U, 45U, 43U, 41U, 39U, 37U, 35U, 34U, 32U, 31U,
        29U, 28U, 27U, 26U, 25U, 24U, 23U, 22U, 22U, 21U, 20U, 19U, 19U, 18U, 18U, 17U,
        64U, 61U, 59U, 56U, 53U, 51U, 48U, 46U, 44U, 42U, 40U, 38U, 37U, 35U, 33U, 32U,
        31U, 29U, 28U, 27U, 26U, 25U, 24U, 24U, 23U, 22U, 21U, 21U, 20U, 19U, 19U, 18U,
        64U, 61U, 59U, 56U, 54U, 52U, 49U, 47U, 45U, 43U, 41U, 39U, 38U, 36U, 35U, 33U,
        32U, 31U, 30U, 29U, 27U, 27U, 26U, 25U, 24U, 23U, 22U, 22U, 21U, 21U, 20U, 19U,
        64U, 62U, 59U, 57U, 55U, 52U, 50U, 48U, 46U, 44U, 42U, 41U, 39U, 37U, 36U, 35U,
        33U, 32U, 31U, 30U, 29U, 28U, 27U, 26U, 25U, 24U, 24U, 23U, 22U, 22U, 21U, 20U,
        64U, 62U, 59U, 57U, 55U, 53U, 51U, 49U, 47U, 45U, 43U, 42U, 40U, 39U, 37U, 36U,
        34U, 33U, 32U, 31U, 30U, 29U, 28U, 27U, 26U, 25U, 25U, 24U, 23U, 23U, 22U, 21U,
        64U, 62U, 60U, 58U, 56U, 54U, 52U, 50U, 48U, 46U, 44U, 43U, 41U, 40U, 38U, 37U,
        35U, 34U, 33U, 32U, 31U, 30U, 29U, 28U, 27U, 26U, 26U, 25U, 24U, 24U, 23U, 22U,
        64U, 62U, 60U, 58U, 56U, 54U, 52U, 50U, 48U, 47U, 45U, 44U, 42U, 41U, 39U, 38U,
        37U, 35U, 34U, 33U, 32U, 31U, 30U, 29U, 28U, 27U, 27U, 26U, 25U, 25U, 24U, 23U,
        64U, 62U, 60U, 58U, 56U, 54U, 53U, 51U, 49U, 48U, 46U, 44U, 43U, 41U, 40U, 39U,
        37U, 36U, 35U, 34U, 33U, 32U, 31U, 30U, 29U, 28U, 28U, 27U, 26U, 26U, 25U, 24U,
        64U, 62U, 60U, 58U, 57U, 55U, 53U, 51U, 50U, 48U, 47U, 45U, 44U, 42U, 41U, 40U,
        38U, 37U, 36U, 35U, 34U, 33U, 32U, 31U, 30U, 29U, 29U, 28U, 27U, 26U, 26U, 25U,
        64U, 62U, 60U, 59U, 57U, 55U, 54U, 52U, 50U, 49U, 47U, 46U, 44U, 43U, 42U, 40U,
        39U, 38U, 37U, 36U, 35U, 34U, 33U, 32U, 31U, 30U, 30U, 29U, 28U, 27U, 27U, 26U,
        64U, 62U, 61U, 59U, 57U, 56U, 54U, 52U, 51U, 49U, 48U, 46U, 45U, 44U, 42U, 41U,
        40U, 39U, 38U, 37U, 36U, 35U, 34U, 33U, 32U, 31U, 30U, 30U, 29U, 28U, 27U, 27U,
        64U, 62U, 61U, 59U, 58U, 56U, 54U, 53U, 51U, 50U, 48U, 47U, 46U, 44U, 43U, 42U,
        41U, 40U, 39U, 38U, 37U, 36U, 35U, 34U, 33U, 32U, 31U, 30U, 30U, 29U, 28U, 28U,
        64U, 62U, 61U, 59U, 58U, 56U, 55U, 53U, 52U, 50U, 49U, 48U, 46U, 45U, 44U, 43U,
        42U, 40U, 39U, 38U, 37U, 36U, 35U, 34U, 34U, 33U, 32U, 31U, 30U, 30U, 29U, 28U,
        64U, 62U, 61U, 59U, 58U, 57U, 55U, 54U, 52U, 51U, 50U, 48U, 47U, 46U, 45U, 43U,
        42U, 41U, 40U, 39U, 38U, 37U, 36U, 35U, 34U, 34U, 33U, 32U, 31U, 31U, 30U, 29U,
        64U, 63U, 61U, 60U, 58U, 57U, 55U, 54U, 53U, 51U, 50U, 49U, 48U, 46U, 45U, 44U,
        43U, 42U, 41U, 40U, 39U, 38U, 37U, 36U, 35U, 34U, 34U, 33U, 32U, 31U, 31U, 30U,
        64U, 63U, 61U, 60U, 58U, 57U, 56U, 54U, 53U, 52U, 50U, 49U, 48U, 47U, 46U, 45U,
        43U, 42U, 41U, 40U, 39U, 38U, 38U, 37U, 36U, 35U, 34U, 33U, 33U, 32U, 31U, 31U,
        64U, 63U, 61U, 60U, 59U, 57U, 56U, 55U, 53U, 52U, 51U, 50U, 48U, 47U, 46U, 45U,
        44U, 43U, 42U, 41U, 40U, 39U, 38U, 37U, 37U, 36U, 35U, 34U, 33U, 33U, 32U, 31U,
        64U, 63U, 61U, 60U, 59U, 57U, 56U, 55U, 54U, 52U, 51U, 50U, 49U, 48U, 47U, 46U,
        45U, 44U, 43U, 42U, 41U, 40U, 39U, 38U, 37U, 36U, 36U, 35U, 34U, 33U, 33U, 32U,
    };

    const auto sign_index = static_cast<std::size_t>(
        (delta_y < 0 ? 8U : delta_y == 0 ? 4U : 0U) |
        (delta_x < 0 ? 2U : delta_x == 0 ? 1U : 0U));
    auto heading_base = kHeadingBases[sign_index];
    if ((heading_base & 0x0040U) != 0U) {
        std::swap(major, minor);
    }
    const auto correction = kHeadingCorrections[(major << 5U) + minor];
    heading_base = static_cast<std::uint16_t>(
        (heading_base & 0xFF00U) |
        ((heading_base + correction) & 0x00FFU));
    return RelativeHeadingMeasurement{
        static_cast<std::uint16_t>(heading_base << 8U),
        distance,
    };
}

RelativeHeadingMeasurement measure_relative_heading_and_distance(
    const std::int32_t origin_x,
    const std::int32_t origin_y,
    const std::int32_t target_x,
    const std::int32_t target_y) {
    return measure_heading_and_distance(
        target_x - origin_x,
        target_y - origin_y);
}

bool is_entity_target_within_range(
    const std::uint8_t target_flags,
    const std::int32_t entity_x,
    const std::int32_t entity_y,
    const std::int32_t target_x,
    const std::int32_t target_y) {
    if ((target_flags & 0x01U) == 0U || (target_flags & 0x40U) != 0U) {
        return false;
    }
    return measure_relative_heading_and_distance(
               entity_x, entity_y, target_x, target_y)
               .distance < 0x200U;
}

std::optional<std::uint16_t> update_entity_heading_from_linked_target(
    const bool has_linked_target,
    const std::uint8_t target_flags,
    const std::uint16_t measured_heading,
    const std::int32_t entity_x,
    const std::int32_t entity_y,
    const std::int32_t target_x,
    const std::int32_t target_y) {
    if (!has_linked_target ||
        !is_entity_target_within_range(
            target_flags, entity_x, entity_y, target_x, target_y)) {
        return std::nullopt;
    }
    return measured_heading;
}

} // namespace shadowrun::ghidra_conversion

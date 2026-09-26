#include "shadowrun_ghidra/first_aid_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::uint8_t add_source_meter(std::uint8_t value,
                              const std::int16_t delta) {
    const auto candidate = static_cast<std::int16_t>(value) + delta;
    if (candidate <= 0) {
        return 0U;
    }
    return static_cast<std::uint8_t>(
        std::min<std::int16_t>(candidate, 10));
}

void advance_physical_meter(FirstAidTargetState& target,
                            const std::int16_t delta) {
    if ((target.flags & 0x01U) == 0U) {
        target.physical_meter = 0U;
        return;
    }
    target.physical_meter = add_source_meter(target.physical_meter, delta);
}

void advance_mental_meter(FirstAidTargetState& target,
                          const std::int16_t delta) {
    if ((target.flags & 0x01U) == 0U) {
        target.mental_meter = 0U;
        return;
    }

    const auto candidate = static_cast<std::int16_t>(target.mental_meter) +
                           delta;
    if (candidate > 0) {
        target.mental_meter = static_cast<std::uint8_t>(
            std::min<std::int16_t>(candidate, 10));
        return;
    }

    // 0x37EC passes a non-positive resulting mental byte to 0x3674 and then
    // clears the mental byte. Keep that source call relationship explicit
    // instead of folding the two meter routines into a generic clamp.
    advance_physical_meter(target, candidate);
    target.mental_meter = 0U;
}

std::uint8_t first_aid_difficulty(const FirstAidTargetState& target) {
    std::uint8_t difficulty = 0U;
    if (target.physical_meter >= 7U) {
        difficulty = 4U;
    } else if (target.physical_meter >= 4U) {
        difficulty = 6U;
    } else {
        difficulty = 8U;
    }

    const auto body_reduction = static_cast<std::uint8_t>(
        target.body_attribute / 3U);
    difficulty = difficulty > body_reduction
                     ? static_cast<std::uint8_t>(difficulty - body_reduction)
                     : 0U;
    return std::max<std::uint8_t>(difficulty, 2U);
}

} // namespace

FirstAidResolution apply_first_aid_menu_effect(
    const FirstAidUserState& user,
    FirstAidTargetState& target,
    const FirstAidRollService& roll_service) {
    FirstAidResolution resolution;
    resolution.roll.roll_count = user.biotech_roll_count;

    if (target.physical_meter == 10U && target.mental_meter == 10U) {
        resolution.result = FirstAidResult::target_already_full;
        return resolution;
    }

    resolution.roll.difficulty = first_aid_difficulty(target);
    if (!roll_service) {
        return resolution;
    }

    resolution.successes = roll_service(resolution.roll.roll_count,
                                         resolution.roll.difficulty);
    resolution.applied_delta = resolution.successes == 0U
                                    ? 1U
                                    : resolution.successes;

    // Source order is 0x3674 followed by 0x37EC. Both services consume the
    // target's active bit before 0x1253E publishes its new status flags.
    advance_physical_meter(target, resolution.applied_delta);
    advance_mental_meter(target, resolution.applied_delta);
    target.flags = static_cast<std::uint8_t>(
        (target.flags & ~0x50U) | 0x01U);
    target.continuation = 0xB45CU;

    resolution.result = FirstAidResult::treated;
    return resolution;
}

} // namespace shadowrun::ghidra_conversion

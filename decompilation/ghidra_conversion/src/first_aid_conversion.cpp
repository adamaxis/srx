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

FirstAidTargetList build_first_aid_target_list(
    const std::array<std::uint8_t, 3>& record_flags) {
    FirstAidTargetList list;
    for (std::size_t index = 0; index < record_flags.size(); ++index) {
        if ((record_flags[index] & 0x01U) == 0U ||
            list.active_count >= list.active_record_indices.size()) {
            continue;
        }
        list.active_record_indices[list.active_count++] = index;
    }
    return list;
}

std::size_t advance_first_aid_target_cursor(
    const std::size_t current_row,
    const std::size_t active_count,
    const std::int8_t delta) {
    const auto row_count = active_count + 1U;
    if (row_count == 0U) {
        return 0U;
    }

    auto row = current_row < row_count ? current_row : 0U;
    if (delta < 0) {
        row = row == 0U ? row_count - 1U : row - 1U;
    } else if (delta > 0) {
        row = (row + 1U) % row_count;
    }
    return row;
}

namespace {

void consume_first_aid_phase(std::uint8_t& item_code,
                             std::uint8_t& phase) {
    if (phase > 1U) {
        --phase;
        return;
    }
    item_code = 0U;
    phase = 0U;
}

} // namespace

FirstAidItemResolution use_first_aid_item(
    std::uint8_t& item_code,
    std::uint8_t& phase,
    const FirstAidUserState& user,
    FirstAidTargetState& target,
    const FirstAidRollService& roll_service) {
    if (item_code < 0x10U || item_code > 0x13U) {
        return {FirstAidItemResult::invalid_item, FirstAidMessage::none, 0U};
    }
    if (phase == 0U) {
        return {FirstAidItemResult::empty_item, FirstAidMessage::none, 0U};
    }

    if (item_code == 0x10U) {
        const auto resolution = apply_first_aid_menu_effect(
            user, target, roll_service);
        if (resolution.result == FirstAidResult::target_already_full) {
            return {FirstAidItemResult::target_not_injured,
                    FirstAidMessage::target_not_injured, 0U};
        }
        if (resolution.result == FirstAidResult::roll_service_unavailable) {
            return {FirstAidItemResult::roll_service_unavailable,
                    FirstAidMessage::none, 0U};
        }
        consume_first_aid_phase(item_code, phase);
        return {FirstAidItemResult::used, FirstAidMessage::medkit_used,
                resolution.applied_delta};
    }

    if (item_code == 0x11U) {
        if (target.mental_meter == 10U) {
            return {FirstAidItemResult::target_not_injured,
                    FirstAidMessage::target_not_injured, 0U};
        }
        advance_mental_meter(target, 2);
        if (target.physical_meter != 0U) {
            target.flags = static_cast<std::uint8_t>(
                (target.flags & ~0x50U) | 0x01U);
        }
    } else {
        if (target.physical_meter == 10U) {
            return {FirstAidItemResult::target_not_injured,
                    FirstAidMessage::target_not_injured, 0U};
        }
        advance_physical_meter(target, 2);
        if (target.mental_meter != 0U) {
            target.flags = static_cast<std::uint8_t>(
                (target.flags & ~0x50U) | 0x01U);
        }
    }

    consume_first_aid_phase(item_code, phase);
    return {FirstAidItemResult::used, FirstAidMessage::slap_patch_used, 2U};
}

} // namespace shadowrun::ghidra_conversion

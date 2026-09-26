#include "shadowrun_ghidra/menu_hud_meter_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t add_low_byte(const std::uint16_t value,
                           const std::uint8_t addition) {
    return static_cast<std::uint16_t>(
        (value & 0xFF00U) |
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(value) + addition));
}

std::uint8_t subtract_low_byte(const std::uint16_t value,
                               const std::uint8_t subtraction) {
    return static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(value) - subtraction);
}

std::uint16_t clamp_display_level(std::uint16_t level) {
    if (static_cast<std::int16_t>(level) <= 0) {
        return 1U;
    }
    return level >= 9U ? 8U : level;
}

} // namespace

std::uint16_t calculate_menu_action_success_percentage(
    std::uint16_t difficulty) {
    const auto minimum_check = static_cast<std::uint16_t>(difficulty - 2U);
    if (static_cast<std::int16_t>(minimum_check) < 0) {
        difficulty = 2U;
    }
    if (difficulty >= 7U) {
        return 10U;
    }
    return static_cast<std::uint16_t>((7U - difficulty) * 20U);
}

std::uint16_t scale_menu_hud_value_by_difficulty(
    const std::uint16_t value,
    std::uint16_t difficulty) {
    const auto percentage =
        calculate_menu_action_success_percentage(difficulty);
    return static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(value) * percentage) / 100U);
}

MenuHudMeterResult calculate_menu_hud_row_meter(
    const MenuHudMeterInputs& inputs,
    const MenuHudMeterCallbacks& callbacks) {
    const auto status = inputs.row_status;
    if (status == 4) {
        if (!callbacks.read_row_availability_value) {
            return {};
        }
        const auto availability = callbacks.read_row_availability_value();
        if (!availability.has_value()) {
            return {};
        }
        std::uint16_t difficulty = 6U;
        if (inputs.special_row_status >= 8) {
            difficulty = 4U;
        } else if (inputs.special_row_status >= 5) {
            difficulty = 5U;
        }
        return {clamp_display_level(scale_menu_hud_value_by_difficulty(
                        *availability, difficulty)),
                MenuHudMeterAction::level_calculated};
    }
    if (status > 4 && status < 8) {
        if (!callbacks.read_row_availability_value) {
            return {};
        }
        const auto availability = callbacks.read_row_availability_value();
        if (!availability.has_value()) {
            return {};
        }
        return {*availability, MenuHudMeterAction::level_calculated};
    }

    if (!callbacks.calculate_action_delay ||
        !callbacks.read_row_availability_value ||
        (status != 11 && !callbacks.select_status_adjusted_scale)) {
        return {};
    }
    const auto action_delay = callbacks.calculate_action_delay();
    const auto availability = callbacks.read_row_availability_value();
    if (!action_delay.has_value() || !availability.has_value()) {
        return {};
    }

    auto level = add_low_byte(*action_delay, inputs.active_runner_modifier);
    level = std::max(level, *availability);
    level = static_cast<std::uint16_t>(level + *availability);
    if (level == 0U) {
        level = 1U;
    }

    const auto cost_minus_one = static_cast<std::uint8_t>(
        inputs.runner_action_cost - 1U);
    const auto cost_check = subtract_low_byte(level, cost_minus_one);
    if (static_cast<std::int8_t>(cost_check) <= 0) {
        return {0U, MenuHudMeterAction::level_calculated};
    }

    const auto first_difficulty = static_cast<std::uint16_t>(
        inputs.runner_timing_difficulty +
        static_cast<std::uint16_t>(inputs.effect_phase) * 2U);
    level = scale_menu_hud_value_by_difficulty(level, first_difficulty);

    std::uint16_t scale_base = 0;
    std::uint16_t difficulty = inputs.profile_low_status;
    if (status == 11) {
        scale_base = inputs.runner_timing_difficulty;
        difficulty = inputs.profile_special_status;
    } else if (status >= 4) {
        const auto adjusted_scale = callbacks.select_status_adjusted_scale();
        if (!adjusted_scale.has_value()) {
            return {};
        }
        scale_base = *adjusted_scale;
        difficulty = inputs.profile_high_status;
    } else {
        const auto adjusted_scale = callbacks.select_status_adjusted_scale();
        if (!adjusted_scale.has_value()) {
            return {};
        }
        scale_base = *adjusted_scale;
    }

    difficulty = static_cast<std::uint16_t>(
        difficulty + static_cast<std::uint16_t>(inputs.effect_phase) * 2U);
    const auto percentage = calculate_menu_action_success_percentage(difficulty);
    const auto reduction = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(scale_base) * percentage) / 100U);
    level = static_cast<std::uint16_t>(level - reduction);

    if (static_cast<std::int16_t>(level) < 0) {
        level = 1U;
    } else if (level >= 9U) {
        level = 8U;
    }
    return {level, MenuHudMeterAction::level_calculated};
}

} // namespace shadowrun::ghidra_conversion

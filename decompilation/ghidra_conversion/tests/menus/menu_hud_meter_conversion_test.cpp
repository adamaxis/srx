#include "shadowrun_ghidra/menus/menu_hud_meter_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (calculate_menu_action_success_percentage(0U) != 100U ||
        calculate_menu_action_success_percentage(2U) != 100U ||
        calculate_menu_action_success_percentage(3U) != 80U ||
        calculate_menu_action_success_percentage(4U) != 60U ||
        calculate_menu_action_success_percentage(5U) != 40U ||
        calculate_menu_action_success_percentage(6U) != 20U ||
        calculate_menu_action_success_percentage(7U) != 10U ||
        calculate_menu_action_success_percentage(40U) != 10U) {
        return 1;
    }
    if (scale_menu_hud_value_by_difficulty(10U, 2U) != 10U ||
        scale_menu_hud_value_by_difficulty(10U, 5U) != 4U) {
        return 7;
    }

    MenuHudMeterCallbacks callbacks{};
    callbacks.read_row_availability_value = [] {
        return std::optional<std::uint16_t>(6U);
    };

    MenuHudMeterInputs direct{};
    direct.row_status = 5;
    const auto direct_result =
        calculate_menu_hud_row_meter(direct, callbacks);
    if (direct_result.action != MenuHudMeterAction::level_calculated ||
        direct_result.level != 6U) {
        return 2;
    }

    MenuHudMeterInputs special_direct{};
    special_direct.row_status = 4;
    special_direct.special_row_status = 9;
    const auto special_direct_result =
        calculate_menu_hud_row_meter(special_direct, callbacks);
    if (special_direct_result.level != 3U) {
        return 8;
    }

    MenuHudMeterInputs full{};
    full.row_status = 3;
    full.active_runner_modifier = 1U;
    full.runner_action_cost = 2U;
    full.profile_low_status = 4U;
    full.runner_timing_difficulty = 4U;
    full.effect_phase = 0U;
    callbacks.calculate_action_delay = [] {
        return std::optional<std::uint16_t>(4U);
    };
    callbacks.select_status_adjusted_scale = [] {
        return std::optional<std::uint16_t>(5U);
    };
    const auto full_result = calculate_menu_hud_row_meter(full, callbacks);
    // max(4+1, 6) * 2 = 12; first difficulty 4 scales it to 7. The
    // selected status-adjusted scale is 5 and profile difficulty 4 removes
    // 3, leaving level 4.
    if (full_result.action != MenuHudMeterAction::level_calculated ||
        full_result.level != 4U) {
        return 3;
    }

    MenuHudMeterInputs special{};
    special.row_status = 11;
    special.runner_action_cost = 1U;
    special.runner_timing_difficulty = 3U;
    special.profile_special_status = 5U;
    callbacks.read_row_availability_value = [] {
        return std::optional<std::uint16_t>(6U);
    };
    callbacks.calculate_action_delay = [] {
        return std::optional<std::uint16_t>(5U);
    };
    special.effect_phase = 0U;
    const auto special_result =
        calculate_menu_hud_row_meter(special, callbacks);
    // max(5,6)*2=12; difficulty 3 scales it to 9. The special scale base is
    // 3 and difficulty 5 removes 1, leaving level 8.
    if (special_result.level != 8U) {
        return 4;
    }

    MenuHudMeterInputs zero_after_reduction{};
    zero_after_reduction.row_status = 3;
    zero_after_reduction.runner_action_cost = 1U;
    zero_after_reduction.profile_low_status = 2U;
    callbacks.read_row_availability_value = [] {
        return std::optional<std::uint16_t>(0U);
    };
    callbacks.calculate_action_delay = [] {
        return std::optional<std::uint16_t>(0U);
    };
    callbacks.select_status_adjusted_scale = [] {
        return std::optional<std::uint16_t>(1U);
    };
    const auto zero_result =
        calculate_menu_hud_row_meter(zero_after_reduction, callbacks);
    if (zero_result.level != 0U) {
        return 5;
    }

    // The ROM uses CMP.W D6,D7 followed by BPL before adding D7 to D6.
    // For a malformed high-bit availability word, that wrapped sign-bit
    // predicate deliberately differs from std::max<unsigned>.
    MenuHudMeterInputs wrapped_compare{};
    wrapped_compare.row_status = 3;
    wrapped_compare.runner_action_cost = 0U;
    wrapped_compare.runner_timing_difficulty = 3U;
    wrapped_compare.profile_low_status = 2U;
    MenuHudMeterCallbacks wrapped_callbacks{};
    wrapped_callbacks.calculate_action_delay = [] {
        return std::optional<std::uint16_t>(0U);
    };
    wrapped_callbacks.read_row_availability_value = [] {
        return std::optional<std::uint16_t>(0x8000U);
    };
    wrapped_callbacks.select_status_adjusted_scale = [] {
        return std::optional<std::uint16_t>(0U);
    };
    const auto wrapped_result =
        calculate_menu_hud_row_meter(wrapped_compare, wrapped_callbacks);
    if (wrapped_result.action != MenuHudMeterAction::level_calculated ||
        wrapped_result.level != 8U) {
        return 9;
    }

    MenuHudMeterCallbacks unavailable{};
    if (calculate_menu_hud_row_meter(direct, unavailable).action !=
        MenuHudMeterAction::dependency_unavailable) {
        return 6;
    }
    return 0;
}

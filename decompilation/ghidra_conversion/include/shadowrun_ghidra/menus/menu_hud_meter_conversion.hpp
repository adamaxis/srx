#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct MenuHudMeterInputs {
    std::int8_t row_status = 0; // selected menu record +0x1A + row
    std::int8_t special_row_status = 0; // profile +0x0C, status-4 branch
    std::uint8_t active_runner_modifier = 0; // FFFBBE +0x00
    std::uint8_t runner_action_cost = 0; // compact record +0x05, then -1
    std::uint8_t runner_timing_difficulty = 0; // compact record +0x06
    std::uint8_t profile_low_status = 0; // FFFBBE +0x09
    std::uint8_t profile_high_status = 0; // FFFBBE +0x0A
    std::uint8_t profile_special_status = 0; // FFFBBE +0x0B
    std::uint8_t effect_phase = 0; // FFE16A
};

struct MenuHudMeterCallbacks {
    // ROM 0x55F44: D1=0 action-delay calculation.
    std::function<std::optional<std::uint16_t>()> calculate_action_delay;
    // ROM 0x19C5C: reads the row's availability/ability value into D7.
    std::function<std::optional<std::uint16_t>()>
        read_row_availability_value;
    // ROM 0x19BF6: selects the separate D6 scale value used by the full
    // calculation after the first difficulty scaling pass.
    std::function<std::optional<std::uint16_t>()>
        select_status_adjusted_scale;
};

enum class MenuHudMeterAction {
    level_calculated,
    dependency_unavailable,
};

struct MenuHudMeterResult {
    std::uint16_t level = 0;
    MenuHudMeterAction action = MenuHudMeterAction::dependency_unavailable;
};

// ROM 0x56632's percentage table: 2->100, 3->80, 4->60, 5->40, 6->20,
// and 7+->10.
std::uint16_t calculate_menu_action_success_percentage(
    std::uint16_t difficulty);

// ROM 0x56632: multiply the input value by that percentage and divide by
// 100, preserving the source word result.
std::uint16_t scale_menu_hud_value_by_difficulty(
    std::uint16_t value,
    std::uint16_t difficulty);

// ROM 0x56506: compute the numeric level used by one row of
// render_menu_hud_rows (0x1792E). Status 4 scales availability by a special
// difficulty bucket, statuses 5..7 use availability directly, and all other
// statuses use the source delay/difficulty calculation.
MenuHudMeterResult calculate_menu_hud_row_meter(
    const MenuHudMeterInputs& inputs,
    const MenuHudMeterCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

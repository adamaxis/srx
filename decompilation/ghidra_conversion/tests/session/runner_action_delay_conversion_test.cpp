#include "shadowrun_ghidra/session/runner_action_delay_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerActionDelayInput normal{};
    normal.active_record_attribute_a = 40U;
    normal.active_record_attribute_b = 80U;
    normal.active_record_modifier = 4U;
    normal.profile_rounding_marker = 7U;
    normal.request = 0;
    if (calculate_runner_action_delay(normal).delay != 39U) {
        return 1;
    }

    normal.active_record_attribute_a = 42U;
    normal.active_record_attribute_b = 80U;
    normal.request = 1;
    if (calculate_runner_action_delay(normal).delay != 40U) {
        return 2;
    }

    normal.request = -1;
    normal.profile_floor = 12U;
    if (calculate_runner_action_delay(normal).delay != 12U) {
        return 3;
    }

    RunnerActionDelayInput positive_mode{};
    positive_mode.active_record_attribute_a = 60U;
    positive_mode.active_record_attribute_b = 60U;
    positive_mode.mode_result = 1;
    positive_mode.profile_rounding_marker = 0U;
    positive_mode.request = 0;
    if (calculate_runner_action_delay(positive_mode).delay != 20U) {
        return 4;
    }
    positive_mode.request = 1;
    positive_mode.profile_floor = 30U;
    if (calculate_runner_action_delay(positive_mode).delay != 30U) {
        return 5;
    }

    RunnerActionDelayInput negative_mode = positive_mode;
    negative_mode.mode_result = -1;
    negative_mode.request = 0;
    if (calculate_runner_action_delay(negative_mode).delay != 40U) {
        return 6;
    }
    negative_mode.request = -1;
    negative_mode.profile_floor = 30U;
    if (calculate_runner_action_delay(negative_mode).delay != 20U) {
        return 7;
    }

    RunnerActionDelayInput wrapped{};
    wrapped.active_record_attribute_a = 250U;
    wrapped.active_record_attribute_b = 20U;
    wrapped.active_record_modifier = 10U;
    wrapped.profile_rounding_marker = 10U;
    wrapped.request = 0;
    // (250 + 20) wraps to 14 before the average, then 7 + 10 + 10 + 10
    // produces 37 and the normal branch returns 18.
    if (calculate_runner_action_delay(wrapped).delay != 18U) {
        return 8;
    }

    return 0;
}

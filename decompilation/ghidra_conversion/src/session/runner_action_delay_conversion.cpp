#include "shadowrun_ghidra/session/runner_action_delay_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

RunnerActionDelayResult calculate_runner_action_delay(
    const RunnerActionDelayInput& input) {
    auto delay = static_cast<std::uint16_t>(
        average_entity_delay_attributes(
            input.active_record_attribute_a,
            input.active_record_attribute_b));

    // 0x55F5A/0x55F6C are byte additions to D6. Keep the source wrap before
    // entering the word-sized scaling branches.
    auto wrapped_delay = static_cast<std::uint8_t>(delay);
    wrapped_delay = static_cast<std::uint8_t>(
        wrapped_delay + input.active_record_modifier);
    wrapped_delay = static_cast<std::uint8_t>(
        wrapped_delay + input.profile_rounding_marker);
    wrapped_delay = static_cast<std::uint8_t>(
        wrapped_delay + input.profile_rounding_marker);
    delay = wrapped_delay;

    if (input.mode_result > 0) {
        const auto third = static_cast<std::uint16_t>(delay / 3U);
        delay = static_cast<std::uint16_t>(delay - third);
        if (input.request == 0) {
            delay = third;
        } else {
            delay = std::min<std::uint16_t>(delay, input.profile_floor);
        }
    } else if (input.mode_result < 0) {
        const auto third = static_cast<std::uint16_t>(delay / 3U);
        // 0x55FC0 computes both components.  A nonnegative request keeps
        // the two-thirds D6 value; a negative request takes D5, the original
        // value minus that two-thirds component, i.e. the one-third value,
        // before entering the shared profile-floor clamp at 0x55F92.
        delay = input.request < 0
                    ? third
                    : static_cast<std::uint16_t>(delay - third);
    } else {
        const bool shifted_bit = (delay & 1U) != 0U;
        delay = static_cast<std::uint16_t>(delay >> 1U);
        if (shifted_bit && input.request != 0) {
            ++delay;
        }
    }

    if (input.request < 0) {
        delay = std::min<std::uint16_t>(delay, input.profile_floor);
    }
    return RunnerActionDelayResult{delay};
}

} // namespace shadowrun::ghidra_conversion

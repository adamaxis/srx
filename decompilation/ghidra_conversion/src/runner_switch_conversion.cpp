#include "shadowrun_ghidra/runner_switch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

RunnerSwitchDecision advance_runner_switch(RunnerSwitchState& state) {
    if (state.records.empty() || state.active_index >= state.records.size()) {
        return {RunnerSwitchResult::invalid_state, state.active_index,
                state.active_index};
    }

    const auto previous_index = state.active_index;
    auto candidate = previous_index;
    int terminal_wraps_remaining = 1;

    // The source table is terminated by record type 2. The bound protects the
    // host conversion when an extracted table is malformed or incomplete.
    for (std::size_t step = 0; step <= state.records.size(); ++step) {
        state.records[candidate].flags = static_cast<std::uint8_t>(
            state.records[candidate].flags & ~0x02U);

        if (state.records[candidate].record_type == 2U) {
            --terminal_wraps_remaining;
            candidate = 0U;
            if (terminal_wraps_remaining < 0) {
                return {RunnerSwitchResult::no_eligible_runner,
                        previous_index, previous_index};
            }
        } else {
            ++candidate;
            if (candidate >= state.records.size()) {
                return {RunnerSwitchResult::no_eligible_runner,
                        previous_index, previous_index};
            }
        }

        state.records[candidate].flags = static_cast<std::uint8_t>(
            state.records[candidate].flags | 0x02U);
        const auto flags = state.records[candidate].flags;
        const bool active = (flags & 0x01U) != 0U;
        const bool suppressed = (flags & (0x40U | 0x10U)) != 0U;
        const bool eligible = active &&
                              (state.suppression_override || !suppressed);
        if (!eligible) {
            continue;
        }
        if (candidate == previous_index) {
            return {RunnerSwitchResult::no_eligible_runner, previous_index,
                    previous_index};
        }

        state.active_index = candidate;
        return {RunnerSwitchResult::switched, previous_index, candidate};
    }

    return {RunnerSwitchResult::no_eligible_runner, previous_index,
            previous_index};
}

} // namespace shadowrun::ghidra_conversion

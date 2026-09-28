#include "shadowrun_ghidra/session/runner_switch_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerSwitchState state{
        {{0U, 0x01U}, {1U, 0x01U}, {2U, 0x01U}}, 0U, false};
    const auto first = advance_runner_switch(state);
    if (!expect(first.result == RunnerSwitchResult::switched &&
                first.previous_index == 0U && first.selected_index == 1U &&
                state.active_index == 1U &&
                (state.records[1].flags & 0x02U) != 0U &&
                (state.records[0].flags & 0x02U) == 0U)) {
        return 1;
    }

    state.records[2].flags = 0x41U;
    state.active_index = 1U;
    const auto skips_suppressed = advance_runner_switch(state);
    if (!expect(skips_suppressed.result == RunnerSwitchResult::switched &&
                skips_suppressed.selected_index == 0U &&
                state.active_index == 0U)) {
        return 2;
    }

    state.active_index = 1U;
    state.suppression_override = true;
    const auto override = advance_runner_switch(state);
    if (!expect(override.result == RunnerSwitchResult::switched &&
                override.selected_index == 2U && state.active_index == 2U)) {
        return 3;
    }

    RunnerSwitchState rejected{
        {{0U, 0x01U}, {1U, 0x00U}, {2U, 0x40U}}, 0U, false};
    const auto no_runner = advance_runner_switch(rejected);
    if (!expect(no_runner.result == RunnerSwitchResult::no_eligible_runner &&
                rejected.active_index == 0U)) {
        return 4;
    }

    RunnerSwitchState wrap{{{0U, 0x01U}, {2U, 0x01U}}, 1U, false};
    const auto wrapped = advance_runner_switch(wrap);
    if (!expect(wrapped.result == RunnerSwitchResult::switched &&
                wrapped.previous_index == 1U && wrapped.selected_index == 0U &&
                wrap.active_index == 0U)) {
        return 5;
    }

    RunnerSwitchState alone{{{2U, 0x01U}}, 0U, false};
    if (!expect(advance_runner_switch(alone).result ==
                    RunnerSwitchResult::no_eligible_runner &&
                alone.active_index == 0U)) {
        return 6;
    }

    // ROM 0x42B0/0x44BC's initial field state: slot types 0, 1, and 2,
    // Joshua active in slot zero, and no recruited primary/secondary slot
    // published yet.  0x391A still leaves the active field record's bit 1
    // marker set after it wraps over the inactive slots.
    RunnerSwitchState initial_field{{{0U, 0x05U}, {1U, 0x00U}, {2U, 0x00U}},
                                   0U, false};
    const auto initial_field_result = advance_runner_switch(initial_field);
    if (!expect(initial_field_result.result ==
                    RunnerSwitchResult::no_eligible_runner &&
                initial_field.active_index == 0U &&
                initial_field.records[0].flags == 0x07U &&
                initial_field.records[1].flags == 0x00U &&
                initial_field.records[2].flags == 0x00U)) {
        return 7;
    }

    RunnerSwitchState invalid{{}, 0U, false};
    if (!expect(advance_runner_switch(invalid).result ==
                    RunnerSwitchResult::invalid_state)) {
        return 8;
    }
    return 0;
}

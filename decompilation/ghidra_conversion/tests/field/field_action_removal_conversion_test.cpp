#include "shadowrun_ghidra/field/field_action_removal_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerActionRemovalState state{};
    state.runner_slot = 2U;
    state.selected_runner_slot = 2U;
    state.runner.action_codes[2U] = 0x2CU;
    state.runner.companion_values[2U] = 0x07U;
    state.action_suffix_marker = 1U;
    state.refund = 250U;
    state.funds = 100U;
    int panels = 0;
    int protected_messages = 0;
    int refund_renders = 0;
    int confirmations = 0;
    int clears = 0;
    const RunnerActionRemovalCallbacks callbacks{
        [&] { ++panels; },
        [&] { ++protected_messages; },
        [&] { ++refund_renders; },
        [&] {
            ++confirmations;
            return true;
        },
        [&] { ++clears; },
    };

    if (!expect(remove_runner_action(state, callbacks) ==
                    RunnerActionRemovalResult::removed &&
                panels == 1 && refund_renders == 1 && confirmations == 1 &&
                clears == 1 && state.funds == 350U &&
                state.runner.action_codes[2U] == 0U &&
                state.runner.companion_values[2U] == 0U &&
                state.action_suffix_marker == 0U &&
                state.menu_selection_cleared)) {
        return 1;
    }

    state.runner.action_codes[2U] = 0x29U;
    state.runner.companion_values[2U] = 0x04U;
    if (!expect(remove_runner_action(state, callbacks) ==
                    RunnerActionRemovalResult::protected_action &&
                protected_messages == 1 && state.funds == 350U &&
                state.runner.companion_values[2U] == 0x04U)) {
        return 2;
    }

    state.runner.action_codes[2U] = 0x18U;
    state.runner.companion_values[2U] = 0x02U;
    const RunnerActionRemovalCallbacks cancel_callbacks{
        {}, {}, {}, [] { return false; }, {}};
    if (!expect(remove_runner_action(state, cancel_callbacks) ==
                    RunnerActionRemovalResult::cancelled &&
                state.funds == 350U &&
                state.runner.action_codes[2U] == 0x18U)) {
        return 3;
    }

    state.runner_slot = 7U;
    if (!expect(remove_runner_action(state, callbacks) ==
                    RunnerActionRemovalResult::empty_slot)) {
        return 4;
    }

    state.runner_slot = 2U;
    state.runner.action_codes[2U] = 0U;
    if (!expect(remove_runner_action(state, callbacks) ==
                    RunnerActionRemovalResult::empty_slot)) {
        return 5;
    }

    return 0;
}

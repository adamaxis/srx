#include "shadowrun_ghidra/field/field_action_description_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionRunnerInventory runner{};
    runner.action_codes[2U] = 4U;
    int panel_refreshes = 0;
    int record_renders = 0;
    int text_renders = 0;
    int waits = 0;
    RunnerActionDescriptionSelection rendered{};
    const RunnerActionDescriptionCallbacks callbacks{
        [&] { ++panel_refreshes; },
        [&](const RunnerActionDescriptionSelection& selection) {
            ++record_renders;
            rendered = selection;
        },
        [&] { ++text_renders; },
        [&] { ++waits; },
    };

    if (!expect(show_runner_action_description(runner, 2U, callbacks) ==
                    RunnerActionDescriptionResult::shown &&
                panel_refreshes == 1 && record_renders == 1 &&
                text_renders == 1 && waits == 1 && rendered.runner_slot == 2U &&
                rendered.action_code == 4U &&
                rendered.description_record_index == 3U)) {
        return 1;
    }

    runner.action_codes[2U] = 0U;
    if (!expect(show_runner_action_description(runner, 2U, callbacks) ==
                    RunnerActionDescriptionResult::empty_slot &&
                panel_refreshes == 2 && record_renders == 1 && waits == 1)) {
        return 2;
    }

    runner.action_codes[2U] = 0x0BU;
    if (!expect(show_runner_action_description(runner, 2U, callbacks) ==
                    RunnerActionDescriptionResult::unsupported_action_code &&
                panel_refreshes == 3 && record_renders == 1 &&
                text_renders == 2 && waits == 2)) {
        return 3;
    }

    if (!expect(show_runner_action_description(runner, 8U, callbacks) ==
                    RunnerActionDescriptionResult::empty_slot &&
                panel_refreshes == 4 && record_renders == 1)) {
        return 4;
    }

    return 0;
}

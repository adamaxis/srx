#include "shadowrun_ghidra/field/field_action_description_conversion.hpp"

namespace shadowrun::ghidra_conversion {

RunnerActionDescriptionResult show_runner_action_description(
    const FieldActionRunnerInventory& runner,
    const std::uint8_t runner_slot,
    const RunnerActionDescriptionCallbacks& callbacks) {
    if (callbacks.refresh_detail_panel) {
        callbacks.refresh_detail_panel();
    }

    if (runner_slot >= runner.action_codes.size()) {
        return RunnerActionDescriptionResult::empty_slot;
    }

    const auto action_code = runner.action_codes[runner_slot];
    if (action_code == 0U) {
        return RunnerActionDescriptionResult::empty_slot;
    }
    if (action_code >= 0x0BU) {
        if (callbacks.render_description_text) {
            callbacks.render_description_text();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return RunnerActionDescriptionResult::unsupported_action_code;
    }

    const RunnerActionDescriptionSelection selection{
        runner_slot,
        action_code,
        static_cast<std::uint8_t>(action_code - 1U),
    };
    if (callbacks.render_record) {
        callbacks.render_record(selection);
    }
    if (callbacks.render_description_text) {
        callbacks.render_description_text();
    }
    if (callbacks.wait_for_controller_event) {
        callbacks.wait_for_controller_event();
    }
    return RunnerActionDescriptionResult::shown;
}

} // namespace shadowrun::ghidra_conversion

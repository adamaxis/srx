#pragma once

#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct RunnerActionDescriptionSelection {
    std::uint8_t runner_slot = 0; // FFF0D0 / active record +0x66 slot
    std::uint8_t action_code = 0; // active record +0x66
    std::uint8_t description_record_index = 0; // action code - 1 in 0xD4540
};

enum class RunnerActionDescriptionResult {
    shown,
    empty_slot,
    unsupported_action_code,
};

struct RunnerActionDescriptionCallbacks {
    // 0x57C5E clears/redraws the five-row action detail panel first.
    std::function<void()> refresh_detail_panel;
    // The source renders the selected 0xD4540 action record followed by the
    // fixed action-description text, then waits for a controller event.
    std::function<void(const RunnerActionDescriptionSelection&)> render_record;
    std::function<void()> render_description_text;
    std::function<void()> wait_for_controller_event;
};

// ROM branch 0x578C4 inside 0x56DC4. Open the selected runner-action
// description using the active record's +0x66 slot bytes. The source accepts
// action codes 1..10 and uses code - 1 to choose the 0xD4540 description row.
RunnerActionDescriptionResult show_runner_action_description(
    const FieldActionRunnerInventory& runner,
    std::uint8_t runner_slot,
    const RunnerActionDescriptionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

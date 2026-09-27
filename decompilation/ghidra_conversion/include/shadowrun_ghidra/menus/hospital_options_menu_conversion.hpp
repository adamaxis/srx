#pragma once

#include "shadowrun_ghidra/core/selection_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class HospitalOption : std::uint8_t {
    medical_attention = 0,
    cyberware = 1,
    leave_hospital = 2,
};

struct HospitalOptionsInputFrame {
    bool move_up = false;
    bool move_down = false;
    bool confirm = false;
    bool alternate_confirm = false;
    bool action = false;
    bool cancel = false;
};

enum class HospitalMenuHandlerOutcome {
    redraw_options,
    return_to_mode_exit,
    leave_hospital,
};

struct HospitalOptionsMenuState {
    HospitalOption selection = HospitalOption::medical_attention;
    std::int8_t menu_result = 0; // FFF0DC
    std::uint8_t mode_result = 1; // FFF0B7, hospital-service mode
};

struct HospitalOptionsMenuCallbacks {
    std::function<void()> reset_mode_display_buffers;
    // Rebuilds the Hospital Options screen: metadata/palette/grid/font setup,
    // title/options text, and the initial cursor presentation.
    std::function<void()> rebuild_hospital_options_screen;
    std::function<void(HospitalOption)> render_cursor;

    // These four callbacks preserve the source frame-service order around
    // 0xBD2, 0xBF2, 0x195C, and 0xE660. Sampling returns typed edge flags.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<HospitalOptionsInputFrame()> sample_controller_input;
    std::function<void()> advance_random_grid_effect;

    // The medical-attention detail/purchase routine is a separate source
    // hill (0xCA2E..0xCDE6), so the boundary is named for that behavior.
    std::function<HospitalMenuHandlerOutcome()> run_medical_attention;

    // Source handler 0xCA16: enter field-mode service with FFF0B7 = 1 and
    // return to the mode-exit caller.
    std::function<void()> run_field_mode_service;

    // Source handler 0xCE7C: run the typed leave-hospital confirmation.
    std::function<HospitalMenuHandlerOutcome()> run_leave_hospital_prompt;

    // B/action input in the outer Hospital Options loop calls 0x391A.
    std::function<void()> handle_mode_action_event;
};

enum class HospitalOptionsMenuExit {
    cancelled,
    returned_to_mode_exit,
    left_hospital,
};

struct HospitalOptionsMenuResult {
    HospitalOptionsMenuExit exit = HospitalOptionsMenuExit::cancelled;
    HospitalOption final_selection = HospitalOption::medical_attention;
};

// ROM 0xC928: run the field-mode Hospital Options menu. This is a typed host
// loop over the three source handlers at 0xCA2E, 0xCA16, and 0xCE7C; it does
// not expose the source handler table or a Genesis RAM address.
HospitalOptionsMenuResult run_hospital_options_menu(
    HospitalOptionsMenuState& state,
    const HospitalOptionsMenuCallbacks& callbacks);

struct LeaveHospitalPromptState {
    std::uint8_t selection = 1; // source defaults to No; 0 = Yes, 1 = No
};

struct LeaveHospitalPromptCallbacks {
    std::function<void()> render_prompt;
    std::function<void(std::uint8_t)> render_cursor;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<HospitalOptionsInputFrame()> sample_controller_input;
    std::function<void()> advance_random_grid_effect;
};

enum class LeaveHospitalPromptResult {
    stay_in_hospital,
    leave_hospital,
};

// ROM 0xCE7C: prompt "Do you wish to leave the hospital?" with Yes/No
// selection. The source's accepted-Yes path unwinds both the dispatch helper
// and the Hospital Options routine; the native conversion returns an explicit
// leave_hospital result instead of reproducing that stack manipulation.
LeaveHospitalPromptResult run_leave_hospital_prompt(
    LeaveHospitalPromptState& state,
    const LeaveHospitalPromptCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

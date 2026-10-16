#pragma once

// Matrix node-function action and status conversion. The source entry was
// previously filed under the field subsystem despite its Matrix tables.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// High-byte event bits sampled from FFDD22 by the Matrix node-function action
// loop at ROM 0x54662. The source tests bit 14 first, then aliases bits 12
// and 13 back to the same action preamble before testing the Matrix exit bit.
enum class MatrixNodeActionInput : std::uint16_t {
    move_previous = 0x0100U,
    move_next = 0x0200U,
    confirm = 0x4000U,
    alternate_confirm = 0x1000U,
    matrix_node_action = 0x2000U,
    leave_to_matrix = 0x8000U,
};

enum class MatrixNodeFunction : std::uint8_t {
    system_access_node = 0,
    central_processing_unit = 1,
    k4_sub_processor_unit = 2,
    input_output_port = 3,
    datastore = 4,
    slave_module = 5,
    invalid = 0xFFU,
};

// ROM 0xE30B6 contains the five alert-band labels used by the Matrix HUD.
enum class MatrixAlertBand : std::uint8_t {
    unknown = 0,
    blue = 1,
    green = 2,
    orange = 3,
    red = 4,
    invalid = 0xFFU,
};

// ROM 0xE4060 contains the 49 sixteen-byte Matrix file/node classifications
// addressed by compact record +0x01. Keep the source's actual labels rather
// than collapsing this field into an unexplained integer.
enum class MatrixFileCategory : std::uint8_t {
    outdated_files = 0,
    system_files,
    management_files,
    project_files,
    security_files,
    dead_projects,
    competition,
    marketing_data,
    simsense_files,
    financial_data,
    confidential_data,
    legal_files,
    prisoner_files,
    case_files,
    unlisted,
    seattle_general,
    renraku,
    fuchi,
    mitsuhama,
    hollywood_correctional_facility,
    aztechnology,
    ucas_federal_government,
    gates_undersea,
    ares,
    lone_star,
    city_hall,
    club_penumbra,
    building_maintenance,
    security,
    executive_area,
    research,
    marketing,
    retail_control,
    data_routing,
    office_management,
    shipping,
    simsense_records,
    terminal,
    matrix_jack,
    security_control,
    cameras,
    maglocks,
    alert_control,
    automated_equipment,
    elevator_control,
    underwater_equipment,
    hvac_systems,
    satellite_feed,
    itos_system,
    invalid = 0xFFU,
};

struct MatrixNodeActionRecord {
    // Compact record +0x00 selects the Matrix node-function action family.
    MatrixNodeFunction node_function = MatrixNodeFunction::invalid;
    // Compact record +0x05 selects the alert-band label and is decremented
    // by the source action-delay gate before the thresholded roll.
    MatrixAlertBand alert_band = MatrixAlertBand::unknown;
    // Compact record +0x01 selects the file/node classification displayed in
    // the node-action status panel.
    MatrixFileCategory file_category = MatrixFileCategory::invalid;
    // Compact record +0x06 is displayed in the status panel and passed as the
    // threshold difficulty to 0x0DBA after the action-delay gate succeeds.
    std::uint8_t action_difficulty = 0;
    std::uint8_t flags = 0;
    bool clears_matrix_node_auxiliary_state = false;
    // The Matrix node-function table at 0x1664D8 +0x04 bounds the visible
    // action cursor.
    std::int8_t action_cursor_limit = 0;
};

struct MatrixNodeActionState {
    std::uint8_t matrix_node_flags = 0; // FFFB30[selected Matrix scene]
    std::int8_t matrix_node_result = -1; // FFF0B7
    std::int8_t matrix_hud_abort = 0; // FFF0B1; entry leaves it untouched
    std::uint8_t matrix_alert_event_counter = 0; // FFF0A8
    std::int8_t action_cursor = 0; // FFF0DC
    std::int8_t matrix_auxiliary_marker = -1; // FFF0A6
    std::int8_t matrix_node_work_marker = 0; // FFF0D4
    bool matrix_navigation_requested = false;
};

struct MatrixNodeActionGateInputs {
    // Result of 0x55F44 with D1 cleared by the 0x54662 caller.
    std::uint16_t calculated_runner_action_delay = 0;
    // Active live-runner record +0x88, loaded into D7 by 0x54662.
    std::uint8_t active_runner_action_delay = 0;
};

enum class MatrixNodeTextRole : std::uint8_t {
    node_function_title,
    action_panel_header,
    alert_band_label,
    file_category_label,
    system_alert_notice,
    action_failure_message,
};

struct MatrixNodeTextRequest {
    MatrixNodeTextRole role = MatrixNodeTextRole::node_function_title;
    MatrixNodeFunction node_function = MatrixNodeFunction::invalid;
    MatrixAlertBand alert_band = MatrixAlertBand::invalid;
    MatrixFileCategory file_category = MatrixFileCategory::invalid;
};

struct MatrixNodeNumberRequest {
    // The only numeric transfer in 0x54A28 is compact record +0x06.
    std::uint8_t action_difficulty = 0;
};

struct MatrixNodeStatusSummary {
    MatrixAlertBand alert_band = MatrixAlertBand::unknown;
    MatrixFileCategory file_category = MatrixFileCategory::invalid;
    std::uint8_t action_difficulty = 0;
};

struct MatrixNodeActionLoopCallbacks {
    std::function<void(std::uint16_t)> set_display_control;
    std::function<void()> reset_display_buffers;
    std::function<void()> refresh_matrix_node_resources;
    std::function<void()> clear_matrix_node_auxiliary_state;
    std::function<void()> upload_matrix_node_resource_block;
    std::function<void()> write_matrix_node_grid;
    std::function<void()> select_matrix_alert_display;
    std::function<void()> refresh_matrix_node_metadata;

    // 0x54A8E: returns true when the Matrix alert event fired and the caller
    // should redraw the outer Matrix-node loop.
    std::function<bool(MatrixNodeActionState&)> advance_random_matrix_alert;
    // Presentation is expressed in Matrix game terms. The host chooses how
    // to draw each request; the conversion supplies the source-derived role
    // and record identity instead of anonymous "render text" calls.
    std::function<void(const MatrixNodeTextRequest&)> render_matrix_text;
    std::function<void(const MatrixNodeNumberRequest&)>
        render_matrix_number;
    // 0xC376 receives D2=direction and publishes FFF0DC.  The conversion
    // updates the typed cursor first, then lets the host draw the new cursor.
    std::function<void(std::int8_t direction, std::int8_t cursor)>
        move_matrix_action_cursor;

    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<std::uint16_t()> sample_controller_input;
    // 0x54662 calls 0x55F44, then reads the active runner's +0x88 byte. +0x88 is the Computer skill (the 0x55F44
    // pool base), not an action delay (docs/24 R7); field names are kept for API stability.
    // The callback may use the typed 0x55F44 conversion and live runner model.
    std::function<MatrixNodeActionGateInputs()>
        calculate_matrix_node_action_gate_inputs;
    // 0x0DBA receives the post-gate trial count and compact +0x06 difficulty.
    std::function<std::int8_t(std::uint16_t trial_count,
                              std::uint8_t difficulty)>
        count_matrix_node_action_rolls;
    std::function<void()> redraw_matrix_node_action_grid;
    std::function<void()> wait_for_controller_event;

    // The source indexes a table of handlers by record kind and cursor.  The
    // targets have not been independently identified, so the boundary stays
    // typed and visible rather than becoming a fake generic dispatch.
    std::function<void(MatrixNodeFunction node_function, std::int8_t cursor)>
        dispatch_matrix_node_action;

    std::function<void()> render_matrix_footer;
    std::function<void()> upload_matrix_resource_pair;
    // The bool reports whether the Matrix boundary published the source's
    // negative FFF0B1 abort marker.  The ROM still resets five timers and
    // performs the next outer redraw before observing that marker.
    std::function<bool()> enter_matrix_navigation;
    std::function<void(std::size_t slot)> reset_matrix_program_slot_timer;
};

// ROM 0x532A8: fill the 35-entry Matrix-node auxiliary buffer with the source
// sentinel. It was previously mislabeled as an audio-menu operation.
void fill_matrix_node_auxiliary_sentinel(std::vector<std::int32_t>& values);

// ROM 0x549E4: preserve the one-call presentation boundary used by the
// Matrix-node loop after an action or alert event.
void redraw_matrix_node_action_grid_service(
    const std::function<void()>& write_matrix_node_grid);

struct MatrixAlertDisplayState {
    std::uint8_t status_code = 0;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x549F6: choose the source alert status tile/code for the Matrix HUD
// and publish the source display/resource dirty bit.
void select_matrix_alert_display(MatrixAlertDisplayState& state,
                                 std::int8_t alert_state);

// ROM 0x54A28: render the Matrix node's alert-band label, file/node
// classification, and action-difficulty number in source order.
void render_matrix_node_status_summary(
    const MatrixNodeStatusSummary& summary,
    const std::function<void(const MatrixNodeTextRequest&)>& render_matrix_text,
    const std::function<void(const MatrixNodeNumberRequest&)>& render_matrix_number);

struct MatrixRandomAlertCallbacks {
    std::function<std::int8_t()> sample_random_bound;
    std::function<void()> transition_hud_mode;
    std::function<void()> select_matrix_alert_display;
    std::function<void(const MatrixNodeTextRequest&)> render_matrix_text;
    std::function<void()> wait_for_vblank_frame;
    std::function<void()> wait_for_controller_event;
    std::function<void()> redraw_matrix_node_action_grid;
};

// ROM 0x54A8E: consume the Matrix alert-event counter and perform the
// source's optional HUD/wait/repeated-grid refresh when the event fires.
bool advance_random_matrix_alert(MatrixNodeActionState& state,
                                  const MatrixRandomAlertCallbacks& callbacks);

// ROM 0x54662: Matrix node-function action loop reached from the world
// transition at 0x1ADE6. The host owns presentation and input callbacks; this
// function preserves the source state machine and call order without VDP or
// ROM access.
bool run_matrix_node_action_loop(
    MatrixNodeActionState& state,
    const MatrixNodeActionRecord& record,
    const MatrixNodeActionLoopCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

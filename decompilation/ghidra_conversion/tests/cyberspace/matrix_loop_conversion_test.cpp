#include "shadowrun_ghidra/cyberspace/matrix_loop_conversion.hpp"

#include <cstdint>
#include <array>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(matrix_program_index(0, 0) == 0 &&
                matrix_program_index(1, 2) == 6 &&
                matrix_program_index(3, 3) == 15)) {
        return 1;
    }
    // ROM ADD.B changes only D7's low byte after MULU.W.  These cases keep
    // the upper product bits instead of silently narrowing the result.
    if (!expect(matrix_program_index(0x40U, 0x03U) == 0x103U &&
                matrix_program_index(0xFFU, 0xFFU) == 0x3FBU)) {
        return 8;
    }
    const std::array<std::uint8_t, 12> availability{1, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0};
    auto available = availability;
    available[4] = 1;
    if (!expect(matrix_program_is_available(0, availability) &&
                !matrix_program_is_available(1, availability) &&
                matrix_program_is_available(12, availability) &&
                matrix_program_is_available(15, availability) &&
                matrix_program_uses_runner_details(0) &&
                matrix_program_uses_runner_details(11) &&
                !matrix_program_uses_runner_details(12))) {
        return 2;
    }

    std::vector<std::string> panel_events;
    if (!expect(!render_matrix_panel(
                    1, availability,
                    {[&] { panel_events.emplace_back("header"); },
                     [&] { panel_events.emplace_back("unavailable"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("icon"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("detail"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("detail-panel"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                      }) &&
                panel_events == std::vector<std::string>{"header", "unavailable"})) {
        return 3;
    }
    panel_events.clear();
    if (!expect(render_matrix_panel(
                    4, available,
                    {[&] { panel_events.emplace_back("header"); },
                     [&] { panel_events.emplace_back("unavailable"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("icon"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("detail"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("detail-panel"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                      }) &&
                panel_events == std::vector<std::string>{"header", "icon", "detail",
                                                          "detail-panel", "line", "line"})) {
        return 4;
    }
    panel_events.clear();
    if (!expect(render_matrix_panel(
                    12, availability,
                    {[&] { panel_events.emplace_back("header"); },
                     [&] { panel_events.emplace_back("unavailable"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("icon"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("detail"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("detail-panel"); },
                      [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                      }) &&
                panel_events == std::vector<std::string>{"header", "icon", "detail",
                                                          "line", "line"})) {
        return 5;
    }

    MatrixRunnerDetailInput no_run{};
    no_run.program_index = 2U;
    std::vector<std::string> detail_events;
    const auto no_run_plan = render_matrix_runner_details(
        no_run,
        {[&] { detail_events.emplace_back("header"); },
         [&] { detail_events.emplace_back("no-run"); },
         [&] { detail_events.emplace_back("special"); },
         [&](const std::uint8_t) { detail_events.emplace_back("primary"); },
         [&](const std::uint8_t, const std::int16_t) {
             detail_events.emplace_back("secondary");
         },
         [&](const std::uint8_t) { detail_events.emplace_back("final"); },
         [&] { return std::optional<std::uint16_t>{1U}; }});
    if (!expect(no_run_plan.branch == MatrixRunnerDetailBranch::no_active_run &&
                detail_events == std::vector<std::string>{"header", "header",
                                                           "no-run"})) {
        return 6;
    }

    MatrixRunnerDetailInput direct{};
    direct.program_index = 4U;
    direct.matrix_run_active = true;
    direct.program_status = 14U;
    direct.delay_input.active_record_attribute_a = 60U;
    direct.delay_input.active_record_attribute_b = 60U;
    direct.delay_input.profile_floor = 0U;
    detail_events.clear();
    const auto direct_plan = render_matrix_runner_details(
        direct,
        {[&] { detail_events.emplace_back("header"); }, {},
         [&] { detail_events.emplace_back("special"); },
         [&](const std::uint8_t count) {
             detail_events.emplace_back("primary:" + std::to_string(count));
         },
         {},
         [&](const std::uint8_t count) {
             detail_events.emplace_back("final:" + std::to_string(count));
         },
         {}});
    if (!expect(direct_plan.branch == MatrixRunnerDetailBranch::direct_status_program &&
                direct_plan.primary_repeat_count == 13U &&
                direct_plan.final_repeat_count == 1U &&
                detail_events == std::vector<std::string>{"header", "header",
                                                           "primary:13", "final:1"})) {
        return 7;
    }

    MatrixRunnerDetailInput special = direct;
    special.program_index = 6U;
    special.program_status = 4U;
    detail_events.clear();
    const auto special_plan = render_matrix_runner_details(
        special,
        {[&] { detail_events.emplace_back("header"); }, {},
         [&] { detail_events.emplace_back("special"); }, {}, {},
         [&](const std::uint8_t count) {
             detail_events.emplace_back("final:" + std::to_string(count));
         }, {}});
    if (!expect(special_plan.branch == MatrixRunnerDetailBranch::special_program_group &&
                detail_events == std::vector<std::string>{"header", "header",
                                                           "special", "final:1"})) {
        return 8;
    }

    MatrixRunnerDetailInput scaled{};
    scaled.program_index = 2U;
    scaled.matrix_run_active = true;
    scaled.program_status = 22U;
    scaled.current_record_value = 4U;
    scaled.delay_input.active_record_attribute_a = 63U;
    scaled.delay_input.active_record_attribute_b = 63U;
    scaled.delay_input.mode_result = 1;
    scaled.delay_input.profile_floor = 0U;
    detail_events.clear();
    const auto scaled_plan = render_matrix_runner_details(
        scaled,
        {[&] { detail_events.emplace_back("header"); }, {}, {},
         [&](const std::uint8_t count) {
             detail_events.emplace_back("primary:" + std::to_string(count));
         },
         [&](const std::uint8_t count, const std::int16_t delta) {
             detail_events.emplace_back("secondary:" + std::to_string(count) +
                                        ":" + std::to_string(delta));
         },
         [&](const std::uint8_t count) {
             detail_events.emplace_back("final:" + std::to_string(count));
         },
         [&] { return std::optional<std::uint16_t>{2U}; }});
    if (!expect(scaled_plan.branch == MatrixRunnerDetailBranch::delay_scaled_program &&
                scaled_plan.primary_value == 43U &&
                scaled_plan.primary_repeat_count == 13U &&
                scaled_plan.eligibility_available &&
                scaled_plan.eligibility_positive &&
                scaled_plan.secondary_repeat_count == 3U &&
                scaled_plan.secondary_destination_delta == 41 &&
                scaled_plan.final_repeat_count == 1U &&
                detail_events == std::vector<std::string>{"header", "header",
                                                           "primary:13",
                                                           "secondary:3:41",
                                                           "final:1"})) {
        return 9;
    }

    const std::array<std::int8_t, 5> loaded_slots{2, -1, 7, -1, -1};
    const auto loaded = select_matrix_program_slot(7, loaded_slots);
    const auto free_slot = select_matrix_program_slot(5, loaded_slots);
    const auto full = select_matrix_program_slot(
        5, std::array<std::int8_t, 5>{0, 1, 2, 3, 4});
    if (!expect(loaded.slot == 2 && loaded.slot_available &&
                loaded.program_already_loaded && free_slot.slot == 1 &&
                free_slot.slot_available && !free_slot.program_already_loaded &&
                !full.slot_available)) {
        return 10;
    }

    MatrixCursorState cursor{};
    cursor.row = 3;
    update_matrix_cursor(cursor, move_up);
    if (!expect(cursor.row == 2 && cursor.column == 0 &&
                cursor.row_marker == 0xE4 && cursor.column_marker == 0x8C &&
                cursor.redraw_requested)) {
        return 7;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, move_left);
    if (!expect(cursor.column == 3 && cursor.column_marker == 0xEC &&
                cursor.redraw_requested)) {
        return 8;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, move_down | move_right);
    if (!expect(cursor.row == 3 && cursor.column == 3 &&
                cursor.row_marker == 0x10C && cursor.column_marker == 0xEC &&
                cursor.redraw_requested)) {
        return 9;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, set_overlay_marker_one);
    if (!expect(cursor.row == 3 && cursor.column == 3 &&
                !cursor.redraw_requested)) {
        return 10;
    }

    MatrixActionState action{};
    apply_matrix_action_input(action, set_overlay_marker_one);
    if (!expect(action.overlay_marker == 1U && action.action_requested &&
                !action.exit_requested)) {
        return 11;
    }
    action = {};
    apply_matrix_action_input(action, set_overlay_marker_special |
                                         set_overlay_marker_zero |
                                         leave_matrix);
    if (!expect(action.overlay_marker == 0xFFU && action.action_requested &&
                !action.exit_requested)) {
        return 12;
    }
    action = {};
    apply_matrix_action_input(action, leave_matrix);
    if (!expect(!action.action_requested && action.exit_requested)) {
        return 13;
    }
    if (!expect(choose_matrix_action_route(0, 0, false) ==
                    MatrixActionRoute::program_slot_menu &&
                choose_matrix_action_route(2, 3, false) ==
                    MatrixActionRoute::program_slot_menu &&
                choose_matrix_action_route(3, 2, false) ==
                    MatrixActionRoute::no_action &&
                choose_matrix_action_route(3, 2, true) ==
                    MatrixActionRoute::column_action_dispatch &&
                choose_matrix_action_route(3, 3, false) ==
                    MatrixActionRoute::no_action &&
                choose_matrix_action_route(3, 3, true) ==
                    MatrixActionRoute::disconnect_confirmation)) {
        return 14;
    }

    MatrixLoopState state{};
    state.overlay_marker = 7;
    state.field_state = 0x12345678U;
    std::vector<std::string> events;
    initialize_matrix_loop(
        state,
        {[&] { events.emplace_back("header"); },
         [&] { events.emplace_back("fixed"); },
         [&](const std::uint16_t command) {
             events.emplace_back("display:" + std::to_string(command));
         },
         [&] { events.emplace_back("plane-a"); },
         [&] { events.emplace_back("upload-reset"); },
         [&](const std::uint32_t source, const std::uint32_t destination) {
             events.emplace_back("stream:" + std::to_string(source) + ":" +
                                 std::to_string(destination));
         },
         [&] { events.emplace_back("metadata"); },
         [&] { events.emplace_back("hud-sprites"); },
         [&] { events.emplace_back("field-hud"); },
         [&] { events.emplace_back("title-sprites"); },
         [&] { events.emplace_back("panel"); },
         [&] { events.emplace_back("indicators"); },
         [&] { events.emplace_back("two-grids"); },
         [&] { events.emplace_back("matrix-panel"); }});

    // Keep the actual numeric command/source evidence explicit without using
    // assertions or a generated baseline.
    if (!expect(state.saved_overlay_marker == 7 &&
                state.saved_field_state == 0x12345678U &&
                state.overlay_marker == 0 && state.field_state == 0 &&
                state.display_dirty && state.cursor.column == 0 &&
                state.cursor.row == 0 && state.cursor.row_marker == 0x94 &&
                state.cursor.column_marker == 0x8C &&
                !state.cursor.redraw_requested &&
                events.size() == 15U && events[0] == "header" &&
                events[1] == "fixed" && events[2] == "display:33076" &&
                events[4] == "upload-reset" && events[5] == "stream:889718:33120" &&
                events[9] == "title-sprites" && events[10] == "panel" &&
                events[12] == "two-grids" && events[13] == "display:33140" &&
                events[14] == "matrix-panel")) {
        return 15;
    }

    return 0;
}

#include "shadowrun_ghidra/matrix_loop_conversion.hpp"

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
                     [&](const std::uint8_t) { panel_events.emplace_back("prepare"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("stats"); }}) &&
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
                     [&](const std::uint8_t) { panel_events.emplace_back("prepare"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("stats"); }}) &&
                panel_events == std::vector<std::string>{"header", "icon", "detail",
                                                          "prepare", "line", "line",
                                                          "stats"})) {
        return 4;
    }
    panel_events.clear();
    if (!expect(render_matrix_panel(
                    12, availability,
                    {[&] { panel_events.emplace_back("header"); },
                     [&] { panel_events.emplace_back("unavailable"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("icon"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("detail"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("prepare"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("line"); },
                     [&](const std::uint8_t) { panel_events.emplace_back("stats"); }}) &&
                panel_events == std::vector<std::string>{"header", "icon", "detail",
                                                          "line", "line"})) {
        return 5;
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
        return 6;
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
                events.size() == 16U && events[0] == "header" &&
                events[1] == "fixed" && events[2] == "display:33076" &&
                events[4] == "upload-reset" && events[5] == "stream:889718:33120" &&
                events[9] == "title-sprites" && events[10] == "display:33140" &&
                events[13] == "two-grids" && events[14] == "display:33140" &&
                events[15] == "matrix-panel")) {
        return 15;
    }

    return 0;
}

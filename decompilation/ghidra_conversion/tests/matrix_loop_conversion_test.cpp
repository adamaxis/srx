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
    if (!expect(matrix_program_is_available(0, availability) &&
                !matrix_program_is_available(1, availability) &&
                matrix_program_is_available(12, availability) &&
                matrix_program_is_available(15, availability) &&
                matrix_program_uses_runner_details(0) &&
                matrix_program_uses_runner_details(11) &&
                !matrix_program_uses_runner_details(12))) {
        return 2;
    }

    MatrixCursorState cursor{};
    cursor.row = 3;
    update_matrix_cursor(cursor, move_up);
    if (!expect(cursor.row == 2 && cursor.column == 0 &&
                cursor.row_marker == 0xE4 && cursor.column_marker == 0x8C &&
                cursor.redraw_requested)) {
        return 3;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, move_left);
    if (!expect(cursor.column == 3 && cursor.column_marker == 0xEC &&
                cursor.redraw_requested)) {
        return 4;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, move_down | move_right);
    if (!expect(cursor.row == 3 && cursor.column == 3 &&
                cursor.row_marker == 0x10C && cursor.column_marker == 0xEC &&
                cursor.redraw_requested)) {
        return 5;
    }
    cursor.redraw_requested = false;
    update_matrix_cursor(cursor, confirm);
    if (!expect(cursor.row == 3 && cursor.column == 3 &&
                !cursor.redraw_requested)) {
        return 6;
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
        return 7;
    }

    return 0;
}

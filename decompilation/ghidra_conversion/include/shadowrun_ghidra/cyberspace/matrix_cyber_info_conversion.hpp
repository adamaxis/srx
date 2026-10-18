#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class CyberInfoPage {
    deck_stats,
    run_info,
};

struct CyberInfoSelectionState {
    std::uint8_t page = 0;
    bool redraw_requested = false;
};

// ROM 0x18AA6's two-entry CyberInfo selector uses the shared C376 cursor.
// Page zero is Deck Stats and page one is Run Info.
void update_cyber_info_selection(CyberInfoSelectionState& state,
                                 std::int8_t step);
CyberInfoPage selected_cyber_info_page(
    const CyberInfoSelectionState& state);

enum class MatrixRunAvailability {
    not_on_run,
    active,
    complete,
};

enum class MatrixRunNode {
    cpu,
    datastore,
};

enum class MatrixRunAction {
    delete_file,
    upload_file,
    crash_system,
    download_file,
};

struct MatrixRunPresentation {
    MatrixRunAvailability availability = MatrixRunAvailability::not_on_run;
    MatrixRunNode node = MatrixRunNode::datastore;
    MatrixRunAction action = MatrixRunAction::delete_file;
};

// ROM 0x18C42: FFF0E3 selects the Run Info branch. A negative value means an
// active run; zero means no run; a positive value means the run is complete.
MatrixRunAvailability classify_matrix_run_availability(
    std::int8_t run_state);

// ROM 0x18C78..0x18CE4: select the displayed node and action strings from
// FFFBF2/FFFBF6. The source table resolves to Delete, Upload, Crash, and
// Download File in that order.
MatrixRunPresentation select_matrix_run_presentation(
    std::int8_t run_state,
    std::int8_t node_state,
    std::int8_t action_state);

} // namespace shadowrun::ghidra_conversion

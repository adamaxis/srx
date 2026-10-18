#include "shadowrun_ghidra/cyberspace/matrix_cyber_info_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    CyberInfoSelectionState selection{};
    if (!expect(selected_cyber_info_page(selection) ==
                CyberInfoPage::deck_stats)) {
        return 1;
    }
    update_cyber_info_selection(selection, 1);
    if (!expect(selection.page == 1U && selection.redraw_requested &&
                selected_cyber_info_page(selection) == CyberInfoPage::run_info)) {
        return 2;
    }
    selection.redraw_requested = false;
    update_cyber_info_selection(selection, 1);
    if (!expect(selection.page == 0U && selection.redraw_requested)) {
        return 3;
    }

    if (!expect(classify_matrix_run_availability(-1) ==
                    MatrixRunAvailability::active &&
                classify_matrix_run_availability(0) ==
                    MatrixRunAvailability::not_on_run &&
                classify_matrix_run_availability(1) ==
                    MatrixRunAvailability::complete)) {
        return 4;
    }

    const auto no_run = select_matrix_run_presentation(0, 0, 0);
    const auto active = select_matrix_run_presentation(-1, 0, 0);
    const auto complete = select_matrix_run_presentation(1, 0, 0);
    if (!expect(no_run.availability == MatrixRunAvailability::not_on_run &&
                active.availability == MatrixRunAvailability::active &&
                complete.availability == MatrixRunAvailability::complete)) {
        return 5;
    }

    const auto cpu = select_matrix_run_presentation(-1, -1, 1);
    const auto upload = select_matrix_run_presentation(-1, 0, -1);
    const auto erase = select_matrix_run_presentation(-1, 0, 0);
    const auto download = select_matrix_run_presentation(-1, 0, 1);
    if (!expect(cpu.node == MatrixRunNode::cpu &&
                cpu.action == MatrixRunAction::crash_system &&
                upload.node == MatrixRunNode::datastore &&
                upload.action == MatrixRunAction::upload_file &&
                erase.action == MatrixRunAction::delete_file &&
                download.action == MatrixRunAction::download_file)) {
        return 6;
    }
    return 0;
}

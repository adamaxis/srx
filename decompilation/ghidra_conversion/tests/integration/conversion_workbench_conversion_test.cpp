#include "shadowrun_ghidra/integration/conversion_workbench.hpp"

#include <cassert>
#include <string>
#include <vector>

int main() {
    const auto report =
        shadowrun::ghidra_conversion::run_conversion_workbench();

    assert(report.passed);
    assert(report.decoded_coordinate_count == 2U);
    assert(report.first_decoded_coordinate == 0x0102);
    assert(report.last_decoded_coordinate == 0x0101);
    assert(report.field_row_table_offset == 0x0064U);
    assert(report.field_tile_word_offset == 0x000EU);
    assert(report.matrix_loaded_memory == 7U);
    assert(report.matrix_installed_program_memory == 8U);
    assert(report.matrix_available_memory == 25);
    assert(report.runner_kind == 7U);
    assert(report.runner_posture == 2U);
    assert(report.runner_ammo_rounds == 18U);
    assert(report.runner_ammo_clips == 3U);
    assert(report.audio_phase == 0xFFU);
    assert(report.audio_transaction_completed);
    assert((report.audio_events ==
            std::vector<std::string>{"begin_bus_access",
                                     "flush_audio_command",
                                     "append_audio_command",
                                     "end_bus_access"}));

    return 0;
}

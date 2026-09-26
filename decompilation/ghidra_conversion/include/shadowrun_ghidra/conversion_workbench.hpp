#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion {

// A deterministic, host-side probe of the converted source contracts. This
// report is deliberately not game state: it is the observable output of the
// conversion layer when several independently converted boundaries are
// exercised together.
struct ConversionWorkbenchReport {
    std::size_t decoded_coordinate_count = 0;
    std::int16_t first_decoded_coordinate = 0;
    std::int16_t last_decoded_coordinate = 0;

    std::uint16_t field_row_table_offset = 0;
    std::uint16_t field_tile_word_offset = 0;
    std::int32_t field_destination_offset = 0;

    std::uint32_t matrix_loaded_memory = 0;
    std::uint32_t matrix_installed_program_memory = 0;
    std::int32_t matrix_available_memory = 0;

    std::uint16_t runner_kind = 0;
    std::uint8_t runner_posture = 0;
    std::uint8_t runner_ammo_rounds = 0;
    std::uint8_t runner_ammo_clips = 0;

    std::uint8_t audio_phase = 0;
    bool audio_transaction_completed = false;
    std::vector<std::string> audio_events;

    bool passed = false;
};

// Runs the conversion-side integration probe. It uses typed host values and
// conversion callbacks only; it does not load the ROM, execute a Genesis
// processor, present a Qt window, or call the native port.
ConversionWorkbenchReport run_conversion_workbench();

// Formats the report for the standalone conversion runner and for inspection
// in CI logs. The format is stable so it can be diffed between conversion
// passes without treating it as a runtime save format.
std::string format_conversion_workbench_report(
    const ConversionWorkbenchReport& report);

} // namespace shadowrun::ghidra_conversion

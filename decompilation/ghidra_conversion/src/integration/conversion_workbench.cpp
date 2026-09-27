#include "shadowrun_ghidra/integration/conversion_workbench.hpp"

#include "shadowrun_ghidra/audio/audio_service_conversion.hpp"
#include "shadowrun_ghidra/core/coordinate_data_conversion.hpp"
#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"
#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/session/runner_roster_conversion.hpp"

#include <iomanip>
#include <sstream>

namespace shadowrun::ghidra_conversion {
namespace {

std::string format_hex(const std::uint32_t value, const int width) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(width)
           << std::setfill('0') << value;
    return stream.str();
}

void append_event(std::vector<std::string>& events, const char* event) {
    events.emplace_back(event);
}

} // namespace

ConversionWorkbenchReport run_conversion_workbench() {
    ConversionWorkbenchReport report;

    // ROM 0x167C's converted source-coordinate stream: a base word, one
    // descending two-value run, and the zero terminator.
    const auto decoded = decode_source_coordinate_stream(
        {0x01U, 0x00U, 0x01U, 0x02U, 0x00U});
    if (decoded && !decoded->empty()) {
        report.decoded_coordinate_count = decoded->size();
        report.first_decoded_coordinate = decoded->front();
        report.last_decoded_coordinate = decoded->back();
    }

    const auto tile_layout = calculate_field_tile_source_layout(
        0x0058U, 0x0068U, 0x0020U);
    report.field_row_table_offset = tile_layout.row_table_byte_offset;
    report.field_tile_word_offset = tile_layout.tile_word_byte_offset;
    report.field_destination_offset = compose_field_tile_destination_offset(
        -3, 0x01F8U);

    MatrixStorageUsage storage;
    storage.total_memory = 40U;
    storage.loaded_slot_usage = {2U, 4U, 1U, 0U, 0U};
    storage.availability_flags[1] = 1U;
    storage.availability_flags[2] = 1U;
    const auto capacity = calculate_matrix_storage_capacity(storage);
    report.matrix_loaded_memory = capacity.loaded_memory;
    report.matrix_installed_program_memory = capacity.installed_program_memory;
    report.matrix_available_memory = capacity.available_memory;

    CompactRunnerRecord compact;
    compact.bytes[0x00U] = 0x00U;
    compact.bytes[0x01U] = 0x07U;
    compact.bytes[0x04U] = 0x09U;
    compact.bytes[0x05U] = 0x02U;
    compact.bytes[0x06U] = 18U;
    compact.bytes[0x07U] = 3U;
    compact.bytes[0x41U] = 1U;
    compact.bytes[0x42U] = 4U;
    compact.bytes[0x43U] = 11U;
    compact.bytes[0x46U] = 0x00U;
    compact.bytes[0x47U] = 0x03U;
    compact.bytes[0x49U] = 2U;

    LiveRunnerRecord live;
    project_compact_runner_record(compact, live);
    const auto runner = read_runner_record_fields(live);
    report.runner_kind = runner.kind;
    report.runner_posture = runner.posture;
    report.runner_ammo_rounds = runner.ammo_rounds;
    report.runner_ammo_clips = runner.ammo_clips;

    AudioServiceTickState audio;
    audio.phase = 0xFEU;
    const AudioServiceCallbacks audio_callbacks{
        [&report] { append_event(report.audio_events, "begin_bus_access"); },
        [&report] { append_event(report.audio_events, "flush_audio_command"); },
        [&report] { append_event(report.audio_events, "append_audio_command"); },
        [&report] { append_event(report.audio_events, "end_bus_access"); },
    };
    report.audio_transaction_completed =
        advance_audio_service_tick(audio, true, audio_callbacks);
    report.audio_phase = audio.phase;

    // These checks are intentionally small and deterministic. The detailed
    // source evidence remains in each conversion function and its focused
    // test; this aggregate only proves that the independently built modules
    // can be composed without introducing a runtime/emulator dependency.
    report.passed = decoded && decoded->size() == 2U &&
                    report.first_decoded_coordinate == 0x0102 &&
                    report.last_decoded_coordinate == 0x0101 &&
                    report.field_row_table_offset == 0x0064U &&
                    report.field_tile_word_offset == 0x000EU &&
                    report.matrix_loaded_memory == 7U &&
                    report.matrix_installed_program_memory == 8U &&
                    report.matrix_available_memory == 25 &&
                    report.runner_kind == 7U && report.runner_posture == 2U &&
                    report.runner_ammo_rounds == 18U &&
                    report.runner_ammo_clips == 3U && report.audio_phase == 0xFFU &&
                    report.audio_transaction_completed &&
                    report.audio_events.size() == 4U;
    return report;
}

std::string format_conversion_workbench_report(
    const ConversionWorkbenchReport& report) {
    std::ostringstream stream;
    stream << "project=shadowrun_ghidra_conversion\n"
           << "boundary=offline-source-conversion\n"
           << "native_port_linked=false\n"
           << "emulator_linked=false\n"
           << "coordinates.count=" << report.decoded_coordinate_count << '\n'
           << "coordinates.first="
           << format_hex(static_cast<std::uint16_t>(
                             report.first_decoded_coordinate),
                         4)
           << '\n'
           << "coordinates.last="
           << format_hex(static_cast<std::uint16_t>(
                             report.last_decoded_coordinate),
                         4)
           << '\n'
           << "field.row_table_offset="
           << format_hex(report.field_row_table_offset, 4) << '\n'
           << "field.tile_word_offset="
           << format_hex(report.field_tile_word_offset, 4) << '\n'
           << "field.destination_offset="
           << format_hex(static_cast<std::uint32_t>(
                             report.field_destination_offset),
                         8)
           << '\n'
           << "matrix.loaded_memory=" << report.matrix_loaded_memory << '\n'
           << "matrix.installed_program_memory="
           << report.matrix_installed_program_memory << '\n'
           << "matrix.available_memory=" << report.matrix_available_memory
           << '\n'
           << "runner.kind=" << report.runner_kind << '\n'
           << "runner.posture=" << static_cast<unsigned>(report.runner_posture)
           << '\n'
           << "runner.ammo_rounds="
           << static_cast<unsigned>(report.runner_ammo_rounds) << '\n'
           << "runner.ammo_clips="
           << static_cast<unsigned>(report.runner_ammo_clips) << '\n'
           << "audio.phase=" << static_cast<unsigned>(report.audio_phase)
           << '\n'
           << "audio.transaction_completed="
           << (report.audio_transaction_completed ? "true" : "false") << '\n'
           << "audio.events=";
    for (std::size_t index = 0; index < report.audio_events.size(); ++index) {
        if (index != 0U) {
            stream << ',';
        }
        stream << report.audio_events[index];
    }
    stream << '\n' << "result=" << (report.passed ? "pass" : "fail")
           << '\n';
    return stream.str();
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/session/runner_action_result_conversion.hpp"

#include "shadowrun_ghidra/world/world_effect_mode_selection_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t subtract_source_byte(const std::uint16_t value,
                                   const std::uint8_t subtraction) {
    const auto low = static_cast<std::uint8_t>(value);
    const auto result = static_cast<std::uint8_t>(low - subtraction);
    return static_cast<std::uint16_t>((value & 0xFF00U) | result);
}

bool source_byte_is_nonpositive(const std::uint16_t value) {
    return static_cast<std::int8_t>(static_cast<std::uint8_t>(value)) <= 0;
}

void publish_result(const std::int8_t result,
                   const RunnerActionResultServices& services,
                   RunnerActionResult& output) {
    output.published_action_result = result;
    if (services.publish_action_result) {
        services.publish_action_result(result);
        output.publication_called = true;
    }
}

} // namespace

RunnerActionResult calculate_and_publish_runner_action_result(
    const RunnerActionResultInput& input,
    const RunnerActionResultServices& services) {
    RunnerActionResult output{};

    // 0x55FD8 clears FFF0DD, then explicitly clears D1 before entering
    // 0x55F44. The caller's request field is therefore not an input to this
    // path even though the shared delay helper supports request-sensitive
    // callers elsewhere.
    auto delay_input = input.delay_input;
    delay_input.request = 0;
    output.action_delay = calculate_runner_action_delay(delay_input).delay;

    const auto availability = read_selected_matrix_program_availability(
        input.selected_program_slot, input.slot_programs,
        input.availability_flags);
    if (!availability.slot_valid) {
        return output;
    }
    output.outcome = RunnerActionResultOutcome::resolved;
    output.program_availability = availability.availability_byte;

    // CMP.W D6,D7 / BPL followed by ADD.W D7,D6. The source compares the
    // delay against the availability byte before adding that byte, and then
    // replaces an all-zero result with one trial.
    const auto bounded_delay = std::min<std::uint16_t>(
        output.action_delay,
        static_cast<std::uint16_t>(output.program_availability));
    output.combined_roll_count = static_cast<std::uint16_t>(
        bounded_delay + static_cast<std::uint16_t>(output.program_availability));
    if (output.combined_roll_count == 0U) {
        output.combined_roll_count = 1U;
    }

    // The compact record's +0x05 byte is decremented as a byte and then
    // subtracted from the low byte of D6. The high byte survives the SUB.B;
    // BLE tests the signed low-byte result, exactly as in the ROM.
    const auto compact_gate = static_cast<std::uint8_t>(
        input.compact_record_value - 1U);
    const auto difference =
        subtract_source_byte(output.combined_roll_count, compact_gate);
    output.compact_gate_difference = static_cast<std::int16_t>(difference);
    if (source_byte_is_nonpositive(difference)) {
        publish_result(-1, services, output);
        return output;
    }

    // 0x562DE adds twice the compact record's +0x06 phase byte modifier.
    // The source clamps the resulting word difficulty to two before calling
    // 0x0DBA.
    output.phase_difficulty = std::max<std::uint16_t>(
        2U, add_world_effect_phase_modifier(
                static_cast<std::uint16_t>(
                    static_cast<std::uint8_t>(input.compact_record_phase)),
                input.world_effect_phase));
    output.phase_roll_performed = true;
    if (!services.sample_random_bound) {
        output.outcome = RunnerActionResultOutcome::random_service_unavailable;
        publish_result(-1, services, output);
        return output;
    }

    output.thresholded_successes = count_thresholded_random_rolls(
        difference, static_cast<std::int16_t>(output.phase_difficulty),
        services.sample_random_bound);
    const auto published = output.thresholded_successes == 0U
                               ? static_cast<std::int8_t>(-1)
                               : static_cast<std::int8_t>(
                                     output.thresholded_successes);
    publish_result(published, services, output);
    output.outcome = RunnerActionResultOutcome::resolved;
    return output;
}

} // namespace shadowrun::ghidra_conversion

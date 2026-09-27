#include "shadowrun_ghidra/session/runner_action_result_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerActionResultInput input{};
    input.delay_input.active_record_attribute_a = 10U;
    input.delay_input.active_record_attribute_b = 10U;
    input.delay_input.request = 99;
    input.selected_program_slot = 0U;
    input.slot_programs[0] = 2;
    input.availability_flags[2] = 2U;
    input.compact_record_value = 2U;
    input.compact_record_phase = 1U;
    input.world_effect_phase = 2U;

    std::vector<std::uint16_t> requested_bounds;
    std::int8_t published = 0;
    RunnerActionResultServices services{};
    services.sample_random_bound = [&requested_bounds](const std::uint16_t bound) {
        requested_bounds.push_back(bound);
        return static_cast<std::int16_t>(4);
    };
    services.publish_action_result = [&published](const std::int8_t result) {
        published = result;
    };

    const auto resolved = calculate_and_publish_runner_action_result(
        input, services);
    if (resolved.outcome != RunnerActionResultOutcome::resolved ||
        resolved.action_delay != 5U || resolved.program_availability != 2U ||
        resolved.combined_roll_count != 4U ||
        resolved.compact_gate_difference != 3 ||
        resolved.phase_difficulty != 5U ||
        resolved.thresholded_successes != 3U ||
        resolved.published_action_result != 3 || published != 3 ||
        !resolved.phase_roll_performed || !resolved.publication_called ||
        requested_bounds.size() != 3U || requested_bounds[0] != 6U) {
        return 1;
    }

    // A nonpositive signed low-byte gate exits before the phase roll and
    // publishes the source sentinel -1.
    auto gated = input;
    gated.compact_record_value = 5U;
    published = 0;
    requested_bounds.clear();
    const auto gated_result = calculate_and_publish_runner_action_result(
        gated, services);
    if (gated_result.outcome != RunnerActionResultOutcome::resolved ||
        gated_result.compact_gate_difference != 0 ||
        gated_result.published_action_result != -1 || published != -1 ||
        gated_result.phase_roll_performed || !requested_bounds.empty()) {
        return 2;
    }

    // The source still publishes -1 when the host random boundary is not
    // wired, but the conversion reports that unresolved boundary explicitly.
    RunnerActionResultServices missing_random{};
    missing_random.publish_action_result = [&published](const std::int8_t result) {
        published = result;
    };
    const auto missing_random_result =
        calculate_and_publish_runner_action_result(input, missing_random);
    if (missing_random_result.outcome !=
            RunnerActionResultOutcome::random_service_unavailable ||
        missing_random_result.published_action_result != -1 || published != -1 ||
        !missing_random_result.phase_roll_performed) {
        return 3;
    }

    auto invalid_slot = input;
    invalid_slot.selected_program_slot = 5U;
    published = 0;
    const auto invalid_result = calculate_and_publish_runner_action_result(
        invalid_slot, services);
    if (invalid_result.outcome !=
            RunnerActionResultOutcome::selected_slot_unavailable ||
        invalid_result.publication_called || published != 0 ||
        invalid_result.phase_roll_performed) {
        return 4;
    }

    // 0x56016 uses SUB.B/BLE. For a wrapped byte such as 0x80 - 0x01, the
    // result is positive-looking (0x7f), but V is set and BLE still exits.
    // A plain signed cast would incorrectly enter the phase roll.
    auto overflow_gate = input;
    overflow_gate.delay_input.active_record_attribute_a = 127U;
    overflow_gate.delay_input.active_record_attribute_b = 127U;
    overflow_gate.delay_input.mode_result = -1;
    overflow_gate.availability_flags[2] = 64U;
    overflow_gate.compact_record_value = 2U;
    published = 0;
    requested_bounds.clear();
    const auto overflow_result = calculate_and_publish_runner_action_result(
        overflow_gate, services);
    if (overflow_result.action_delay != 85U ||
        overflow_result.combined_roll_count != 128U ||
        overflow_result.compact_gate_difference != 127 ||
        overflow_result.published_action_result != -1 || published != -1 ||
        overflow_result.phase_roll_performed || !requested_bounds.empty()) {
        return 5;
    }

    return 0;
}

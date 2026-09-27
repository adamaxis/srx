#pragma once

#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/cyberspace/matrix_program_slot_conversion.hpp"
#include "shadowrun_ghidra/session/runner_action_delay_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// Typed inputs for ROM 0x55FD8 and its inline continuation at 0x55FFC..0x5604A.
// The compact record is the selected FF1100 + (FFF118 * 0x14) row; only the
// two bytes consumed by this action result are exposed here.
struct RunnerActionResultInput {
    RunnerActionDelayInput delay_input{};
    std::uint8_t selected_program_slot = 0; // FFF0D4 -> 0x19C54
    std::array<std::int8_t, 5> slot_programs{};
    std::array<std::uint8_t, 12> availability_flags{};
    std::uint8_t compact_record_value = 0; // FF1100 row +0x05
    std::uint8_t compact_record_phase = 0; // FF1100 row +0x06
    std::uint8_t world_effect_phase = 0;   // FFE16A
};

struct RunnerActionResultServices {
    BoundedRandomRollBoundary sample_random_bound;
    // Source writes the final signed result to active live-record +0x9F.
    std::function<void(std::int8_t)> publish_action_result;
};

enum class RunnerActionResultOutcome {
    selected_slot_unavailable,
    random_service_unavailable,
    resolved,
};

struct RunnerActionResult {
    RunnerActionResultOutcome outcome =
        RunnerActionResultOutcome::selected_slot_unavailable;
    std::uint16_t action_delay = 0;
    std::uint8_t program_availability = 0;
    std::uint16_t combined_roll_count = 0;
    std::int16_t compact_gate_difference = 0;
    std::uint16_t phase_difficulty = 0;
    std::uint8_t thresholded_successes = 0;
    std::int8_t published_action_result = -1;
    bool phase_roll_performed = false;
    bool publication_called = false;
};

// ROM 0x55FD8 plus its omitted inline continuation through 0x5604A. Clear
// the source result, calculate the delay/availability count, subtract the
// compact record gate, optionally roll at phase-adjusted difficulty, and
// publish successes or -1 to the typed action-result field.
RunnerActionResult calculate_and_publish_runner_action_result(
    const RunnerActionResultInput& input,
    const RunnerActionResultServices& services);

} // namespace shadowrun::ghidra_conversion

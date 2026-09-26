#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct RunnerSwitchRecord {
    // Source live-record +0x3E. Type 2 is the terminal selectable record
    // that causes the scan to wrap to the first record.
    std::uint16_t record_type = 0;
    // Source live-record +0x10. Bits 0, 1, 4, and 6 are consumed by 0x391A.
    std::uint8_t flags = 0;
};

struct RunnerSwitchState {
    std::vector<RunnerSwitchRecord> records;
    std::size_t active_index = 0;
    // Source FFF09D. When nonzero, the bit-4/bit-6 suppression condition is
    // bypassed; this remains a named source gate rather than a guessed mode.
    bool suppression_override = false;
};

enum class RunnerSwitchResult {
    switched,
    no_eligible_runner,
    invalid_state,
};

struct RunnerSwitchDecision {
    RunnerSwitchResult result = RunnerSwitchResult::invalid_state;
    std::size_t previous_index = 0;
    std::size_t selected_index = 0;
};

// ROM 0x391A: walk records after the current record, wrap after the type-2
// record, set the transient selection marker on each candidate, and select
// the first eligible different runner. No eligible runner leaves the active
// index unchanged; it does not synthesize or install a runner.
RunnerSwitchDecision advance_runner_switch(RunnerSwitchState& state);

} // namespace shadowrun::ghidra_conversion

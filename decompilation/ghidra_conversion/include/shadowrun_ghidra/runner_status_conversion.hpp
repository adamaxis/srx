#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct RunnerStatusRecord {
    std::uint8_t flags_at_0x10 = 0;
    std::uint8_t timer_at_0x4c = 0;
    std::uint8_t timer_at_0x9c = 0;
};

using RunnerStatusPreparationBoundary =
    std::function<void(std::size_t, RunnerStatusRecord&, std::uint8_t)>;
using RunnerStatusFinalizeBoundary =
    std::function<void(std::size_t, RunnerStatusRecord&)>;

// ROM 0x59BCC: clear the transient status bits in the three live runner
// records, invoke the source preparation/finalization boundaries in record
// order, then seed the two ten-frame timers in every record.
void reset_runner_status_triplet(
    std::array<RunnerStatusRecord, 3>& records,
    const RunnerStatusPreparationBoundary& prepare_record = {},
    const RunnerStatusFinalizeBoundary& finalize_record = {});

// ROM 0x59C34: add the caller's byte to the same source field in all three
// runner records. The source uses add.b, so unsigned eight-bit wraparound is
// intentional here.
void adjust_runner_source_byte_triplet(
    std::array<std::uint8_t, 3>& source_bytes, std::uint8_t delta);

} // namespace shadowrun::ghidra_conversion

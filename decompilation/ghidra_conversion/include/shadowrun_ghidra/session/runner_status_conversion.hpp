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

// ROM 0x59C34 awards Karma to each runner; the source value is live-record
// +0x8D, which the Karma screen reads at 0x0D3EA. Preserve the game's
// unsigned eight-bit Karma range without exposing record layout to callers.
void award_party_karma(std::array<std::uint8_t, 3>& party_karma,
                      std::uint8_t amount);

} // namespace shadowrun::ghidra_conversion

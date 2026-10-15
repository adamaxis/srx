#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

using BoundedRandomRollBoundary =
    std::function<std::int16_t(std::uint16_t bound)>;

constexpr std::uint16_t kSourceRandomMultiplier = 0x20E5U;

// ROM FFF9F8 is the ONLY random state (title, field, menus, combat, Matrix all share it; docs/24 R1-R4).
// source_random_seed() is that word; every srx stream must alias it instead of keeping a private copy.
std::uint16_t& source_random_seed();
// ROM 0x0C2C/0x0C3C cold boot: D1 (= 0x0000FFFF from the 0x6DA DBRA) += each long FF0000..FFFF7F while clearing.
std::uint16_t cold_boot_source_random_seed(const std::uint32_t* work_ram_longs);
// ROM 0x0796 warm reset (FFF9E8 == 0x12345678): FFF9F8 += frame counter FFF9EC (low word).
void warm_reset_source_random_seed(std::uint32_t frame_counter);
// Debugger hook: called after every advance with (kind, bound, result, seed_after). kind: 0 raw, 1 bounded, 2 dice.
using SourceRandomObserver = std::function<void(int kind, std::uint16_t bound, std::int32_t result, std::uint16_t seed)>;
void set_source_random_observer(SourceRandomObserver observer);

// ROM 0x0D44: MULU #$20E5, ADDQ.W #1 into FFF9F8. Host holds the seed word.
std::uint16_t advance_source_random_seed(std::uint16_t& seed);

// ROM 0x0D3A: tick 0x0D44, MULU D1, SWAP, EXT.L. High word of seed*bound.
std::int16_t sample_source_random_bound(std::uint16_t& seed,
                                        std::uint16_t bound);

// ROM 0x0DBA: D6 is a signed word; SUBQ #1 / BPL / CLR.W -> zero or negative counts roll ONE die (docs/24 R5).
// ROM 0x0DBA: perform at least one six-sided roll, retry face 6 while adding
// six to the accumulated result, and count rolls meeting the signed threshold.
std::uint8_t count_thresholded_random_rolls(
    std::uint16_t trial_count,
    std::int16_t threshold,
    const BoundedRandomRollBoundary& roll_random);

} // namespace shadowrun::ghidra_conversion

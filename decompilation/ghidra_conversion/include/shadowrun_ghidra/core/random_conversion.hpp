#pragma once

#include <cstdint>
#include <functional>
#include <vector>

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

// ROM JSR sites to 0x0D3A/0x0D44 verified in disassembly.s and C1a GPGX traces.
namespace source_random_site {
inline constexpr std::uint32_t unknown = 0U;
inline constexpr std::uint32_t dice = 0x0DD0U;
inline constexpr std::uint32_t actor_pair_collision = 0xEC90U;
inline constexpr std::uint32_t world_effect_probability = 0x1E1C2U;
inline constexpr std::uint32_t world_effect_sequence_first = 0x59988U;
inline constexpr std::uint32_t world_effect_sequence_second = 0x59992U;
inline constexpr std::uint32_t boot_scroll = 0x5DACEU;
// 0x45C6: actor spawn initial behavior delay (JSR 0x0D44; andi #0xF; scene 0x37 -> 0x28; store 0xB6,A1).
inline constexpr std::uint32_t actor_spawn_delay = 0x45C6U;
// 0x1DFF8: field alarm half-chance (JSR 0x0D44; cmpi #0x8000; bcs).
inline constexpr std::uint32_t field_alarm_half_chance = 0x1DFF8U;
// E660 random grid effect: five JSR 0x0D3A sites (D1 = 2, 8, 3 / 4, 4).
// 0x54C4: jitter effect offset word (JSR 0x0D44; andi #7; btst #1) in 0x5494.
inline constexpr std::uint32_t jitter_offset = 0x54C4U;
// 0x167AE resource activity: index JSR 0x0D3A at 0x1697A, marker JSR 0x0D44 at 0x16986.
inline constexpr std::uint32_t world_activity_resource_index = 0x1697AU;
inline constexpr std::uint32_t world_activity_marker = 0x16986U;
// ROM 0x24396: no-ID security guard's post-Talk dispatch check (bound 100).
inline constexpr std::uint32_t security_guard_dispatch = 0x24396U;
inline constexpr std::uint32_t random_grid_branch = 0xE664U;
inline constexpr std::uint32_t random_grid_branch_zero_primary = 0xE672U;
inline constexpr std::uint32_t random_grid_branch_zero_variant = 0xE688U;
inline constexpr std::uint32_t random_grid_branch_nonzero_primary = 0xE6BCU;
inline constexpr std::uint32_t random_grid_branch_nonzero_variant = 0xE6D8U;
}  // namespace source_random_site

struct SourceRandomTraceEntry {
    std::uint64_t draw_index = 0U;
    std::uint16_t seed_before = 0U;
    std::uint16_t seed_after = 0U;
    std::uint32_t rom_site = 0U;
    std::uint16_t bound = 0U;
};

// Optional in-memory draw trace (headless QA). Records only draws on the shared FFF9F8 word.
void set_source_random_trace_enabled(bool enabled);
void clear_source_random_trace();
const std::vector<SourceRandomTraceEntry>& source_random_trace();

// Site-tagged draws on the shared FFF9F8 word (ROM 0x0D44 / 0x0D3A).
std::uint16_t next_source_random_word(std::uint32_t rom_site);
std::int16_t source_random_bound(std::uint16_t bound, std::uint32_t rom_site);

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

#include "shadowrun_ghidra/core/random_conversion.hpp"

#include <cstddef>
#include <utility>

namespace shadowrun::ghidra_conversion {

namespace {
std::uint16_t g_source_random_seed = 0xFFFFU;      // cold boot with zeroed work RAM (D1 = 0x0000FFFF)
SourceRandomObserver g_observer;
bool g_trace_enabled = false;
std::uint64_t g_next_draw_index = 0U;
std::vector<SourceRandomTraceEntry> g_trace;
void trace(std::uint16_t before, std::uint16_t after, std::uint32_t site, std::uint16_t bound) {
    if (g_trace_enabled) g_trace.push_back({++g_next_draw_index, before, after, site, bound});
}
void observe(int kind, std::uint16_t bound, std::int32_t result, std::uint16_t seed) {
    if (g_observer) g_observer(kind, bound, result, seed);
}
}  // namespace

std::uint16_t& source_random_seed() { return g_source_random_seed; }

std::uint16_t cold_boot_source_random_seed(const std::uint32_t* work_ram_longs) {
    std::uint32_t d1 = 0x0000FFFFU;
    for (std::size_t i = 0; i < 0x3FE0U; ++i) d1 += work_ram_longs[i];
    g_source_random_seed = static_cast<std::uint16_t>(d1);
    return g_source_random_seed;
}

void warm_reset_source_random_seed(const std::uint32_t frame_counter) {
    g_source_random_seed = static_cast<std::uint16_t>(g_source_random_seed + frame_counter);
}

void set_source_random_observer(SourceRandomObserver observer) { g_observer = std::move(observer); }

void set_source_random_trace_enabled(const bool enabled) { g_trace_enabled = enabled; }
void clear_source_random_trace() { g_trace.clear(); g_next_draw_index = 0U; }
const std::vector<SourceRandomTraceEntry>& source_random_trace() { return g_trace; }

std::uint16_t next_source_random_word(const std::uint32_t rom_site) {
    const auto enabled = g_trace_enabled;
    g_trace_enabled = false;
    const auto before = g_source_random_seed;
    const auto word = advance_source_random_seed(g_source_random_seed);
    g_trace_enabled = enabled;
    trace(before, word, rom_site, 0U);
    return word;
}

std::int16_t source_random_bound(const std::uint16_t bound, const std::uint32_t rom_site) {
    const auto enabled = g_trace_enabled;
    g_trace_enabled = false;
    const auto before = g_source_random_seed;
    const auto rolled = sample_source_random_bound(g_source_random_seed, bound);
    g_trace_enabled = enabled;
    trace(before, g_source_random_seed, rom_site, bound);
    return rolled;
}

std::uint16_t advance_source_random_seed(std::uint16_t& seed) {
    const auto before = seed;
    seed = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(seed * kSourceRandomMultiplier) + 1U);
    observe(0, 0U, seed, seed);
    if (&seed == &g_source_random_seed) trace(before, seed, 0U, 0U);
    return seed;
}

std::int16_t sample_source_random_bound(std::uint16_t& seed,
                                        const std::uint16_t bound) {
    const auto before = seed;
    seed = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(seed * kSourceRandomMultiplier) + 1U);  // 0xD44 inline: one event per call
    const auto raw = seed;
    const auto high = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(raw) * bound) >> 16U);
    observe(1, bound, static_cast<std::int16_t>(high), seed);
    if (&seed == &g_source_random_seed) trace(before, seed, 0U, bound);
    return static_cast<std::int16_t>(high);
}

std::uint8_t count_thresholded_random_rolls(
    const std::uint16_t trial_count,
    const std::int16_t threshold,
    const BoundedRandomRollBoundary& roll_random) {
    std::uint8_t qualifying_rolls = 0;
    const auto signed_count = static_cast<std::int16_t>(trial_count);
    const auto trials = static_cast<std::uint16_t>(signed_count <= 0 ? 1 : signed_count);

    for (std::uint16_t trial = 0; trial < trials; ++trial) {
        std::int32_t accumulated_roll = 0;
        while (true) {
            const auto face = static_cast<std::int32_t>(
                (roll_random ? roll_random(6U) : 0) + 1);
            if (face == 6) {
                accumulated_roll += face;
                continue;
            }
            if (accumulated_roll + face >= threshold) {
                qualifying_rolls = static_cast<std::uint8_t>(
                    qualifying_rolls + 1U);
            }
            break;
        }
    }
    observe(2, trial_count, qualifying_rolls, g_source_random_seed);
    return qualifying_rolls;
}

} // namespace shadowrun::ghidra_conversion

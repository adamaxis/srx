#include "shadowrun_ghidra/core/random_conversion.hpp"

#include <array>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    {   // docs/24 R5: negative counts roll one die (ROM 0x0DBA SUBQ/BPL/CLR.W), not 65535.
        int calls = 0;
        count_thresholded_random_rolls(static_cast<std::uint16_t>(-3), 2,
            [&calls](std::uint16_t) { ++calls; return std::int16_t{0}; });
        if (calls != 1) return 20;
        std::vector<std::uint32_t> ram(0x3FE0U, 0U);
        if (cold_boot_source_random_seed(ram.data()) != 0xFFFFU) return 21;   // D1 = 0x0000FFFF
        warm_reset_source_random_seed(0x10002U);
        if (source_random_seed() != 0x0001U) return 22;
        int events = 0;
        set_source_random_observer([&events](int, std::uint16_t, std::int32_t, std::uint16_t) { ++events; });
        sample_source_random_bound(source_random_seed(), 6U);
        set_source_random_observer({});
        if (events != 1) return 23;
    }

    const std::vector<std::int16_t> rolls{5, 2, 0, 4};
    std::size_t cursor = 0;
    const auto successes = count_thresholded_random_rolls(
        3U, 7,
        [&rolls, &cursor](const std::uint16_t bound) {
            if (bound != 6U || cursor >= rolls.size()) {
                return static_cast<std::int16_t>(0);
            }
            return rolls[cursor++];
        });
    if (!expect(successes == 1U && cursor == 4U)) {
        return 1;
    }

    cursor = 0;
    if (!expect(count_thresholded_random_rolls(
                    0U, 1,
                    [&rolls, &cursor](const std::uint16_t) {
                        ++cursor;
                        return static_cast<std::int16_t>(0);
                    }) == 1U &&
                cursor == 1U)) {
        return 2;
    }

    std::uint16_t seed = 0;
    if (!expect(advance_source_random_seed(seed) == 1U && seed == 1U)) {
        return 3;
    }
    seed = 0x8000U;
    const auto rolled = sample_source_random_bound(seed, 6U);
    if (!expect(seed == 0x8001U && rolled == 3)) {
        return 4;
    }
    seed = 0xFFFFU;
    if (!expect(advance_source_random_seed(seed) == 0xDF1CU)) {
        return 5;
    }

    // [C] ROM 0x0D44 and verified C1a GPGX trace (gpgx_a.tsv rows 1-13) from seed 0x03D2.
    constexpr std::array<std::uint16_t, 13> expected{
        0xAADB, 0x35E8, 0x3889, 0xB28E, 0x7907, 0x2344, 0x0BD5,
        0x358A, 0x2473, 0xFAE0, 0x6A61, 0x48C6, 0xD91F};
    source_random_seed() = 0x03D2U;
    clear_source_random_trace();
    set_source_random_trace_enabled(true);
    for (const auto value : expected) {
        if (!expect(next_source_random_word(source_random_site::actor_pair_collision) == value)) return 6;
    }
    source_random_bound(6U, source_random_site::dice);
    set_source_random_trace_enabled(false);
    const auto& t = source_random_trace();
    if (!expect(t.size() == expected.size() + 1U)) return 7;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (!expect(t[i].draw_index == i + 1U && t[i].seed_after == expected[i] &&
                    t[i].rom_site == 0xEC90U && t[i].bound == 0U)) return 8;
        if (i > 0 && !expect(t[i].seed_before == t[i - 1].seed_after)) return 9;
    }
    if (!expect(t.back().rom_site == 0x0DD0U && t.back().bound == 6U &&
                t.back().seed_before == 0xD91FU)) return 10;
    clear_source_random_trace();
    source_random_seed() = 0x03D2U;
    next_source_random_word(0U);
    if (!expect(source_random_trace().empty())) return 11;

    return 0;
}

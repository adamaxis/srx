#include "shadowrun_ghidra/core/random_conversion.hpp"

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

    return 0;
}

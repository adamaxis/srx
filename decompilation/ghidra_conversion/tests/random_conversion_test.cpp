#include "shadowrun_ghidra/random_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

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

    return 0;
}

#include "shadowrun_ghidra/hud_mode_conversion.hpp"

#include <array>
#include <cstdint>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 64> table{};
    for (std::uint8_t index = 0; index < table.size(); ++index) {
        table[index] = index;
    }

    const auto ordinary = select_hud_mode({1U, 6}, true, table);
    if (!expect(ordinary.available && ordinary.table_index == 12U &&
                ordinary.value == 12U)) {
        return 1;
    }
    const auto alternate = select_hud_mode({1U, 7}, true, table);
    if (!expect(alternate.available && alternate.table_index == 15U &&
                alternate.value == 15U)) {
        return 2;
    }
    const auto outside_range = select_hud_mode({1U, 16}, true, table);
    if (!expect(outside_range.table_index == 33U &&
                outside_range.value == 33U)) {
        return 3;
    }
    const auto inactive = select_hud_mode({0U, 7}, true, table);
    const auto unknown = select_hud_mode({1U, -1}, false, table);
    if (!expect(!inactive.available && inactive.value == 0x45U &&
                !unknown.available && unknown.value == 0x45U &&
                !hud_mode_record_is_eligible({1U, -1}))) {
        return 4;
    }

    return 0;
}

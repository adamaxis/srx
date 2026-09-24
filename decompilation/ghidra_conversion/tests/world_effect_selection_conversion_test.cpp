#include "shadowrun_ghidra/world_effect_selection_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectSelectionTable table{};
    table.row_by_context = {0U, 1U};
    table.direct_handles.resize(18U);
    table.indirection_enabled.resize(18U, 0U);
    table.indirection_targets.resize(4U);

    // Context 0, rolls 2 and 3 select row 0, column 5.
    table.direct_handles[5U] = 2U;
    table.indirection_enabled[5U] = 1U;
    table.indirection_targets[2U] = 3U;
    std::size_t first_roll_cursor = 0U;
    const auto redirected = select_world_effect_sequence(
        0U, table,
        [&first_roll_cursor](const std::uint16_t bound) {
            if (bound != kWorldEffectSelectionRollBound) {
                return std::uint16_t{0U};
            }
            return std::array<std::uint16_t, 2>{2U, 3U}
                [first_roll_cursor++];
        });
    if (!expect(redirected.has_value() && *redirected == 3U)) {
        return 1;
    }

    table.indirection_enabled[5U] = 0U;
    if (!expect(!select_world_effect_sequence(
                     0U, table,
                     [](const std::uint16_t) { return std::uint16_t{2U}; })
                     .has_value())) {
        return 2;
    }

    // Context 1, rolls 4 and 4 select row 1, column 8 directly.
    table.direct_handles[17U] = 1U;
    table.indirection_enabled[17U] = 1U;
    table.indirection_targets[1U] = 1U;
    if (!expect(select_world_effect_sequence(
                    1U, table,
                    [](const std::uint16_t) { return std::uint16_t{4U}; })
                    == std::optional<std::size_t>{1U})) {
        return 3;
    }

    if (!expect(!select_world_effect_sequence(
                     2U, table,
                     [](const std::uint16_t) { return std::uint16_t{0U}; })
                     .has_value() &&
                !select_world_effect_sequence(
                     0U, table,
                     [](const std::uint16_t) { return std::uint16_t{5U}; })
                     .has_value() &&
                !select_world_effect_sequence(0U, table, {})
                     .has_value())) {
        return 4;
    }

    return 0;
}

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

    const auto direct_run = select_run_hud_mode(
        {1U, 8}, true, false, table, {});
    int resolver_calls = 0;
    const auto resolved_run = select_run_hud_mode(
        {1U, 8}, false, true, table,
        {[&] {
            ++resolver_calls;
            return 9;
        }});
    const auto blocked_run = select_run_hud_mode(
        {1U, 8}, false, false, table, {[&] { return 0; }});
    if (!expect(direct_run.available && direct_run.table_index == 16U &&
                direct_run.value == 16U && resolved_run.available &&
                resolved_run.table_index == 19U && resolved_run.value == 19U &&
                resolver_calls == 1 && !blocked_run.available)) {
        return 5;
    }

    const std::vector<HudModeAuxiliaryRecord> auxiliary_records{
        {8, 4}, {8, 9}, {2, 12}};
    const auto unchanged = select_hud_mode_auxiliary_value(
        0x1234U, auxiliary_records, 0U, -1);
    const auto alternate_auxiliary = select_hud_mode_auxiliary_value(
        0x1234U, auxiliary_records, 0U, 1);
    const auto ordinary_auxiliary = select_hud_mode_auxiliary_value(
        0x1234U, auxiliary_records, 1U, 0);
    const auto missing_auxiliary = select_hud_mode_auxiliary_value(
        0x1234U, std::vector<HudModeAuxiliaryRecord>{{8, 3}}, 0U, 1);
    if (!expect(unchanged.valid && unchanged.preserved_value == 0x1234U &&
                unchanged.source_index == 0U &&
                unchanged.adjusted_auxiliary_value == 4U &&
                alternate_auxiliary.valid && alternate_auxiliary.source_index == 1U &&
                alternate_auxiliary.adjusted_auxiliary_value == 11U &&
                ordinary_auxiliary.adjusted_auxiliary_value == 10U &&
                !missing_auxiliary.valid)) {
        return 6;
    }

    return 0;
}

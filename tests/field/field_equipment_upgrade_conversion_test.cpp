#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldEquipmentUpgradeTable table{};
    table.meter_thresholds = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
                              9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U};
    FieldEquipmentUpgradeState state{};
    state.selection_category = 3U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 2U);
    state.cost = 90U;
    state.funds = 500U;
    state.meter_tens = 3U;
    state.meter_ones = 0U;
    state.archetype_status = 1;
    state.tracked_meter_values = {5U, 1U, 2U};
    int grids = 0;
    int unavailable = 0;
    int poor = 0;
    int confirmations = 0;
    const FieldEquipmentUpgradeCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++unavailable; },
        [&] { ++poor; },
        [&] {
            ++confirmations;
            return true;
        },
    };

    // Category 3 receives the source +1 cyberware adjustment, so 30 - 4 + 1
    // becomes 27 before the two decimal digits are stored.
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                grids == 1 && confirmations == 1 && state.funds == 410U &&
                state.meter_tens == 2U && state.meter_ones == 7U &&
                state.upgrade_flags ==
                    static_cast<std::uint16_t>((1U << 2U) | (1U << 3U)) &&
                state.saved_meter_tens == 2U &&
                state.tracked_meter_values[0] == 2U &&
                state.tracked_meter_values[1] == 1U)) {
        return 1;
    }

    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::unavailable &&
                unavailable == 1 && state.funds == 410U)) {
        return 2;
    }

    state.selection_category = 5U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 5U);
    state.meter_tens = 4U;
    state.meter_ones = 0U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                (state.upgrade_flags & (1U << 6U)) != 0U &&
                (state.upgrade_flags & (1U << 5U)) == 0U &&
                state.funds == 410U)) {
        return 3;
    }

    state.selection_category = 5U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 8U);
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::unavailable &&
                unavailable == 2)) {
        return 4;
    }

    // Categories 7+ scan the fourth replacement bit as well. Bit 15 is the
    // source terminal state at 0x5722A/0x5723C and cannot resolve a target.
    state.selection_category = 7U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 15U);
    state.meter_tens = 9U;
    state.meter_ones = 9U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::unavailable &&
                unavailable == 3 && state.upgrade_flags ==
                    static_cast<std::uint16_t>(1U << 15U))) {
        return 10;
    }

    state.selection_category = 0U;
    state.upgrade_flags = 0U;
    state.meter_tens = 0U;
    state.meter_ones = 1U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::unavailable &&
                unavailable == 4)) {
        return 5;
    }

    state.selection_category = 1U;
    state.upgrade_flags = 0U;
    state.meter_tens = 9U;
    state.meter_ones = 9U;
    state.funds = 5U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::insufficient_funds &&
                poor == 1 && state.funds == 5U)) {
        return 6;
    }

    state.funds = 500U;
    const FieldEquipmentUpgradeCallbacks cancel_callbacks{
        {}, {}, {}, [] { return false; }};
    if (!expect(apply_field_equipment_upgrade(table, state, cancel_callbacks) ==
                    FieldEquipmentUpgradeResult::cancelled &&
                state.upgrade_flags == 0U && state.funds == 500U)) {
        return 7;
    }

    return 0;
}

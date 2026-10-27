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
    int already_owned = 0;
    int spurs_conflict = 0;
    int poor = 0;
    int insufficient_essence = 0;
    int confirmations = 0;
    const FieldEquipmentUpgradeCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++already_owned; },
        [&] { ++spurs_conflict; },
        [&] { ++poor; },
        [&] { ++insufficient_essence; },
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
                state.upgrade_flags == static_cast<std::uint16_t>(1U << 3U) &&
                state.saved_meter_tens == 2U &&
                state.tracked_meter_values[0] == 2U &&
                state.tracked_meter_values[1] == 1U)) {
        return 1;
    }

    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::already_owned &&
                already_owned == 1 && state.funds == 410U)) {
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
                    FieldEquipmentUpgradeResult::already_owned &&
                already_owned == 2)) {
        return 4;
    }

    // ROM 0x5725C begins Dermal Plating at bit 9, advances through bits 10
    // and 11, and replaces the previous tier after each purchase.
    int dermal_confirmations = 0;
    int dermal_already_owned = 0;
    const FieldEquipmentUpgradeCallbacks dermal_callbacks{
        {},
        [&] { ++dermal_already_owned; },
        {},
        {},
        {},
        [&] {
            ++dermal_confirmations;
            return true;
        },
    };
    state.selection_category = 6U;
    state.upgrade_flags = 0U;
    state.meter_tens = 9U;
    state.meter_ones = 9U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(
                    table, state, dermal_callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                state.upgrade_flags == static_cast<std::uint16_t>(1U << 9U) &&
                dermal_confirmations == 1)) {
        return 15;
    }
    if (!expect(apply_field_equipment_upgrade(
                    table, state, dermal_callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                state.upgrade_flags == static_cast<std::uint16_t>(1U << 10U) &&
                dermal_confirmations == 2)) {
        return 16;
    }
    if (!expect(apply_field_equipment_upgrade(
                    table, state, dermal_callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                state.upgrade_flags == static_cast<std::uint16_t>(1U << 11U) &&
                dermal_confirmations == 3)) {
        return 17;
    }
    if (!expect(apply_field_equipment_upgrade(
                    table, state, dermal_callbacks) ==
                    FieldEquipmentUpgradeResult::already_owned &&
                state.upgrade_flags == static_cast<std::uint16_t>(1U << 11U) &&
                dermal_confirmations == 3 && dermal_already_owned == 1)) {
        return 18;
    }

    // ROM 0x5722A scans only Wired Reflexes bits 12..14. Bit 14 is the
    // terminal tier; the unrelated bit 15 does not change target selection.
    state.selection_category = 7U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 14U);
    state.meter_tens = 9U;
    state.meter_ones = 9U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::already_owned &&
                already_owned == 3 && state.upgrade_flags ==
                    static_cast<std::uint16_t>(1U << 14U) &&
                confirmations == 2)) {
        return 10;
    }

    state.upgrade_flags = static_cast<std::uint16_t>(1U << 15U);
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::applied &&
                state.upgrade_flags == static_cast<std::uint16_t>(
                    (1U << 12U) | (1U << 15U)) &&
                state.funds == 410U && confirmations == 3)) {
        return 13;
    }

    if (!expect(field_equipment_upgrade_display_essence_cost(
                    table, 0U, 0U) == std::optional<std::uint8_t>{1U}) ||
        !expect(field_equipment_upgrade_display_essence_cost(
                    table, 3U, static_cast<std::uint16_t>(1U << 2U)) ==
                std::optional<std::uint8_t>{3U}) ||
        !expect(field_equipment_upgrade_display_essence_cost(
                    table, 6U, 0U) == std::optional<std::uint8_t>{12U}) ||
        !expect(field_equipment_upgrade_display_essence_cost(
                    table, 7U, 0U) == std::optional<std::uint8_t>{13U}) ||
        !expect(field_equipment_upgrade_display_essence_cost(
                    table, 7U, static_cast<std::uint16_t>(1U << 12U)) ==
                std::optional<std::uint8_t>{14U}) ||
        !expect(!field_equipment_upgrade_display_essence_cost(
                     table, 8U, 0U)
                     .has_value())) {
        return 14;
    }

    state.selection_category = 0U;
    state.upgrade_flags = 0U;
    state.meter_tens = 0U;
    state.meter_ones = 1U;
    state.funds = 500U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::insufficient_essence &&
                insufficient_essence == 1 && confirmations == 4)) {
        return 5;
    }

    state.selection_category = 1U;
    state.upgrade_flags = 0U;
    state.meter_tens = 9U;
    state.meter_ones = 9U;
    state.funds = 5U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::insufficient_funds &&
                poor == 1 && state.funds == 5U && confirmations == 4)) {
        return 6;
    }

    state.funds = 500U;
    const FieldEquipmentUpgradeCallbacks cancel_callbacks{
        {}, {}, {}, {}, {}, [] { return false; }};
    if (!expect(apply_field_equipment_upgrade(table, state, cancel_callbacks) ==
                    FieldEquipmentUpgradeResult::cancelled &&
                state.upgrade_flags == 0U && state.funds == 500U)) {
        return 7;
    }

    state.selection_category = 2U;
    state.upgrade_flags = static_cast<std::uint16_t>(1U << 3U);
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::
                        spurs_make_hand_razors_unnecessary &&
                spurs_conflict == 1 && confirmations == 4)) {
        return 11;
    }
    state.selection_category = 8U;
    if (!expect(apply_field_equipment_upgrade(table, state, callbacks) ==
                    FieldEquipmentUpgradeResult::invalid_selection &&
                confirmations == 4)) {
        return 12;
    }

    return 0;
}

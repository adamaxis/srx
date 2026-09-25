#include "shadowrun_ghidra/inventory_action_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields record{};
    record.inventory_item_codes[5] = 0x15U;
    record.inventory_phase_bytes[5] = 4U;
    const auto selected = read_selected_inventory_item(record, 1U, 2U);
    if (!expect(selected && selected->slot == 5U &&
                selected->item_code == 0x15U && selected->phase == 4U)) {
        return 1;
    }
    if (!expect(!read_selected_inventory_item(record, 0U, 0U))) {
        return 2;
    }

    if (!expect(classify_selected_item(0x10U, true) ==
                SelectedItemBranch::first_aid &&
                classify_selected_item(0x13U) ==
                    SelectedItemBranch::automatic_use &&
                classify_selected_item(0x15U) ==
                    SelectedItemBranch::weapon_or_gear &&
                classify_selected_item(0x20U) ==
                    SelectedItemBranch::item_action &&
                classify_selected_item(0x2CU) == SelectedItemBranch::armor)) {
        return 3;
    }

    InventoryActionMenuState menu{};
    menu.column = 1U;
    menu.row = 2U;
    auto result = advance_inventory_action_loop(
        record, menu, InventoryActionInput{false, false, true, false, false});
    if (!expect(result.event == InventoryActionEvent::opened && menu.open)) {
        return 4;
    }
    result = advance_inventory_action_loop(
        record, menu, InventoryActionInput{false, true, false, false, false});
    if (!expect(result.event == InventoryActionEvent::action_moved &&
                menu.action_selector == 3U)) {
        return 5;
    }
    result = advance_inventory_action_loop(
        record, menu, InventoryActionInput{false, false, true, false, false});
    if (!expect(result.event == InventoryActionEvent::dispatched &&
                result.choice == InventoryActionChoice::discard &&
                result.branch == SelectedItemBranch::weapon_or_gear &&
                !menu.open)) {
        return 6;
    }

    record.equipped_inventory_slot = 5;
    record.equipped_item_code = 0x15U;
    record.ammo_rounds = 10U;
    if (!expect(toggle_selected_equipment(
                    record, *selected, 100U, 50U) ==
                EquipmentMutation::unequipped &&
                record.equipped_inventory_slot == -1 &&
                record.equipped_item_code == 0U)) {
        return 7;
    }
    if (!expect(convert_equipped_ammo(50U, 100U, 25U) == 13U)) {
        return 8;
    }

    record.inventory_item_codes[0] = 0x2CU;
    record.inventory_phase_bytes[0] = 1U;
    const auto armor = read_selected_inventory_item(record, 0U, 0U);
    if (!expect(armor && toggle_selected_equipment(
                    record, *armor, 0U, 0U) ==
                EquipmentMutation::armor_equipped &&
                record.equipped_armor_index == 1U &&
                record.inventory_phase_bytes[0] == 1U)) {
        return 9;
    }
    if (!expect(!can_discard_selected_item(SelectedInventoryItem{7U, 0x29U, 1U}) &&
                can_trade_selected_item(0x16U) &&
                !can_trade_selected_item(0x17U))) {
        return 10;
    }

    RunnerRecordFields discard_record{};
    discard_record.inventory_item_codes[7] = 0x28U;
    discard_record.inventory_phase_bytes[7] = 2U;
    const auto discard_item = read_selected_inventory_item(discard_record, 1U, 3U);
    if (!expect(discard_item &&
                discard_selected_item(discard_record, *discard_item) ==
                    EquipmentMutation::discarded &&
                discard_record.inventory_item_codes[7] == 0U &&
                discard_record.inventory_phase_bytes[7] == 0U)) {
        return 11;
    }

    RunnerRecordFields recipient{};
    record.inventory_item_codes[1] = 0x15U;
    record.inventory_phase_bytes[1] = 3U;
    const auto trade_item = read_selected_inventory_item(record, 1U, 0U);
    if (!expect(trade_item &&
                transfer_selected_item(record, *trade_item, recipient, 0U) &&
                recipient.inventory_item_codes[0] == 0x15U &&
                recipient.inventory_phase_bytes[0] == 3U &&
                record.inventory_item_codes[1] == 0U)) {
        return 12;
    }

    if (!expect(route_field_command(2U) == FieldCommandRoute::clips &&
                route_field_command(6U) == FieldCommandRoute::pocket_secretary &&
                !route_field_command(7U))) {
        return 13;
    }
    return 0;
}

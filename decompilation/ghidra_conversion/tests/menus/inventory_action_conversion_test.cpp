#include "shadowrun_ghidra/menus/inventory_action_conversion.hpp"

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
                classify_selected_item(0x14U) ==
                    SelectedItemBranch::automatic_use &&
                classify_selected_item(0x16U) ==
                    SelectedItemBranch::automatic_use &&
                classify_selected_item(0x20U) ==
                    SelectedItemBranch::automatic_use &&
                classify_selected_item(0x15U) ==
                    SelectedItemBranch::weapon_or_gear &&
                classify_selected_item(0x17U) ==
                    SelectedItemBranch::weapon_or_gear &&
                classify_selected_item(0x2CU) == SelectedItemBranch::armor)) {
        return 3;
    }
    for (std::uint8_t item_code = 0x18U; item_code < 0x2CU;
         ++item_code) {
        if (!expect(classify_selected_item(item_code) ==
                    SelectedItemBranch::automatic_use)) {
            return 13;
        }
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
    if (!expect(convert_equipped_ammo(50U, 100U, 25U) == 13U &&
                convert_equipped_ammo(3U, 6U, 12U) == 6U &&
                convert_equipped_ammo(1U, 7U, 50U) == 8U &&
                convert_equipped_ammo(15U, 0U, 50U) == 8U)) {
        return 8;
    }

    // ROM 0x125FA stores the old gun's remaining fill percentage when it is
    // unequipped. Re-equipping another gun applies that percentage to the
    // new gun's own capacity; ammo does not remain as raw rounds from gun 1.
    RunnerRecordFields ammo_record{};
    ammo_record.inventory_item_codes[0] = 1U;
    ammo_record.inventory_item_codes[1] = 2U;
    ammo_record.equipped_inventory_slot = 0;
    ammo_record.equipped_item_code = 1U;
    ammo_record.ammo_rounds = 3U;
    const auto gun_one = read_selected_inventory_item(ammo_record, 0U, 0U);
    const auto gun_two = read_selected_inventory_item(ammo_record, 1U, 0U);
    if (!expect(gun_one && gun_two &&
                toggle_selected_equipment(ammo_record, *gun_one, 6U, 6U) ==
                    EquipmentMutation::unequipped &&
                ammo_record.equipped_inventory_slot == -1 &&
                ammo_record.equipped_item_code == 0U &&
                ammo_record.ammo_rounds == 50U)) {
        return 14;
    }
    if (!expect(toggle_selected_equipment(ammo_record, *gun_two, 0U, 12U) ==
                    EquipmentMutation::equipped &&
                ammo_record.equipped_inventory_slot == 1 &&
                ammo_record.equipped_item_code == 2U &&
                ammo_record.ammo_rounds == 6U)) {
        return 15;
    }

    // ROM item code 3 has an 11-round capacity. The source rounds 4/11 up to
    // 37 percent on unequip, then applies 37 percent to capacity 11 and gets
    // 4 rounds when that same weapon is equipped again.
    RunnerRecordFields eleven_round_gun{};
    eleven_round_gun.inventory_item_codes[0] = 3U;
    eleven_round_gun.equipped_inventory_slot = 0;
    eleven_round_gun.equipped_item_code = 3U;
    eleven_round_gun.ammo_rounds = 4U;
    const auto gun_eleven =
        read_selected_inventory_item(eleven_round_gun, 0U, 0U);
    if (!expect(gun_eleven &&
                toggle_selected_equipment(eleven_round_gun, *gun_eleven,
                                          11U, 11U) ==
                    EquipmentMutation::unequipped &&
                eleven_round_gun.equipped_inventory_slot == -1 &&
                eleven_round_gun.equipped_item_code == 0U &&
                eleven_round_gun.ammo_rounds == 37U)) {
        return 23;
    }
    if (!expect(toggle_selected_equipment(eleven_round_gun, *gun_eleven,
                                          0U, 11U) ==
                    EquipmentMutation::equipped &&
                eleven_round_gun.equipped_inventory_slot == 0 &&
                eleven_round_gun.equipped_item_code == 3U &&
                eleven_round_gun.ammo_rounds == 4U)) {
        return 24;
    }

    // Switching directly between two guns follows the same path. A one-round
    // gun at 1/7 capacity rounds up to 15%, then 15% of a 50-round weapon
    // rounds half-up to eight rounds, matching the ROM's DIVU branches.
    RunnerRecordFields direct_switch{};
    direct_switch.inventory_item_codes[0] = 1U;
    direct_switch.inventory_item_codes[1] = 2U;
    direct_switch.equipped_inventory_slot = 0;
    direct_switch.equipped_item_code = 1U;
    direct_switch.ammo_rounds = 1U;
    const auto direct_target =
        read_selected_inventory_item(direct_switch, 1U, 0U);
    if (!expect(direct_target &&
                toggle_selected_equipment(direct_switch, *direct_target,
                                          7U, 50U) ==
                    EquipmentMutation::equipped &&
                direct_switch.equipped_inventory_slot == 1 &&
                direct_switch.equipped_item_code == 2U &&
                direct_switch.ammo_rounds == 8U)) {
        return 16;
    }

    // Non-weapon gear does not reinterpret the stored percentage using its
    // own +0x24 value. That shared +0x4E value remains a percentage until a
    // weapon is equipped again.
    RunnerRecordFields gear_switch{};
    gear_switch.inventory_item_codes[0] = 1U;
    gear_switch.inventory_item_codes[1] = 0x15U;
    gear_switch.inventory_item_codes[2] = 2U;
    gear_switch.equipped_inventory_slot = 0;
    gear_switch.equipped_item_code = 1U;
    gear_switch.ammo_rounds = 1U;
    const auto gear = read_selected_inventory_item(gear_switch, 1U, 0U);
    const auto later_gun = read_selected_inventory_item(gear_switch, 0U, 1U);
    if (!expect(gear && later_gun &&
                toggle_selected_equipment(gear_switch, *gear, 7U, 200U) ==
                    EquipmentMutation::equipped &&
                gear_switch.equipped_item_code == 0x15U &&
                gear_switch.ammo_rounds == 15U &&
                toggle_selected_equipment(gear_switch, *later_gun, 200U,
                                          50U) ==
                    EquipmentMutation::equipped &&
                gear_switch.equipped_item_code == 2U &&
                gear_switch.ammo_rounds == 8U)) {
        return 17;
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
    RunnerRecordFields armor_replacement{};
    armor_replacement.inventory_item_codes[0] = 0x2CU;
    armor_replacement.inventory_phase_bytes[0] = 1U;
    armor_replacement.inventory_item_codes[1] = 0x2DU;
    armor_replacement.inventory_phase_bytes[1] = 2U;
    armor_replacement.equipped_armor_index = 1U;
    const auto replacement_armor =
        read_selected_inventory_item(armor_replacement, 1U, 0U);
    if (!expect(replacement_armor &&
                toggle_selected_equipment(armor_replacement,
                                          *replacement_armor, 0U, 0U) ==
                    EquipmentMutation::armor_unequipped &&
                armor_replacement.equipped_armor_index == 0U &&
                armor_replacement.inventory_item_codes[0] == 0x2CU &&
                armor_replacement.inventory_phase_bytes[0] == 1U &&
                armor_replacement.inventory_item_codes[1] == 0x2DU &&
                armor_replacement.inventory_phase_bytes[1] == 0U &&
                toggle_selected_equipment(armor_replacement,
                                          *replacement_armor, 0U, 0U) ==
                    EquipmentMutation::armor_equipped &&
                armor_replacement.equipped_armor_index == 2U &&
                armor_replacement.inventory_phase_bytes[1] == 2U)) {
        return 18;
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

    std::array<RunnerRecordFields, kInventoryTradePartySlotCount> party{};
    std::array<bool, kInventoryTradePartySlotCount> active_party{
        true, false, false};
    party[0].inventory_item_codes[0] = 0x15U;
    party[0].inventory_phase_bytes[0] = 6U;
    const SelectedInventoryItem party_item{0U, 0x15U, 6U};
    InventoryTradeState trade_state{};
    auto trade = begin_inventory_trade(
        party, active_party, 0U, party_item, trade_state);
    if (!expect(trade.event == InventoryTradeEvent::no_recipient &&
                !trade_state.open)) {
        return 14;
    }

    active_party = {true, true, true};
    party[1].inventory_item_codes.fill(1U);
    trade = begin_inventory_trade(
        party, active_party, 0U, party_item, trade_state);
    if (!expect(trade.event == InventoryTradeEvent::recipient_opened &&
                trade.recipient_slot == 1U &&
                trade.recipient_inventory_full &&
                !trade.yes_selected && trade_state.open)) {
        return 15;
    }
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{false, false, true});
    if (!expect(trade.event == InventoryTradeEvent::full_recipient_advanced &&
                trade.recipient_slot == 2U &&
                !trade.recipient_inventory_full &&
                trade.recipient_inventory_slot == 0U &&
                !trade.yes_selected)) {
        return 16;
    }
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{true, false, false});
    if (!expect(trade.event == InventoryTradeEvent::choice_moved &&
                trade.yes_selected)) {
        return 17;
    }
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{false, false, true});
    if (!expect(trade.event == InventoryTradeEvent::transferred &&
                !trade_state.open &&
                party[0].inventory_item_codes[0] == 0U &&
                party[2].inventory_item_codes[0] == 0x15U &&
                party[2].inventory_phase_bytes[0] == 6U)) {
        return 18;
    }

    party = {};
    party[0].inventory_item_codes[0] = 0x15U;
    active_party = {true, true, true};
    trade = begin_inventory_trade(
        party, active_party, 0U, party_item, trade_state);
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{false, false, true});
    if (!expect(trade.event == InventoryTradeEvent::recipient_advanced &&
                trade.recipient_slot == 2U && !trade.yes_selected)) {
        return 19;
    }
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{false, true, false});
    if (!expect(trade.event == InventoryTradeEvent::choice_moved &&
                trade.yes_selected)) {
        return 20;
    }
    trade = advance_inventory_trade(
        party, active_party, trade_state,
        InventoryTradeInput{false, false, true});
    if (!expect(trade.event == InventoryTradeEvent::transferred &&
                party[2].inventory_item_codes[0] == 0x15U)) {
        return 21;
    }

    if (!expect(route_field_command(2U) == FieldCommandRoute::clips &&
                route_field_command(6U) == FieldCommandRoute::pocket_secretary &&
                !route_field_command(7U))) {
        return 22;
    }
    return 0;
}

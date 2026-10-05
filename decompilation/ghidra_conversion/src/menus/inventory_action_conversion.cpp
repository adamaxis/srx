#include "shadowrun_ghidra/menus/inventory_action_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {
namespace {

std::uint16_t active_weapon_ammo_percent(const std::uint8_t rounds,
                                         const std::uint8_t capacity) {
    const auto numerator = static_cast<std::uint32_t>(rounds) * 100U;
    if (capacity == 0U) {
        // ROM 0x125FA skips DIVU when the item factor is zero and then keeps
        // the low byte of rounds * 100 in the shared +0x4E meter.
        return static_cast<std::uint16_t>(numerator & 0xFFFFU);
    }
    auto percent = numerator / capacity;
    if ((numerator % capacity) != 0U) {
        // The source increments for any nonzero division remainder.
        ++percent;
    }
    return static_cast<std::uint16_t>(percent & 0xFFFFU);
}

std::uint8_t scale_ammo_percent_to_capacity(
    const std::uint16_t percent, const std::uint8_t capacity) {
    const auto scaled = static_cast<std::uint32_t>(percent) * capacity;
    auto result = scaled / 100U;
    if ((scaled % 100U) >= 50U) {
        ++result;
    }
    return static_cast<std::uint8_t>(std::min<std::uint32_t>(result, 0xFFU));
}

} // namespace

std::optional<FieldCommandRoute> route_field_command(
    const std::uint8_t source_selector) {
    if (source_selector > 6U) {
        return std::nullopt;
    }
    return static_cast<FieldCommandRoute>(source_selector);
}

std::optional<SelectedInventoryItem> read_selected_inventory_item(
    const RunnerRecordFields& record,
    const std::uint8_t column,
    const std::uint8_t row) {
    if (column > 1U || row > 3U) {
        return std::nullopt;
    }
    const auto slot = static_cast<std::size_t>(row) * 2U + column;
    const auto item_code = record.inventory_item_codes[slot];
    if (item_code == 0U) {
        return std::nullopt;
    }
    return SelectedInventoryItem{
        slot, item_code, record.inventory_phase_bytes[slot]};
}

SelectedItemBranch classify_selected_item(
    const std::uint8_t item_code,
    const bool pending_medical_action) {
    if (pending_medical_action && item_code >= 0x10U && item_code <= 0x12U) {
        return SelectedItemBranch::first_aid;
    }
    // 0x121C0 is the source acknowledgement path for these utility and
    // spell-focus item codes.
    if (item_code == 0x13U || item_code == 0x14U || item_code == 0x16U ||
        (item_code >= 0x18U && item_code < 0x2CU)) {
        return SelectedItemBranch::automatic_use;
    }
    if (item_code < 0x18U) {
        return SelectedItemBranch::weapon_or_gear;
    }
    return SelectedItemBranch::armor;
}

namespace {

std::uint8_t wrap_action_selector(const std::uint8_t current,
                                  const bool move_left,
                                  const bool move_right) {
    if (move_left && !move_right) {
        return current <= 2U ? 5U : static_cast<std::uint8_t>(current - 1U);
    }
    if (move_right && !move_left) {
        return current >= 5U ? 2U : static_cast<std::uint8_t>(current + 1U);
    }
    return current;
}

InventoryActionChoice choice_from_selector(const std::uint8_t selector) {
    return static_cast<InventoryActionChoice>(
        std::clamp<std::uint8_t>(selector, 2U, 5U));
}

} // namespace

InventoryActionResult advance_inventory_action_loop(
    const RunnerRecordFields& record,
    InventoryActionMenuState& state,
    const InventoryActionInput& input) {
    if (!state.open) {
        if (!input.confirm) {
            return {};
        }
        const auto selected = read_selected_inventory_item(
            record, state.column, state.row);
        if (!selected) {
            return {InventoryActionEvent::empty_cell, std::nullopt,
                    std::nullopt, std::nullopt};
        }
        state.open = true;
        state.action_selector = 2U;
        return {InventoryActionEvent::opened, selected, std::nullopt,
                std::nullopt};
    }

    const auto next_selector = wrap_action_selector(
        state.action_selector, input.left, input.right);
    if (next_selector != state.action_selector) {
        state.action_selector = next_selector;
        return {InventoryActionEvent::action_moved, std::nullopt,
                choice_from_selector(state.action_selector), std::nullopt};
    }

    if (input.cancel || input.start) {
        state.open = false;
        state.action_selector = 2U;
        return {InventoryActionEvent::closed, std::nullopt, std::nullopt,
                std::nullopt};
    }
    if (!input.confirm) {
        return {};
    }

    const auto selected = read_selected_inventory_item(
        record, state.column, state.row);
    if (!selected) {
        state.open = false;
        state.action_selector = 2U;
        return {InventoryActionEvent::empty_cell, std::nullopt,
                std::nullopt, std::nullopt};
    }

    const auto choice = choice_from_selector(state.action_selector);
    if (choice == InventoryActionChoice::cancel) {
        state.open = false;
        state.action_selector = 2U;
        return {InventoryActionEvent::closed, selected, choice, std::nullopt};
    }

    state.open = false;
    state.action_selector = 2U;
    const auto branch = choice == InventoryActionChoice::equip_or_use
                            ? classify_selected_item(selected->item_code, true)
                            : classify_selected_item(selected->item_code);
    return {InventoryActionEvent::dispatched, selected, choice, branch};
}

std::uint8_t convert_equipped_ammo(const std::uint8_t old_rounds,
                                   const std::uint8_t old_factor,
                                   const std::uint8_t new_factor) {
    if (new_factor == 0U) {
        return 0U;
    }
    const auto normalized_percent = old_factor == 0U
        ? static_cast<std::uint32_t>(old_rounds)
        : static_cast<std::uint32_t>(
              active_weapon_ammo_percent(old_rounds, old_factor));
    return scale_ammo_percent_to_capacity(
        static_cast<std::uint16_t>(normalized_percent), new_factor);
}

EquipmentMutation toggle_selected_equipment(
    RunnerRecordFields& record,
    const SelectedInventoryItem& selected,
    const std::uint8_t old_item_factor,
    const std::uint8_t selected_item_factor) {
    if (selected.slot >= record.inventory_item_codes.size() ||
        record.inventory_item_codes[selected.slot] != selected.item_code ||
        selected.item_code == 0U) {
        return EquipmentMutation::invalid_selection;
    }

    const auto branch = classify_selected_item(selected.item_code);
    if (branch == SelectedItemBranch::armor) {
        const auto armor_index = static_cast<std::uint8_t>(
            selected.item_code - 0x2BU);
        if (record.equipped_armor_index == armor_index) {
            record.equipped_armor_index = 0U;
            record.inventory_phase_bytes[selected.slot] = 0U;
            return EquipmentMutation::armor_unequipped;
        }
        record.equipped_armor_index = armor_index;
        record.inventory_phase_bytes[selected.slot] = armor_index;
        return EquipmentMutation::armor_equipped;
    }
    if (branch != SelectedItemBranch::weapon_or_gear) {
        return EquipmentMutation::rejected_item_action;
    }

    const auto old_rounds_or_percent = record.ammo_rounds;
    const auto equipped_slot = record.equipped_inventory_slot;
    const bool current_slot_is_valid =
        equipped_slot >= 0 &&
        static_cast<std::size_t>(equipped_slot) <
            record.inventory_item_codes.size();
    const bool current_item_is_weapon =
        current_slot_is_valid && record.equipped_item_code != 0U &&
        record.equipped_item_code < 0x0DU;
    const bool same_equipped_slot =
        current_slot_is_valid &&
        static_cast<std::size_t>(equipped_slot) == selected.slot;
    const bool selected_item_is_weapon = selected.item_code < 0x0DU;

    // 0x125FA saves the active magazine as a percentage and clears +0x4E.
    // On the same-slot unequip path 0x12222 restores that percentage; it does
    // not restore the original round count.
    const auto carried_percent = current_item_is_weapon
        ? static_cast<std::uint8_t>(active_weapon_ammo_percent(
              old_rounds_or_percent, old_item_factor) & 0xFFU)
        : old_rounds_or_percent;

    if (same_equipped_slot) {
        record.ammo_rounds = current_item_is_weapon
            ? carried_percent
            : old_rounds_or_percent;
        record.equipped_inventory_slot = -1;
        record.equipped_item_code = 0U;
        return EquipmentMutation::unequipped;
    }

    if (selected_item_is_weapon) {
        record.ammo_rounds = current_item_is_weapon
            ? scale_ammo_percent_to_capacity(
                  active_weapon_ammo_percent(old_rounds_or_percent,
                                             old_item_factor),
                  selected_item_factor)
            : convert_equipped_ammo(carried_percent, 0U,
                                    selected_item_factor);
    } else {
        // A weapon switching to gear leaves its normalized percentage in
        // +0x4E; gear and unequipped states already carry that percentage.
        record.ammo_rounds = carried_percent;
    }
    record.equipped_inventory_slot = static_cast<std::int8_t>(selected.slot);
    record.equipped_item_code = selected.item_code;
    return EquipmentMutation::equipped;
}

bool can_discard_selected_item(const SelectedInventoryItem& selected) {
    return !(selected.slot == 7U && selected.item_code >= 0x29U &&
             selected.item_code <= 0x2BU);
}

EquipmentMutation discard_selected_item(
    RunnerRecordFields& record,
    const SelectedInventoryItem& selected) {
    if (selected.slot >= record.inventory_item_codes.size() ||
        record.inventory_item_codes[selected.slot] != selected.item_code ||
        selected.item_code == 0U || !can_discard_selected_item(selected)) {
        return EquipmentMutation::invalid_selection;
    }

    if (record.equipped_inventory_slot >= 0 &&
        static_cast<std::size_t>(record.equipped_inventory_slot) ==
            selected.slot) {
        record.equipped_inventory_slot = -1;
        record.equipped_item_code = 0U;
        record.ammo_rounds = 0U;
    }
    if (selected.item_code >= 0x2CU &&
        record.equipped_armor_index ==
            static_cast<std::uint8_t>(selected.item_code - 0x2BU)) {
        record.equipped_armor_index = 0U;
    }
    record.inventory_item_codes[selected.slot] = 0U;
    record.inventory_phase_bytes[selected.slot] = 0U;
    return EquipmentMutation::discarded;
}

bool can_trade_selected_item(const std::uint8_t item_code) {
    return item_code < 0x17U || item_code >= 0x2CU;
}

bool transfer_selected_item(
    RunnerRecordFields& source,
    const SelectedInventoryItem& selected,
    RunnerRecordFields& recipient,
    const std::size_t recipient_slot) {
    if (selected.slot >= source.inventory_item_codes.size() ||
        recipient_slot >= recipient.inventory_item_codes.size() ||
        source.inventory_item_codes[selected.slot] != selected.item_code ||
        selected.item_code == 0U ||
        recipient.inventory_item_codes[recipient_slot] != 0U ||
        !can_trade_selected_item(selected.item_code)) {
        return false;
    }

    recipient.inventory_item_codes[recipient_slot] = selected.item_code;
    recipient.inventory_phase_bytes[recipient_slot] =
        selected.item_code >= 0x2CU ? 0U : selected.phase;

    if (source.equipped_inventory_slot >= 0 &&
        static_cast<std::size_t>(source.equipped_inventory_slot) ==
            selected.slot) {
        source.equipped_inventory_slot = -1;
        source.equipped_item_code = 0U;
        source.ammo_rounds = 0U;
    }
    if (selected.item_code >= 0x2CU &&
        source.equipped_armor_index ==
            static_cast<std::uint8_t>(selected.item_code - 0x2BU)) {
        source.equipped_armor_index = 0U;
    }
    source.inventory_item_codes[selected.slot] = 0U;
    source.inventory_phase_bytes[selected.slot] = 0U;
    return true;
}

namespace {

std::optional<std::size_t> next_inventory_trade_recipient(
    const std::array<bool, kInventoryTradePartySlotCount>& active_slots,
    const std::size_t source_slot,
    const std::size_t after_slot) {
    if (source_slot >= kInventoryTradePartySlotCount ||
        after_slot >= kInventoryTradePartySlotCount) {
        return std::nullopt;
    }
    for (std::size_t step = 1U; step <= kInventoryTradePartySlotCount; ++step) {
        const auto candidate =
            (after_slot + step) % kInventoryTradePartySlotCount;
        if (candidate != source_slot && active_slots[candidate]) {
            return candidate;
        }
    }
    return std::nullopt;
}

void select_inventory_trade_recipient(
    const std::array<RunnerRecordFields, kInventoryTradePartySlotCount>&
        records,
    const std::size_t recipient_slot,
    InventoryTradeState& state) {
    state.recipient_slot = recipient_slot;
    const auto& items = records[recipient_slot].inventory_item_codes;
    const auto free_slot = std::find(items.begin(), items.end(), 0U);
    state.recipient_inventory_full = free_slot == items.end();
    state.recipient_inventory_slot = state.recipient_inventory_full
        ? 0U
        : static_cast<std::size_t>(free_slot - items.begin());
    // FFF114 is cleared and toggled by the initial 0x127B0 draw, making No
    // the source default every time a new recipient is shown.
    state.yes_selected = false;
}

InventoryTradeResult inventory_trade_result(
    const InventoryTradeEvent event,
    const InventoryTradeState& state) {
    return InventoryTradeResult{event, state.recipient_slot,
                                state.recipient_inventory_slot,
                                state.recipient_inventory_full,
                                state.yes_selected};
}

} // namespace

InventoryTradeResult begin_inventory_trade(
    const std::array<RunnerRecordFields, kInventoryTradePartySlotCount>&
        records,
    const std::array<bool, kInventoryTradePartySlotCount>& active_slots,
    const std::size_t source_slot,
    const SelectedInventoryItem& selected,
    InventoryTradeState& state) {
    state = InventoryTradeState{};
    if (source_slot >= kInventoryTradePartySlotCount ||
        !active_slots[source_slot] ||
        selected.slot >= records[source_slot].inventory_item_codes.size() ||
        records[source_slot].inventory_item_codes[selected.slot] !=
            selected.item_code ||
        selected.item_code == 0U ||
        !can_trade_selected_item(selected.item_code)) {
        return inventory_trade_result(InventoryTradeEvent::transfer_failed,
                                      state);
    }

    state.open = true;
    state.selected_item = selected;
    state.source_slot = source_slot;
    const auto recipient = next_inventory_trade_recipient(
        active_slots, source_slot, source_slot);
    if (!recipient.has_value()) {
        state = InventoryTradeState{};
        return inventory_trade_result(InventoryTradeEvent::no_recipient,
                                      state);
    }
    select_inventory_trade_recipient(records, *recipient, state);
    return inventory_trade_result(InventoryTradeEvent::recipient_opened,
                                  state);
}

InventoryTradeResult advance_inventory_trade(
    std::array<RunnerRecordFields, kInventoryTradePartySlotCount>& records,
    const std::array<bool, kInventoryTradePartySlotCount>& active_slots,
    InventoryTradeState& state,
    const InventoryTradeInput& input) {
    if (!state.open) {
        return inventory_trade_result(InventoryTradeEvent::no_change, state);
    }
    if (input.left || input.right) {
        state.yes_selected = !state.yes_selected;
        return inventory_trade_result(InventoryTradeEvent::choice_moved,
                                      state);
    }
    if (!input.confirm) {
        return inventory_trade_result(InventoryTradeEvent::no_change, state);
    }

    if (state.recipient_inventory_full || !state.yes_selected) {
        const auto recipient = next_inventory_trade_recipient(
            active_slots, state.source_slot, state.recipient_slot);
        if (!recipient.has_value()) {
            state.open = false;
            return inventory_trade_result(InventoryTradeEvent::finished,
                                          state);
        }
        const auto event = state.recipient_inventory_full
            ? InventoryTradeEvent::full_recipient_advanced
            : InventoryTradeEvent::recipient_advanced;
        select_inventory_trade_recipient(records, *recipient, state);
        return inventory_trade_result(event, state);
    }

    const auto transferred = transfer_selected_item(
        records[state.source_slot], state.selected_item,
        records[state.recipient_slot], state.recipient_inventory_slot);
    state.open = false;
    return inventory_trade_result(
        transferred ? InventoryTradeEvent::transferred
                    : InventoryTradeEvent::transfer_failed,
        state);
}

} // namespace shadowrun::ghidra_conversion

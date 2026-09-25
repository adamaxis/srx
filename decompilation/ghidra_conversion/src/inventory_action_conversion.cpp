#include "shadowrun_ghidra/inventory_action_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

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
    // 0x121C0 is the source acknowledgement path for these utility items.
    if (item_code == 0x13U || item_code == 0x14U || item_code == 0x16U) {
        return SelectedItemBranch::automatic_use;
    }
    if (item_code < 0x18U) {
        return SelectedItemBranch::weapon_or_gear;
    }
    if (item_code < 0x2CU) {
        return SelectedItemBranch::item_action;
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
    std::uint32_t normalized = old_rounds;
    if (old_factor != 0U) {
        normalized = normalized * 100U / old_factor;
    }
    const auto scaled = normalized * new_factor;
    auto result = scaled / 100U;
    if ((scaled % 100U) >= 50U) {
        ++result;
    }
    return static_cast<std::uint8_t>(std::min<std::uint32_t>(result, 0xFFU));
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

    const auto old_rounds = record.ammo_rounds;
    if (record.equipped_inventory_slot >= 0 &&
        static_cast<std::size_t>(record.equipped_inventory_slot) ==
            selected.slot) {
        if (record.equipped_item_code < 0x0DU) {
            record.ammo_rounds = convert_equipped_ammo(
                old_rounds, old_item_factor, old_item_factor);
        }
        record.equipped_inventory_slot = -1;
        record.equipped_item_code = 0U;
        return EquipmentMutation::unequipped;
    }

    record.ammo_rounds = convert_equipped_ammo(
        old_rounds, old_item_factor, selected_item_factor);
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

} // namespace shadowrun::ghidra_conversion

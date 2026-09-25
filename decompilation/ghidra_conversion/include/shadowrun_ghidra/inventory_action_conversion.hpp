#pragma once

#include "shadowrun_ghidra/runner_record_access_conversion.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x10D30/0x11740 command selectors.  Selectors zero and one are the
// two inventory columns; the remaining values enter their own source panel.
enum class FieldCommandRoute : std::uint8_t {
    inventory_left = 0,
    inventory_right = 1,
    clips = 2,
    attributes_and_skills = 3,
    cyberware = 4,
    magic = 5,
    pocket_secretary = 6,
};

std::optional<FieldCommandRoute> route_field_command(
    std::uint8_t source_selector);

struct SelectedInventoryItem {
    std::size_t slot = 0;
    std::uint8_t item_code = 0;
    std::uint8_t phase = 0;
};

// ROM 0x12148: FFF0D0 selects the column and FFF0D1 selects the row.  The
// source reads record + 0x66 + 2*row + column; no item is synthesized when
// the cell is empty.
std::optional<SelectedInventoryItem> read_selected_inventory_item(
    const RunnerRecordFields& record,
    std::uint8_t column,
    std::uint8_t row);

enum class SelectedItemBranch : std::uint8_t {
    first_aid,
    automatic_use,
    weapon_or_gear,
    item_action,
    armor,
};

// ROM 0x12168 dispatches by item code.  The pending medical branch is the
// source FFF09F gate reached by the first action edge for codes 0x10..0x12.
SelectedItemBranch classify_selected_item(
    std::uint8_t item_code,
    bool pending_medical_action = false);

enum class InventoryActionChoice : std::uint8_t {
    equip_or_use = 2,
    discard = 3,
    trade = 4,
    cancel = 5,
};

struct InventoryActionInput {
    bool left = false;
    bool right = false;
    bool confirm = false;
    bool cancel = false;
    bool start = false;
};

struct InventoryActionMenuState {
    std::uint8_t column = 0;
    std::uint8_t row = 0;
    std::uint8_t action_selector =
        static_cast<std::uint8_t>(InventoryActionChoice::equip_or_use);
    bool open = false;
};

enum class InventoryActionEvent : std::uint8_t {
    no_change,
    empty_cell,
    opened,
    action_moved,
    dispatched,
    closed,
};

struct InventoryActionResult {
    InventoryActionEvent event = InventoryActionEvent::no_change;
    std::optional<SelectedInventoryItem> item;
    std::optional<InventoryActionChoice> choice;
    std::optional<SelectedItemBranch> branch;
};

// ROM 0x11740 and inline 0x118C6.  The first confirm opens the four-choice
// action row; a later confirm dispatches the selected choice.  Left/right
// wraps 2..5 while cancel/start returns to the saved inventory cell.
InventoryActionResult advance_inventory_action_loop(
    const RunnerRecordFields& record,
    InventoryActionMenuState& state,
    const InventoryActionInput& input);

enum class EquipmentMutation : std::uint8_t {
    no_change,
    equipped,
    unequipped,
    armor_equipped,
    armor_unequipped,
    rejected_item_action,
    invalid_selection,
    discarded,
};

// ROM 0x125FA/0x12256..0x12284.  The source converts the old meter through
// the old item factor, scales it by the selected item factor, and rounds at
// a remainder of fifty.  Factors are extracted item-record fields, not ROM
// addresses retained by the host conversion.
std::uint8_t convert_equipped_ammo(std::uint8_t old_rounds,
                                   std::uint8_t old_factor,
                                   std::uint8_t new_factor);

// ROM 0x121E0..0x12292.  Weapon/gear state (+0x56/+0x57/+0x4E) and armor
// state (+0x76 and the selected +0x6E byte) remain separate from possession.
EquipmentMutation toggle_selected_equipment(
    RunnerRecordFields& record,
    const SelectedInventoryItem& selected,
    std::uint8_t old_item_factor,
    std::uint8_t selected_item_factor);

// ROM 0x12634/0x1275C.  The three special items in cell seven are protected
// by a source-specific failure branch and must not be silently deleted.
bool can_discard_selected_item(const SelectedInventoryItem& selected);

EquipmentMutation discard_selected_item(
    RunnerRecordFields& record,
    const SelectedInventoryItem& selected);

// ROM 0x12836 rejects 0x17..0x2B as non-tradeable special items.
bool can_trade_selected_item(std::uint8_t item_code);

// ROM 0x129FE.  Ordinary item phase bytes move with the item; armor carries
// no phase, and equipped source weapon/armor state is cleared on transfer.
bool transfer_selected_item(
    RunnerRecordFields& source,
    const SelectedInventoryItem& selected,
    RunnerRecordFields& recipient,
    std::size_t recipient_slot);

} // namespace shadowrun::ghidra_conversion

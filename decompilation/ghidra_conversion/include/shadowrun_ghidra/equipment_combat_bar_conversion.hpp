#pragma once

#include "shadowrun_ghidra/entity_action_math_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EquipmentCombatInput {
    // Record +0x77..+0x7E attribute bytes used by the command overview.
    std::array<std::uint8_t, 8> attributes{};
    // Record +0x81..+0x8C skill bytes. The source selects these by item class.
    std::array<std::uint8_t, 12> skills{};
    // Eight inventory item codes and their adjacent +0x6E phase bytes.
    std::array<std::uint8_t, 8> inventory_item_codes{};
    std::array<std::uint8_t, 8> inventory_phase_bytes{};
    // Record +0x56/+0x57: equipped display state and active item code.
    std::int8_t display_state = 0;
    std::uint8_t active_item_code = 0;
    // Record +0x4D and the selected inventory slot used by 0x12148.
    std::uint8_t posture = 0;
    std::uint8_t selected_inventory_slot = 0;
    // Record +0x90: installed cyberware bits.
    std::uint16_t cyberware_flags = 0;
    // Record +0x11 bit 5, consumed by ROM 0x55E62 before its item-0x27
    // phase scan.
    std::uint8_t action_flags = 0;
    // Global +0xFFF0D0. Values below two use the selector-action gate;
    // values two and above enable the special equipped-state path.
    std::uint8_t source_mode = 0;
    // ROM 0x12148's selected action code. When absent, the conversion uses
    // the item code at selected_inventory_slot as the host-side equivalent.
    std::optional<std::uint8_t> source_action_code;
    // Record +0x80/+0x7F/+0x9D used by 0x12FA4 -> 0x10660 and the negative
    // +0x56 attack calculation.
    std::uint8_t special_item18_base = 0;
    std::uint8_t roll_count_cap = 0;
    std::uint8_t auxiliary_roll_bonus = 0;
    // Decoded D4EA0/166AD4 row consumed by the special action effect path.
    // Keeping it optional makes the unresolved data-table boundary explicit.
    std::optional<EntityActionCatalogRecord> special_action_record;
};

struct EquipmentCombatSpecialActionResult {
    // ROM 0x12D7C calls 0x12FA4 before it calculates the attack strip.
    bool menu_service_called = false;
    bool residual_calculated = false;
    bool effect_written = false;
    std::uint16_t adjusted_roll_count = 0;
    std::uint16_t effective_difficulty = 0;
    std::int16_t residual_effect = 0;
};

struct EquipmentCombatBarResult {
    std::uint8_t attack_bar = 0;
    std::uint8_t defense_bar = 0;
    bool attack_written = true;
    bool defense_written = true;
    std::uint16_t command_average = 0;
    std::uint16_t attack_value = 0;
    std::uint16_t defense_value = 0;
    EquipmentCombatSpecialActionResult special_action{};
};

// ROM 0x55ED6: byte-wrapped command overview base from attributes +0x78,
// +0x7B, +0x7C and the first item-code-0x28 phase.
std::uint16_t calculate_equipment_command_average(
    const EquipmentCombatInput& input);

// ROM 0x55E62: apply the entity flag or first item-code-0x27 phase reduction
// used by the defense strip.
std::uint16_t apply_equipment_defense_action_modifier(
    const EquipmentCombatInput& input,
    std::uint16_t value);

// ROM 0x12C74: merge the selected skill into the running strip value. The
// lower of the two values is retained before the skill is added, and zero is
// replaced by one.
std::uint16_t merge_equipment_action_skill_value(
    std::uint16_t current,
    std::uint16_t skill);

// ROM 0x12CEE and its helpers 0x55ED6/0x55F20/0x55E62/0x12C74:
// calculate the command-screen attack/defense strips from typed runner
// fields. For source mode >= 2, a negative +0x56 uses 0x12FA4/0x10660 for
// the special action effect and then computes its attack strip from +0x80,
// +0x7F, +0x9D, and the matching inventory phase.
EquipmentCombatBarResult compute_equipment_combat_bars(
    const EquipmentCombatInput& input);

} // namespace shadowrun::ghidra_conversion

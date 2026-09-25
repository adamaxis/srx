#pragma once

#include <array>
#include <cstdint>

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
};

struct EquipmentCombatBarResult {
    std::uint8_t attack_bar = 0;
    std::uint8_t defense_bar = 0;
    bool attack_written = true;
    bool defense_written = true;
    std::uint16_t command_average = 0;
    std::uint16_t attack_value = 0;
    std::uint16_t defense_value = 0;
};

// ROM 0x12CEE and its normal-path helpers 0x55ED6/0x55F20/0x12C74:
// calculate the command-screen attack/defense strips from typed runner
// fields. The negative +0x56 special-item branch remains unsupported and is
// reported by attack_written=false rather than guessed.
EquipmentCombatBarResult compute_equipment_combat_bars(
    const EquipmentCombatInput& input);

} // namespace shadowrun::ghidra_conversion

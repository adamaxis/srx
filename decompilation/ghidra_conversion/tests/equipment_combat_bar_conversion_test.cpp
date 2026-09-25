#include "shadowrun_ghidra/equipment_combat_bar_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EquipmentCombatInput input{};
    input.attributes = {2U, 4U, 0U, 0U, 4U, 4U, 0U, 0U};
    input.skills[1] = 2U;
    input.skills[2] = 3U;
    input.inventory_item_codes[0] = 0x28U;
    input.inventory_phase_bytes[0] = 2U;
    input.inventory_item_codes[6] = 2U;
    input.display_state = 0;
    input.active_item_code = 2U;
    input.posture = 2U;
    input.selected_inventory_slot = 6U;

    const auto result = compute_equipment_combat_bars(input);
    if (result.command_average != 8U || result.attack_value != 6U ||
        result.attack_bar != 5U || result.defense_value != 18U ||
        result.defense_bar != 17U || !result.attack_written ||
        !result.defense_written) {
        return 1;
    }

    input.display_state = 0x7F;
    input.skills[5] = 4U;
    const auto unarmed = compute_equipment_combat_bars(input);
    if (unarmed.attack_value != 8U || unarmed.attack_bar != 7U) {
        return 2;
    }

    input.inventory_item_codes[6] = 0x10U;
    const auto utility_selected = compute_equipment_combat_bars(input);
    if (utility_selected.defense_written ||
        utility_selected.attack_bar != unarmed.attack_bar) {
        return 3;
    }

    input.display_state = -1;
    const auto special = compute_equipment_combat_bars(input);
    if (special.attack_written || special.defense_written) {
        return 4;
    }

    return 0;
}

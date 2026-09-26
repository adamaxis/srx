#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields record{};
    record.race_index = 2U;
    record.species_index = 4U;
    record.archetype_index = 5U;
    record.attributes[3] = 17U;
    record.skills[8] = 23U;
    record.cyberware_flags = 0x0221U;

    const auto attributes = read_attributes_and_skills(record);
    if (!expect(attributes.race_index == 2U &&
                attributes.species_index == 4U &&
                attributes.archetype_index == 5U &&
                attributes.attributes[3] == 17U && attributes.skills[8] == 23U)) {
        return 1;
    }

    const auto cyberware = read_cyberware_detail(record);
    if (!expect(cyberware.flags == 0x0221U && cyberware.installed[0] &&
                cyberware.installed[5] && cyberware.installed[9] &&
                !cyberware.installed[1] && !cyberware.installed[10])) {
        return 2;
    }

    record.flags = 1U;
    record.equipped_inventory_slot = 2;
    record.equipped_item_code = 3U;
    record.ammo_rounds = 2U;
    if (!expect(advance_entity_asset_meter(record, -1, 6U) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 1U &&
                advance_entity_asset_meter(record, 10, 6U) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 6U)) {
        return 3;
    }

    record.equipped_item_code = 0x0DU;
    record.inventory_item_codes[2] = 0x0DU;
    record.inventory_phase_bytes[2] = 1U;
    if (!expect(advance_entity_asset_meter(record, -1, 0U) ==
                    RunnerAssetOperationResult::equipment_cleared &&
                record.inventory_item_codes[2] == 0U &&
                record.equipped_item_code == 0U &&
                record.equipped_inventory_slot == 0x7F)) {
        return 4;
    }

    record.equipped_inventory_slot = 1;
    record.equipped_item_code = 3U;
    record.ammo_rounds = 2U;
    record.record_type = 3U;
    if (!expect(service_entity_ammo(record, -1, 6U) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 1U && adjust_runner_posture(record, -9) == 0U &&
                adjust_runner_posture(record, 9) == 4U)) {
        return 5;
    }

    record.posture = 2U;
    if (!expect(apply_runner_posture_input(record, 0x0800U) ==
                    RunnerPostureLoopResult::adjusted &&
                record.posture == 3U &&
                apply_runner_posture_input(record, 0x0400U) ==
                    RunnerPostureLoopResult::adjusted &&
                record.posture == 2U &&
                apply_runner_posture_input(record, 0x0400U) ==
                    RunnerPostureLoopResult::adjusted &&
                apply_runner_posture_input(record, 0x0400U) ==
                    RunnerPostureLoopResult::adjusted &&
                record.posture == 0U &&
                apply_runner_posture_input(record, 0x0100U) ==
                    RunnerPostureLoopResult::finished &&
                apply_runner_posture_input(record, 0x4000U) ==
                    RunnerPostureLoopResult::finished &&
                apply_runner_posture_input(record, 0U) ==
                    RunnerPostureLoopResult::waiting)) {
        return 6;
    }

    return 0;
}

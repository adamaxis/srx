#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

#include <vector>

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

    const auto& item_catalog = us_item_catalog();
    record.equipped_item_code = 4U;
    record.ammo_rounds = 1U;
    if (!expect(advance_entity_asset_meter(record, 50, item_catalog) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 25U)) {
        return 10;
    }

    auto mismatched_catalog = item_catalog;
    mismatched_catalog[3].item_code = 0U;
    record.ammo_rounds = 7U;
    if (!expect(advance_entity_asset_meter(record, 1, mismatched_catalog) ==
                    RunnerAssetOperationResult::missing_source_data &&
                record.ammo_rounds == 7U)) {
        return 11;
    }

    record.record_type = 0U;
    record.equipped_item_code = 3U;
    record.ammo_rounds = 2U;
    if (!expect(service_entity_ammo(record, 10, item_catalog) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 11U)) {
        return 12;
    }

    record.record_type = 3U;
    record.ammo_rounds = 2U;
    if (!expect(service_entity_ammo(record, 10, mismatched_catalog) ==
                    RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 1U)) {
        return 13;
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
                apply_runner_posture_input(record, 0x2000U) ==
                    RunnerPostureLoopResult::finished &&
                apply_runner_posture_input(record, 0x4000U) ==
                    RunnerPostureLoopResult::finished &&
                apply_runner_posture_input(record, 0U) ==
                    RunnerPostureLoopResult::waiting)) {
        return 6;
    }

    record.posture = 3U;
    std::vector<int> refresh_events;
    const RunnerPostureRefreshCallbacks refresh_callbacks{
        [&] { refresh_events.push_back(1); },
        [&] { refresh_events.push_back(2); },
        [&] { refresh_events.push_back(3); },
        [&] { refresh_events.push_back(4); },
        [&] { refresh_events.push_back(5); },
    };
    if (!expect(adjust_runner_posture_and_refresh(
                    record, 2, refresh_callbacks) == 4U &&
                record.posture == 4U &&
                refresh_events == std::vector<int>{1, 2, 3, 4, 5, 5})) {
        return 7;
    }

    std::vector<int> posture_loop_events;
    const std::vector<std::uint16_t> sampled_inputs{0x0800U, 0x0400U,
                                                    0x2000U};
    std::size_t sample_index = 0;
    std::vector<std::int8_t> posture_deltas;
    const RunnerPostureLoopCallbacks loop_callbacks{
        [&] { posture_loop_events.push_back(1); },
        [&] { posture_loop_events.push_back(2); },
        [&]() {
            posture_loop_events.push_back(3);
            return sampled_inputs[sample_index++];
        },
        [&] { posture_loop_events.push_back(4); },
        [&](const std::int8_t delta) { posture_deltas.push_back(delta); },
    };
    if (!expect(run_runner_posture_adjustment(loop_callbacks) ==
                    RunnerPostureLoopResult::finished &&
                sample_index == sampled_inputs.size() &&
                posture_deltas == std::vector<std::int8_t>{0, 1, -1, 0} &&
                posture_loop_events ==
                    std::vector<int>{1, 2, 3, 4, 1, 2, 3, 4,
                                     1, 2, 3, 4})) {
        return 8;
    }

    record.posture = 1U;
    std::vector<int> dialog_events;
    if (!expect(adjust_menu_dialog_posture(
                    record, -2,
                    {[&] { dialog_events.push_back(1); },
                     [&] { dialog_events.push_back(2); }}) == 0U &&
                record.posture == 0U &&
                dialog_events == std::vector<int>{1, 2})) {
        return 9;
    }

    return 0;
}

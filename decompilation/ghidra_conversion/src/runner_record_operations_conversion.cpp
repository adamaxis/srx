#include "shadowrun_ghidra/runner_record_operations_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

AttributeSkillValues read_attributes_and_skills(
    const RunnerRecordFields& record) {
    AttributeSkillValues result{};
    result.race_index = record.race_index;
    result.archetype_index = record.archetype_index;
    result.attributes = record.attributes;
    result.skills = record.skills;
    return result;
}

CyberwareValues read_cyberware_detail(const RunnerRecordFields& record) {
    CyberwareValues result{};
    result.flags = record.cyberware_flags;
    for (std::size_t bit = 0; bit < result.installed.size(); ++bit) {
        result.installed[bit] =
            (record.cyberware_flags & (std::uint16_t{1U} << bit)) != 0U;
    }
    return result;
}

RunnerAssetOperationResult advance_entity_asset_meter(
    RunnerRecordFields& record, const std::int8_t delta,
    const std::uint8_t ammo_limit) {
    if ((record.flags & 1U) == 0U || record.equipped_inventory_slot < 0 ||
        record.equipped_inventory_slot == 0x7F ||
        static_cast<std::size_t>(record.equipped_inventory_slot) >=
            kRunnerRecordInventorySlots || record.equipped_item_code == 0U) {
        return record.equipped_inventory_slot < 0 ||
                       record.equipped_inventory_slot == 0x7F
                   ? RunnerAssetOperationResult::invalid_equipped_slot
                   : RunnerAssetOperationResult::no_change;
    }

    const auto slot = static_cast<std::size_t>(record.equipped_inventory_slot);
    if (record.equipped_item_code < 0x0DU) {
        const int next = static_cast<int>(record.ammo_rounds) + delta;
        const auto bounded = static_cast<std::uint8_t>(
            std::clamp(next, 0, static_cast<int>(ammo_limit)));
        if (bounded == record.ammo_rounds) {
            return RunnerAssetOperationResult::no_change;
        }
        record.ammo_rounds = bounded;
        return RunnerAssetOperationResult::ammo_changed;
    }

    if (record.equipped_item_code < 0x10U) {
        const int next =
            static_cast<int>(record.inventory_phase_bytes[slot]) + delta;
        if (next <= 0) {
            record.inventory_phase_bytes[slot] = 0;
            record.inventory_item_codes[slot] = 0;
            record.equipped_inventory_slot = 0x7F;
            record.equipped_item_code = 0;
            return RunnerAssetOperationResult::equipment_cleared;
        }
        const auto bounded = static_cast<std::uint8_t>(std::min(next, 0xFF));
        if (bounded == record.inventory_phase_bytes[slot]) {
            return RunnerAssetOperationResult::no_change;
        }
        record.inventory_phase_bytes[slot] = bounded;
        return RunnerAssetOperationResult::inventory_phase_changed;
    }

    return RunnerAssetOperationResult::no_change;
}

RunnerAssetOperationResult service_entity_ammo(
    RunnerRecordFields& record, const std::int8_t meter_delta,
    const std::uint8_t ammo_limit) {
    if (record.ammo_rounds == 0U || record.equipped_item_code == 0U) {
        return RunnerAssetOperationResult::no_change;
    }
    if (record.record_type < 3U) {
        return advance_entity_asset_meter(record, meter_delta, ammo_limit);
    }
    --record.ammo_rounds;
    return RunnerAssetOperationResult::ammo_changed;
}

std::uint8_t adjust_runner_posture(RunnerRecordFields& record,
                                   const std::int8_t delta) {
    const int next = static_cast<int>(record.posture) + delta;
    record.posture = static_cast<std::uint8_t>(std::clamp(next, 0, 4));
    return record.posture;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

AttributeSkillValues read_attributes_and_skills(
    const RunnerRecordFields& record) {
    AttributeSkillValues result{};
    result.race_index = record.race_index;
    result.species_index = record.species_index;
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

RunnerAssetOperationResult advance_entity_asset_meter(
    RunnerRecordFields& record, const std::int8_t delta,
    const UsItemCatalog& item_catalog) {
    // 0x036FC only reaches D4540 for the low action-code weapon branch. Keep
    // the source's empty/sentinel state behavior separate from a genuinely
    // missing catalog row.
    if (record.equipped_item_code >= 1U &&
        record.equipped_item_code < 0x0DU) {
        const auto row = static_cast<std::size_t>(
            record.equipped_item_code - 1U);
        if (row >= item_catalog.size() ||
            item_catalog[row].item_code != record.equipped_item_code) {
            return RunnerAssetOperationResult::missing_source_data;
        }
        return advance_entity_asset_meter(
            record, delta, item_catalog[row].source_combat_factor);
    }
    return advance_entity_asset_meter(record, delta, 0U);
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

RunnerAssetOperationResult service_entity_ammo(
    RunnerRecordFields& record, const std::int8_t meter_delta,
    const UsItemCatalog& item_catalog) {
    if (record.ammo_rounds == 0U || record.equipped_item_code == 0U) {
        return RunnerAssetOperationResult::no_change;
    }
    if (record.record_type < 3U) {
        return advance_entity_asset_meter(record, meter_delta, item_catalog);
    }
    // 0x038AC's non-runner branch consumes one round directly and therefore
    // never consults the item catalog.
    --record.ammo_rounds;
    return RunnerAssetOperationResult::ammo_changed;
}

std::uint8_t adjust_runner_posture(RunnerRecordFields& record,
                                   const std::int8_t delta) {
    const int next = static_cast<int>(record.posture) + delta;
    record.posture = static_cast<std::uint8_t>(std::clamp(next, 0, 4));
    return record.posture;
}

std::uint8_t adjust_runner_posture_and_refresh(
    RunnerRecordFields& record, const std::int8_t delta,
    const RunnerPostureRefreshCallbacks& callbacks) {
    // ROM 0x107CE performs the record mutation before every presentation
    // service and calls the magic Success/Defense refresh twice at the end.
    const auto posture = adjust_runner_posture(record, delta);
    call_once(callbacks.write_grid);
    call_once(callbacks.render_encoded_text);
    call_once(callbacks.write_three_grids_then_service);
    call_once(callbacks.refresh_magic_action_effect_value);
    call_once(callbacks.refresh_magic_menu_success_or_defense_values);
    call_once(callbacks.refresh_magic_menu_success_or_defense_values);
    return posture;
}

std::uint8_t adjust_menu_dialog_posture(
    RunnerRecordFields& record, const std::int8_t delta,
    const MenuDialogPostureCallbacks& callbacks) {
    const auto posture = adjust_runner_posture(record, delta);
    // ROM 0x11AE4 has no combat/action refresh calls; its service tail is
    // exactly one grid followed by one encoded-text render.
    call_once(callbacks.write_grid);
    call_once(callbacks.render_encoded_text);
    return posture;
}

RunnerPostureLoopResult apply_runner_posture_input(
    RunnerRecordFields& record, const std::uint16_t input) {
    constexpr std::uint16_t kFinishImmediately = 0x0100U;
    constexpr std::uint16_t kRaisePosture = 0x0800U;
    constexpr std::uint16_t kLowerPosture = 0x0400U;
    constexpr std::uint16_t kFinishPosture =
        0x4000U | 0x2000U | 0x1000U;
    if ((input & kFinishImmediately) != 0U) {
        adjust_runner_posture(record, 0);
        return RunnerPostureLoopResult::finished;
    }
    if ((input & kRaisePosture) != 0U) {
        adjust_runner_posture(record, 1);
        return RunnerPostureLoopResult::adjusted;
    }
    if ((input & kLowerPosture) != 0U) {
        adjust_runner_posture(record, -1);
        return RunnerPostureLoopResult::adjusted;
    }
    if ((input & kFinishPosture) != 0U) {
        adjust_runner_posture(record, 0);
        return RunnerPostureLoopResult::finished;
    }
    return RunnerPostureLoopResult::waiting;
}

RunnerPostureLoopResult run_runner_posture_adjustment(
    const RunnerPostureLoopCallbacks& callbacks) {
    if (!callbacks.sample_controller_input ||
        !callbacks.adjust_and_refresh_posture) {
        return RunnerPostureLoopResult::waiting;
    }

    // ROM 0x10722 enters 0x107CE once with D0=0 before sampling input.
    callbacks.adjust_and_refresh_posture(0);
    for (;;) {
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.latch_controller_events);
        const auto input = callbacks.sample_controller_input();
        call_once(callbacks.advance_menu_sprite_overlay);

        if ((input & 0x0100U) != 0U) {
            callbacks.adjust_and_refresh_posture(0);
            return RunnerPostureLoopResult::finished;
        }
        if ((input & 0x0800U) != 0U) {
            callbacks.adjust_and_refresh_posture(1);
            continue;
        }
        if ((input & 0x0400U) != 0U) {
            callbacks.adjust_and_refresh_posture(-1);
            continue;
        }
        if ((input & 0x4000U) != 0U || (input & 0x1000U) != 0U ||
            (input & 0x2000U) != 0U) {
            callbacks.adjust_and_refresh_posture(0);
            return RunnerPostureLoopResult::finished;
        }
    }
}

} // namespace shadowrun::ghidra_conversion

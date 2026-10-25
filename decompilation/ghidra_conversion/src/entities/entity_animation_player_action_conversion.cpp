#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_animation_dispatch_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

using AssetAdvance =
    std::function<RunnerAssetOperationResult(RunnerRecordFields&)>;

void service_audio(const PlayerActionDispatchCallbacks& callbacks) {
    if (callbacks.service_audio_event) {
        callbacks.service_audio_event(0x4BU);
    }
}

PlayerActionDispatchResult dispatch_player_action(
    RunnerRecordFields& record, const AssetAdvance& advance_asset,
    const PlayerActionDelayLookup& delay_lookup,
    const PlayerActionDispatchCallbacks& callbacks) {
    if (record.ammo_rounds != 0U) {
        return {PlayerActionDispatchStatus::waiting_for_loaded_rounds,
                RunnerAssetOperationResult::no_change};
    }
    if (record.ammo_clips == 0U) {
        return {PlayerActionDispatchStatus::waiting_for_clip_reserve,
                RunnerAssetOperationResult::no_change};
    }
    if (record.animation_action_gate >= 0) {
        return {PlayerActionDispatchStatus::waiting_for_animation_gate,
                RunnerAssetOperationResult::no_change};
    }
    if (!delay_lookup || !advance_asset) {
        return {PlayerActionDispatchStatus::weapon_template_unresolved,
                RunnerAssetOperationResult::missing_source_data};
    }
    const auto action_delay =
        delay_lookup(record, PlayerActionDelayStage::weapon_action);
    const auto shared_delay =
        delay_lookup(record, PlayerActionDelayStage::shared_action);
    if (!action_delay.has_value() || !shared_delay.has_value()) {
        return {PlayerActionDispatchStatus::weapon_template_unresolved,
                RunnerAssetOperationResult::missing_source_data};
    }

    RunnerAssetOperationResult asset_result =
        RunnerAssetOperationResult::no_change;
    EntityAnimationActionServiceState action_state{record.ammo_clips};
    service_entity_animation_action(
        action_state,
        {[&callbacks] { service_audio(callbacks); },
         [&record, action_delay] {
             record.animation_action_gate = *action_delay;
         },
         [&record, &advance_asset, &asset_result] {
             asset_result = advance_asset(record);
         }});
    record.ammo_clips = action_state.pending_actions;

    service_audio(callbacks);
    record.animation_action_gate = *shared_delay;
    return {PlayerActionDispatchStatus::serviced, asset_result};
}

} // namespace

void advance_active_entity_animation_action_gate(
    RunnerRecordFields& record) {
    if ((record.flags & 0x0100U) == 0U ||
        record.animation_action_gate < 0) {
        return;
    }
    --record.animation_action_gate;
}

PlayerActionDispatchResult dispatch_player_action_if_ready(
    RunnerRecordFields& record, const UsItemCatalog& item_catalog,
    const PlayerActionDelayLookup& delay_lookup,
    const PlayerActionDispatchCallbacks& callbacks) {
    return dispatch_player_action(
        record,
        [&item_catalog](RunnerRecordFields& current) {
            return advance_entity_asset_meter(
                current, static_cast<std::int8_t>(0x26), item_catalog);
        },
        delay_lookup, callbacks);
}

PlayerActionDispatchResult dispatch_player_action_if_ready(
    RunnerRecordFields& record,
    const WeaponMagazineCapacityLookup& magazine_capacity,
    const PlayerActionDelayLookup& delay_lookup,
    const PlayerActionDispatchCallbacks& callbacks) {
    return dispatch_player_action(
        record,
        [&magazine_capacity](RunnerRecordFields& current) {
            return advance_entity_asset_meter(
                current, static_cast<std::int8_t>(0x26),
                magazine_capacity);
        },
        delay_lookup, callbacks);
}

} // namespace shadowrun::ghidra_conversion

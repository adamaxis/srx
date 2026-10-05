#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_animation_dispatch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

EntityDelayAttributes delay_attributes(const RunnerRecordFields& record) {
    return {record.attributes[1U], record.attributes[4U],
            record.cyberware_flags};
}

EntityDelayScale action_delay_scale(const std::uint8_t item_code) {
    // ROM 0x14226 subtracts one from +0x57, then selects D0=2 for source
    // codes 4, 0xA, and 0xB (equipped action codes 5, 11, and 12).
    return item_code == 5U || item_code == 11U || item_code == 12U
               ? EntityDelayScale::Double
               : EntityDelayScale::Normal;
}

void service_audio(const PlayerActionDispatchCallbacks& callbacks) {
    if (callbacks.service_audio_event) {
        callbacks.service_audio_event(0x4BU);
    }
}

void update_delay(RunnerRecordFields& record,
                  const EntityDelayScale scale) {
    EntityDelayState delay{record.animation_action_gate};
    update_entity_animation_delay(delay, delay_attributes(record), scale);
    record.animation_action_gate = delay.animation_delay;
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

    RunnerAssetOperationResult asset_result =
        RunnerAssetOperationResult::no_change;
    EntityAnimationActionServiceState action_state{record.ammo_clips};
    service_entity_animation_action(
        action_state,
        {[&callbacks] { service_audio(callbacks); },
         [&record] { update_delay(record, action_delay_scale(
                                                   record.equipped_item_code)); },
         [&record, &item_catalog, &asset_result] {
             asset_result = advance_entity_asset_meter(
                 record, static_cast<std::int8_t>(0x26), item_catalog);
         }});
    record.ammo_clips = action_state.pending_actions;

    // The successful carry from 0xAAD2 enters 0xACAE's shared action tail.
    // It emits the same event a second time and replaces the action-specific
    // delay with the source's normal-scale delay.
    service_audio(callbacks);
    update_delay(record, EntityDelayScale::Normal);

    return {PlayerActionDispatchStatus::serviced, asset_result};
}

} // namespace shadowrun::ghidra_conversion

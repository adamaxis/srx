#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class PlayerActionDispatchStatus {
    waiting_for_loaded_rounds,
    waiting_for_clip_reserve,
    waiting_for_animation_gate,
    serviced,
};

struct PlayerActionDispatchResult {
    PlayerActionDispatchStatus status =
        PlayerActionDispatchStatus::waiting_for_loaded_rounds;
    RunnerAssetOperationResult asset_result =
        RunnerAssetOperationResult::no_change;
};

struct PlayerActionDispatchCallbacks {
    std::function<void(std::uint16_t)> service_audio_event;
};

// ROM 0xA8EA decrements an active record's signed +0xDC animation gate once
// per field behavior frame, stopping after it becomes negative.
void advance_active_entity_animation_action_gate(
    RunnerRecordFields& record);

// ROM 0xAAD2 gates an empty-ammo player action on +0x4E == 0, +0x4F != 0,
// and signed +0xDC < 0. 0x14226 dispatches audio 0x4B, updates the source
// delay, consumes one shared clip, and adds 0x26 to the equipped asset meter.
// When 0xAAD2 is called from 0xACAE, that parent then dispatches 0x4B and a
// normal delay update again; this function preserves that complete caller
// order while mutating only the typed player record.
PlayerActionDispatchResult dispatch_player_action_if_ready(
    RunnerRecordFields& record, const UsItemCatalog& item_catalog,
    const PlayerActionDispatchCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

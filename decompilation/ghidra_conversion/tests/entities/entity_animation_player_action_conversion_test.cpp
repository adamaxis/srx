#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const PlayerActionDelayLookup delay_lookup =
        [](const RunnerRecordFields&, const PlayerActionDelayStage stage)
            -> std::optional<std::int16_t> {
        return stage == PlayerActionDelayStage::weapon_action ? 53 : 106;
    };

    RunnerRecordFields record{};
    record.flags = 0x0100U;
    record.equipped_inventory_slot = 0;
    record.equipped_item_code = 1U;
    record.ammo_rounds = 2U;
    record.ammo_clips = 3U;
    record.animation_action_gate = -1;
    if (!expect(dispatch_player_action_if_ready(record, us_item_catalog(),
                                                delay_lookup)
                        .status ==
                    PlayerActionDispatchStatus::waiting_for_loaded_rounds &&
                record.ammo_rounds == 2U && record.ammo_clips == 3U)) {
        return 1;
    }

    record.ammo_rounds = 0U;
    record.ammo_clips = 0U;
    if (!expect(dispatch_player_action_if_ready(record, us_item_catalog(),
                                                delay_lookup)
                        .status ==
                    PlayerActionDispatchStatus::waiting_for_clip_reserve &&
                record.ammo_rounds == 0U && record.ammo_clips == 0U)) {
        return 2;
    }

    record.ammo_clips = 3U;
    record.animation_action_gate = 0;
    if (!expect(dispatch_player_action_if_ready(record, us_item_catalog(),
                                                delay_lookup)
                        .status ==
                    PlayerActionDispatchStatus::waiting_for_animation_gate &&
                record.ammo_clips == 3U)) {
        return 3;
    }

    advance_active_entity_animation_action_gate(record);
    if (!expect(record.animation_action_gate == -1)) {
        return 4;
    }
    advance_active_entity_animation_action_gate(record);
    if (!expect(record.animation_action_gate == -1)) {
        return 5;
    }

    std::vector<std::uint16_t> events;
    std::vector<std::uint8_t> clips_at_audio;
    std::vector<std::uint8_t> rounds_at_audio;
    const auto reload = dispatch_player_action_if_ready(
        record, us_item_catalog(), delay_lookup,
        {[&](const std::uint16_t event) {
            events.push_back(event);
            clips_at_audio.push_back(record.ammo_clips);
            rounds_at_audio.push_back(record.ammo_rounds);
        }});
    if (!expect(reload.status == PlayerActionDispatchStatus::serviced &&
                reload.asset_result == RunnerAssetOperationResult::ammo_changed &&
                record.ammo_rounds == 6U && record.ammo_clips == 2U &&
                record.animation_action_gate == 106 &&
                events == std::vector<std::uint16_t>{0x4BU, 0x4BU} &&
                clips_at_audio == std::vector<std::uint8_t>{3U, 2U} &&
                rounds_at_audio == std::vector<std::uint8_t>{0U, 6U})) {
        return 6;
    }

    // Source bit 0 is in +0x10, the high byte of RunnerRecordFields::flags.
    // The asset meter must ignore the unrelated low-byte flag at +0x11.
    RunnerRecordFields source_flag{};
    source_flag.flags = 0x0001U;
    source_flag.equipped_inventory_slot = 0;
    source_flag.equipped_item_code = 1U;
    source_flag.ammo_rounds = 0U;
    if (!expect(advance_entity_asset_meter(
                    source_flag, static_cast<std::int8_t>(0x26),
                    us_item_catalog()) == RunnerAssetOperationResult::no_change &&
                source_flag.ammo_rounds == 0U)) {
        return 7;
    }
    source_flag.flags = 0x0100U;
    if (!expect(advance_entity_asset_meter(
                    source_flag, static_cast<std::int8_t>(0x26),
                    us_item_catalog()) == RunnerAssetOperationResult::ammo_changed &&
                source_flag.ammo_rounds == 6U)) {
        return 8;
    }

    RunnerRecordFields non_active_timer{};
    non_active_timer.animation_action_gate = 1;
    advance_active_entity_animation_action_gate(non_active_timer);
    if (!expect(non_active_timer.animation_action_gate == 1)) {
        return 9;
    }

    return 0;
}

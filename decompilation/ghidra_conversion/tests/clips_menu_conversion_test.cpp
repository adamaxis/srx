#include "shadowrun_ghidra/clips_menu_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(cycle_clips_command(2U, -1) == 5U &&
                cycle_clips_command(5U, 1) == 2U &&
                cycle_clips_command(3U, 1) == 4U)) {
        return 1;
    }

    RunnerRecordFields record{};
    record.ammo_clips = 2U;
    record.equipped_inventory_slot = 1;
    record.equipped_item_code = 4U;
    record.ammo_rounds = 5U;
    if (!expect(gate_clips_command(record, ClipsCommand::reload) ==
                    ClipsGate::ready &&
                reload_one_clip(record, 30U, false) ==
                    ReloadResult::confirmation_required &&
                reload_one_clip(record, 30U, true) == ReloadResult::reloaded &&
                record.ammo_clips == 1U && record.ammo_rounds == 30U)) {
        return 2;
    }

    record.equipped_item_code = 0x0DU;
    if (!expect(gate_clips_command(record, ClipsCommand::reload) ==
                    ClipsGate::item_does_not_use_clips &&
                reload_one_clip(record, 30U, true) ==
                    ReloadResult::item_does_not_use_clips)) {
        return 3;
    }
    record.equipped_item_code = 4U;
    record.ammo_clips = 0U;
    if (!expect(gate_clips_command(record, ClipsCommand::trade) ==
                    ClipsGate::out_of_ammo_clips)) {
        return 4;
    }

    std::array<RunnerRecordFields, 3> party{};
    party[0].flags = 0x01U;
    party[0].ammo_clips = 7U;
    party[1].flags = 0x01U;
    party[1].ammo_clips = 2U;
    party[2].flags = 0x03U;
    party[2].ammo_clips = 9U;
    ClipTradeState trade{};
    if (!expect(open_clip_trade(party, 0U, trade) &&
                trade.recipient_records.size() == 1U &&
                trade.recipient_records[0] == 1U && trade.quantity == 7U)) {
        return 5;
    }
    trade.command = 4U;
    trade.quantity = 3U;
    if (!expect(advance_clip_trade(
                    party, 0U, trade,
                    ClipTradeInput{false, false, true, false, false}) ==
                    ClipTradeEvent::transferred &&
                party[0].ammo_clips == 4U && party[1].ammo_clips == 5U &&
                !trade.active)) {
        return 6;
    }

    std::array<RunnerRecordFields, 3> distribution{};
    distribution[0].flags = 0x01U;
    distribution[0].ammo_clips = 4U;
    distribution[1].flags = 0x01U;
    distribution[1].ammo_clips = 3U;
    distribution[2].flags = 0x03U;
    distribution[2].ammo_clips = 2U;
    if (!expect(distribute_clips(distribution) ==
                    DistributionResult::distributed &&
                distribution[0].ammo_clips == 3U &&
                distribution[1].ammo_clips == 3U &&
                distribution[2].ammo_clips == 3U)) {
        return 7;
    }

    distribution[1].flags = 0U;
    distribution[2].flags = 0U;
    if (!expect(distribute_clips(distribution) ==
                DistributionResult::no_party_members)) {
        return 8;
    }
    return 0;
}

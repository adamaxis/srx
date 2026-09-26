#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

std::uint8_t cycle_clips_command(const std::uint8_t current,
                                 const std::int8_t step) {
    const auto candidate = static_cast<std::int16_t>(current) + step;
    if (candidate < static_cast<std::int16_t>(ClipsCommand::reload)) {
        return static_cast<std::uint8_t>(ClipsCommand::cancel);
    }
    if (candidate > static_cast<std::int16_t>(ClipsCommand::cancel)) {
        return static_cast<std::uint8_t>(ClipsCommand::reload);
    }
    return static_cast<std::uint8_t>(candidate);
}

ClipsGate gate_clips_command(const RunnerRecordFields& record,
                             const ClipsCommand command) {
    if (record.ammo_clips == 0U) {
        return ClipsGate::out_of_ammo_clips;
    }
    if (command == ClipsCommand::reload &&
        (record.equipped_inventory_slot < 0 ||
         record.equipped_item_code >= 0x0DU)) {
        return ClipsGate::item_does_not_use_clips;
    }
    return ClipsGate::ready;
}

ReloadResult reload_one_clip(RunnerRecordFields& record,
                             const std::uint8_t capacity,
                             const bool confirmed) {
    const auto gate = gate_clips_command(record, ClipsCommand::reload);
    if (gate == ClipsGate::out_of_ammo_clips) {
        return ReloadResult::out_of_ammo_clips;
    }
    if (gate == ClipsGate::item_does_not_use_clips) {
        return ReloadResult::item_does_not_use_clips;
    }
    if (record.ammo_rounds != 0U && !confirmed) {
        return ReloadResult::confirmation_required;
    }

    --record.ammo_clips;
    const auto loaded = static_cast<unsigned>(record.ammo_rounds) + 0x26U;
    const auto capped = capacity == 0U
                             ? loaded
                             : std::min<unsigned>(loaded, capacity);
    record.ammo_rounds = static_cast<std::uint8_t>(capped & 0xFFU);
    return ReloadResult::reloaded;
}

std::vector<std::size_t> build_clip_trade_recipients(
    const std::array<RunnerRecordFields, 3>& records,
    const std::size_t source_record) {
    std::vector<std::size_t> recipients;
    for (std::size_t index = 0; index < records.size(); ++index) {
        if (index == source_record || (records[index].flags & 0x02U) != 0U ||
            (records[index].flags & 0x01U) == 0U) {
            continue;
        }
        recipients.push_back(index);
    }
    return recipients;
}

bool open_clip_trade(const std::array<RunnerRecordFields, 3>& records,
                     const std::size_t source_record,
                     ClipTradeState& state) {
    if (source_record >= records.size()) {
        return false;
    }
    state.recipient_records =
        build_clip_trade_recipients(records, source_record);
    state.recipient_index = 0U;
    state.command = 2U;
    state.quantity = records[source_record].ammo_clips;
    state.active = !state.recipient_records.empty();
    return state.active;
}

namespace {

std::uint8_t wrap_trade_command(const std::uint8_t current,
                                const bool move_left,
                                const bool move_right) {
    if (move_left && !move_right) {
        return current <= 2U ? 5U : static_cast<std::uint8_t>(current - 1U);
    }
    if (move_right && !move_left) {
        return current >= 5U ? 2U : static_cast<std::uint8_t>(current + 1U);
    }
    return current;
}

} // namespace

ClipTradeEvent advance_clip_trade(
    std::array<RunnerRecordFields, 3>& records,
    const std::size_t source_record,
    ClipTradeState& state,
    const ClipTradeInput& input) {
    if (!state.active || source_record >= records.size() ||
        state.recipient_records.empty()) {
        return ClipTradeEvent::no_change;
    }

    const auto next_command = wrap_trade_command(
        state.command, input.left, input.right);
    if (next_command != state.command) {
        state.command = next_command;
        return ClipTradeEvent::command_moved;
    }

    if (input.cancel || input.start) {
        state.active = false;
        return ClipTradeEvent::closed;
    }
    if (!input.confirm) {
        return ClipTradeEvent::no_change;
    }

    auto& source = records[source_record];
    // The nested source table reuses selector values 2..5 with different
    // labels: Increase, Decrease, Trade, Change Recipient.
    switch (state.command) {
    case static_cast<std::uint8_t>(ClipsCommand::reload):
        state.quantity = state.quantity >= source.ammo_clips
                             ? 1U
                             : static_cast<std::uint8_t>(state.quantity + 1U);
        return ClipTradeEvent::quantity_changed;
    case static_cast<std::uint8_t>(ClipsCommand::distribute): {
        const auto recipient = state.recipient_records[state.recipient_index];
        if (state.quantity == 0U || state.quantity > source.ammo_clips ||
            recipient >= records.size()) {
            return ClipTradeEvent::no_change;
        }
        source.ammo_clips = static_cast<std::uint8_t>(
            source.ammo_clips - state.quantity);
        records[recipient].ammo_clips = static_cast<std::uint8_t>(
            records[recipient].ammo_clips + state.quantity);
        state.active = false;
        return ClipTradeEvent::transferred;
    }
    case static_cast<std::uint8_t>(ClipsCommand::trade):
        state.quantity = state.quantity <= 1U
                             ? source.ammo_clips
                             : static_cast<std::uint8_t>(state.quantity - 1U);
        return ClipTradeEvent::quantity_changed;
    case static_cast<std::uint8_t>(ClipsCommand::cancel):
        state.recipient_index = (state.recipient_index + 1U) %
                                state.recipient_records.size();
        return ClipTradeEvent::recipient_changed;
    }
    return ClipTradeEvent::no_change;
}

DistributionResult distribute_clips(
    std::array<RunnerRecordFields, 3>& records) {
    std::size_t active_count = 0U;
    unsigned total_clips = 0U;
    for (const auto& record : records) {
        if ((record.flags & 0x01U) == 0U) {
            continue;
        }
        ++active_count;
        total_clips += record.ammo_clips;
    }
    if (active_count < 2U) {
        return DistributionResult::no_party_members;
    }

    const auto quotient = total_clips / active_count;
    const auto remainder = total_clips % active_count;
    for (auto& record : records) {
        if ((record.flags & 0x01U) == 0U) {
            continue;
        }
        auto distributed = static_cast<unsigned>(
            static_cast<std::uint8_t>(quotient & 0xFFU));
        if ((record.flags & 0x02U) != 0U) {
            distributed += remainder;
        }
        record.ammo_clips = static_cast<std::uint8_t>(distributed & 0xFFU);
    }
    return DistributionResult::distributed;
}

} // namespace shadowrun::ghidra_conversion

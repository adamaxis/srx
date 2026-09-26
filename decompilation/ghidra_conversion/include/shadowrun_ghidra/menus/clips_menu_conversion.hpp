#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class ClipsCommand : std::uint8_t {
    reload = 2,
    trade = 3,
    distribute = 4,
    cancel = 5,
};

// ROM 0x118C6 as used by the Clips top-level and nested command rows.
std::uint8_t cycle_clips_command(std::uint8_t current, std::int8_t step);

enum class ClipsGate : std::uint8_t {
    ready,
    out_of_ammo_clips,
    item_does_not_use_clips,
};

// ROM 0x12062 gates every Clips command on +0x4F, while 0x11CCA adds the
// equipped-item/no-clip weapon test for Reload.
ClipsGate gate_clips_command(const RunnerRecordFields& record,
                             ClipsCommand command);

enum class ReloadResult : std::uint8_t {
    reloaded,
    confirmation_required,
    out_of_ammo_clips,
    item_does_not_use_clips,
};

// ROM 0x11CCA/0x11D86/0x36FC. `capacity` is the extracted item-record
// capacity byte at source item data +0x24; the conversion has no ROM lookup.
ReloadResult reload_one_clip(RunnerRecordFields& record,
                             std::uint8_t capacity,
                             bool confirmed);

struct ClipTradeState {
    std::vector<std::size_t> recipient_records;
    std::size_t recipient_index = 0;
    std::uint8_t command = 2;
    std::uint8_t quantity = 0;
    bool active = false;
};

// ROM 0x11DD2. An active party member is eligible when +0x10 bit 0 is set,
// the record is not the source, and +0x10 bit 1 is clear.
std::vector<std::size_t> build_clip_trade_recipients(
    const std::array<RunnerRecordFields, 3>& records,
    std::size_t source_record);

bool open_clip_trade(const std::array<RunnerRecordFields, 3>& records,
                     std::size_t source_record,
                     ClipTradeState& state);

enum class ClipTradeEvent : std::uint8_t {
    no_change,
    command_moved,
    quantity_changed,
    recipient_changed,
    transferred,
    closed,
};

struct ClipTradeInput {
    bool left = false;
    bool right = false;
    bool confirm = false;
    bool cancel = false;
    bool start = false;
};

// ROM 0x11E7E..0x11F90 and 0x120E4. The nested commands are Increase Clips,
// Decrease Clips, Trade Clips, and Change Recipient. Quantity wraps between
// one and the source record's complete clip count.
ClipTradeEvent advance_clip_trade(
    std::array<RunnerRecordFields, 3>& records,
    std::size_t source_record,
    ClipTradeState& state,
    const ClipTradeInput& input);

enum class DistributionResult : std::uint8_t {
    distributed,
    no_party_members,
};

// ROM 0x11FDE/0x12014. Active records receive the quotient; records with
// +0x10 bit 1 set also receive the source remainder byte.
DistributionResult distribute_clips(
    std::array<RunnerRecordFields, 3>& records);

} // namespace shadowrun::ghidra_conversion

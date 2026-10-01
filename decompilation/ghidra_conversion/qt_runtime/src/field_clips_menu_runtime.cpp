#include "field_clips_menu_runtime.hpp"

#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kLeft = 0x0400U;
constexpr std::uint16_t kRight = 0x0800U;
constexpr std::uint16_t kB = 0x1000U;
constexpr std::uint16_t kA = 0x4000U;
constexpr std::uint16_t kStart = 0x8000U;

ClipsMenuMessage message_for_reload_result(const ReloadResult result) {
    switch (result) {
    case ReloadResult::out_of_ammo_clips:
        return ClipsMenuMessage::out_of_ammo;
    case ReloadResult::item_does_not_use_clips:
        return ClipsMenuMessage::item_does_not_use_clips;
    case ReloadResult::reloaded:
    case ReloadResult::confirmation_required:
        return ClipsMenuMessage::none;
    }
    return ClipsMenuMessage::none;
}

ClipsMenuMessage message_for_clips_gate(const ClipsGate gate) {
    switch (gate) {
    case ClipsGate::out_of_ammo_clips:
        return ClipsMenuMessage::out_of_ammo;
    case ClipsGate::item_does_not_use_clips:
        return ClipsMenuMessage::item_does_not_use_clips;
    case ClipsGate::ready:
        return ClipsMenuMessage::none;
    }
    return ClipsMenuMessage::none;
}

} // namespace

void FieldClipsMenuRuntime::open() noexcept {
    open_ = true;
    reload_confirmation_open_ = false;
    reload_confirmation_yes_ = false;
    selection_ = static_cast<std::uint8_t>(ClipsCommand::reload);
    message_ = ClipsMenuMessage::none;
}

void FieldClipsMenuRuntime::close() noexcept {
    open_ = false;
    reload_confirmation_open_ = false;
    reload_confirmation_yes_ = false;
    selection_ = static_cast<std::uint8_t>(ClipsCommand::reload);
    message_ = ClipsMenuMessage::none;
}

ClipsMenuEvent FieldClipsMenuRuntime::show_message(
    const ClipsMenuMessage message) noexcept {
    message_ = message;
    return ClipsMenuEvent::message_shown;
}

ClipsMenuEvent FieldClipsMenuRuntime::advance(
    const std::uint16_t controller_events,
    RunnerRecordFields& active_record,
    const std::uint8_t capacity) {
    if (!open_) {
        return ClipsMenuEvent::no_change;
    }

    // The converted source feedback branch waits for a controller edge
    // before restoring the Clips row. Keep the message state separate from
    // the selector so an acknowledgement cannot accidentally trigger the
    // underlying command.
    if (message_ != ClipsMenuMessage::none) {
        if ((controller_events & (kLeft | kRight | kB | kA | kStart)) == 0U) {
            return ClipsMenuEvent::no_change;
        }
        message_ = ClipsMenuMessage::none;
        return ClipsMenuEvent::message_acknowledged;
    }

    if (reload_confirmation_open_) {
        if ((controller_events & (kLeft | kRight)) != 0U &&
            (controller_events & (kLeft | kRight)) != (kLeft | kRight)) {
            reload_confirmation_yes_ = !reload_confirmation_yes_;
            return ClipsMenuEvent::confirmation_toggled;
        }
        if ((controller_events & (kB | kStart)) != 0U) {
            reload_confirmation_open_ = false;
            reload_confirmation_yes_ = false;
            return ClipsMenuEvent::closed;
        }
        if ((controller_events & kA) == 0U) {
            return ClipsMenuEvent::no_change;
        }
        reload_confirmation_open_ = false;
        const auto result = reload_one_clip(
            active_record, capacity, reload_confirmation_yes_);
        reload_confirmation_yes_ = false;
        if (result == ReloadResult::reloaded) {
            message_ = ClipsMenuMessage::reload_success;
            return ClipsMenuEvent::reloaded;
        }
        const auto message = message_for_reload_result(result);
        return message == ClipsMenuMessage::none
                   ? ClipsMenuEvent::no_change
                   : show_message(message);
    }

    if ((controller_events & (kB | kStart)) != 0U) {
        close();
        return ClipsMenuEvent::closed;
    }

    const auto left = (controller_events & kLeft) != 0U;
    const auto right = (controller_events & kRight) != 0U;
    if (left != right) {
        const auto next = cycle_clips_command(
            selection_, left ? static_cast<std::int8_t>(-1)
                             : static_cast<std::int8_t>(1));
        if (next != selection_) {
            selection_ = next;
            return ClipsMenuEvent::selection_moved;
        }
    }
    if ((controller_events & kA) == 0U) {
        return ClipsMenuEvent::no_change;
    }

    const auto command = static_cast<ClipsCommand>(selection_);
    // ROM 0x12062 is the common post-dispatch gate, not a Reload-only
    // condition: Trade and Distribute also stop on an empty +0x4F clip count.
    // Keep the source gate in the converted Clips module instead of allowing
    // the host adapter to fabricate a zero-quantity trade.
    const auto gate = gate_clips_command(active_record, command);
    const auto gate_message = message_for_clips_gate(gate);
    if (gate_message != ClipsMenuMessage::none) {
        return show_message(gate_message);
    }

    switch (command) {
    case ClipsCommand::reload: {
        const auto result = reload_one_clip(active_record, capacity, false);
        if (result == ReloadResult::confirmation_required) {
            reload_confirmation_open_ = true;
            reload_confirmation_yes_ = false;
            return ClipsMenuEvent::confirmation_opened;
        }
        if (result == ReloadResult::reloaded) {
            message_ = ClipsMenuMessage::reload_success;
            return ClipsMenuEvent::reloaded;
        }
        const auto message = message_for_reload_result(result);
        return message == ClipsMenuMessage::none
                   ? ClipsMenuEvent::no_change
                   : show_message(message);
    }
    case ClipsCommand::trade: {
        // The current Qt field session has Joshua only. Ask the converted
        // recipient filter rather than inventing a recipient; the source
        // result is the same no-party branch until recruited records are
        // connected to this session.
        std::array<RunnerRecordFields, 3> records{};
        records[0] = active_record;
        records[0].flags |= 0x01U;
        ClipTradeState trade{};
        return open_clip_trade(records, 0U, trade)
                   ? ClipsMenuEvent::no_change
                   : show_message(ClipsMenuMessage::no_party_members);
    }
    case ClipsCommand::distribute: {
        std::array<RunnerRecordFields, 3> records{};
        records[0] = active_record;
        records[0].flags |= 0x01U;
        return distribute_clips(records) == DistributionResult::distributed
                   ? ClipsMenuEvent::no_change
                   : show_message(ClipsMenuMessage::no_party_members);
    }
    case ClipsCommand::cancel:
        close();
        return ClipsMenuEvent::closed;
    }
    return ClipsMenuEvent::no_change;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

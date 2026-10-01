#include "field_clips_menu_runtime.hpp"

#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"

#include <array>
#include <iostream>

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

ClipsMenuEvent advance(FieldClipsMenuRuntime& menu,
                       RunnerRecordFields& record,
                       const std::uint16_t events) {
    std::array<RunnerRecordFields, 3> records{};
    records[0] = record;
    records[0].flags = static_cast<std::uint16_t>(records[0].flags | 0x01U);
    const auto result = menu.advance(events, records, 0U, 30U);
    record = records[0];
    return result;
}

} // namespace

int main() {
    FieldClipsMenuRuntime menu;
    RunnerRecordFields record{};
    record.equipped_inventory_slot = 0;
    record.equipped_item_code = 1;
    record.ammo_clips = 3;
    record.ammo_rounds = 0;

    menu.open();
    if (!expect(menu.is_open() && menu.selection() == 2U,
                "Clips runtime did not start at Reload") ||
        !expect(advance(menu, record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 3U,
                "Clips runtime did not move to Trade") ||
        !expect(advance(menu, record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 4U,
                "Clips runtime did not move to Distribute") ||
        !expect(advance(menu, record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 5U,
                "Clips runtime did not move to Cancel") ||
        !expect(advance(menu, record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 2U,
                "Clips runtime did not wrap Cancel to Reload")) {
        return 1;
    }

    const auto before_trade = record;
    if (!expect(advance(menu, record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 3U,
                "Clips runtime did not reselect Trade") ||
        !expect(advance(menu, record, 0x4000U) ==
                    ClipsMenuEvent::message_shown &&
                    menu.message() == ClipsMenuMessage::no_party_members &&
                    record.ammo_clips == before_trade.ammo_clips,
                "Clips trade gate did not preserve the source record") ||
        !expect(advance(menu, record, 0x1000U) ==
                    ClipsMenuEvent::message_acknowledged &&
                    menu.message() == ClipsMenuMessage::none,
                "Clips message was not acknowledged")) {
        return 2;
    }

    // A loaded weapon takes the source confirmation branch. No only closes
    // that prompt; it must not close the Clips menu or mutate ammunition.
    FieldClipsMenuRuntime confirmation_menu;
    auto confirmation_record = record;
    confirmation_record.ammo_rounds = 5U;
    confirmation_menu.open();
    if (!expect(advance(confirmation_menu, confirmation_record, 0x4000U) ==
                    ClipsMenuEvent::confirmation_opened &&
                    confirmation_menu.reload_confirmation_open() &&
                    !confirmation_menu.reload_confirmation_yes() &&
                    confirmation_record.ammo_clips == record.ammo_clips &&
                    confirmation_record.ammo_rounds == 5U,
                "Clips reload did not open the source confirmation branch") ||
        !expect(advance(confirmation_menu, confirmation_record, 0x1000U) ==
                    ClipsMenuEvent::closed &&
                    confirmation_menu.is_open() &&
                    !confirmation_menu.reload_confirmation_open() &&
                    confirmation_record.ammo_clips == record.ammo_clips &&
                    confirmation_record.ammo_rounds == 5U,
                "Clips No closed the menu instead of only the confirmation") ||
        !expect(advance(confirmation_menu, confirmation_record, 0x4000U) ==
                    ClipsMenuEvent::confirmation_opened,
                "Clips could not reopen its confirmation prompt")) {
        return 3;
    }
    if (!expect(advance(confirmation_menu, confirmation_record, 0x0400U) ==
                    ClipsMenuEvent::confirmation_toggled &&
                    confirmation_menu.reload_confirmation_yes(),
                "Clips confirmation did not move to Yes") ||
        !expect(advance(confirmation_menu, confirmation_record, 0x4000U) ==
                    ClipsMenuEvent::reloaded &&
                    confirmation_menu.message() ==
                        ClipsMenuMessage::reload_success &&
                    confirmation_record.ammo_clips == 2U &&
                    confirmation_record.ammo_rounds == 30U,
                "Clips Yes did not apply the converted reload arithmetic") ||
        !expect(advance(confirmation_menu, confirmation_record, 0x1000U) ==
                    ClipsMenuEvent::message_acknowledged &&
                    advance(confirmation_menu, confirmation_record, 0x1000U) ==
                        ClipsMenuEvent::closed &&
                    !confirmation_menu.is_open(),
                "Clips confirmation success did not acknowledge and close")) {
        return 4;
    }

    // 0x12062 gates every actionable Clips branch, not just Reload.
    FieldClipsMenuRuntime empty_clip_menu;
    auto empty_clip_record = record;
    empty_clip_record.ammo_clips = 0U;
    empty_clip_menu.open();
    if (!expect(advance(empty_clip_menu, empty_clip_record, 0x0800U) ==
                    ClipsMenuEvent::selection_moved &&
                    empty_clip_menu.selection() == 3U,
                "Clips empty-count test did not select Trade") ||
        !expect(advance(empty_clip_menu, empty_clip_record, 0x4000U) ==
                    ClipsMenuEvent::message_shown &&
                    empty_clip_menu.message() == ClipsMenuMessage::out_of_ammo &&
                    empty_clip_record.ammo_clips == 0U,
                "Clips common gate allowed zero-clip Trade") ||
        !expect(advance(empty_clip_menu, empty_clip_record, 0x1000U) ==
                    ClipsMenuEvent::message_acknowledged,
                "Clips zero-count message was not acknowledged")) {
        return 5;
    }

    // Return to Reload, enter the source direct-reload path, and verify the
    // +0x26 load amount and capacity clamp through the converted routine.
    if (!expect(advance(menu, record, 0x0400U) ==
                    ClipsMenuEvent::selection_moved &&
                    menu.selection() == 2U,
                "Clips runtime did not return to Reload") ||
        !expect(advance(menu, record, 0x4000U) == ClipsMenuEvent::reloaded &&
                    menu.message() == ClipsMenuMessage::reload_success &&
                    record.ammo_clips == 2U && record.ammo_rounds == 30U,
                "Clips reload arithmetic was not applied")) {
        return 3;
    }
    if (!expect(advance(menu, record, 0x1000U) ==
                    ClipsMenuEvent::message_acknowledged,
                "Clips reload message was not acknowledged") ||
        !expect(advance(menu, record, 0x1000U) == ClipsMenuEvent::closed &&
                    !menu.is_open(),
                "Clips runtime did not close on B")) {
        return 4;
    }

    // A real recruited record is required before the converted nested trade
    // routine can open. Exercise the complete selector/quantity/transfer path
    // with typed party records; the live new-game session correctly has only
    // Joshua and therefore remains on the source no-party branch above.
    FieldClipsMenuRuntime trade_menu;
    std::array<RunnerRecordFields, 3> party{};
    party[0] = record;
    party[0].flags = static_cast<std::uint16_t>(party[0].flags | 0x01U);
    party[0].ammo_clips = 4U;
    party[1].flags = 0x01U;
    party[1].ammo_clips = 1U;
    trade_menu.open();
    if (!expect(trade_menu.advance(0x0800U, party, 0U, 30U) ==
                    ClipsMenuEvent::selection_moved &&
                    trade_menu.selection() == 3U,
                "Clips trade test did not select Trade") ||
        !expect(trade_menu.advance(0x4000U, party, 0U, 30U) ==
                    ClipsMenuEvent::trade_opened &&
                    trade_menu.trade_open() &&
                    trade_menu.trade_quantity() == 4U &&
                    trade_menu.trade_recipient_record() == 1U,
                "Clips trade did not open from the converted recipient list") ||
        !expect(trade_menu.advance(0x0800U, party, 0U, 30U) ==
                    ClipsMenuEvent::trade_selection_moved &&
                    trade_menu.trade_selection() == 3U,
                "Clips nested trade did not move to Decrease") ||
        !expect(trade_menu.advance(0x0800U, party, 0U, 30U) ==
                    ClipsMenuEvent::trade_selection_moved &&
                    trade_menu.trade_selection() == 4U,
                "Clips nested trade did not move to Trade Clips") ||
        !expect(trade_menu.advance(0x4000U, party, 0U, 30U) ==
                    ClipsMenuEvent::trade_transferred &&
                    !trade_menu.trade_open() &&
                    trade_menu.message() == ClipsMenuMessage::exchange_complete &&
                    party[0].ammo_clips == 0U && party[1].ammo_clips == 5U,
                "Clips nested trade did not apply the converted transfer") ||
        !expect(trade_menu.advance(0x1000U, party, 0U, 30U) ==
                    ClipsMenuEvent::message_acknowledged,
                "Clips exchange completion did not acknowledge")) {
        return 6;
    }
    return 0;
}

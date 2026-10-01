#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

// Host/controller state for the converted 0x11B58 Clips loop.  The source
// selector values are retained because the converted Clips routines use the
// same 2..5 range for both the top-level and nested rows.  This class is a
// small adapter around those routines; it is not a ROM-address interpreter.
enum class ClipsMenuMessage : std::uint8_t {
    none,
    out_of_ammo,
    item_does_not_use_clips,
    reload_success,
    no_party_members,
};

enum class ClipsMenuEvent : std::uint8_t {
    no_change,
    selection_moved,
    confirmation_opened,
    confirmation_toggled,
    reloaded,
    message_shown,
    message_acknowledged,
    closed,
};

class FieldClipsMenuRuntime final {
public:
    void open() noexcept;
    void close() noexcept;

    bool is_open() const noexcept { return open_; }
    bool reload_confirmation_open() const noexcept {
        return reload_confirmation_open_;
    }
    bool reload_confirmation_yes() const noexcept {
        return reload_confirmation_yes_;
    }
    std::uint8_t selection() const noexcept { return selection_; }
    ClipsMenuMessage message() const noexcept { return message_; }

    // Controller events are the typed high-byte edges published by the
    // converted controller sampler: left 0x0400, right 0x0800, B 0x1000,
    // C 0x2000, A 0x4000, Start 0x8000.  `capacity` is the source item
    // record's +0x24 clip capacity, supplied by the typed item catalog.
    ClipsMenuEvent advance(std::uint16_t controller_events,
                           RunnerRecordFields& active_record,
                           std::uint8_t capacity);

private:
    ClipsMenuEvent show_message(ClipsMenuMessage message) noexcept;

    bool open_ = false;
    bool reload_confirmation_open_ = false;
    bool reload_confirmation_yes_ = false;
    std::uint8_t selection_ = 2U;
    ClipsMenuMessage message_ = ClipsMenuMessage::none;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

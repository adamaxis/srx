#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// Confirmed fields of the selected 0x14-byte menu record used by 0x56434,
// 0x56482, and 0x564D6. The host model exposes their roles instead of the
// original byte offsets.
struct MenuSelectionTimingRecord {
    std::uint8_t service_timer = 0; // source record +0x0F
    std::uint8_t variant_timer = 0; // source record +0x10
    std::uint8_t progression_timing = 0; // source record +0x06
};

struct MenuSelectionTimingState {
    std::vector<MenuSelectionTimingRecord> records;
    std::size_t selected_record = 0; // source FFF118
    std::int8_t intro_marker = 0; // source FFF0A3
    std::uint8_t phase_counter = 0; // source FFF0CE
    std::array<std::uint8_t, 3> phase_values{}; // FF018D/FF028D/FF038D
};

struct MenuSelectionTimingCallbacks {
    // ROM 0x1A468: audio-frame initialization is a service boundary.
    std::function<void()> initialize_audio_frame;
    // ROM 0x1A154: clears the selected menu record after variant expiry.
    std::function<void()> clear_selected_menu_record;
};

struct MenuSelectionServiceRefreshCallbacks {
    MenuSelectionTimingCallbacks timing_callbacks;
    // ROM 0x17E04's recovered readiness predicate and repeated HUD-grid
    // writer. The source helper performs three writes when the predicate is
    // true and none when it is false.
    std::function<bool()> hud_grid_ready;
    std::function<void()> write_hud_grid;
};

enum class MenuSelectionTimingAction {
    updated,
    expired,
    selected_record_unavailable,
};

struct MenuSelectionTimingResult {
    MenuSelectionTimingAction action =
        MenuSelectionTimingAction::selected_record_unavailable;
    bool selected_record_cleared = false;
};

// ROM 0x56434: clear the intro marker, subtract the menu-service delta from
// record +0x0F, and roll the shared phase counters when that timer expires.
MenuSelectionTimingResult advance_menu_selection_service_timer(
    MenuSelectionTimingState& state,
    std::uint8_t delta,
    const MenuSelectionTimingCallbacks& callbacks);

struct MenuSelectionServiceRefreshResult {
    MenuSelectionTimingResult timing{};
    bool hud_grid_refreshed = false;
};

// ROM 0x5668C: derive the service delta as half of the previous world-effect
// count, advance record +0x0F through 0x56434, then call 0x17E04 for the HUD
// refresh. This is kept as one typed operation because the source routine is
// the shared boundary used by the menu-selection countdown.
MenuSelectionServiceRefreshResult advance_menu_selection_service_and_refresh(
    MenuSelectionTimingState& state,
    std::uint8_t previous_effect_count,
    const MenuSelectionServiceRefreshCallbacks& callbacks);

// ROM 0x56482: clear the intro marker, subtract the variant delta from record
// +0x10, and clear the selected menu record when the signed byte expires.
MenuSelectionTimingResult advance_menu_selection_variant_timer(
    MenuSelectionTimingState& state,
    std::uint8_t delta,
    const MenuSelectionTimingCallbacks& callbacks);

// ROM 0x564D6: halve the caller's progression delta, clamp it to one, then
// subtract it from record +0x06 and clamp the result to the source minimum 2.
MenuSelectionTimingResult reduce_menu_progression_timing(
    MenuSelectionTimingState& state,
    std::uint16_t delta);

} // namespace shadowrun::ghidra_conversion

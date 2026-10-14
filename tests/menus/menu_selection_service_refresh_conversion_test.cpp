#include "shadowrun_ghidra/menus/menu_selection_timing_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    MenuSelectionTimingState state{};
    state.records.push_back({10U, 0U, 0U});
    state.selected_record = 0U;
    state.intro_marker = 4;
    int audio_frames = 0;
    int grid_writes = 0;
    MenuSelectionServiceRefreshCallbacks callbacks{};
    callbacks.timing_callbacks.initialize_audio_frame = [&] { ++audio_frames; };
    callbacks.hud_grid_ready = [] { return true; };
    callbacks.write_hud_grid = [&] { ++grid_writes; };

    const auto refreshed = advance_menu_selection_service_and_refresh(
        state, 9U, callbacks);
    if (refreshed.timing.action != MenuSelectionTimingAction::updated ||
        state.records[0].service_timer != 6U || state.intro_marker != 0 ||
        audio_frames != 1 || !refreshed.hud_grid_refreshed ||
        grid_writes != 3) {
        return 1;
    }

    state.records[0].service_timer = 1U;
    state.phase_counter = 1U;
    state.phase_values = {4U, 5U, 6U};
    callbacks.hud_grid_ready = [] { return false; };
    const auto expired = advance_menu_selection_service_and_refresh(
        state, 3U, callbacks);
    if (expired.timing.action != MenuSelectionTimingAction::expired ||
        state.records[0].service_timer != 0U || state.phase_counter != 0x0FU ||
        state.phase_values != std::array<std::uint8_t, 3>{5U, 6U, 7U} ||
        expired.hud_grid_refreshed || audio_frames != 2 || grid_writes != 3) {
        return 2;
    }

    MenuSelectionTimingState unavailable{};
    unavailable.selected_record = 1U;
    const auto missing = advance_menu_selection_service_and_refresh(
        unavailable, 8U, callbacks);
    if (missing.timing.action !=
            MenuSelectionTimingAction::selected_record_unavailable ||
        missing.hud_grid_refreshed) {
        return 3;
    }
    return 0;
}

#include "shadowrun_ghidra/menu_selection_timing_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    MenuSelectionTimingState state{};
    state.records.resize(2);
    state.selected_record = 1;
    state.intro_marker = -1;
    state.records[1].service_timer = 7U;
    state.records[1].variant_timer = 8U;
    state.records[1].progression_timing = 9U;
    state.phase_counter = 4U;
    state.phase_values = {1U, 2U, 3U};

    int audio_frames = 0;
    int cleared_records = 0;
    MenuSelectionTimingCallbacks callbacks{};
    callbacks.initialize_audio_frame = [&] { ++audio_frames; };
    callbacks.clear_selected_menu_record = [&] { ++cleared_records; };

    const auto service =
        advance_menu_selection_service_timer(state, 2U, callbacks);
    if (service.action != MenuSelectionTimingAction::updated ||
        state.records[1].service_timer != 5U || state.intro_marker != 0 ||
        audio_frames != 1) {
        return 1;
    }

    state.records[1].service_timer = 2U;
    const auto service_expired =
        advance_menu_selection_service_timer(state, 2U, callbacks);
    if (service_expired.action != MenuSelectionTimingAction::expired ||
        state.records[1].service_timer != 0U || state.phase_counter != 2U ||
        state.phase_values != std::array<std::uint8_t, 3>{1U, 2U, 3U}) {
        return 2;
    }

    state.phase_counter = 1U;
    state.records[1].service_timer = 1U;
    const auto rollover =
        advance_menu_selection_service_timer(state, 1U, callbacks);
    if (rollover.action != MenuSelectionTimingAction::expired ||
        state.phase_counter != 0x0FU ||
        state.phase_values != std::array<std::uint8_t, 3>{2U, 3U, 4U}) {
        return 3;
    }

    const auto variant =
        advance_menu_selection_variant_timer(state, 3U, callbacks);
    if (variant.action != MenuSelectionTimingAction::updated ||
        state.records[1].variant_timer != 5U || cleared_records != 0) {
        return 4;
    }

    state.records[1].variant_timer = 2U;
    const auto variant_expired =
        advance_menu_selection_variant_timer(state, 3U, callbacks);
    if (variant_expired.action != MenuSelectionTimingAction::expired ||
        !variant_expired.selected_record_cleared ||
        state.records[1].variant_timer != 0U || cleared_records != 1) {
        return 5;
    }

    const auto progression = reduce_menu_progression_timing(state, 4U);
    if (progression.action != MenuSelectionTimingAction::updated ||
        state.records[1].progression_timing != 7U) {
        return 6;
    }

    state.records[1].progression_timing = 3U;
    reduce_menu_progression_timing(state, 4U);
    if (state.records[1].progression_timing != 2U) {
        return 7;
    }

    state.records[1].progression_timing = 2U;
    reduce_menu_progression_timing(state, 1U);
    if (state.records[1].progression_timing != 2U) {
        return 8;
    }

    MenuSelectionTimingState unavailable{};
    unavailable.selected_record = 1U;
    if (advance_menu_selection_service_timer(unavailable, 1U, callbacks)
            .action != MenuSelectionTimingAction::selected_record_unavailable) {
        return 9;
    }
    return 0;
}

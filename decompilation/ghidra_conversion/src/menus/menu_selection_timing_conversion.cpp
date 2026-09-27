#include "shadowrun_ghidra/menus/menu_selection_timing_conversion.hpp"

#include "shadowrun_ghidra/menus/menu_hud_service_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

std::uint8_t subtract_byte(const std::uint8_t value,
                           const std::uint8_t delta) {
    return static_cast<std::uint8_t>(value - delta);
}

bool source_signed_positive(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) > 0;
}

bool source_signed_negative(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) < 0;
}

void roll_shared_phase_counter(MenuSelectionTimingState& state) {
    state.phase_counter = subtract_byte(state.phase_counter, 2U);
    if (source_signed_negative(state.phase_counter)) {
        state.phase_counter = 0x0FU;
        for (auto& phase_value : state.phase_values) {
            phase_value = static_cast<std::uint8_t>(phase_value + 1U);
        }
    }
}

MenuSelectionTimingRecord* selected_record(MenuSelectionTimingState& state) {
    if (state.selected_record >= state.records.size()) {
        return nullptr;
    }
    return &state.records[state.selected_record];
}

} // namespace

MenuSelectionTimingResult advance_menu_selection_service_timer(
    MenuSelectionTimingState& state,
    const std::uint8_t delta,
    const MenuSelectionTimingCallbacks& callbacks) {
    if (callbacks.initialize_audio_frame) {
        callbacks.initialize_audio_frame();
    }
    state.intro_marker = 0;
    auto* record = selected_record(state);
    if (record == nullptr) {
        return {MenuSelectionTimingAction::selected_record_unavailable, false};
    }

    const auto updated = subtract_byte(record->service_timer, delta);
    record->service_timer = updated;
    if (source_signed_positive(updated)) {
        return {MenuSelectionTimingAction::updated, false};
    }

    record->service_timer = 0;
    roll_shared_phase_counter(state);
    return {MenuSelectionTimingAction::expired, false};
}

MenuSelectionServiceRefreshResult advance_menu_selection_service_and_refresh(
    MenuSelectionTimingState& state,
    const std::uint8_t previous_effect_count,
    const MenuSelectionServiceRefreshCallbacks& callbacks) {
    const auto delta = static_cast<std::uint8_t>(previous_effect_count >> 1U);
    const auto timing = advance_menu_selection_service_timer(
        state, delta, callbacks.timing_callbacks);
    const auto refreshed = refresh_hud_grid_if_ready(
        callbacks.hud_grid_ready, callbacks.write_hud_grid);
    return {timing, refreshed};
}

MenuSelectionTimingResult advance_menu_selection_variant_timer(
    MenuSelectionTimingState& state,
    const std::uint8_t delta,
    const MenuSelectionTimingCallbacks& callbacks) {
    if (callbacks.initialize_audio_frame) {
        callbacks.initialize_audio_frame();
    }
    state.intro_marker = 0;
    auto* record = selected_record(state);
    if (record == nullptr) {
        return {MenuSelectionTimingAction::selected_record_unavailable, false};
    }

    const auto updated = subtract_byte(record->variant_timer, delta);
    record->variant_timer = updated;
    if (!source_signed_negative(updated)) {
        return {MenuSelectionTimingAction::updated, false};
    }

    record->variant_timer = 0;
    if (callbacks.clear_selected_menu_record) {
        callbacks.clear_selected_menu_record();
    }
    roll_shared_phase_counter(state);
    return {MenuSelectionTimingAction::expired, true};
}

MenuSelectionTimingResult reduce_menu_progression_timing(
    MenuSelectionTimingState& state,
    const std::uint16_t delta) {
    auto* record = selected_record(state);
    if (record == nullptr) {
        return {MenuSelectionTimingAction::selected_record_unavailable, false};
    }

    auto halved = static_cast<std::uint16_t>(delta >> 1U);
    if (static_cast<std::int16_t>(halved) <= 0) {
        halved = 1U;
    }
    record->progression_timing = subtract_byte(
        record->progression_timing, static_cast<std::uint8_t>(halved));

    // CMP.B #2 followed by BPL uses the source signed-byte flags. Keeping
    // this as a wrapped comparison preserves the source behavior for values
    // outside the normal small timing range as well.
    const auto minimum_check = subtract_byte(record->progression_timing, 2U);
    if (source_signed_negative(minimum_check)) {
        record->progression_timing = 2U;
    }
    return {MenuSelectionTimingAction::updated, false};
}

} // namespace shadowrun::ghidra_conversion

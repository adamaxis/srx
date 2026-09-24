#include "shadowrun_ghidra/title_intro_schedule_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool advance_title_intro_schedule(
    TitleIntroScheduleState& state,
    const std::vector<TitleIntroScheduleEntry>& entries,
    const std::function<void(std::uint32_t)>& run_action) {
    --state.countdown;
    if (state.countdown >= 0) {
        return false;
    }
    if (state.current_entry >= entries.size()) {
        return true;
    }

    const auto& entry = entries[state.current_entry];
    ++state.current_entry;
    if (run_action) {
        run_action(entry.action_id);
    }
    if (state.current_entry >= entries.size()) {
        return true;
    }

    state.countdown = entries[state.current_entry].delay_frames;
    return state.countdown == 0;
}

TitleIntroScheduleStopReason run_title_intro_schedule(
    TitleIntroScheduleState& state,
    const std::vector<TitleIntroScheduleEntry>& entries,
    std::uint16_t frame_budget,
    const TitleIntroScheduleCallbacks& callbacks) {
    state.x_offset = 0;
    state.x_velocity = 0;
    state.y_offset = 0;
    state.fade_counter = 0;
    state.dirty_flags = static_cast<std::uint8_t>(
        state.dirty_flags | 0x0CU);
    state.current_entry = 0;
    state.countdown = entries.empty() ? 0 : entries.front().delay_frames;

    if (callbacks.clear_display_buffer) {
        callbacks.clear_display_buffer();
    }
    if (callbacks.prepare_title_resources) {
        callbacks.prepare_title_resources();
    }
    if (callbacks.load_title_font) {
        callbacks.load_title_font();
    }
    if (callbacks.restore_scene_records) {
        callbacks.restore_scene_records();
    }
    state.dirty_flags = static_cast<std::uint8_t>(
        state.dirty_flags | 0x01U);
    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }

    for (std::uint32_t frame = 0; frame <= frame_budget; ++frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (advance_title_intro_schedule(state, entries,
                                         callbacks.run_action)) {
            return TitleIntroScheduleStopReason::schedule_complete;
        }
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return TitleIntroScheduleStopReason::external_stop;
        }
    }
    return TitleIntroScheduleStopReason::frame_budget;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/title_intro/title_intro_schedule_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::vector<TitleIntroScheduleEntry> entries{{2U, 10U},
                                                       {1U, 11U},
                                                       {0U, 12U}};
    TitleIntroScheduleState state{};
    std::vector<std::uint32_t> actions;
    const auto run_action = [&actions](std::uint32_t action_id) {
        actions.push_back(action_id);
    };
    state.countdown = entries.front().delay_frames;
    if (advance_title_intro_schedule(state, entries, run_action) ||
        advance_title_intro_schedule(state, entries, run_action) ||
        advance_title_intro_schedule(state, entries, run_action) ||
        actions != std::vector<std::uint32_t>{10U} ||
        state.current_entry != 1U || state.countdown != 1) {
        return 1;
    }
    if (advance_title_intro_schedule(state, entries, run_action) ||
        !advance_title_intro_schedule(state, entries, run_action) ||
        actions != std::vector<std::uint32_t>{10U, 11U} ||
        state.current_entry != 2U) {
        return 2;
    }

    TitleIntroScheduleState loop_state{};
    std::vector<int> order;
    bool stop = false;
    TitleIntroScheduleCallbacks callbacks{};
    callbacks.clear_display_buffer = [&order] { order.push_back(1); };
    callbacks.prepare_title_resources = [&order] { order.push_back(2); };
    callbacks.load_title_font = [&order] { order.push_back(3); };
    callbacks.restore_scene_records = [&order] { order.push_back(4); };
    callbacks.sample_controller = [&order] { order.push_back(5); };
    callbacks.wait_for_vblank = [&order] { order.push_back(6); };
    callbacks.latch_controller_events = [&order] { order.push_back(7); };
    callbacks.stop_requested = [&stop] { return stop; };
    callbacks.run_action = [&actions](std::uint32_t action_id) {
        actions.push_back(action_id);
    };
    const auto reason = run_title_intro_schedule(
        loop_state, {{0U, 20U}, {1U, 21U}, {0U, 22U}}, 10U, callbacks);
    if (reason != TitleIntroScheduleStopReason::schedule_complete ||
        actions.size() != 4U || actions[2] != 20U || actions[3] != 21U ||
        loop_state.dirty_flags != 0x0DU || order.size() < 7U ||
        order[0] != 1 || order[1] != 2 || order[2] != 3 || order[3] != 4 ||
        order[4] != 5 || order[5] != 6 || order[6] != 5 || order[7] != 7) {
        return 3;
    }
    return 0;
}

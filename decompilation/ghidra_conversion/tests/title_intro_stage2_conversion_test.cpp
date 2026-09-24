#include "shadowrun_ghidra/title_intro_stage2_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroStage2State state{};
    TitleIntroScheduleState schedule{};
    const std::vector<TitleIntroScheduleEntry> entries{{0U, 1U}, {0U, 2U}};
    std::vector<std::uint32_t> actions;
    std::vector<int> resets;
    std::uint32_t first_frames = 0;
    std::uint32_t second_frames = 0;
    std::uint32_t scene_draws = 0;
    std::uint32_t scroll_text = 0;
    std::uint32_t late_text = 0;
    TitleIntroStage2Callbacks callbacks{};
    callbacks.render_scene_entities = [&] { ++scene_draws; };
    callbacks.run_scheduled_action = [&](std::uint32_t id) {
        actions.push_back(id);
    };
    callbacks.wait_for_vblank = [&] {
        ++state.current_frame;
        if (state.first_loop_frame <= 0x00A0U) {
            ++first_frames;
        } else {
            ++second_frames;
        }
    };
    callbacks.reset_overlay = [&] { resets.push_back(1); };
    callbacks.reset_sequence = [&] { resets.push_back(2); };
    callbacks.reset_persistent_sequence = [&] { resets.push_back(3); };
    callbacks.reset_motion = [&] { resets.push_back(4); };
    callbacks.reset_dual_sprites = [&] { resets.push_back(5); };
    callbacks.reset_timed_sprites = [&] { resets.push_back(6); };
    callbacks.reset_targeted_sweep = [&] { resets.push_back(7); };
    callbacks.reset_linear_sweep_a = [&] { resets.push_back(8); };
    callbacks.reset_linear_sweep_b = [&] { resets.push_back(9); };
    callbacks.reset_final_projection = [&] { resets.push_back(10); };
    callbacks.render_scroll_text = [&] { ++scroll_text; };
    callbacks.render_late_text = [&] { ++late_text; };

    if (run_title_intro_stage2(state, schedule, entries, callbacks) !=
            TitleIntroStage2StopReason::source_complete ||
        first_frames != 0xA1U || second_frames != 0x1F1U ||
        state.first_loop_frame != 0xA1U || state.second_loop_frame != 0x1F1U ||
        state.x_offset != 0 || state.y_offset != 0 ||
        state.x_phase != 0x0178U || state.y_phase != 0x00D1U ||
        actions.size() != 2U || resets.size() != 10U || scene_draws == 0U ||
        scroll_text != 1U || late_text != 1U ||
        state.animation_start_frame == 0U) {
        return 1;
    }
    return 0;
}

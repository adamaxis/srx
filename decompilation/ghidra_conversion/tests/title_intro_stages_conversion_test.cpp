#include "shadowrun_ghidra/title_intro_stages_conversion.hpp"

#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroStagesState state{};
    std::vector<std::string> order;
    TitleIntroStagesCallbacks callbacks{};
    callbacks.sample_controller = [&] { order.emplace_back("sample"); };
    callbacks.service_audio = [&] { order.emplace_back("audio"); };
    callbacks.run_schedule = [&] { order.emplace_back("schedule"); };
    callbacks.prepare_mode_entry = [&] { order.emplace_back("prepare"); };
    callbacks.run_stage2 = [&] { order.emplace_back("stage2"); };
    callbacks.run_orchestrator = [&] { order.emplace_back("orchestrator"); };
    callbacks.run_stage_loop = [&] { order.emplace_back("stage_loop"); };
    callbacks.step_palette_transition = [&] { order.emplace_back("palette"); };
    callbacks.enter_post_intro_mode = [&] { order.emplace_back("post_intro"); };

    const auto result = run_title_intro_stages(state, callbacks);
    const std::vector<std::string> expected{
        "sample", "audio", "schedule", "prepare", "audio", "stage2",
        "prepare", "audio", "orchestrator", "stage_loop", "palette",
        "post_intro", "sample", "palette", "prepare"};
    if (result != TitleIntroStagesStopReason::source_complete ||
        state.controller_marker != 0xFFU || state.completion_mode != 2U ||
        state.stop_marker != 0 || order != expected) {
        return 1;
    }

    TitleIntroStagesState interrupted{};
    std::vector<std::string> interrupted_order;
    TitleIntroStagesCallbacks interrupted_callbacks{};
    interrupted_callbacks.sample_controller =
        [&] { interrupted_order.emplace_back("sample"); };
    interrupted_callbacks.service_audio =
        [&] { interrupted_order.emplace_back("audio"); };
    interrupted_callbacks.run_schedule = [&] {
        interrupted_order.emplace_back("schedule");
        interrupted.stop_marker = 1;
    };
    interrupted_callbacks.prepare_mode_entry =
        [&] { interrupted_order.emplace_back("prepare"); };
    interrupted_callbacks.run_stage2 =
        [&] { interrupted_order.emplace_back("stage2"); };
    interrupted_callbacks.step_palette_transition =
        [&] { interrupted_order.emplace_back("palette"); };

    const auto interrupted_result =
        run_title_intro_stages(interrupted, interrupted_callbacks);
    const std::vector<std::string> interrupted_expected{
        "sample", "audio", "schedule", "sample", "palette", "prepare"};
    if (interrupted_result != TitleIntroStagesStopReason::interrupted ||
        interrupted.completion_mode != 0U ||
        interrupted_order != interrupted_expected) {
        return 1;
    }
    return 0;
}

#include "shadowrun_ghidra/menus/hospital_options_menu_conversion.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    HospitalOptionsMenuState state{};
    std::deque<HospitalOptionsInputFrame> frames{
        {false, false, true, false, false, false}, // Medical Attn.
        {false, true, false, false, false, false},  // Cyberware
        {false, false, true, false, false, false},
    };
    std::vector<std::string> order;
    int medical_calls = 0;
    int field_service_calls = 0;
    const auto result = run_hospital_options_menu(
        state,
        {[&] { order.emplace_back("reset"); },
         [&] { order.emplace_back("rebuild"); },
         [&](const HospitalOption option) {
             order.emplace_back(option == HospitalOption::cyberware
                                    ? "cursor-cyberware"
                                    : "cursor-other");
         },
         [&] { order.emplace_back("vblank"); },
         [&] { order.emplace_back("latch"); },
         [&] {
             order.emplace_back("sample");
             const auto input = frames.front();
             frames.pop_front();
             return input;
         },
         [&] { order.emplace_back("random"); },
         [&] {
             ++medical_calls;
             order.emplace_back("medical");
             return HospitalMenuHandlerOutcome::redraw_options;
         },
         [&] {
             ++field_service_calls;
             order.emplace_back("field-service");
         },
         {},
         {}});

    if (result.exit != HospitalOptionsMenuExit::returned_to_mode_exit ||
        result.final_selection != HospitalOption::cyberware ||
        state.menu_result != -1 || state.mode_result != 1U ||
        medical_calls != 1 || field_service_calls != 1 ||
        order.front() != "reset" || order[1] != "rebuild" ||
        order[2] != "cursor-other" || order[3] != "vblank" ||
        order[4] != "latch" || order[5] != "sample" ||
        order[6] != "random" || order[7] != "medical" ||
        order[8] != "rebuild" || order[9] != "cursor-other" ||
        order.back() != "field-service") {
        return 1;
    }

    HospitalOptionsMenuState priority_state{};
    std::deque<HospitalOptionsInputFrame> priority_frames{
        {true, true, false, false, false, false},
        {false, false, false, false, true, true},
        {false, false, false, false, false, true},
    };
    int action_calls = 0;
    const auto cancelled = run_hospital_options_menu(
        priority_state,
        {{},
         {},
         {},
         {},
         {},
         [&] {
             const auto input = priority_frames.front();
             priority_frames.pop_front();
             return input;
         },
         {},
         {},
         {},
         {},
         [&] { ++action_calls; }});
    if (cancelled.exit != HospitalOptionsMenuExit::cancelled ||
        cancelled.final_selection != HospitalOption::leave_hospital ||
        action_calls != 1) {
        return 2;
    }

    LeaveHospitalPromptState prompt_state{};
    std::deque<HospitalOptionsInputFrame> prompt_frames{
        {true, false, false, false, false, false},
        {false, false, false, true, false, false},
    };
    std::vector<std::uint8_t> cursor_history;
    const auto prompt_result = run_leave_hospital_prompt(
        prompt_state,
        {[&] {},
         [&](const std::uint8_t selection) { cursor_history.push_back(selection); },
         {},
         {},
         [&] {
             const auto input = prompt_frames.front();
             prompt_frames.pop_front();
             return input;
         },
         {}});
    if (prompt_result != LeaveHospitalPromptResult::leave_hospital ||
        prompt_state.selection != 0U ||
        cursor_history != std::vector<std::uint8_t>{1U, 0U}) {
        return 3;
    }

    return 0;
}

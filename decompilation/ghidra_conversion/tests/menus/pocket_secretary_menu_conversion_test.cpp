#include "shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::string> order;
    PocketSecretaryMenuState state{};
    initialize_pocket_secretary_menu(
        state, false,
        {[&] { order.emplace_back("assets"); },
         [&] { order.emplace_back("grid"); },
         [&] { order.emplace_back("text"); },
         [&] { order.emplace_back("numbers"); },
         {},
         {}});
    if (!expect(state.open && state.selection == 0U &&
                state.maximum_selection == 4U &&
                order == std::vector<std::string>{"assets", "grid", "text",
                                                  "numbers"})) {
        return 1;
    }

    if (!expect(apply_pocket_secretary_input(state, 0x0100U, {}) ==
                    PocketSecretaryMenuEvent::selection_moved &&
                state.selection == 4U &&
                apply_pocket_secretary_input(state, 0x0200U, {}) ==
                    PocketSecretaryMenuEvent::selection_moved &&
                state.selection == 0U)) {
        return 2;
    }

    std::vector<std::string> actions;
    std::uint8_t dispatched_selection = 0xFFU;
    if (!expect(apply_pocket_secretary_input(
                    state, 0x4000U,
                    {{}, {}, {}, {},
                     [&](const std::uint8_t selection) {
                         dispatched_selection = selection;
                         actions.emplace_back("dispatch");
                         return true;
                     },
                     {}}) == PocketSecretaryMenuEvent::action_dispatched &&
                dispatched_selection == 0U &&
                actions == std::vector<std::string>{"dispatch"})) {
        return 3;
    }

    bool mode_called = false;
    if (!expect(apply_pocket_secretary_input(
                    state, 0x2000U,
                    {{}, {}, {}, {}, {}, [&] { mode_called = true; }}) ==
                    PocketSecretaryMenuEvent::mode_action &&
                mode_called)) {
        return 4;
    }

    begin_pocket_secretary_cyberdeck_unavailable(state);
    if (!expect(state.open && state.cyberdeck_unavailable_wait &&
                !state.group_items_wait &&
                apply_pocket_secretary_input(state, 0U, {}) ==
                    PocketSecretaryMenuEvent::waiting &&
                state.cyberdeck_unavailable_wait &&
                apply_pocket_secretary_input(state, 0x8000U, {}) ==
                    PocketSecretaryMenuEvent::subview_dismissed &&
                state.open && !state.cyberdeck_unavailable_wait)) {
        return 5;
    }

    begin_pocket_secretary_group_items(state, 0U);
    if (!expect(state.open && !state.cyberdeck_unavailable_wait &&
                state.group_items_wait && state.group_item_flags == 0U &&
                apply_pocket_secretary_input(state, 0U, {}) ==
                    PocketSecretaryMenuEvent::waiting &&
                state.group_items_wait &&
                apply_pocket_secretary_input(state, 0x4000U, {}) ==
                    PocketSecretaryMenuEvent::subview_dismissed &&
                state.open && !state.group_items_wait)) {
        return 6;
    }
    if (!expect(pocket_secretary_group_item_display_order(
                    (std::uint32_t{1} << 14U) | (std::uint32_t{1} << 1U) |
                    std::uint32_t{1}) ==
                std::vector<std::size_t>{14U, 1U, 0U})) {
        return 7;
    }

    begin_pocket_secretary_notebook(state, false,
                                   std::array<bool, 3>{true, false, true});
    if (!expect(state.notebook.open && state.notebook.option_count == 4U &&
                state.notebook.selection == 0U &&
                state.notebook.visible_options[0] ==
                    NotebookMenuOption::current_run &&
                state.notebook.visible_options[1] ==
                    NotebookMenuOption::tips_and_clues &&
                state.notebook.visible_options[2] ==
                    NotebookMenuOption::contacts &&
                state.notebook.visible_options[3] ==
                    NotebookMenuOption::shadowrunners &&
                apply_pocket_secretary_input(state, 0x4000U, {}) ==
                    PocketSecretaryMenuEvent::notebook_current_run_no_run &&
                state.notebook.no_current_run_wait &&
                apply_pocket_secretary_input(state, 0U, {}) ==
                    PocketSecretaryMenuEvent::waiting &&
                state.notebook.no_current_run_wait &&
                apply_pocket_secretary_input(state, 0x8000U, {}) ==
                    PocketSecretaryMenuEvent::notebook_response_dismissed &&
                state.notebook.open &&
                !state.notebook.no_current_run_wait)) {
        return 10;
    }
    if (!expect(apply_pocket_secretary_input(state, 0x0100U, {}) ==
                    PocketSecretaryMenuEvent::notebook_selection_moved &&
                state.notebook.selection == 3U &&
                apply_pocket_secretary_input(state, 0x0200U, {}) ==
                    PocketSecretaryMenuEvent::notebook_selection_moved &&
                state.notebook.selection == 0U &&
                apply_pocket_secretary_input(state, 0x8000U, {}) ==
                    PocketSecretaryMenuEvent::notebook_returned &&
                !state.notebook.open && state.open &&
                state.selection == 0U)) {
        return 11;
    }

    PocketSecretaryMenuState tips_state{};
    initialize_pocket_secretary_menu(tips_state, false, {});
    begin_pocket_secretary_notebook(tips_state, false,
                                   std::array<bool, 3>{true, false, false});
    tips_state.notebook.selection = 1U;
    const std::array<std::uint32_t, 3> tip_flags{
        0x00000005U, 0x00000002U, 0x80000000U};
    begin_pocket_secretary_notebook_tips(tips_state, tip_flags);
    const auto& tips = tips_state.notebook.tips;
    if (!expect(tips.open && tips.visible_count == 4U &&
                tips.selection == 0U &&
                tips.visible_source_indices[0] == 0U &&
                tips.visible_source_indices[1] == 2U &&
                tips.visible_source_indices[2] == 33U &&
                tips.visible_source_indices[3] == 95U)) {
        return 16;
    }
    if (!expect(apply_pocket_secretary_input(tips_state, 0x0400U, {}) ==
                    PocketSecretaryMenuEvent::
                        notebook_tips_selection_moved &&
                tips_state.notebook.tips.selection == 0U &&
                apply_pocket_secretary_input(tips_state, 0x0800U, {}) ==
                    PocketSecretaryMenuEvent::
                        notebook_tips_selection_moved &&
                tips_state.notebook.tips.selection == 1U &&
                apply_pocket_secretary_input(tips_state, 0x0800U, {}) ==
                    PocketSecretaryMenuEvent::
                        notebook_tips_selection_moved &&
                apply_pocket_secretary_input(tips_state, 0x0800U, {}) ==
                    PocketSecretaryMenuEvent::
                        notebook_tips_selection_moved &&
                tips_state.notebook.tips.selection == 3U &&
                apply_pocket_secretary_input(tips_state, 0x0800U, {}) ==
                    PocketSecretaryMenuEvent::
                        notebook_tips_selection_moved &&
                tips_state.notebook.tips.selection == 3U)) {
        return 17;
    }
    bool tips_mode_action_called = false;
    if (!expect(apply_pocket_secretary_input(
                    tips_state, 0x2020U,
                    {{}, {}, {}, {}, {}, [&] {
                         tips_mode_action_called = true;
                     }}) ==
                    PocketSecretaryMenuEvent::notebook_tips_mode_action &&
                tips_mode_action_called && tips_state.notebook.tips.open &&
                apply_pocket_secretary_input(tips_state, 0x4000U, {}) ==
                    PocketSecretaryMenuEvent::notebook_tips_returned &&
                !tips_state.notebook.tips.open &&
                tips_state.notebook.open)) {
        return 18;
    }
    begin_pocket_secretary_notebook_tips(tips_state, tip_flags);
    if (!expect(apply_pocket_secretary_input(tips_state, 0x1010U, {}) ==
                    PocketSecretaryMenuEvent::notebook_tips_returned &&
                !tips_state.notebook.tips.open)) {
        return 19;
    }

    PocketSecretaryMenuState extended{};
    initialize_pocket_secretary_menu(extended, true, {});
    if (!expect(extended.maximum_selection == 5U &&
                apply_pocket_secretary_input(extended, 0x0100U, {}) ==
                    PocketSecretaryMenuEvent::selection_moved &&
                extended.selection == 5U)) {
        return 8;
    }

    if (!expect(apply_pocket_secretary_input(extended, 0x8000U, {}) ==
                    PocketSecretaryMenuEvent::exited &&
                !extended.open &&
                apply_pocket_secretary_input(extended, 0U, {}) ==
                    PocketSecretaryMenuEvent::exited)) {
        return 9;
    }

    DismissRunnerMenuState dismiss{};
    begin_dismiss_runner_menu(dismiss, {1U, 2U}, 2U);
    if (!expect(dismiss.open && dismiss.selection == 0U &&
                dismiss.candidate_count == 2U &&
                dismiss.candidate_slots ==
                    std::array<std::uint8_t, 2>{1U, 2U} &&
                apply_dismiss_runner_input(dismiss, 0x0100U) ==
                    DismissRunnerMenuEvent::selection_moved &&
                dismiss.selection == 2U &&
                apply_dismiss_runner_input(dismiss, 0x0200U) ==
                    DismissRunnerMenuEvent::selection_moved &&
                dismiss.selection == 0U)) {
        return 12;
    }
    if (!expect(apply_dismiss_runner_input(dismiss, 0x2000U) ==
                    DismissRunnerMenuEvent::abort_selected &&
                !dismiss.open)) {
        return 13;
    }
    begin_dismiss_runner_menu(dismiss, {1U, 2U}, 9U);
    if (!expect(dismiss.candidate_count == 2U &&
                apply_dismiss_runner_input(dismiss, 0x0200U) ==
                    DismissRunnerMenuEvent::selection_moved &&
                dismiss.selection == 1U &&
                apply_dismiss_runner_input(dismiss, 0x1000U) ==
                    DismissRunnerMenuEvent::runner_selected &&
                !dismiss.open)) {
        return 14;
    }
    begin_dismiss_runner_menu(dismiss, {}, 0U);
    if (!expect(apply_dismiss_runner_input(dismiss, 0x8000U) ==
                    DismissRunnerMenuEvent::returned &&
                !dismiss.open)) {
        return 15;
    }
    return 0;
}

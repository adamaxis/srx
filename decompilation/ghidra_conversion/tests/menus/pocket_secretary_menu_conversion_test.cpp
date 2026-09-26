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

    PocketSecretaryMenuState extended{};
    initialize_pocket_secretary_menu(extended, true, {});
    if (!expect(extended.maximum_selection == 5U &&
                apply_pocket_secretary_input(extended, 0x0100U, {}) ==
                    PocketSecretaryMenuEvent::selection_moved &&
                extended.selection == 5U)) {
        return 5;
    }

    if (!expect(apply_pocket_secretary_input(extended, 0x8000U, {}) ==
                    PocketSecretaryMenuEvent::exited &&
                !extended.open &&
                apply_pocket_secretary_input(extended, 0U, {}) ==
                    PocketSecretaryMenuEvent::exited)) {
        return 6;
    }
    return 0;
}

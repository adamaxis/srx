#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint16_t> inputs{0x0100U, 0x1000U};
    std::size_t input_index = 0;
    int labels = 0;
    int choices = 0;
    int moves = 0;
    int confirms = 0;
    const auto accepted = run_menu_confirmation_prompt(MenuConfirmationCallbacks{
        [&] { ++labels; },
        [&] { ++choices; },
        {},
        {},
        [&] { return inputs[input_index++]; },
        [&] { ++moves; },
        [&] { ++confirms; },
    });
    if (!expect(accepted.closed && accepted.accepted &&
                accepted.selected_option == 0U && labels == 1 && choices == 2 &&
                moves == 1 && confirms == 1)) {
        return 1;
    }

    inputs = {0x4000U};
    input_index = 0;
    const auto rejected = run_menu_confirmation_prompt(MenuConfirmationCallbacks{
        {}, {}, {}, {}, [&] { return inputs[input_index++]; }, {}, {}
    });
    if (!expect(rejected.closed && !rejected.accepted &&
                rejected.selected_option == 1U)) {
        return 2;
    }

    inputs = {0x1000U};
    input_index = 0;
    const auto default_rejected = run_menu_confirmation_prompt(
        MenuConfirmationCallbacks{{}, {}, {}, {},
                                  [&] { return inputs[input_index++]; }, {}, {}});
    if (!expect(default_rejected.closed && !default_rejected.accepted &&
                default_rejected.selected_option == 1U)) {
        return 3;
    }

    return 0;
}

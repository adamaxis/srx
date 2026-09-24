#include "shadowrun_ghidra/entity_action_state_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionState state{9U, 8U, 7U, 6U};
    std::vector<int> order;
    set_entity_action_state(state, 0U, 3U, 4U,
                            [&order] { order.push_back(1); },
                            [&order] { order.push_back(2); });
    if (state.action_code != 0U || state.state_code != 3U ||
        state.reset_marker != 0U || state.active_marker != 7U ||
        order != std::vector<int>{1, 2}) {
        return 1;
    }

    set_entity_action_state(state, 5U, 3U, 4U,
                            [&order] { order.push_back(3); },
                            [&order] { order.push_back(4); });
    if (state.action_code != 5U || state.state_code != 4U ||
        state.active_marker != 0U || state.reset_marker != 0U ||
        order != std::vector<int>{1, 2, 3, 4}) {
        return 2;
    }
    return 0;
}

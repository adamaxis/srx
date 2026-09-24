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

    EntityActionCandidateDispatchState dispatch{9U, 0x4BU, false};
    dispatch_entity_action_candidates(
        dispatch, [&order] { order.push_back(5); },
        [&order] { order.push_back(6); });
    if (dispatch.service_marker != 0U ||
        order != std::vector<int>{1, 2, 3, 4, 5}) {
        return 3;
    }

    dispatch.alternate_mode = true;
    dispatch_entity_action_candidates(
        dispatch, [&order] { order.push_back(7); },
        [&order] { order.push_back(8); });
    if (order != std::vector<int>{1, 2, 3, 4, 5, 7, 8}) {
        return 4;
    }

    advance_entity_action_threshold([&order] { order.push_back(9); },
                                    [&order] { order.push_back(10); });
    if (order != std::vector<int>{1, 2, 3, 4, 5, 7, 8, 9, 10}) {
        return 5;
    }
    return 0;
}

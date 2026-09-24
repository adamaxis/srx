#include "shadowrun_ghidra/entity_action_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    service_entity_action_menu([&order] { order.push_back(1); },
                               [&order] { order.push_back(2); });
    render_entity_action_menu([&order] { order.push_back(3); },
                              [&order] { order.push_back(4); },
                              [&order] { order.push_back(5); });
    if (order != std::vector<int>{1, 2, 3, 4, 5}) {
        return 1;
    }

    EntityActionMenuState state{2U};
    advance_entity_action_menu(state, [&order] { order.push_back(6); },
                               [&order] { order.push_back(7); },
                               [&order] { order.push_back(8); });
    state.available_count = 1U;
    advance_entity_action_menu(state, [&order] { order.push_back(9); },
                               [&order] { order.push_back(10); },
                               [&order] { order.push_back(11); });
    if (order != std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) {
        return 2;
    }
    return 0;
}

#include "shadowrun_ghidra/field/mode_display_reset_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeDisplayResetState state{9U, 8U};
    std::vector<int> order;
    reset_mode_display_buffers(
        state,
        {[&order] { order.push_back(1); },
         [&order] { order.push_back(2); },
         [&order] { order.push_back(3); },
         [&order] { order.push_back(4); },
         [&order] { order.push_back(5); },
         [&order] { order.push_back(6); }});
    if (state.primary_transfer_marker != 0U ||
        state.secondary_transfer_marker != 0U ||
        order != std::vector<int>{1, 2, 3, 4, 5, 6}) {
        return 1;
    }
    return 0;
}

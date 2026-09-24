#include "shadowrun_ghidra/mode_resource_variant_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;
    ModeResourceVariantState state{};
    std::vector<int> order;
    prepare_mode_resource_variant_b(
        state, [&] { order.push_back(1); }, [&] { order.push_back(2); });
    if (!state.resource_state_refreshed || !state.hud_panel_requested ||
        order != std::vector<int>{1, 2}) {
        return 1;
    }
    int transfers = 0;
    prepare_mode_resource_variant_a(state, [&] { ++transfers; });
    if (state.wrapped_transfer_count != 0x50U || transfers != 0x50) {
        return 2;
    }
    return 0;
}

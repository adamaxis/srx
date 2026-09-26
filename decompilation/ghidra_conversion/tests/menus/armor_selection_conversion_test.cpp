#include "shadowrun_ghidra/menus/armor_selection_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    ArmorModifierTable modifiers{};
    modifiers.primary[1] = 3U;
    modifiers.secondary[1] = 5U;
    if (!expect(subtract_armor_primary_modifier(0x120AU, 2U, modifiers) ==
                    0x1207U &&
                subtract_armor_secondary_modifier(0x120AU, 2U, modifiers) ==
                    0x1205U &&
                subtract_armor_primary_modifier(0x120AU, 0U, modifiers) ==
                    0x120AU &&
                subtract_armor_secondary_modifier(0x1202U, 2U, modifiers) ==
                    0x12FDU)) {
        return 1;
    }

    ArmorSelectionState state{2U, 4U};
    std::vector<std::string> order;
    const ArmorSelectionCallbacks callbacks{
        [&] { order.emplace_back("text"); },
        [&] { order.emplace_back("first"); },
        [&] { order.emplace_back("grid"); },
        [&] { order.emplace_back("second"); },
        [&] { order.emplace_back("empty"); },
    };
    const auto preview =
        apply_armor_selection(state, 0x2EU, true, callbacks);
    if (!expect(preview == ArmorSelectionResult::previewed_and_restored &&
                state.equipped_armor_index == 2U &&
                state.selected_inventory_phase == 4U &&
                order == std::vector<std::string>{"text", "text", "first",
                                                   "grid", "second", "grid"})) {
        return 2;
    }

    order.clear();
    const auto refresh =
        apply_armor_selection(state, 0x2EU, false, callbacks);
    if (!expect(refresh == ArmorSelectionResult::refreshed_current &&
                order == std::vector<std::string>{"text", "text", "first",
                                                   "grid", "second", "grid"})) {
        return 3;
    }

    ArmorSelectionState empty{};
    int empty_messages = 0;
    const auto rejected = apply_armor_selection(
        empty, 0x2CU, false,
        ArmorSelectionCallbacks{ {}, {}, {}, {}, [&] { ++empty_messages; } });
    if (!expect(rejected == ArmorSelectionResult::rejected_no_equipped_armor &&
                empty_messages == 1)) {
        return 4;
    }

    if (!expect(apply_armor_selection(state, 0x2BU, true, {}) ==
                    ArmorSelectionResult::rejected_item_code &&
                state.equipped_armor_index == 2U &&
                state.selected_inventory_phase == 4U)) {
        return 5;
    }
    return 0;
}

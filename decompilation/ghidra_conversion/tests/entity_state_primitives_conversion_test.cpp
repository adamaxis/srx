#include "shadowrun_ghidra/entity_state_primitives_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint8_t, 16> slots{};
    slots.fill(0xFFU);
    initialize_entity_slot_order(slots);
    for (std::uint8_t index = 0; index < slots.size(); ++index) {
        if (!expect(slots[index] == index)) {
            return 1;
        }
    }

    EntityReactionState state{0x20U, false};
    schedule_entity_collision_recheck(state);
    if (!expect(state.flags == 0x21U && state.reevaluate_collision)) {
        return 2;
    }

    schedule_entity_collision_recheck(state);
    if (!expect(state.flags == 0x21U && state.reevaluate_collision)) {
        return 3;
    }

    std::array<std::uint8_t, 18> effects{};
    effects.fill(1U);
    effects[7] = 0U;
    if (!expect(allocate_first_free_effect_slot(effects) == 7U &&
                effects[7] == 1U)) {
        return 4;
    }
    if (!expect(allocate_first_free_effect_slot(effects) == 18U)) {
        return 5;
    }

    return 0;
}

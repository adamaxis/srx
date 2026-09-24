#include "shadowrun_ghidra/entity_state_primitives_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void initialize_entity_slot_order(
    std::array<std::uint8_t, 16>& slot_order) {
    for (std::uint8_t index = 0; index < slot_order.size(); ++index) {
        slot_order[index] = index;
    }
}

void schedule_entity_collision_recheck(EntityReactionState& state) {
    state.flags = static_cast<std::uint8_t>(state.flags | 0x01U);
    state.reevaluate_collision = true;
}

std::uint8_t allocate_first_free_effect_slot(
    std::array<std::uint8_t, 18>& slot_state) {
    for (std::size_t index = 0; index < slot_state.size(); ++index) {
        if (slot_state[index] == 0U) {
            slot_state[index] = 1U;
            return static_cast<std::uint8_t>(index);
        }
    }
    return static_cast<std::uint8_t>(slot_state.size());
}

} // namespace shadowrun::ghidra_conversion

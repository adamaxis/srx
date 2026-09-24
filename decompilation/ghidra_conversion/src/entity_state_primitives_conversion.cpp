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

} // namespace shadowrun::ghidra_conversion

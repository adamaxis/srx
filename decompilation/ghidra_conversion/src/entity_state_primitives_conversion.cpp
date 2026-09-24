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

void clear_effect_slot(
    std::array<std::uint8_t, 18>& slot_state,
    const std::size_t slot_index) {
    if (slot_index < slot_state.size()) {
        slot_state[slot_index] = 0U;
    }
}

void clear_effect_slots(std::array<std::uint8_t, 18>& slot_state) {
    slot_state.fill(0U);
}

void clear_entity_effect_pending_records(
    EntityEffectPendingTableState& state) {
    state.global_marker = false;
    for (auto& flags : state.pending_flags) {
        flags = static_cast<std::uint8_t>(flags & 0xFEU);
    }
}

void initialize_entity_behavior_record(EntityBehaviorRecordState& state) {
    state = EntityBehaviorRecordState{
        static_cast<std::int16_t>(-12),
        1U,
        0x1000U,
        0x4000U,
        0x01U,
        EntityRecordContinuation::BehaviorUpdate};
}

} // namespace shadowrun::ghidra_conversion

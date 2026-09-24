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

    EntityBehaviorRecordState behavior_record{
        42, 42U, 42U, 42U, 0xFFU,
        EntityRecordContinuation::BehaviorUpdate};
    initialize_entity_behavior_record(behavior_record);
    if (!expect(behavior_record.vertical_offset == -12 &&
                behavior_record.animation_state == 1U &&
                behavior_record.behavior_timer == 0x1000U &&
                behavior_record.step_interval == 0x4000U &&
                behavior_record.flags == 0x01U)) {
        return 6;
    }

    return 0;
}

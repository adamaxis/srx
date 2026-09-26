#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"

#include <array>
#include <algorithm>

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

    std::array<std::uint8_t, 18> record_effects{};
    record_effects[0] = 1U;
    const auto record_allocation =
        allocate_first_free_effect_slot_record(record_effects);
    if (!expect(record_allocation.slot == 1U &&
                record_allocation.record_parameter == 0x0061U &&
                !record_allocation.table_full && record_effects[1] == 1U)) {
        return 12;
    }

    record_effects.fill(1U);
    const auto full_record_allocation =
        allocate_first_free_effect_slot_record(record_effects);
    if (!expect(full_record_allocation.slot == 18U &&
                full_record_allocation.record_parameter == 0x01F9U &&
                full_record_allocation.table_full)) {
        return 13;
    }

    std::array<std::uint8_t, 18> local_effects{};
    local_effects[0] = 1U;
    if (!expect(allocate_first_free_effect_slot_local(local_effects) == 1U &&
                local_effects[1] == 1U)) {
        return 10;
    }

    effects[3] = 1U;
    clear_effect_slot(effects, 3U);
    clear_effect_slot(effects, 99U);
    if (!expect(effects[3] == 0U && effects[4] == 1U)) {
        return 6;
    }
    clear_effect_slots(effects);
    if (!expect(std::all_of(effects.begin(), effects.end(),
                            [](const std::uint8_t value) { return value == 0U; }))) {
        return 7;
    }

    EntityEffectPendingTableState pending{true, {0x01U, 0x02U, 0x03U,
                                                  0x04U, 0x05U, 0x06U,
                                                  0x07U, 0x08U, 0x09U,
                                                  0x0AU, 0x0BU, 0x0CU,
                                                  0x0DU}};
    clear_entity_effect_pending_records(pending);
    if (!expect(!pending.global_marker && pending.pending_flags[0] == 0U &&
                pending.pending_flags[1] == 0x02U &&
                pending.pending_flags[12] == 0x0CU)) {
        return 8;
    }

    if (!expect(find_first_entity_action_below_threshold(
                    {0x15U, 0x12U, 0x0CU, 0x04U, 0x01U, 0x00U, 0x00U,
                     0x00U}) == 0x0CU &&
                find_first_entity_action_below_threshold(
                    {0x15U, 0x12U, 0x0DU, 0x20U, 0xFFU, 0x20U, 0x20U,
                     0x20U}) == 0U)) {
        return 9;
    }

    EntityRuntimeCleanupState cleanup{};
    cleanup.transient_flags.fill(0xFFU);
    cleanup.masked_flags.fill(0xFFU);
    clear_entity_runtime_fields(cleanup, 0x5AU);
    if (!expect(std::all_of(cleanup.transient_flags.begin(),
                            cleanup.transient_flags.end(),
                            [](const std::uint8_t value) { return value == 0U; }) &&
                std::all_of(cleanup.masked_flags.begin(), cleanup.masked_flags.end(),
                            [](const std::uint8_t value) { return value == 0x5AU; }))) {
        return 10;
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
        return 11;
    }

    return 0;
}

#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityReactionState {
    std::uint8_t flags = 0;
    bool reevaluate_collision = false;
};

enum class EntityRecordContinuation {
    BehaviorUpdate,
};

struct EntityBehaviorRecordState {
    std::int16_t vertical_offset = 0;
    std::uint16_t animation_state = 0;
    std::uint16_t behavior_timer = 0;
    std::uint16_t step_interval = 0;
    std::uint8_t flags = 0;
    EntityRecordContinuation continuation =
        EntityRecordContinuation::BehaviorUpdate;
};

// ROM 0xA7E4: initialize the sixteen entity references in slot order. The
// source stores record pointers at a fixed stride; host state keeps stable
// slot indices instead.
void initialize_entity_slot_order(
    std::array<std::uint8_t, 16>& slot_order);

// ROM 0xAE5E: mark the entity's pending reaction bit and schedule its
// collision-recheck continuation.
void schedule_entity_collision_recheck(EntityReactionState& state);

// ROM 0x1A2CC: claim the first free byte-sized effect slot. The source scans
// eighteen slots and returns 18 as the full-table sentinel.
std::uint8_t allocate_first_free_effect_slot(
    std::array<std::uint8_t, 18>& slot_state);

// ROM 0x15118: clear a newly allocated entity record's source fields and
// install the default behavior-update continuation and movement constants.
void initialize_entity_behavior_record(EntityBehaviorRecordState& state);

} // namespace shadowrun::ghidra_conversion

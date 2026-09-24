#pragma once

#include <array>
#include <cstddef>
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

// ROM 0x1A304: clear one effect-slot occupancy byte. Out-of-range host
// indices are ignored instead of exposing the source's unchecked pointer.
void clear_effect_slot(
    std::array<std::uint8_t, 18>& slot_state,
    std::size_t slot_index);

// ROM 0x1A310: clear the complete eighteen-slot effect table.
void clear_effect_slots(std::array<std::uint8_t, 18>& slot_state);

struct EntityEffectPendingTableState {
    bool global_marker = false;
    std::array<std::uint8_t, 13> pending_flags{};
};

// ROM 0x1A322: clear the global marker and bit 0 in the thirteen source
// entity records used by the effect update pass.
void clear_entity_effect_pending_records(EntityEffectPendingTableState& state);

// ROM 0x13AF2: scan the eight-byte entity action/item area and return the
// first value below the source threshold, or zero when no value qualifies.
std::uint8_t find_first_entity_action_below_threshold(
    const std::array<std::uint8_t, 8>& action_values,
    std::uint8_t threshold = 0x0DU);

struct EntityRuntimeCleanupState {
    std::array<std::uint8_t, 10> transient_flags{};
    std::array<std::uint8_t, 10> masked_flags{};
};

// ROM 0x05622: clear one transient byte in ten entity records and apply the
// caller mask to the paired per-record flags.
void clear_entity_runtime_fields(
    EntityRuntimeCleanupState& state,
    std::uint8_t mask);

// ROM 0x15118: clear a newly allocated entity record's source fields and
// install the default behavior-update continuation and movement constants.
void initialize_entity_behavior_record(EntityBehaviorRecordState& state);

} // namespace shadowrun::ghidra_conversion

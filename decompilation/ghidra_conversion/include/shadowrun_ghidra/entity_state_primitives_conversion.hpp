#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct EntityReactionState {
    std::uint8_t flags = 0;
    bool reevaluate_collision = false;
};

// ROM 0xA7E4: initialize the sixteen entity references in slot order. The
// source stores record pointers at a fixed stride; host state keeps stable
// slot indices instead.
void initialize_entity_slot_order(
    std::array<std::uint8_t, 16>& slot_order);

// ROM 0xAE5E: mark the entity's pending reaction bit and schedule its
// collision-recheck continuation.
void schedule_entity_collision_recheck(EntityReactionState& state);

} // namespace shadowrun::ghidra_conversion

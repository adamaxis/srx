#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityBoundaryTransitionState {
    bool linked_record_present = false;
    std::uint16_t timer = 0;
    std::uint32_t continuation = 0;
};

struct EntityBoundaryTransitionCallbacks {
    std::function<void()> reposition_linked_entity;
    std::function<void()> service_linked_entity;
};

// ROM 0x13E8C: service the linked-record branch, or install the source
// continuation while the unlinked timer remains within 0x3C frames.
void service_entity_boundary_transition(
    EntityBoundaryTransitionState& state,
    const EntityBoundaryTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

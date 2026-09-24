#include "shadowrun_ghidra/entity_boundary_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void service_entity_boundary_transition(
    EntityBoundaryTransitionState& state,
    const EntityBoundaryTransitionCallbacks& callbacks) {
    if (state.linked_record_present) {
        if (callbacks.reposition_linked_entity) {
            callbacks.reposition_linked_entity();
        }
        if (callbacks.service_linked_entity) {
            callbacks.service_linked_entity();
        }
        return;
    }

    if (state.timer <= 0x003CU) {
        state.continuation = 0xB45CU;
    }
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/entities/entity_animation_resource_conversion.hpp"

namespace shadowrun::ghidra_conversion {

EntityAnimationResourceGateResult run_entity_animation_resource_gate(
    EntityAnimationResourceGateState& state,
    const EntityAnimationResourceGateCallbacks& callbacks) {
    EntityAnimationResourceGateResult result{};

    if (state.global_entity_gate) {
        result.action =
            EntityAnimationResourceGateAction::blocked_by_global_gate;
        return result;
    }

    if (state.special_reaction_gate) {
        if (callbacks.schedule_world_activity_timeout) {
            callbacks.schedule_world_activity_timeout();
        }
        result.action = EntityAnimationResourceGateAction::timeout_scheduled;
        return result;
    }

    if ((state.entity_status & 0x01U) != 0U ||
        (state.entity_flags & 0x20U) == 0U) {
        result.action = EntityAnimationResourceGateAction::not_eligible;
        return result;
    }

    if (!callbacks.inspect_resource_decision) {
        result.action =
            EntityAnimationResourceGateAction::resource_decision_unresolved;
        return result;
    }
    result.resource_decision_called = true;
    if (callbacks.inspect_resource_decision()) {
        result.action =
            EntityAnimationResourceGateAction::resource_decision_blocked;
        return result;
    }

    auto difference = static_cast<std::uint16_t>(
        state.entity_reference - state.global_reference);
    if (static_cast<std::int16_t>(difference) < 0) {
        difference = static_cast<std::uint16_t>(0U - difference);
    }
    result.absolute_reference_distance = difference;
    if (difference >= 0x4000U) {
        result.action = EntityAnimationResourceGateAction::out_of_range;
        return result;
    }

    if (state.resource_mode != 2U) {
        state.entity_status = static_cast<std::uint8_t>(
            state.entity_status | 0x01U);
        result.action = EntityAnimationResourceGateAction::marked_ready;
        return result;
    }

    if ((state.object_flags & 0x80U) != 0U) {
        result.action = EntityAnimationResourceGateAction::object_busy;
        return result;
    }

    if (callbacks.begin_resource_load) {
        callbacks.begin_resource_load();
    }
    state.entity_status = static_cast<std::uint8_t>(
        state.entity_status | 0x01U);
    result.resource_load_started = true;
    result.action = EntityAnimationResourceGateAction::resource_queued;
    return result;
}

} // namespace shadowrun::ghidra_conversion

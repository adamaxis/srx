#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationResourceGateState {
    bool global_entity_gate = false;  // FFDF84
    bool special_reaction_gate = false; // FFE16F
    std::uint8_t entity_status = 0;   // record +0x9E
    std::uint8_t entity_flags = 0;    // record +0x10
    std::uint16_t entity_reference = 0; // record +0x2E
    std::uint16_t global_reference = 0; // FF13AE
    std::uint16_t resource_mode = 0;  // FFDF40
    std::uint8_t object_flags = 0;    // object +0x11
};

enum class EntityAnimationResourceGateAction {
    blocked_by_global_gate,
    timeout_scheduled,
    not_eligible,
    resource_decision_blocked,
    resource_decision_unresolved,
    out_of_range,
    object_busy,
    marked_ready,
    resource_queued,
};

struct EntityAnimationResourceGateCallbacks {
    std::function<void()> schedule_world_activity_timeout;
    // ROM 0x14D8E returns its decision in CCR carry. A true result is the
    // source's blocked/stop path.
    std::function<bool()> inspect_resource_decision;
    std::function<void()> begin_resource_load;
};

struct EntityAnimationResourceGateResult {
    EntityAnimationResourceGateAction action =
        EntityAnimationResourceGateAction::blocked_by_global_gate;
    std::uint16_t absolute_reference_distance = 0;
    bool resource_decision_called = false;
    bool resource_load_started = false;
};

// ROM 0x13F76: guard resource work by the global/reaction gates, require the
// entity's status/flag eligibility, run the 0x14D8E source decision, compare
// the signed reference distance, and either mark the entity ready or queue
// the mode-2 resource load.
EntityAnimationResourceGateResult run_entity_animation_resource_gate(
    EntityAnimationResourceGateState& state,
    const EntityAnimationResourceGateCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

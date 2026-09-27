#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;

struct Observations {
    std::vector<std::string> calls;
    std::size_t active_continuations = 0;
    std::size_t fallback_calls = 0;
    std::size_t collision_dispatches = 0;
    std::size_t action_advances = 0;
    std::size_t completed_transitions = 0;
    std::size_t cleared_effects = 0;
};

void record(Observations& observations, const char* call) {
    observations.calls.emplace_back(call);
}

EntityBehaviorFrameCallbacks make_callbacks(Observations& observations) {
    EntityBehaviorFrameCallbacks callbacks{};
    callbacks.dispatch_entity_action_candidates = [&observations] {
        record(observations, "dispatch-candidates");
    };
    callbacks.noop_entity_action_hook = [&observations](
                                              EntityBehaviorFrameRecord&) {
        record(observations, "noop-action-hook");
        return static_cast<std::uint8_t>(0xA5U);
    };
    callbacks.complete_entity_behavior_transition =
        [&observations](EntityBehaviorFrameRecord&) {
            ++observations.completed_transitions;
            record(observations, "complete-transition");
        };
    callbacks.clear_effect_slot_state = [&observations](
                                            EntityBehaviorFrameRecord&) {
        ++observations.cleared_effects;
        record(observations, "clear-effect-slot");
    };
    callbacks.handle_mode_action_event = [&observations](
                                             EntityBehaviorFrameRecord&) {
        record(observations, "handle-mode-action");
    };
    callbacks.advance_entity_behavior_action =
        [&observations](EntityBehaviorFrameRecord&) {
            ++observations.action_advances;
            record(observations, "advance-action");
        };
    callbacks.enter_entity_behavior_transition =
        [&observations](EntityBehaviorFrameRecord&) {
            record(observations, "enter-transition");
        };
    callbacks.dispatch_entity_collision_behavior =
        [&observations](EntityBehaviorFrameRecord&) {
            ++observations.collision_dispatches;
            record(observations, "dispatch-collision");
        };
    callbacks.apply_mode_five_boundary_motion =
        [&observations](EntityBehaviorFrameRecord&) {
            record(observations, "mode-five-boundary");
        };
    callbacks.resolve_entity_proximity_interactions =
        [&observations](EntityBehaviorFrameRecord&) {
            record(observations, "resolve-proximity");
        };
    callbacks.clear_entity_effect_if_forced = [&observations](
                                                  EntityBehaviorFrameRecord&) {
        record(observations, "clear-forced-effect");
    };
    callbacks.resolve_entity_fallback_movement =
        [&observations](EntityBehaviorFrameRecord& record_state) {
            ++observations.fallback_calls;
            assert(record_state.continuation_kind ==
                   EntityBehaviorContinuationKind::fallback_movement);
            record_state.animation_elapsed = 7;
            record(observations, "fallback-movement");
        };
    callbacks.run_entity_continuation =
        [&observations](EntityBehaviorFrameRecord& record_state) {
            ++observations.active_continuations;
            if (record_state.continuation_kind ==
                EntityBehaviorContinuationKind::ordinary) {
                record(observations, "ordinary-continuation");
            } else {
                record(observations, "special-continuation");
            }
        };
    return callbacks;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<EntityBehaviorFrameRecord, 16> records{};

    // Bit 2 enters the transition service. The source's bset path starts the
    // transition, then the common tail performs boundary/proximity work and
    // temporarily routes the requested fallback movement.
    records[0].active = true;
    records[0].behavior_flags = 0x04U;
    records[0].record_flags = 0x06U;
    records[0].animation_elapsed = 1;

    // A negative transition timer with B0D4/B10C completion performs the
    // completion/effect-clear branch before the common continuation tail.
    records[1].active = true;
    records[1].behavior_flags = 0x04U;
    records[1].animation_elapsed = -1;
    records[1].continuation_kind =
        EntityBehaviorContinuationKind::transition_b0d4;

    // Bit 1 routes directly to the behavior-action service even without the
    // collision/transition bit. The +0x11 bit-4 hook result is also retained.
    records[2].active = true;
    records[2].behavior_flags = 0x06U;
    records[2].record_flags = 0x10U;
    records[2].animation_elapsed = 2;
    records[2].transition_rate_threshold = 1U;
    records[2].transition_gate_marker = 1U;

    // Bit 6 suppresses the transition branch but leaves the common collision
    // tail active, matching the source's 0xA922 jump to 0xAA74.
    records[3].active = true;
    records[3].behavior_flags = 0x44U;
    records[3].animation_elapsed = 9;

    // A nonzero +0x9C rate gate and +0x4C marker reach AA54. With no action
    // bit, the source dispatches collision behavior because bit 2 is set.
    records[4].active = true;
    records[4].behavior_flags = 0x04U;
    records[4].animation_elapsed = 2;
    records[4].transition_rate_threshold = 1U;
    records[4].transition_gate_marker = 1U;

    // The negative-source path has the same type-0x0C/0x80 hold when the
    // nonzero rate gate is active. Seed 0x81 because the service decrements
    // +0x16 before testing it.
    records[5].active = true;
    records[5].behavior_flags = 0x14U;
    records[5].animation_elapsed = static_cast<std::int16_t>(0x0081);
    records[5].entity_type = 0x000CU;
    records[5].transition_rate_threshold = 1U;

    Observations observations;
    const auto callbacks = make_callbacks(observations);
    assert(service_entity_behavior_frames(records, callbacks) ==
           EntityBehaviorFrameServiceResult::completed);
    assert(observations.calls.front() == "dispatch-candidates");
    assert(observations.fallback_calls == 1U);
    assert(observations.collision_dispatches == 1U);
    assert(observations.action_advances == 1U);
    assert(observations.completed_transitions == 1U);
    assert(observations.cleared_effects == 1U);
    assert(observations.active_continuations == 6U);
    assert(records[0].continuation_kind ==
           EntityBehaviorContinuationKind::ordinary);
    assert(records[0].record_flags == 0x00U);
    assert(records[2].action_result == 0xA5U);
    assert(records[2].animation_elapsed == 1);
    assert(records[5].animation_elapsed == 0x0080);

    auto missing_callbacks = callbacks;
    missing_callbacks.run_entity_continuation = {};
    assert(service_entity_behavior_frames(records, missing_callbacks) ==
           EntityBehaviorFrameServiceResult::callback_unresolved);
    return 0;
}

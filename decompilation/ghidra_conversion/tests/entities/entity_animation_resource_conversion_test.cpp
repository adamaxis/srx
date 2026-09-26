#include "shadowrun_ghidra/entities/entity_animation_resource_conversion.hpp"

#include <cassert>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<int> calls;
    const EntityAnimationResourceGateCallbacks callbacks{
        [&] { calls.push_back(1); },
        [&] {
            calls.push_back(2);
            return false;
        },
        [&] { calls.push_back(3); }};

    EntityAnimationResourceGateState blocked{true, false, 0U, 0x20U};
    assert(run_entity_animation_resource_gate(blocked, callbacks).action ==
           EntityAnimationResourceGateAction::blocked_by_global_gate);
    assert(calls.empty());

    EntityAnimationResourceGateState timeout{false, true, 0U, 0x20U};
    assert(run_entity_animation_resource_gate(timeout, callbacks).action ==
           EntityAnimationResourceGateAction::timeout_scheduled);
    assert(calls == std::vector<int>{1});

    EntityAnimationResourceGateState ineligible{false, false, 1U, 0x20U};
    assert(run_entity_animation_resource_gate(ineligible, callbacks).action ==
           EntityAnimationResourceGateAction::not_eligible);
    ineligible.entity_status = 0U;
    ineligible.entity_flags = 0U;
    assert(run_entity_animation_resource_gate(ineligible, callbacks).action ==
           EntityAnimationResourceGateAction::not_eligible);

    EntityAnimationResourceGateState decision_blocked{
        false, false, 0U, 0x20U, 0x1000U, 0x0800U, 0U, 0U};
    EntityAnimationResourceGateCallbacks blocking_callbacks{
        {},
        [&] {
            calls.push_back(4);
            return true;
        },
        {}};
    assert(run_entity_animation_resource_gate(
               decision_blocked, blocking_callbacks)
               .action == EntityAnimationResourceGateAction::resource_decision_blocked);
    assert(calls.back() == 4);

    EntityAnimationResourceGateState unresolved{
        false, false, 0U, 0x20U, 0x1000U, 0x0800U, 0U, 0U};
    assert(run_entity_animation_resource_gate(unresolved, {}) .action ==
           EntityAnimationResourceGateAction::resource_decision_unresolved);

    EntityAnimationResourceGateState too_far{
        false, false, 0U, 0x20U, 0x5000U, 0x0800U, 0U, 0U};
    const auto far_result = run_entity_animation_resource_gate(
        too_far, callbacks);
    assert(far_result.action == EntityAnimationResourceGateAction::out_of_range &&
           far_result.absolute_reference_distance == 0x4800U);

    EntityAnimationResourceGateState ordinary{
        false, false, 0U, 0x20U, 0x1000U, 0x0800U, 0U, 0U};
    const auto ordinary_result = run_entity_animation_resource_gate(
        ordinary, callbacks);
    assert(ordinary_result.action == EntityAnimationResourceGateAction::marked_ready &&
           (ordinary.entity_status & 0x01U) != 0U);

    EntityAnimationResourceGateState busy{
        false, false, 0U, 0x20U, 0x1000U, 0x0800U, 2U, 0x80U};
    assert(run_entity_animation_resource_gate(busy, callbacks).action ==
           EntityAnimationResourceGateAction::object_busy);

    EntityAnimationResourceGateState queued{
        false, false, 0U, 0x20U, 0x1000U, 0x0800U, 2U, 0U};
    const auto queued_result = run_entity_animation_resource_gate(
        queued, callbacks);
    assert(queued_result.action == EntityAnimationResourceGateAction::resource_queued &&
           queued_result.resource_load_started &&
           (queued.entity_status & 0x01U) != 0U &&
           calls.back() == 3);
    return 0;
}

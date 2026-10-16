#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion {

bool FieldMapEventCoordinateScope::contains(
    const std::uint16_t x, const std::uint16_t y) const noexcept {
    return x >= min_x && x <= max_x && y >= min_y && y <= max_y;
}

FieldMapEventSystem::FieldMapEventSystem(
    std::vector<FieldMapEventRule> rules)
    : rules_(std::move(rules)) {}

void FieldMapEventSystem::reset_session() {
    fired_this_scene_visit_.clear();
    fired_this_session_.clear();
    active_scene_id_ = 0U;
    scene_visit_generation_ = 0U;
    collision_dispatch_count_ = 0U;
    last_collision_dispatch_.reset();
}

void FieldMapEventSystem::begin_scene_visit(const std::uint16_t scene_id) {
    active_scene_id_ = scene_id;
    ++scene_visit_generation_;
    fired_this_scene_visit_.clear();
}

std::vector<FieldMapEventActivation>
FieldMapEventSystem::dispatch_scene_entry(
    const FieldMapEventTriggerContext& context) {
    if (active_scene_id_ != context.scene_id) {
        begin_scene_visit(context.scene_id);
    }
    return evaluate(FieldMapEventTrigger::scene_entry, context);
}

std::vector<FieldMapEventActivation>
FieldMapEventSystem::evaluate_actor_scope(
    const FieldMapEventTriggerContext& context) {
    if (active_scene_id_ != context.scene_id) {
        begin_scene_visit(context.scene_id);
    }
    return evaluate(FieldMapEventTrigger::actor_within_scope, context);
}

FieldMapCollisionEventDispatch FieldMapEventSystem::record_collision(
    const FieldMapCollisionEventInput& input,
    const FieldMapEventPolicyDecision& decision) noexcept {
    ++collision_dispatch_count_;
    last_collision_dispatch_ = FieldMapCollisionEventDispatch{
        collision_dispatch_count_, input, decision};
    return *last_collision_dispatch_;
}

std::vector<FieldMapEventActivation> FieldMapEventSystem::evaluate(
    const FieldMapEventTrigger trigger,
    const FieldMapEventTriggerContext& context) {
    std::vector<FieldMapEventActivation> activations;
    if (!context.actor_present) {
        return activations;
    }

    for (const auto& rule : rules_) {
        if (rule.trigger != trigger || rule.scene_id != context.scene_id ||
            (context.state_flags & rule.required_state_flags) !=
                rule.required_state_flags ||
            (rule.scope.has_value() &&
             !rule.scope->contains(context.world_x, context.world_y))) {
            continue;
        }

        if (rule.repeat_policy ==
                FieldMapEventRepeatPolicy::once_per_scene_visit &&
            fired_this_scene_visit_.count(rule.id) != 0U) {
            continue;
        }
        if (rule.repeat_policy ==
                FieldMapEventRepeatPolicy::once_per_session &&
            fired_this_session_.count(rule.id) != 0U) {
            continue;
        }

        if (rule.repeat_policy ==
            FieldMapEventRepeatPolicy::once_per_scene_visit) {
            fired_this_scene_visit_.insert(rule.id);
        } else if (rule.repeat_policy ==
                   FieldMapEventRepeatPolicy::once_per_session) {
            fired_this_session_.insert(rule.id);
        }
        activations.push_back({rule.id, rule.action, rule.handler});
    }
    return activations;
}

} // namespace shadowrun::ghidra_conversion

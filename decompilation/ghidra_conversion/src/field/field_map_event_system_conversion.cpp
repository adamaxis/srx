#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion {

FieldMapEventPolicyDecision resolve_field_map_collision_event(
    const FieldMapCollisionEventInput& input) noexcept {
    FieldMapEventPolicyDecision decision{};
    decision.raw_collision_code = input.raw_collision_code;
    const auto event_code = static_cast<std::uint8_t>(
        input.raw_collision_code & 0x001FU);
    decision.source_event_code = event_code;
    decision.event_code = event_code;

    switch (event_code) {
    case 0x1FU:
        decision.action = FieldMapEventAction::begin_event_31_activity;
        return decision;
    case 0x1EU:
        decision.action = FieldMapEventAction::begin_event_30_activity;
        return decision;
    case 0x1DU:
        if (!input.activity_sequence_active) {
            decision.action = FieldMapEventAction::begin_event_29_activity;
        } else {
            decision.action = FieldMapEventAction::skip_world_interaction;
        }
        return decision;
    case 0x1CU:
        if (!input.activity_sequence_active) {
            decision.action = FieldMapEventAction::begin_event_28_activity;
        } else {
            decision.action = FieldMapEventAction::skip_world_interaction;
        }
        return decision;
    case 0x1BU:
        decision.action = FieldMapEventAction::begin_event_27_activity;
        return decision;
    case 0x1AU:
        decision.action = FieldMapEventAction::mark_projected_terrain;
        if ((input.raw_collision_code & 0x00FFU) != 0x00DAU &&
            !input.world_effect_control_bit) {
            decision.projected_terrain_event_published = true;
        }
        return decision;
    default:
        break;
    }

    if ((input.scene_id == 0x14U || input.world_mode == 2U) &&
        event_code >= 2U && event_code <= 5U &&
        !input.terrain_event_suppressed) {
        decision.action = FieldMapEventAction::begin_maglock_interaction;
        return decision;
    }

    if (event_code == 0x18U) {
        decision.action = FieldMapEventAction::begin_security_terminal;
        return decision;
    }

    if (event_code == 0x19U) {
        if (input.field_transition_context == 0x13U) {
            decision.action = FieldMapEventAction::begin_event_25_activity;
            return decision;
        }
        decision.rewrite_direction_probe = true;
        decision.direction_probe_value = -0x58;
        decision.rewrite_quantized_delta = true;
        decision.quantized_delta = 0x00A8U;
        decision.raw_collision_code = 0x00A8U;
        decision.event_code = 8U;
    }

    if ((input.field_global_flags & 0x50U) != 0U) {
        decision.action = FieldMapEventAction::begin_field_mode_exit;
        return decision;
    }

    decision.mode_event_code_published = true;
    if (input.world_mode == 2U && input.scene_id != 0x14U &&
        decision.event_code == 0U &&
        (input.interaction_gate != 0U ||
         input.activity_sequence_active)) {
        decision.action = FieldMapEventAction::skip_world_interaction;
        return decision;
    }

    decision.action = FieldMapEventAction::project_world_interaction;
    return decision;
}

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

FieldMapCollisionEventDispatch FieldMapEventSystem::dispatch_collision(
    const FieldMapCollisionEventInput& input) noexcept {
    ++collision_dispatch_count_;
    last_collision_dispatch_ = FieldMapCollisionEventDispatch{
        collision_dispatch_count_, input,
        resolve_field_map_collision_event(input)};
    return *last_collision_dispatch_;
}

FieldScriptRoute FieldMapEventSystem::resolve_script_route(
    const FieldScriptRouteContext& context) const noexcept {
    return resolve_field_script_route(context);
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
        activations.push_back({rule.id, rule.action});
    }
    return activations;
}

} // namespace shadowrun::ghidra_conversion

#pragma once

#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

namespace shadowrun::ghidra_conversion {

// A collision record is input to the map-event system. The policy result is a
// typed native action; callers bind that action to movement, dialogue, or a
// scene/activity transition without interpreting ROM addresses at runtime.
enum class FieldMapEventAction {
    project_world_interaction,
    skip_world_interaction,
    begin_event_31_activity,
    begin_event_30_activity,
    begin_event_29_activity,
    begin_event_28_activity,
    begin_event_27_activity,
    mark_projected_terrain,
    begin_maglock_interaction,
    begin_security_terminal,
    begin_event_25_activity,
    begin_field_mode_exit,
};

struct FieldMapCollisionEventInput {
    std::uint16_t scene_id = 0;
    std::uint16_t world_mode = 0;
    std::uint16_t field_transition_context = 0;
    std::uint16_t interaction_gate = 0;
    std::uint16_t raw_collision_code = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint8_t field_global_flags = 0;
    bool activity_sequence_active = false;
    bool terrain_event_suppressed = false;
    bool world_effect_control_bit = false;
};

struct FieldMapEventPolicyDecision {
    FieldMapEventAction action =
        FieldMapEventAction::project_world_interaction;
    std::uint8_t source_event_code = 0;
    std::uint8_t event_code = 0;
    std::uint16_t raw_collision_code = 0;
    bool mode_event_code_published = false;
    bool projected_terrain_event_published = false;
    bool rewrite_direction_probe = false;
    std::int8_t direction_probe_value = 0;
    bool rewrite_quantized_delta = false;
    std::uint16_t quantized_delta = 0;
};

FieldMapEventPolicyDecision resolve_field_map_collision_event(
    const FieldMapCollisionEventInput& input) noexcept;

enum class FieldMapEventTrigger {
    scene_entry,
    actor_within_scope,
};

enum class FieldMapEventRepeatPolicy {
    every_match,
    once_per_scene_visit,
    once_per_session,
};

enum class FieldMapEventScriptAction {
    start_new_game_opening_walk,
    dispatch_native_script,
};

enum class FieldMapEventStateFlag : std::uint32_t {
    new_game_opening_completed = 1U << 0U,
};

constexpr std::uint32_t field_map_event_flag(
    const FieldMapEventStateFlag flag) noexcept {
    return static_cast<std::uint32_t>(flag);
}

struct FieldMapEventCoordinateScope {
    std::uint16_t min_x = 0;
    std::uint16_t max_x = 0;
    std::uint16_t min_y = 0;
    std::uint16_t max_y = 0;

    bool contains(std::uint16_t x, std::uint16_t y) const noexcept;
};

struct FieldMapEventRule {
    std::uint32_t id = 0;
    std::uint16_t scene_id = 0;
    FieldMapEventTrigger trigger = FieldMapEventTrigger::scene_entry;
    std::optional<FieldMapEventCoordinateScope> scope;
    std::uint32_t required_state_flags = 0;
    FieldMapEventRepeatPolicy repeat_policy =
        FieldMapEventRepeatPolicy::once_per_scene_visit;
    FieldMapEventScriptAction action =
        FieldMapEventScriptAction::dispatch_native_script;
};

struct FieldMapEventTriggerContext {
    std::uint16_t scene_id = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint32_t state_flags = 0;
    bool actor_present = false;
};

struct FieldMapEventActivation {
    std::uint32_t event_id = 0;
    FieldMapEventScriptAction action =
        FieldMapEventScriptAction::dispatch_native_script;
};

struct FieldMapCollisionEventDispatch {
    std::uint32_t sequence = 0;
    FieldMapCollisionEventInput input{};
    FieldMapEventPolicyDecision decision{};
};

class FieldMapEventSystem final {
public:
    explicit FieldMapEventSystem(
        std::vector<FieldMapEventRule> rules = {});

    void reset_session();
    void begin_scene_visit(std::uint16_t scene_id);
    std::vector<FieldMapEventActivation> dispatch_scene_entry(
        const FieldMapEventTriggerContext& context);
    std::vector<FieldMapEventActivation> evaluate_actor_scope(
        const FieldMapEventTriggerContext& context);

    FieldMapCollisionEventDispatch dispatch_collision(
        const FieldMapCollisionEventInput& input) noexcept;
    FieldScriptRoute resolve_script_route(
        const FieldScriptRouteContext& context) const noexcept;

    std::uint32_t collision_dispatch_count() const noexcept {
        return collision_dispatch_count_;
    }
    const std::optional<FieldMapCollisionEventDispatch>&
    last_collision_dispatch() const noexcept {
        return last_collision_dispatch_;
    }
    std::uint16_t active_scene_id() const noexcept {
        return active_scene_id_;
    }
    std::uint32_t scene_visit_generation() const noexcept {
        return scene_visit_generation_;
    }

private:
    std::vector<FieldMapEventActivation> evaluate(
        FieldMapEventTrigger trigger,
        const FieldMapEventTriggerContext& context);

    std::vector<FieldMapEventRule> rules_;
    std::unordered_set<std::uint32_t> fired_this_scene_visit_;
    std::unordered_set<std::uint32_t> fired_this_session_;
    std::uint16_t active_scene_id_ = 0;
    std::uint32_t scene_visit_generation_ = 0;
    std::uint32_t collision_dispatch_count_ = 0;
    std::optional<FieldMapCollisionEventDispatch> last_collision_dispatch_;
};

std::vector<FieldMapEventRule> default_field_map_event_rules();

} // namespace shadowrun::ghidra_conversion

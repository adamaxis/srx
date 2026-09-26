#include "shadowrun_ghidra/entity_animation_spawn_probe_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

std::uint16_t fixed_point_integer_word(const std::int32_t value) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(value) >> 16U);
}

void finish_marker(EntityAnimationSpawnProbeState& state) {
    state.source_marker = 0;
}

} // namespace

EntityAnimationSpawnProbeResult probe_entity_spawn_collision_window(
    EntityAnimationSpawnProbeState& state,
    const EntityAnimationSpawnProbeRequest& request,
    const EntityAnimationSpawnProbeCallbacks& callbacks) {
    state = EntityAnimationSpawnProbeState{};
    state.source_marker = -1;
    state.linked_target = request.linked_target;

    // 0x14D8E enters 0x14FF8 with the protected-source marker active. The
    // existing typed 0x14FF8 conversion owns record initialization; this
    // routine only supplies its source-derived inputs and consumes the
    // resulting projected fields.
    initialize_entity_spawn_record(
        state.scratch_record,
        request.parent,
        0xFFFFU,
        true,
        state.linked_target,
        request.prepare_spawn);
    state.probe_position = EntityFixedPointPosition{
        static_cast<std::int32_t>(state.scratch_record.position_x),
        static_cast<std::int32_t>(state.scratch_record.position_y),
        state.scratch_record.heading_offset_x,
        state.scratch_record.heading_offset_y};

    EntityAnimationSpawnProbeResult result{};
    if (!callbacks.terrain_probe || !callbacks.collision_search) {
        finish_marker(state);
        result.action = EntityAnimationSpawnProbeAction::boundary_unresolved;
        result.boundary_unresolved = true;
        return result;
    }

    for (std::uint16_t remaining =
             static_cast<std::uint16_t>(kEntitySpawnProbeAttemptCount - 1U);
         ; --remaining) {
        ++state.attempts;
        const auto probe = probe_entity_world_position(
            state.probe_position, callbacks.terrain_probe);
        state.last_probe = probe;

        // 0x14DBE writes D5/D6 back to the scratch record before testing D0;
        // the host keeps that long-coordinate write in the typed probe
        // position because the source record's word fields are not a runtime
        // memory layout.
        state.probe_position.world_x = probe.world_x;
        state.probe_position.world_y = probe.world_y;

        // 0x14DD0-0x14DD6: a terrain hit is terminal only when the source
        // boundary flag is clear. FFDFE2 is represented by the request's
        // explicit host policy field; it is not conflated with record flags.
        if (probe.terrain_result != 0U && !request.terrain_boundary_active) {
            finish_marker(state);
            result.action = EntityAnimationSpawnProbeAction::terrain_blocked;
            result.source_carry = true;
            return result;
        }

        EntityCollisionSearchContext search_context{};
        search_context.probe_x = fixed_point_integer_word(probe.world_x);
        search_context.probe_y = fixed_point_integer_word(probe.world_y);
        search_context.source_flags = state.scratch_record.behavior_defaults.flags;
        search_context.collision_gate_active = true;
        search_context.excluded_candidate =
            request.protected_source_candidate;
        const auto collision = callbacks.collision_search(search_context);
        state.last_collision = collision;
        if (collision.source_carry) {
            finish_marker(state);
            result.action = EntityAnimationSpawnProbeAction::collision_boundary;
            // 0x14DEE clears CCR before returning from 0x14D8E.
            result.source_carry = false;
            return result;
        }

        if (remaining == 0U) {
            break;
        }
    }

    finish_marker(state);
    result.action = EntityAnimationSpawnProbeAction::attempt_window_exhausted;
    result.source_carry = false;
    return result;
}

} // namespace shadowrun::ghidra_conversion

#include "shadowrun_ghidra/entities/entity_animation_fallback_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

std::uint16_t add_word(const std::uint16_t value, const std::int16_t delta) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(value) +
        static_cast<std::int32_t>(delta));
}

std::uint16_t offset_heading(const std::uint16_t heading,
                             const bool subtract) {
    return static_cast<std::uint16_t>(
        heading + (subtract ? static_cast<std::uint16_t>(-0x6000)
                            : static_cast<std::uint16_t>(0x6000)));
}

} // namespace

EntityAnimationFallbackResult prepare_entity_animation_fallback_target(
    EntityAnimationFallbackState& state,
    const std::optional<EntityAnimationFallbackReference>& linked_target,
    const EntityAnimationFallbackReference& player_reference,
    const EntityAnimationFallbackCallbacks& callbacks) {
    EntityAnimationFallbackResult result{};
    const auto& reference = linked_target.has_value()
                                ? linked_target.value()
                                : player_reference;

    if (!callbacks.probe_terrain || !callbacks.apply_close_range) {
        result.boundary_unresolved = true;
        return result;
    }

    const bool subtract_offset =
        state.entity_movement_mode == 0 ||
        (state.entity_movement_mode == 1 && reference.movement_mode == 0);
    const auto heading = offset_heading(reference.heading, subtract_offset);
    constexpr std::int16_t kProjectionMagnitude = 0x00E0;
    const auto projected = transform_entity_heading_vector(
        heading, kProjectionMagnitude);
    result.projected_vector = projected;

    const auto forward_x = add_word(reference.world_x, projected.horizontal);
    const auto forward_y = add_word(reference.world_y, projected.vertical);
    result.forward_terrain_result =
        callbacks.probe_terrain(forward_x, forward_y);

    if (result.forward_terrain_result != 0 &&
        result.forward_terrain_result >= 0) {
        const auto reverse_x = add_word(reference.world_x,
                                        static_cast<std::int16_t>(
                                            -projected.horizontal));
        const auto reverse_y = add_word(reference.world_y,
                                        static_cast<std::int16_t>(
                                            -projected.vertical));
        result.reverse_terrain_result =
            callbacks.probe_terrain(reverse_x, reverse_y);
        if (*result.reverse_terrain_result == 0 ||
            *result.reverse_terrain_result < 0) {
            state.target_x = reverse_x;
            state.target_y = reverse_y;
            result.selection =
                EntityAnimationFallbackTargetSelection::reverse_projection;
        } else {
            state.target_x = reference.world_x;
            state.target_y = reference.world_y;
            result.selection =
                EntityAnimationFallbackTargetSelection::reference_position;
        }
    } else {
        state.target_x = forward_x;
        state.target_y = forward_y;
        result.selection =
            EntityAnimationFallbackTargetSelection::forward_projection;
    }

    if (callbacks.apply_close_range()) {
        result.action = EntityAnimationFallbackAction::close_range_boundary;
        result.source_carry = true;
        return result;
    }

    result.action = EntityAnimationFallbackAction::target_prepared;
    result.source_carry = false;
    return result;
}

} // namespace shadowrun::ghidra_conversion

#pragma once

#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct EntityAnimationFallbackReference {
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t heading = 0;
    std::int16_t movement_mode = 0;
};

struct EntityAnimationFallbackState {
    std::int16_t entity_movement_mode = 0; // record +0x3E
    std::uint16_t target_x = 0; // record +0x96
    std::uint16_t target_y = 0; // record +0x98
};

using EntityAnimationFallbackTerrainProbe =
    std::function<std::int16_t(std::uint16_t world_x,
                               std::uint16_t world_y)>;
using EntityAnimationFallbackCloseRangeBoundary = std::function<bool()>;

struct EntityAnimationFallbackCallbacks {
    EntityAnimationFallbackTerrainProbe probe_terrain;
    EntityAnimationFallbackCloseRangeBoundary apply_close_range;
};

enum class EntityAnimationFallbackTargetSelection {
    forward_projection,
    reverse_projection,
    reference_position,
};

enum class EntityAnimationFallbackAction {
    target_prepared,
    close_range_boundary,
    boundary_unresolved,
};

struct EntityAnimationFallbackResult {
    EntityAnimationFallbackAction action =
        EntityAnimationFallbackAction::boundary_unresolved;
    EntityAnimationFallbackTargetSelection selection =
        EntityAnimationFallbackTargetSelection::reference_position;
    bool source_carry = false;
    bool boundary_unresolved = false;
    std::optional<EntityHeadingVector> projected_vector;
    std::int16_t forward_terrain_result = 0;
    std::optional<std::int16_t> reverse_terrain_result;
};

// ROM 0x14508: choose the linked target or player reference, construct the
// source's offset heading, try the projected position in both directions,
// publish +0x96/+0x98, and pass through the 0x144E8 close-range carry gate.
EntityAnimationFallbackResult prepare_entity_animation_fallback_target(
    EntityAnimationFallbackState& state,
    const std::optional<EntityAnimationFallbackReference>& linked_target,
    const EntityAnimationFallbackReference& player_reference,
    const EntityAnimationFallbackCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

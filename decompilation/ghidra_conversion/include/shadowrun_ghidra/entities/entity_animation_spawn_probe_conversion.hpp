#pragma once

#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"

#include <cstdint>
#include <cstddef>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// 0x14D8E runs the newly initialized scratch record through this fixed
// number of source attempts. DBF with D7=0x1A means 0x1B probes, not 0x1A.
inline constexpr std::uint8_t kEntitySpawnProbeAttemptCount = 0x1BU;

struct EntityAnimationSpawnProbeState {
    SpawnEntityRecordState scratch_record{};
    std::optional<SpawnEntityLinkedState> linked_target;
    EntityFixedPointPosition probe_position{};
    std::int16_t source_marker = 0;
    std::uint8_t attempts = 0;
    std::optional<EntityWorldProbeResult> last_probe;
    bool last_terrain_boundary_active = false;
    bool last_outside_bounds = false;
    std::optional<EntityCollisionSearchResult> last_collision;
};

struct EntityAnimationSpawnProbeRequest {
    SpawnEntityParentState parent{};
    std::optional<SpawnEntityLinkedState> linked_target;

    // 0x15ABA compares the scan record against the saved source A1 while
    // 0xFFDFD8 is nonzero. A host index is the typed equivalent of that
    // source identity when the caller uses the shared collision table.
    std::optional<std::size_t> protected_source_candidate;

    // The 0x14FF8 preparation call is suppressed while 0xFFDFD8 is -1. Keep
    // the callback explicit so this conversion does not hide an asset/world
    // preparation side effect in the native model.
    SpawnPreparationBoundary prepare_spawn;
};

using EntitySpawnProbeCollisionSearch =
    std::function<EntityCollisionSearchResult(
        const EntityCollisionSearchContext&)>;

struct EntityAnimationSpawnProbeCallbacks {
    EntityTerrainProbe terrain_probe;
    EntitySpawnProbeCollisionSearch collision_search;
};

enum class EntityAnimationSpawnProbeAction {
    terrain_blocked,
    collision_boundary,
    attempt_window_exhausted,
    boundary_unresolved,
};

struct EntityAnimationSpawnProbeResult {
    EntityAnimationSpawnProbeAction action =
        EntityAnimationSpawnProbeAction::boundary_unresolved;

    // This is the carry returned by 0x14D8E itself. The inner 0x15ABA carry
    // is retained in last_collision and deliberately does not escape the
    // outer routine: 0x14DEE clears CCR before returning.
    bool source_carry = false;
    bool boundary_unresolved = false;
};

// ROM 0x14D8E: initialize a temporary spawn record, repeatedly probe its
// projected world position, and stop on a terrain boundary or the terminal
// collision-search carry. Coordinates and search policy are typed host state;
// no ROM pointers or emulated address space are exposed.
EntityAnimationSpawnProbeResult probe_entity_spawn_collision_window(
    EntityAnimationSpawnProbeState& state,
    const EntityAnimationSpawnProbeRequest& request,
    const EntityAnimationSpawnProbeCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

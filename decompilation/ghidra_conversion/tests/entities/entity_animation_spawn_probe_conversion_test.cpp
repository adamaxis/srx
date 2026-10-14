#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationSpawnProbeRequest request{};
    request.parent = SpawnEntityParentState{
        0x2000U, 0x3000U, 0x6000U, 0U, 0U, 0U, 0U};
    request.protected_source_candidate = 0U;
    std::uint32_t preparation_calls = 0;
    request.prepare_spawn = [&preparation_calls]() { ++preparation_calls; };

    EntityAnimationSpawnProbeState unresolved{};
    const auto unresolved_result = probe_entity_spawn_collision_window(
        unresolved, request, EntityAnimationSpawnProbeCallbacks{});
    if (expect(unresolved_result.boundary_unresolved &&
               !unresolved_result.source_carry &&
               unresolved_result.action ==
                   EntityAnimationSpawnProbeAction::boundary_unresolved) != 0) {
        return 7;
    }

    std::uint32_t terrain_calls = 0;
    std::int32_t first_probe_x = 0;
    std::int32_t first_probe_y = 0;
    EntityAnimationSpawnProbeState terrain_blocked{};
    const auto terrain_result = probe_entity_spawn_collision_window(
        terrain_blocked,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [&terrain_calls, &first_probe_x, &first_probe_y](
                const std::int32_t world_x, const std::int32_t world_y) {
                if (terrain_calls == 0U) {
                    first_probe_x = world_x;
                    first_probe_y = world_y;
                }
                ++terrain_calls;
                return EntityTerrainProbeSample{1U, false, false};
            },
            [](const EntityCollisionSearchContext&) {
                return EntityCollisionSearchResult{};
            }});
    if (expect(terrain_result.action ==
                   EntityAnimationSpawnProbeAction::terrain_blocked &&
               terrain_result.source_carry &&
               terrain_blocked.source_marker == 0 &&
               terrain_blocked.attempts == 1U && terrain_calls == 1U &&
               first_probe_x ==
                   static_cast<std::int32_t>(static_cast<std::int16_t>(
                       terrain_blocked.scratch_record.position_x)) * 0x10000 +
                       static_cast<std::int32_t>(
                           terrain_blocked.scratch_record.heading_offset_x) *
                           0x100 &&
               first_probe_y ==
                   static_cast<std::int32_t>(static_cast<std::int16_t>(
                       terrain_blocked.scratch_record.position_y)) * 0x10000 +
                       static_cast<std::int32_t>(
                           terrain_blocked.scratch_record.heading_offset_y) *
                           0x100 &&
               preparation_calls == 0U) != 0) {
        return 1;
    }

    EntityAnimationSpawnProbeState exhausted{};
    std::uint32_t exhausted_calls = 0;
    const auto exhausted_result = probe_entity_spawn_collision_window(
        exhausted,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [&exhausted_calls](const std::int32_t, const std::int32_t) {
                ++exhausted_calls;
                return EntityTerrainProbeSample{0U, false, false};
            },
            [](const EntityCollisionSearchContext&) {
                return EntityCollisionSearchResult{};
            }});
    if (expect(exhausted_result.action ==
                   EntityAnimationSpawnProbeAction::attempt_window_exhausted &&
               !exhausted_result.source_carry &&
               exhausted.source_marker == 0 &&
               exhausted.attempts == kEntitySpawnProbeAttemptCount &&
               exhausted_calls == kEntitySpawnProbeAttemptCount) != 0) {
        return 2;
    }

    EntityAnimationSpawnProbeState boundary_allowed{};
    const auto boundary_result = probe_entity_spawn_collision_window(
        boundary_allowed,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [](const std::int32_t, const std::int32_t) {
                return EntityTerrainProbeSample{1U, true, false};
            },
            [](const EntityCollisionSearchContext&) {
                return EntityCollisionSearchResult{};
            }});
    if (expect(boundary_result.action ==
                   EntityAnimationSpawnProbeAction::attempt_window_exhausted &&
               !boundary_result.source_carry &&
               boundary_allowed.attempts == kEntitySpawnProbeAttemptCount) !=
        0) {
        return 3;
    }

    EntityAnimationSpawnProbeState collision_boundary{};
    std::uint32_t collision_calls = 0;
    const auto collision_result = probe_entity_spawn_collision_window(
        collision_boundary,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [](const std::int32_t, const std::int32_t) {
                return EntityTerrainProbeSample{0U, false, false};
            },
            [&collision_calls](const EntityCollisionSearchContext&) {
                ++collision_calls;
                EntityCollisionSearchResult collision{};
                collision.source_carry = collision_calls == 3U;
                return collision;
            }});
    if (expect(collision_result.action ==
                   EntityAnimationSpawnProbeAction::collision_boundary &&
               !collision_result.source_carry &&
               collision_boundary.source_marker == 0 &&
               collision_boundary.attempts == 3U &&
               collision_boundary.last_collision.has_value() &&
               collision_boundary.last_collision->source_carry) != 0) {
        return 4;
    }

    EntityAnimationSpawnProbeState shared_search{};
    const auto shared_result = probe_entity_spawn_collision_window(
        shared_search,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [](const std::int32_t, const std::int32_t) {
                return EntityTerrainProbeSample{0U, false, false};
            },
            [](const EntityCollisionSearchContext& context) {
                const std::vector<CollisionCandidate> candidates{
                    {context.probe_x, context.probe_y, 0x01U, true, 0U, 0U}};
                return find_colliding_entity(context, candidates);
            }});
    if (expect(shared_result.action ==
                   EntityAnimationSpawnProbeAction::collision_boundary &&
               shared_search.last_collision.has_value() &&
               shared_search.last_collision->source_carry &&
               !shared_search.last_collision->candidate_index.has_value()) != 0) {
        return 5;
    }

    EntityAnimationSpawnProbeState fresh_boundary_sample{};
    std::uint32_t boundary_sample_calls = 0;
    const auto fresh_boundary_result = probe_entity_spawn_collision_window(
        fresh_boundary_sample,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [&boundary_sample_calls](const std::int32_t, const std::int32_t) {
                ++boundary_sample_calls;
                return boundary_sample_calls == 1U
                    ? EntityTerrainProbeSample{1U, true, false}
                    : EntityTerrainProbeSample{1U, false, false};
            },
            [](const EntityCollisionSearchContext&) {
                return EntityCollisionSearchResult{};
            }});
    if (expect(fresh_boundary_result.action ==
                   EntityAnimationSpawnProbeAction::terrain_blocked &&
               fresh_boundary_result.source_carry &&
               fresh_boundary_sample.attempts == 2U &&
               fresh_boundary_sample.last_terrain_boundary_active == false) !=
        0) {
        return 6;
    }

    return 0;
}

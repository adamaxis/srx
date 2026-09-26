#include "shadowrun_ghidra/entity_animation_spawn_probe_conversion.hpp"

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
        0x8000U, 0x8000U, 0x6000U, 0U, 0U, 0U, 0U};
    request.protected_source_candidate = 0U;
    std::uint32_t preparation_calls = 0;
    request.prepare_spawn = [&preparation_calls]() { ++preparation_calls; };

    std::uint32_t terrain_calls = 0;
    EntityAnimationSpawnProbeState terrain_blocked{};
    const auto terrain_result = probe_entity_spawn_collision_window(
        terrain_blocked,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [&terrain_calls](const std::int32_t, const std::int32_t) {
                ++terrain_calls;
                return static_cast<std::uint16_t>(1U);
            },
            [](const EntityCollisionSearchContext&) {
                return EntityCollisionSearchResult{};
            }});
    if (expect(terrain_result.action ==
                   EntityAnimationSpawnProbeAction::terrain_blocked &&
               terrain_result.source_carry &&
               terrain_blocked.source_marker == 0 &&
               terrain_blocked.attempts == 1U && terrain_calls == 1U &&
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
                return static_cast<std::uint16_t>(0U);
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

    auto boundary_request = request;
    boundary_request.terrain_boundary_active = true;
    EntityAnimationSpawnProbeState boundary_allowed{};
    const auto boundary_result = probe_entity_spawn_collision_window(
        boundary_allowed,
        boundary_request,
        EntityAnimationSpawnProbeCallbacks{
            [](const std::int32_t, const std::int32_t) {
                return static_cast<std::uint16_t>(1U);
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
                return static_cast<std::uint16_t>(0U);
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
                return static_cast<std::uint16_t>(0U);
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

    return 0;
}

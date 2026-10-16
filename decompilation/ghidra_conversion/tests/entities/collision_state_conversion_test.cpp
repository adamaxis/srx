#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    shadowrun::ghidra_conversion::EntityCollisionRecord entity{
        0xAAAAAAAAU};
    std::uint16_t observed_x = 0;
    std::uint16_t observed_y = 0;

    const auto result =
        shadowrun::ghidra_conversion::evaluate_entity_collision(
            entity, 0x1234, 0xFEDC,
            [&observed_x, &observed_y](const std::uint16_t x,
                                       const std::uint16_t y) {
                observed_x = x;
                observed_y = y;
                return shadowrun::ghidra_conversion::CollisionEvaluation{
                    static_cast<std::int16_t>(0x8003), 0x7F102030U};
            });

    if (expect(result == static_cast<std::int16_t>(0x8003)) != 0 ||
        expect(entity.packed_state == 0x7F102030U) != 0 ||
        expect(observed_x == 0x1234) != 0 ||
        expect(observed_y == 0xFEDC) != 0) {
        return 1;
    }

    const std::vector<CollisionCandidate> candidates{
        {0x1000U, 0x2000U, 0x01U, true, 0U, 0U},
        {0x1400U, 0x2000U, 0x41U, true, 0U, 0U},
        {0x10A0U, 0x20A0U, 0x01U, true, 0U, 0U}};
    const auto found = find_colliding_entity(
        EntityCollisionSearchContext{
            0x1000U, 0x2000U, 0x08U, false, false, std::nullopt},
        candidates);
    if (expect(found.candidate_index.has_value() &&
               found.source_carry &&
               found.candidate_index.value() == 0U &&
               found.source_position_updated && found.source_x == 0x1000U &&
               found.source_y == 0x2010U) != 0) {
        return 2;
    }

    const auto gated = find_colliding_entity(
        EntityCollisionSearchContext{
            0x1000U, 0x2000U, 0U, false, true, 1U},
        candidates);
    if (expect(!gated.candidate_index.has_value()) != 0) {
        return 3;
    }

    const auto override_exit = find_colliding_entity(
        EntityCollisionSearchContext{
            0x1000U, 0x2000U, 0U, true, false, std::nullopt},
        candidates);
    if (expect(!override_exit.candidate_index.has_value() &&
               override_exit.source_carry) != 0) {
        return 4;
    }

    const auto override_compatible = find_colliding_entity(
        EntityCollisionSearchContext{
            0x1000U, 0x2000U, 0x08U, true, false, std::nullopt},
        candidates);
    if (expect(!override_compatible.candidate_index.has_value() &&
               override_compatible.source_carry) != 0) {
        return 8;
    }

    EntityCollisionRecoveryState recovery{
        EntityFixedPointPosition{0x10000000, 0x20000000, 1, 0}, 2};
    const auto recovered = advance_entity_collision_recovery(
        recovery,
        false,
        [](const std::int32_t x, const std::int32_t y) {
            if (x != 0x10000100 || y != 0x20000000) {
                return static_cast<std::uint16_t>(0x0001U);
            }
            return static_cast<std::uint16_t>(0U);
        },
        EntityCollisionSearchContext{
            0U, 0U, 0x08U, false, false, std::nullopt},
        std::vector<CollisionCandidate>{
            {0x1000U, 0x2000U, 0x01U, true, 0U, 0U}});
    if (expect(recovered.action ==
                   EntityCollisionRecoveryAction::collision_search &&
               recovered.source_carry &&
               recovered.collision_search.has_value() &&
               recovered.collision_search->candidate_index.has_value() &&
               recovered.collision_search->candidate_index.value() == 0U &&
               recovery.retry_counter == 1 &&
               recovery.position.world_x == 0x10000100) != 0) {
        return 5;
    }

    EntityCollisionRecoveryState exhausted{
        EntityFixedPointPosition{0U, 0U, 0, 0}, 0};
    const auto exhausted_result = advance_entity_collision_recovery(
        exhausted,
        false,
        [](const std::int32_t, const std::int32_t) {
            return static_cast<std::uint16_t>(0U);
        },
        EntityCollisionSearchContext{},
        {});
    if (expect(exhausted_result.action ==
                   EntityCollisionRecoveryAction::retry_exhausted &&
               exhausted_result.source_carry &&
               exhausted.retry_counter == -1) != 0) {
        return 6;
    }

    EntityCollisionRecoveryState terrain_blocked{
        EntityFixedPointPosition{0U, 0U, 0, 0}, 2};
    const auto terrain_result = advance_entity_collision_recovery(
        terrain_blocked,
        false,
        [](const std::int32_t, const std::int32_t) {
            return static_cast<std::uint16_t>(0x0001U);
        },
        EntityCollisionSearchContext{},
        {});
    if (expect(terrain_result.action ==
                   EntityCollisionRecoveryAction::terrain_probe_blocked &&
               terrain_result.source_carry &&
               terrain_blocked.retry_counter == 2) != 0) {
        return 7;
    }

    EntityCollisionRecoveryState source_ordered_boundary{
        EntityFixedPointPosition{0x10000000, 0x20000000, 1, 0}, 2};
    const auto source_ordered_result = advance_entity_collision_recovery(
        source_ordered_boundary,
        [](const std::int32_t x, const std::int32_t y) {
            if (x != 0x10000100 || y != 0x20000000) {
                return EntityTerrainProbeSample{1U, false, false};
            }
            return EntityTerrainProbeSample{1U, true, false};
        },
        EntityCollisionSearchContext{},
        {});
    if (expect(source_ordered_result.action ==
                   EntityCollisionRecoveryAction::collision_search &&
               !source_ordered_result.source_carry &&
               source_ordered_result.terrain_boundary_active &&
               source_ordered_result.terrain_result == 1U &&
               source_ordered_boundary.retry_counter == 1 &&
               source_ordered_boundary.position.world_x == 0x10000100) != 0) {
        return 8;
    }

    EntityCollisionRecoveryState source_ordered_blocked{
        EntityFixedPointPosition{0U, 0U, 0, 0}, 2};
    const auto source_ordered_blocked_result =
        advance_entity_collision_recovery(
            source_ordered_blocked,
            [](const std::int32_t, const std::int32_t) {
                return EntityTerrainProbeSample{0x7FU, false, true};
            },
            EntityCollisionSearchContext{},
            {});
    if (expect(source_ordered_blocked_result.action ==
                   EntityCollisionRecoveryAction::terrain_probe_blocked &&
               source_ordered_blocked_result.source_carry &&
               source_ordered_blocked_result.outside_bounds_marker &&
               source_ordered_blocked.retry_counter == 2) != 0) {
        return 9;
    }

    return 0;
}

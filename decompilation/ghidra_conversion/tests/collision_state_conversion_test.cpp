#include "shadowrun_ghidra/collision_state_conversion.hpp"

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
    if (expect(!override_exit.candidate_index.has_value()) != 0) {
        return 4;
    }

    return 0;
}

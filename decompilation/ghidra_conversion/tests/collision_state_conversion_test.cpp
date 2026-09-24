#include "shadowrun_ghidra/collision_state_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
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

    return 0;
}

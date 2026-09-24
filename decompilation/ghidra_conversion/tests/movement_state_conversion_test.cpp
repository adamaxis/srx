#include "shadowrun_ghidra/movement_state_conversion.hpp"

#include <array>
#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    shadowrun::ghidra_conversion::EntityMovementState entity{
        0x06,
        0xE000,
        0x2000,
        shadowrun::ghidra_conversion::MovementContinuation::ReevaluateCollision,
        0x00,
        0x1234,
        0xFFFF,
        {0}};
    const std::array<std::uint16_t, 8> animations{
        0x1000, 0x1002, 0x1004, 0x1006,
        0x1008, 0x100A, 0x100C, 0x100E};
    std::uint16_t observed_x = 0;
    std::uint16_t observed_y = 0;

    shadowrun::ghidra_conversion::reset_entity_movement_state(
        entity, animations, 0x3456, 0x789A,
        [&observed_x, &observed_y](const std::uint16_t x,
                                   const std::uint16_t y) {
            observed_x = x;
            observed_y = y;
            return shadowrun::ghidra_conversion::CollisionEvaluation{
                0, 0x01020304U};
        });

    // 0x2000 rotates left by three to 0x0001, selecting animation slot 1.
    if (expect((entity.movement_flags & 0x02U) == 0U) != 0 ||
        expect(entity.current_heading == 0x2000) != 0 ||
        expect(entity.continuation ==
               shadowrun::ghidra_conversion::MovementContinuation::
                   ReevaluateCollision) != 0 ||
        expect((entity.animation_flags & 0x01U) != 0U) != 0 ||
        expect(entity.movement_status == 0) != 0 ||
        expect(entity.direction_animation == 0x1002) != 0 ||
        expect(entity.collision.packed_state == 0x01020304U) != 0 ||
        expect(observed_x == 0x3456) != 0 || expect(observed_y == 0x789A) != 0) {
        return 1;
    }

    return 0;
}

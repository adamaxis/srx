#include "shadowrun_ghidra/movement_fallback_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MovementFallbackState first_failure{
        0x1000, 0x2000, 0, 0x1000, 0x9000, 0};
    std::vector<std::uint16_t> first_slots;
    resolve_entity_fallback_movement(
        first_failure, false, {},
        [&first_slots](const std::uint16_t slot) {
            first_slots.push_back(slot);
            return MovementFallbackAttempt{-9, 0x1111, 0x2222};
        },
        0x12345678U);
    if (!expect(first_slots == std::vector<std::uint16_t>{2}) ||
        !expect(first_failure.position_x == 0x1111 &&
                first_failure.position_y == 0x2222) ||
        !expect(first_failure.movement_status == 1) ||
        !expect(first_failure.continuation == 0x12345678U)) {
        return 1;
    }

    MovementFallbackState second_failure{
        0x3000, 0x4000, 0, 0x1000, 0x9000, 0};
    std::vector<std::uint16_t> second_slots;
    resolve_entity_fallback_movement(
        second_failure, false, {},
        [&second_slots](const std::uint16_t slot) {
            second_slots.push_back(slot);
            if (second_slots.size() == 1) {
                return MovementFallbackAttempt{1, 0xAAAA, 0xBBBB};
            }
            return MovementFallbackAttempt{-9, 0x3333, 0x4444};
        },
        0xCAFEBABEU);
    if (!expect(second_slots == std::vector<std::uint16_t>({2, 6})) ||
        !expect(second_failure.position_x == 0x3333 &&
                second_failure.position_y == 0x4444) ||
        !expect(second_failure.movement_status == 2) ||
        !expect(second_failure.continuation == 0xCAFEBABEU)) {
        return 2;
    }

    MovementFallbackState all_pass{
        0x5000, 0x6000, 2, 0x1000, 0x9000, 0};
    std::vector<std::uint16_t> all_pass_slots;
    resolve_entity_fallback_movement(
        all_pass, true, {},
        [&all_pass_slots](const std::uint16_t slot) {
            all_pass_slots.push_back(slot);
            return MovementFallbackAttempt{1, 0xEEEE, 0xFFFF};
        },
        0x0BADF00DU);
    if (!expect(all_pass_slots == std::vector<std::uint16_t>({10, 14, 2})) ||
        !expect(all_pass.position_x == 0x5000 &&
                all_pass.position_y == 0x6000) ||
        !expect(all_pass.movement_status == 2) ||
        !expect(all_pass.continuation == 0x0BADF00DU)) {
        return 3;
    }

    return 0;
}

#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"

#include <array>
#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    const std::array<shadowrun::ghidra_conversion::MovementOffset, 8> offsets{
        shadowrun::ghidra_conversion::MovementOffset{1, -2},
        shadowrun::ghidra_conversion::MovementOffset{3, -4},
        shadowrun::ghidra_conversion::MovementOffset{5, -6},
        shadowrun::ghidra_conversion::MovementOffset{7, -8},
        shadowrun::ghidra_conversion::MovementOffset{-9, 10},
        shadowrun::ghidra_conversion::MovementOffset{-11, 12},
        shadowrun::ghidra_conversion::MovementOffset{-13, 14},
        shadowrun::ghidra_conversion::MovementOffset{-15, 16}};

    const auto preserved =
        shadowrun::ghidra_conversion::probe_entity_directional_collision(
            {0x1000, 0x2000, 4, 0x02}, offsets,
            [](const std::uint16_t x, const std::uint16_t y) {
                return static_cast<std::int16_t>(
                    (x == 0x0FF7 && y == 0x200A) ? -1 : 3);
            });
    if (expect(preserved.collision_code == -1) != 0 ||
        expect(preserved.candidate_x == 0x0FF7) != 0 ||
        expect(preserved.candidate_y == 0x200A) != 0 ||
        expect(preserved.source_direction_slot == 8) != 0) {
        return 1;
    }

    const auto blocked =
        shadowrun::ghidra_conversion::probe_entity_directional_collision(
            {0x1000, 0x2000, 7, 0}, offsets,
            [](const std::uint16_t, const std::uint16_t) {
                return static_cast<std::int16_t>(-3);
            });
    if (expect(blocked.collision_code == 0x7F) != 0 ||
        expect(blocked.candidate_x == 0x0FF1) != 0 ||
        expect(blocked.candidate_y == 0x2010) != 0 ||
        expect(blocked.source_direction_slot == 14) != 0) {
        return 1;
    }

    const shadowrun::ghidra_conversion::MovementResultPolicy ordinary{};
    if (expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   static_cast<std::int16_t>(0x8000), ordinary) ==
               static_cast<std::int16_t>(0x8000)) != 0 ||
        expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   4, ordinary) == 4) != 0) {
        return 1;
    }

    const shadowrun::ghidra_conversion::MovementResultPolicy transition{
        true, 0, 0};
    if (expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   static_cast<std::int16_t>(0x8000), transition) == 0x7F) !=
        0) {
        return 1;
    }

    const shadowrun::ghidra_conversion::MovementResultPolicy high_result{};
    if (expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   static_cast<std::int16_t>(-8), high_result) == 0x7F) != 0) {
        return 1;
    }

    const shadowrun::ghidra_conversion::MovementResultPolicy scene_rule{
        false, 0x14, 0};
    if (expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   static_cast<std::int16_t>(0x8003), scene_rule) == 0x7F) !=
            0 ||
        expect(shadowrun::ghidra_conversion::apply_movement_result_policy(
                   static_cast<std::int16_t>(0x8006), scene_rule) ==
               static_cast<std::int16_t>(0x8006)) != 0) {
        return 1;
    }

    return 0;
}

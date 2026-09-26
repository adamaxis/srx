#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    const MovementResultPolicy policy{};

    EntityMovementStepState clear_step{
        0x1000, 0x2000, 0x2000, 0, 0, 7, 0, 0, 1, 1};
    int reset_count = 0;
    advance_entity_movement_step(
        clear_step, 0x4567, 4, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{0, 0x1111, 0x2222, 4};
        },
        []() { return false; }, []() { return true; },
        [](const std::uint16_t) { return static_cast<std::int16_t>(1); },
        [&reset_count]() { ++reset_count; });
    if (!expect(clear_step.collision_context == 0x4567 &&
                clear_step.movement_substate == 0x1000 &&
                clear_step.position_x == 0x1111 &&
                clear_step.position_y == 0x2222 &&
                clear_step.event_flags == 0 &&
                reset_count == 0)) {
        return 1;
    }

    EntityMovementStepState aligned_rejected{
        0x3000, 0x4000, 0x4000, 0, 0, 0, 0, 0, 1, 0};
    advance_entity_movement_step(
        aligned_rejected, 9, 2, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x3333, 0x4444, 2};
        },
        []() { return false; }, []() { return false; },
        [](const std::uint16_t) { return static_cast<std::int16_t>(1); },
        [&reset_count]() { ++reset_count; });
    if (!expect(aligned_rejected.position_x == 0x3333 &&
                aligned_rejected.position_y == 0x4444 &&
                aligned_rejected.collision_flags == 0 &&
                reset_count == 0)) {
        return 2;
    }

    EntityMovementStepState diagonal_commit{
        0x5000, 0x6000, 0x2000, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::uint16_t> policy_slots;
    advance_entity_movement_step(
        diagonal_commit, 11, 6, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x5555, 0x6666, 0x34};
        },
        []() { return false; }, []() { return true; },
        [&policy_slots](const std::uint16_t slot) {
            policy_slots.push_back(slot);
            return static_cast<std::int16_t>(-9);
        },
        [&reset_count]() { ++reset_count; });
    if (!expect(policy_slots == std::vector<std::uint16_t>{0}) ||
        !expect(diagonal_commit.position_x == 0x5555 &&
                diagonal_commit.position_y == 0x6666 &&
                diagonal_commit.animation_heading == 0x4003 &&
                (diagonal_commit.movement_flags & 4U) != 0U &&
                reset_count == 0)) {
        return 3;
    }

    EntityMovementStepState diagonal_reset{
        0, 0, 0x6000, 0, 0, 0, 0, 0, 0, 0};
    advance_entity_movement_step(
        diagonal_reset, 12, 6, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x7777, 0x8888, 0};
        },
        []() { return false; }, []() { return true; },
        [](const std::uint16_t) { return static_cast<std::int16_t>(1); },
        [&reset_count]() { ++reset_count; });
    if (!expect(diagonal_reset.route_status == -1) ||
        !expect(reset_count == 1) ||
        !expect(diagonal_reset.position_x == 0 &&
                diagonal_reset.position_y == 0)) {
        return 4;
    }

    return 0;
}

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
        []() { return false; },
        []() { return MovementRouteResult{false, std::nullopt}; },
        [](const std::uint16_t) { return MovementProbeResult{}; },
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
        []() { return false; },
        []() {
            return MovementRouteResult{
                false, MovementProbeResult{0, 0x3335, 0x4446, 6}};
        },
        [](const std::uint16_t) { return MovementProbeResult{}; },
        [&reset_count]() { ++reset_count; });
    if (!expect(aligned_rejected.position_x == 0x3335 &&
                aligned_rejected.position_y == 0x4446 &&
                aligned_rejected.collision_flags == 0 &&
                reset_count == 0)) {
        return 2;
    }

    EntityMovementStepState missing_aligned_fallback{
        0x3000, 0x4000, 0x4000, 0, 0, 0, 0, 0, 1, 0};
    int missing_fallback_reset_count = 0;
    advance_entity_movement_step(
        missing_aligned_fallback, 10, 2, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x3333, 0x4444, 2};
        },
        []() { return false; },
        []() { return MovementRouteResult{false, std::nullopt}; },
        [](const std::uint16_t) { return MovementProbeResult{}; },
        [&missing_fallback_reset_count]() {
            ++missing_fallback_reset_count;
        });
    if (!expect(missing_aligned_fallback.position_x == 0x3000 &&
                missing_aligned_fallback.position_y == 0x4000 &&
                missing_aligned_fallback.route_status == -1 &&
                missing_fallback_reset_count == 1)) {
        return 3;
    }

    EntityMovementStepState diagonal_commit{
        0x5000, 0x6000, 0x2000, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::uint16_t> policy_slots;
    advance_entity_movement_step(
        diagonal_commit, 11, 6, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x5555, 0x6666, 0x34};
        },
        []() { return false; },
        []() { return MovementRouteResult{true, std::nullopt}; },
        [&policy_slots](const std::uint16_t slot) {
            policy_slots.push_back(slot);
            return MovementProbeResult{-9, 0x5151, 0x6161, 0};
        },
        [&reset_count]() { ++reset_count; });
    if (!expect(policy_slots == std::vector<std::uint16_t>{0}) ||
        !expect(diagonal_commit.position_x == 0x5151 &&
                diagonal_commit.position_y == 0x6161 &&
                diagonal_commit.animation_heading == 0x0000 &&
                (diagonal_commit.movement_flags & 4U) != 0U &&
                reset_count == 0)) {
        return 4;
    }

    EntityMovementStepState alternate_diagonal_commit{
        0x5000, 0x6000, 0x2000, 0, 0, 0, 0, 0, 0, 0};
    policy_slots.clear();
    advance_entity_movement_step(
        alternate_diagonal_commit, 11, 6, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x5555, 0x6666, 0x34};
        },
        []() { return false; },
        []() { return MovementRouteResult{true, std::nullopt}; },
        [&policy_slots](const std::uint16_t slot) {
            policy_slots.push_back(slot);
            return slot == 0U
                       ? MovementProbeResult{1, 0x5151, 0x6161, 0}
                       : MovementProbeResult{-9, 0x5252, 0x6262, 4};
        },
        [&reset_count]() { ++reset_count; });
    if (!expect(policy_slots == std::vector<std::uint16_t>{0, 4}) ||
        !expect(alternate_diagonal_commit.position_x == 0x5252 &&
                alternate_diagonal_commit.position_y == 0x6262 &&
                alternate_diagonal_commit.animation_heading == 0x4000 &&
                (alternate_diagonal_commit.movement_flags & 4U) != 0U &&
                reset_count == 0)) {
        return 5;
    }

    EntityMovementStepState diagonal_reset{
        0, 0, 0x6000, 0, 0, 0, 0, 0, 0, 0};
    advance_entity_movement_step(
        diagonal_reset, 12, 6, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x7777, 0x8888, 0};
        },
        []() { return false; },
        []() { return MovementRouteResult{true, std::nullopt}; },
        [](const std::uint16_t) {
            return MovementProbeResult{1, 0x9999, 0xAAAA, 2};
        },
        [&reset_count]() { ++reset_count; });
    if (!expect(diagonal_reset.route_status == -1) ||
        !expect(reset_count == 1) ||
        !expect(diagonal_reset.position_x == 0 &&
                diagonal_reset.position_y == 0)) {
        return 6;
    }

    if (!expect(field_movement_step_context_word(0U) == 0x0028U) ||
        !expect(field_movement_step_context_word(2U) == 0xFFD8U) ||
        !expect(field_movement_step_context_word(8U) == 0x0028U) ||
        !expect(field_movement_step_context_word(14U) == 0x0028U)) {
        return 7;
    }

    EntityMovementStepState pass{
        0x1000, 0x2000, 0x2000, 0, 0, 0, 0, 0, 0, 0, 7};
    std::uint16_t probed_slot = 0xFFFFU;
    begin_entity_movement_pass(
        pass, policy,
        [&probed_slot](const std::uint16_t slot) {
            probed_slot = slot;
            return MovementProbeResult{0, 0x1111, 0x2222, 0};
        },
        []() { return false; },
        []() { return MovementRouteResult{true, std::nullopt}; },
        [](const std::uint16_t) { return MovementProbeResult{}; },
        []() {});
    if (!expect(pass.collision_extent == -12) ||
        !expect(probed_slot == 2U) ||
        !expect(pass.collision_context == 0xFFD8U) ||
        !expect(pass.position_x == 0x1111 && pass.position_y == 0x2222)) {
        return 8;
    }

    EntityMovementStepState north{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    begin_entity_movement_pass(
        north, policy,
        [](const std::uint16_t) {
            return MovementProbeResult{0, 1, 2, 0};
        },
        []() { return false; },
        []() { return MovementRouteResult{true, std::nullopt}; },
        [](const std::uint16_t) { return MovementProbeResult{}; },
        []() {});
    if (!expect(north.collision_extent == -12) ||
        !expect(north.collision_context == 0x0028U)) {
        return 9;
    }

    return 0;
}

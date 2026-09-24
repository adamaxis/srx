#include "shadowrun_ghidra/entity_transition_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    const std::array<std::uint16_t, 8> c042_animations{
        0x4000, 0x4002, 0x4004, 0x4006,
        0x4008, 0x400A, 0x400C, 0x400E};
    const std::array<std::uint16_t, 8> c0a2_animations{
        0x5000, 0x5002, 0x5004, 0x5006,
        0x5008, 0x500A, 0x500C, 0x500E};

    EntityTransitionState spawn_entity{
        0, 0, 0x1234, 0xFFF8, 0x2000, 0, 0,
        EntityTransitionContinuation::TargetCollisionCheck, {}, {4, 0, 2}};
    EntityBehaviorSignature spawn_signature{1, 2};
    std::vector<int> spawn_events;
    const auto spawned = synchronize_entity_behavior_spawn(
        spawn_entity, spawn_signature,
        [](EntityBehaviorSignature& signature) {
            signature.primary = 3;
        },
        [&spawn_events]() { spawn_events.push_back(1); },
        [&spawn_events]() -> std::optional<ChildAllocation> {
            spawn_events.push_back(2);
            return ChildAllocation{0x123, {}};
        },
        {0, 1, 0}, c042_animations,
        [&spawn_events]() { spawn_events.push_back(3); });

    if (expect(spawned.has_value()) != 0 ||
        expect(spawned->resource_id == 0x123) != 0 ||
        expect(spawned->position_x == 0x1234) != 0 ||
        expect(spawned->position_y == 0x0008) != 0 ||
        expect(spawn_entity.type == 6) != 0 ||
        expect(spawn_entity.subtype == 0) != 0 ||
        expect(spawn_entity.direction_animation == 0x4002) != 0 ||
        expect(spawn_entity.delay.animation_delay == 106) != 0 ||
        expect(spawn_entity.continuation ==
               EntityTransitionContinuation::SpawnCollisionCheck) != 0 ||
        expect(spawn_events == std::vector<int>({1, 2, 3})) != 0) {
        return 11;
    }

    SpawnedBehaviorEntity child{};
    initialize_spawned_behavior_entity(child, 0x456, 0x1000, 0xFFF8);
    if (expect(child.resource_id == 0x456) != 0 ||
        expect(child.animation == 4) != 0 || expect(child.behavior_type == 1) != 0 ||
        expect(child.position_x == 0x1000) != 0 ||
        expect(child.position_y == 0x0008) != 0 ||
        expect(child.vertical_offset == 0x2000) != 0 ||
        expect((child.render_flags & 1U) != 0U) != 0 ||
        expect((child.lifecycle_flags & 1U) != 0U) != 0) {
        return 12;
    }

    EntityTransitionState target_entity{
        0x0C, 0, 0, 0, 0, 0x4000, 0,
        EntityTransitionContinuation::SpawnCollisionCheck, {}, {0, 0, 0}};
    EntityBehaviorSignature target_signature{4, 5};
    std::vector<std::uint16_t> actions;
    std::vector<int> target_events;
    const auto target_child = configure_entity_target_transition(
        target_entity, {40, 80, 0}, true,
        []() { return static_cast<std::uint16_t>(0x4000); },
        [&actions](const std::uint16_t action) { actions.push_back(action); },
        c0a2_animations,
        []() { return true; }, true, target_signature,
        [](EntityBehaviorSignature& signature) {
            signature.secondary = 6;
        },
        [&target_events]() { target_events.push_back(1); },
        [&target_events]() -> std::optional<ChildAllocation> {
            target_events.push_back(2);
            return ChildAllocation{0x789, {}};
        },
        [&target_events]() { target_events.push_back(3); });

    if (expect(actions == std::vector<std::uint16_t>({0x55, 0x4E})) != 0 ||
        expect(target_child.has_value()) != 0 ||
        expect(target_child->position_y == 0x0010) != 0 ||
        expect(target_entity.heading == 0x4000) != 0 ||
        expect(target_entity.direction_animation == 0x5004) != 0 ||
        expect(target_entity.delay.animation_delay == 15) != 0 ||
        expect(target_entity.continuation ==
               EntityTransitionContinuation::TargetCollisionCheck) != 0 ||
        expect(target_events == std::vector<int>({3, 1, 2})) != 0) {
        return 13;
    }

    return 0;
}

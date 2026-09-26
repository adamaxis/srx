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

    SpawnEntityParentState parent{
        0x1000U, 0x2000U, 0x0000U, 0x08U, 0x12U, 0x03U, 0x44U};
    SpawnEntityRecordState spawned_record{};
    bool prepared = false;
    std::optional<SpawnEntityLinkedState> linked_target =
        SpawnEntityLinkedState{0x1000U, 0x2100U, 0x01U, false};
    initialize_entity_spawn_record(
        spawned_record,
        parent,
        0xFFFFU,
        false,
        linked_target,
        [&prepared]() { prepared = true; });
    if (expect(prepared && linked_target->parent_backlink_set &&
               spawned_record.behavior_defaults.vertical_offset == -12 &&
               spawned_record.behavior_defaults.flags == 0x09U &&
               spawned_record.flags == 0x09U &&
               spawned_record.position_x == 0x0F50U &&
               spawned_record.position_y == 0x2000U &&
               spawned_record.heading == 0xC000U &&
               spawned_record.source_identifier == 0xFFFFU &&
               spawned_record.behavior_code == 0x0014U &&
               spawned_record.directional_probe_x == 0x0E80U &&
               spawned_record.directional_probe_y == 0x1FC0U &&
               spawned_record.heading_offset_x == -24576 &&
               spawned_record.heading_offset_y == 0) != 0) {
        return 14;
    }

    std::optional<SpawnEntityLinkedState> inactive_target =
        SpawnEntityLinkedState{0x3000U, 0x3000U, 0U, false};
    SpawnEntityRecordState no_link_record{};
    prepared = false;
    initialize_entity_spawn_record(
        no_link_record,
        parent,
        7U,
        true,
        inactive_target,
        [&prepared]() { prepared = true; });
    if (expect(!inactive_target->parent_backlink_set &&
               no_link_record.heading == parent.heading && !prepared) != 0) {
        return 15;
    }

    EntitySpawnCycleState cycle_state{};
    cycle_state.parent = SpawnEntityParentState{
        0x1000U, 0x2000U, 0x4000U, 0x08U, 0x01U, 0x08U, 0x22U};
    cycle_state.animation_counter = 0x0010U;
    cycle_state.global_mode = 2U;
    std::vector<char> cycle_events;
    std::vector<std::uint16_t> available_slots{9U, 10U};
    const auto cycle = process_entity_spawn_cycle(
        cycle_state,
        [&cycle_events]() {
            cycle_events.push_back('g');
            return true;
        },
        [&available_slots, &cycle_events]() -> std::optional<std::uint16_t> {
            cycle_events.push_back('a');
            if (available_slots.empty()) {
                return std::nullopt;
            }
            const auto slot = available_slots.front();
            available_slots.erase(available_slots.begin());
            return slot;
        },
        [&cycle_events]() { cycle_events.push_back('p'); },
        [&cycle_events]() { cycle_events.push_back('r'); },
        [&cycle_events](const std::uint16_t resource_id) {
            cycle_events.push_back(resource_id == 0x4AU ? 'e' : 'x');
        },
        [&cycle_events](const std::uint16_t slot) {
            cycle_events.push_back(slot == 10U ? 'm' : 'x');
        });
    if (expect(cycle.action == EntitySpawnCycleAction::spawned &&
               cycle.primary.has_value() && cycle.secondary.has_value() &&
               cycle.primary_slot == 9U && cycle.secondary_slot == 10U &&
               cycle.selected_resource_id == 0x4AU &&
               cycle.special_follow_up &&
               cycle.primary->lifecycle_state == 2U &&
               cycle.primary->saved_position_x == 0x10B0U &&
               cycle.primary->field_0x89 == 0x20U &&
               cycle.primary->field_0x8b == 1U &&
               cycle.secondary->position_x == 0x1180U &&
               cycle.secondary->position_y == 0x1FC0U &&
               cycle.secondary->animation_resource == 0x28U &&
               cycle.secondary->continuation_address == 0xAC4EU &&
               cycle.secondary->record_flags == 1U &&
               cycle_events == std::vector<char>({'g', 'a', 'r', 'p', 'a', 'e', 'm'})) != 0) {
        return 16;
    }

    EntitySpawnCycleState expired_state = cycle_state;
    expired_state.animation_counter = 0x003DU;
    expired_state.parent.flags = 0x02U;
    bool gate_called = false;
    const auto expired = process_entity_spawn_cycle(
        expired_state,
        [&gate_called]() {
            gate_called = true;
            return true;
        },
        {},
        {},
        {},
        {},
        {});
    if (expect(expired.action == EntitySpawnCycleAction::animation_window_expired &&
               expired.overflow_latch_set && !gate_called) != 0) {
        return 17;
    }

    return 0;
}

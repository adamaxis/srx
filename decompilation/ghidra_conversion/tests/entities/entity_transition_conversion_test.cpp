#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

std::optional<std::uint8_t> weapon_rounds_per_action(
    const std::uint8_t action_code) {
    if (action_code == 0U || action_code > 12U) {
        return std::nullopt;
    }
    return action_code >= 8U && action_code <= 10U ? 3U : 1U;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    if (expect(select_entity_spawn_audio_event(1U, 0U) == 0x1CU) != 0 ||
        expect(select_entity_spawn_audio_event(8U, 0U) == 0x1EU) != 0 ||
        expect(select_entity_spawn_audio_event(11U, 0U) == 0x1DU) != 0 ||
        expect(select_entity_spawn_audio_event(1U, 0x02U) == 0x3FU) != 0 ||
        expect(select_entity_spawn_audio_event(8U, 0x02U) == 0x4AU) != 0 ||
        expect(!select_entity_spawn_audio_event(0U, 0U).has_value()) != 0 ||
        expect(!select_entity_spawn_audio_event(16U, 0U).has_value()) != 0) {
        return 26;
    }
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
               spawned_record.position_x == 0x1000U &&
               spawned_record.position_y == 0x20B0U &&
               spawned_record.heading == 0x8000U &&
               spawned_record.source_identifier == 0xFFFFU &&
               spawned_record.behavior_code == 0x0014U &&
               spawned_record.directional_probe_x == 0x1000U &&
               spawned_record.directional_probe_y == 0x2020U &&
               spawned_record.heading_offset_x == 0 &&
               spawned_record.heading_offset_y == 24575) != 0) {
        return 14;
    }

    constexpr std::array<std::uint16_t, 8> headings{
        0x0000U, 0x2000U, 0x4000U, 0x6000U,
        0x8000U, 0xA000U, 0xC000U, 0xE000U};
    constexpr std::array<std::int16_t, 8> source_position_x{
        0, 124, 176, 124, 0, -124, -176, -124};
    constexpr std::array<std::int16_t, 8> source_position_y{
        -176, -124, 0, 124, 176, 124, 0, -124};
    constexpr std::array<std::int16_t, 8> source_probe_x{
        64, 288, 384, 208, 0, -384, -384, -256};
    constexpr std::array<std::int16_t, 8> source_probe_y{
        -256, -240, -64, 80, 32, 64, -64, -192};
    for (std::size_t direction = 0U; direction < headings.size(); ++direction) {
        auto directional_parent = parent;
        directional_parent.position_x = 0x1000U;
        directional_parent.position_y = 0x2000U;
        directional_parent.heading = headings[direction];
        SpawnEntityRecordState directional_record{};
        std::optional<SpawnEntityLinkedState> no_directional_target;
        initialize_entity_spawn_record(
            directional_record, directional_parent, 0xFFFFU, true,
            no_directional_target, {});
        const auto add_source_delta = [](const std::uint16_t base,
                                         const std::int16_t delta) {
            return static_cast<std::uint16_t>(
                static_cast<std::int32_t>(base) + delta);
        };
        if (directional_record.position_x != add_source_delta(
                directional_parent.position_x,
                source_position_x[direction])) {
            return 27;
        }
        if (directional_record.position_y != add_source_delta(
                directional_parent.position_y,
                source_position_y[direction])) {
            return 27;
        }
        if (directional_record.directional_probe_x != add_source_delta(
                directional_parent.position_x,
                source_probe_x[direction])) {
            return 27;
        }
        if (directional_record.directional_probe_y != add_source_delta(
                directional_parent.position_y,
                source_probe_y[direction])) {
            return 27;
        }
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
    cycle_state.equipment = EntityEquipmentState{6U, 2};
    cycle_state.animation_counter = 0x0010U;
    cycle_state.global_mode = 2U;
    cycle_state.source_status_flags = 0x02U;
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
        [&cycle_events](const std::uint16_t audio_event) {
            cycle_events.push_back(audio_event == 0x4AU ? 'e' : 'x');
        },
        [&cycle_events](const std::uint16_t slot) {
            cycle_events.push_back(slot == 10U ? 'm' : 'x');
        },
        [&cycle_events](EntityEquipmentState& equipment,
                        const std::uint8_t source_action_code) {
            return service_entity_ammo(
                equipment, source_action_code,
                weapon_rounds_per_action,
                [&cycle_events, &equipment](const std::int8_t delta) {
                    cycle_events.push_back('w');
                    const auto adjusted =
                        static_cast<int>(equipment.current_ammo) + delta;
                    equipment.current_ammo = static_cast<std::uint8_t>(
                        adjusted > 0 ? adjusted : 0);
                });
        });
    if (expect(cycle.action == EntitySpawnCycleAction::spawned &&
               cycle.ammo_service_action ==
                   EntityAmmoServiceAction::delegated_to_weapon_service &&
               cycle_state.equipment.current_ammo == 3U &&
               cycle.primary.has_value() && cycle.secondary.has_value() &&
               cycle.primary_slot == 9U && cycle.secondary_slot == 10U &&
               cycle.selected_audio_event == 0x4AU &&
               cycle.special_follow_up &&
               cycle.primary->lifecycle_state == 2U &&
               cycle.primary->saved_position_x == 0x10B0U &&
               cycle.primary->field_0x89 == 0x20U &&
               cycle.primary->field_0x8b == 1U &&
               cycle.secondary->position_x == 0x1180U &&
               cycle.secondary->position_y == 0x1FC0U &&
               cycle.secondary->behavior_defaults.animation_state == 0U &&
               cycle.secondary->behavior_defaults.behavior_timer == 0x1000U &&
               cycle.secondary->animation_resource == 0x28U &&
               cycle.secondary->continuation_address == 0xAC4EU &&
               cycle.secondary->record_flags == 1U &&
               cycle_events ==
                   std::vector<char>({'g', 'w', 'a', 'r', 'p', 'a', 'e'})) != 0) {
        return 16;
    }

    EntitySpawnCycleState regular_state = cycle_state;
    regular_state.source_status_flags = 0U;
    regular_state.equipment.current_ammo = 3U;
    std::vector<std::uint16_t> regular_slots{9U, 10U};
    const auto regular_cycle = process_entity_spawn_cycle(
        regular_state,
        []() { return true; },
        [&regular_slots]() -> std::optional<std::uint16_t> {
            if (regular_slots.empty()) {
                return std::nullopt;
            }
            const auto slot = regular_slots.front();
            regular_slots.erase(regular_slots.begin());
            return slot;
        },
        {}, {}, {}, {},
        [](EntityEquipmentState& equipment,
           const std::uint8_t source_action_code) {
            return service_entity_ammo(
                equipment, source_action_code,
                weapon_rounds_per_action,
                [&equipment](const std::int8_t delta) {
                    const auto adjusted =
                        static_cast<int>(equipment.current_ammo) + delta;
                    equipment.current_ammo = static_cast<std::uint8_t>(
                        adjusted > 0 ? adjusted : 0);
                });
        });
    if (expect(regular_cycle.action == EntitySpawnCycleAction::spawned &&
               regular_state.equipment.current_ammo == 0U &&
               regular_cycle.selected_audio_event == 0x001EU) != 0) {
        return 18;
    }

    EntitySpawnCycleState unavailable_slot_state = cycle_state;
    unavailable_slot_state.equipment.current_ammo = 6U;
    std::vector<char> unavailable_slot_events;
    const auto unavailable_slot = process_entity_spawn_cycle(
        unavailable_slot_state,
        [&unavailable_slot_events]() {
            unavailable_slot_events.push_back('g');
            return true;
        },
        [&unavailable_slot_events]() -> std::optional<std::uint16_t> {
            unavailable_slot_events.push_back('a');
            return std::nullopt;
        },
        {}, {}, {}, {},
        [&unavailable_slot_events](EntityEquipmentState& equipment,
                                   const std::uint8_t source_action_code) {
            return service_entity_ammo(
                equipment, source_action_code,
                weapon_rounds_per_action,
                [&unavailable_slot_events, &equipment](
                    const std::int8_t delta) {
                    unavailable_slot_events.push_back('w');
                    const auto adjusted =
                        static_cast<int>(equipment.current_ammo) + delta;
                    equipment.current_ammo = static_cast<std::uint8_t>(
                        adjusted > 0 ? adjusted : 0);
                });
        });
    if (expect(unavailable_slot.action ==
                   EntitySpawnCycleAction::no_primary_slot &&
               unavailable_slot_state.equipment.current_ammo == 3U &&
               unavailable_slot_events ==
                   std::vector<char>({'g', 'w', 'a'})) != 0) {
        return 21;
    }

    EntitySpawnCycleState inactive_ammo_state = cycle_state;
    inactive_ammo_state.equipment.current_ammo = 0U;
    bool inactive_allocator_called = false;
    const auto inactive_ammo = process_entity_spawn_cycle(
        inactive_ammo_state,
        []() { return true; },
        [&inactive_allocator_called]() -> std::optional<std::uint16_t> {
            inactive_allocator_called = true;
            return 9U;
        },
        {}, {}, {}, {},
        [](EntityEquipmentState& equipment,
           const std::uint8_t source_action_code) {
            return service_entity_ammo(
                equipment, source_action_code, weapon_rounds_per_action,
                [](const std::int8_t) {});
        });
    if (expect(inactive_ammo.action ==
                   EntitySpawnCycleAction::ammo_service_inactive &&
               inactive_ammo.ammo_service_action ==
                   EntityAmmoServiceAction::no_action &&
               !inactive_allocator_called)) {
        return 19;
    }

    bool unresolved_allocator_called = false;
    const auto unresolved_ammo = process_entity_spawn_cycle(
        cycle_state,
        []() { return true; },
        [&unresolved_allocator_called]() -> std::optional<std::uint16_t> {
            unresolved_allocator_called = true;
            return 9U;
        },
        {}, {}, {}, {});
    if (expect(unresolved_ammo.action ==
                   EntitySpawnCycleAction::ammo_service_unresolved &&
               !unresolved_allocator_called)) {
        return 20;
    }

    EntitySpawnCycleState expired_state = cycle_state;
    expired_state.animation_counter = 0x003DU;
    expired_state.entity_flags = 0x02U;
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

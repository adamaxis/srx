#include "shadowrun_ghidra/entity_effect_dispatch_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityEffectDispatchSource type_six_source{
        0x1000U, 0x2000U, 0x06U, 0x44U, 9U};
    std::array<EntityEffectDispatchTarget, 10> entities{};
    entities[0] = EntityEffectDispatchTarget{
        true, false, 0x1200U, 0x2000U, 0x1111U, 0x2222U};
    entities[1] = EntityEffectDispatchTarget{
        true, false, 0x1400U, 0x2000U, 0U, 0U};
    entities[2] = EntityEffectDispatchTarget{
        true, true, 0x1200U, 0x2000U, 0U, 0U};
    const auto scan = scan_entity_effect_targets(
        type_six_source, 0U, entities);
    if (!expect(scan.recognized_behavior &&
                scan.selected_nearby_target == std::optional<std::size_t>(0) &&
                scan.linked_field_0x0A == 0x1111U &&
                scan.linked_field_0x0C == 0x2222U &&
                entities[0].scan_type == 0x06U &&
                (entities[0].scan_flags & 0x01U) != 0U &&
                entities[0].parent_source == std::optional<std::uint16_t>(9U) &&
                entities[0].effect_type == 0x06U &&
                entities[1].scan_type == 0x06U &&
                (entities[1].scan_flags & 0x01U) != 0U &&
                entities[1].effect_type == 0U &&
                entities[2].scan_type == 0U)) {
        return 1;
    }

    EntityEffectDispatchSource type_three_source{
        0x1000U, 0x2000U, 0x03U, 0x12U, 4U};
    std::array<EntityEffectDispatchTarget, 10> marking_entities{};
    marking_entities[0] = EntityEffectDispatchTarget{
        true, false, 0x1800U, 0x2000U, 0xAAAAU, 0xBBBBU};
    marking_entities[1] = EntityEffectDispatchTarget{
        true, false, 0x1800U, 0x2200U, 0U, 0U};
    const auto marking_scan = scan_entity_effect_targets(
        type_three_source, 0U, marking_entities);
    if (!expect(marking_scan.recognized_behavior &&
                !marking_scan.selected_nearby_target.has_value() &&
                marking_entities[0].scan_type == 0x03U &&
                marking_entities[1].scan_type == 0x03U &&
                marking_entities[0].scan_flags == 0U &&
                !marking_entities[0].parent_source.has_value())) {
        return 2;
    }

    std::vector<int> events;
    EntityEffectDispatchCallbacks callbacks{
        [&events]() { events.push_back(1); },
        [&events](const std::uint8_t type,
                  const std::uint16_t audio,
                  const std::size_t target) {
            events.push_back(type == 0x06U && audio == 0x0038U &&
                                     target == 0U
                                 ? 2
                                 : 9);
        },
        [&events](const std::uint8_t type, const std::size_t target) {
            events.push_back(type == 0x06U && target == 0U ? 3 : 9);
        },
        {}};
    std::array<std::uint8_t, 18> effect_slots{};
    const auto dispatched = dispatch_entity_effect(
        type_six_source, 0U, entities, effect_slots, callbacks);
    if (!expect(dispatched.action ==
                    EntityEffectDispatchAction::applied_to_nearby_target &&
                dispatched.target_index == std::optional<std::size_t>(0) &&
                dispatched.allocation_performed &&
                dispatched.allocation.slot == 0U &&
                dispatched.allocation.record_parameter == 0x0049U &&
                dispatched.recognized_behavior &&
                dispatched.linked_field_0x0A == 0x1111U &&
                dispatched.linked_field_0x0C == 0x2222U &&
                events == std::vector<int>({1, 2, 3}))) {
        return 3;
    }
    if (!expect(entities[0].effect_type == 0x06U &&
                entities[0].effect_slot == 0U &&
                entities[0].effect_record_parameter == 0x0049U &&
                entities[0].effect_phase == 5U &&
                entities[0].effect_subphase == 0U &&
                entities[0].parent_source ==
                    std::optional<std::uint16_t>(9U) &&
                effect_slots[0] == 1U)) {
        return 4;
    }

    bool direct_called = false;
    EntityEffectDispatchCallbacks direct_callbacks{
        {}, {}, {}, [&direct_called](const EntityEffectDispatchSource& source) {
            direct_called = source.behavior_type == 0x07U;
        }};
    std::array<std::uint8_t, 18> direct_slots{};
    const auto direct = dispatch_entity_effect(
        EntityEffectDispatchSource{0U, 0U, 0x07U, 0U, 1U}, std::nullopt,
        entities, direct_slots, direct_callbacks);
    if (!expect(direct.action ==
                    EntityEffectDispatchAction::direct_spawn_boundary &&
                direct_called && direct_slots[0] == 0U)) {
        return 5;
    }

    const auto missing = dispatch_entity_effect(
        type_six_source, std::nullopt, entities, direct_slots, callbacks);
    if (!expect(missing.action ==
                    EntityEffectDispatchAction::no_linked_target &&
                !missing.allocation_performed && direct_slots[0] == 0U)) {
        return 6;
    }

    std::array<std::uint8_t, 18> full_slots{};
    full_slots.fill(1U);
    const auto full = dispatch_entity_effect(
        type_three_source, 0U, marking_entities, full_slots, callbacks);
    if (!expect(full.action == EntityEffectDispatchAction::effect_table_full &&
                !full.allocation_performed && full.target_index ==
                    std::nullopt)) {
        return 7;
    }

    EntityEffectDispatchSource no_nearby_source{
        0x1000U, 0x2000U, 0x0DU, 0U, 2U};
    std::array<EntityEffectDispatchTarget, 10> distant_entities{};
    distant_entities[0] = EntityEffectDispatchTarget{
        false, false, 0x1000U, 0x2000U, 1U, 2U};
    distant_entities[1] = EntityEffectDispatchTarget{
        true, false, 0x1FFFU, 0x2FFFU, 0U, 0U};
    std::array<std::uint8_t, 18> no_nearby_slots{};
    const auto no_nearby = dispatch_entity_effect(
        no_nearby_source, 0U, distant_entities, no_nearby_slots, {});
    if (!expect(no_nearby.action ==
                    EntityEffectDispatchAction::nearby_target_not_found &&
                no_nearby.allocation_performed && no_nearby_slots[0] == 1U &&
                distant_entities[0].parent_source ==
                    std::optional<std::uint16_t>(2U))) {
        return 8;
    }

    std::vector<int> secondary_events;
    EntityEffectDispatchCallbacks secondary_callbacks{
        {},
        [&secondary_events](std::uint8_t type,
                            std::uint16_t audio,
                            std::size_t target) {
            secondary_events.push_back(type == 2U && audio == 0x34U &&
                                               target == 1U
                                           ? 1
                                           : 9);
        },
        [&secondary_events](std::uint8_t type, std::size_t target) {
            secondary_events.push_back(type == 2U && target == 1U ? 2 : 9);
        },
        {}};
    const auto secondary = apply_entity_secondary_effect(
        1U, marking_entities, effect_slots, secondary_callbacks);
    if (!expect(secondary.action == EntitySecondaryEffectAction::applied &&
                secondary.allocation_performed &&
                marking_entities[1].effect_type == 2U &&
                marking_entities[1].effect_slot == 1U &&
                marking_entities[1].effect_record_parameter == 0x0061U &&
                marking_entities[1].effect_phase == 0U &&
                marking_entities[1].effect_subphase == 0U &&
                secondary_events == std::vector<int>({1, 2}))) {
        return 9;
    }

    if (!expect(apply_entity_secondary_effect(
                    99U, marking_entities, effect_slots, {})
                    .action == EntitySecondaryEffectAction::invalid_target)) {
        return 10;
    }

    return 0;
}

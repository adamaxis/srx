#include "shadowrun_ghidra/entities/entity_effect_spawn_conversion.hpp"

#include <algorithm>
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

    EntityEffectSpawnParent parent{
        0x1000U, 0x2000U, 0x0000U, 0x0010U,
        0x12U, 0x03U, 0x44U, 0x9DU};
    std::vector<int> events;
    EntityEffectSpawnCallbacks callbacks{
        [&events]() -> std::optional<std::uint16_t> {
            events.push_back(1);
            return 7U;
        },
        [&events](const std::int16_t amount) {
            events.push_back(amount == -1 ? 2 : 9);
        },
        [&events]() { events.push_back(3); },
        {}};
    std::array<std::uint8_t, 18> effect_slots{};
    const auto no_target = spawn_entity_effect_record(
        parent, std::nullopt, effect_slots, callbacks);
    if (!expect(no_target.action ==
                    EntityEffectSpawnAction::spawned_without_target &&
                !no_target.source_carry && no_target.record.has_value() &&
                events == std::vector<int>({2, 1, 3}))) {
        return 1;
    }
    const auto& no_target_record = *no_target.record;
    if (!expect(no_target_record.entity_slot == 7U &&
                no_target_record.world_x == 0x1000U &&
                // ROM 0x15A4C[0] = (0, -0xB0), 0x15A6C[0] = (0x40, -0x100).
                no_target_record.world_y == 0x1F50U &&
                no_target_record.directional_probe_x == 0x1040U &&
                no_target_record.directional_probe_y == 0x1F00U &&
                no_target_record.heading == 0x0000U &&
                no_target_record.movement_scale == 0x2000U &&
                no_target_record.animation_resource == 0U &&
                no_target_record.effect_slot == 0U &&
                no_target_record.effect_record_parameter == 0x0049U &&
                no_target_record.effect_auxiliary_offset == 0x0920U &&
                no_target_record.effect_distance_steps == 0x00B4U &&
                no_target_record.movement_offset == -0x500 &&
                no_target_record.vertical_velocity == 0xF400U &&
                no_target_record.lifetime == 0x0013U &&
                no_target_record.behavior_type == 2U &&
                no_target_record.field_0x56 == 0x12U &&
                no_target_record.field_0x57 == 0x03U &&
                no_target_record.field_0x9D == 0x9DU &&
                no_target_record.source_marker == 0x22U &&
                no_target_record.active &&
                no_target_record.continuation_address == 0x15390U &&
                no_target_record.behavior_defaults.vertical_offset == -12 &&
                no_target_record.behavior_defaults.animation_state == 1U &&
                no_target_record.behavior_defaults.behavior_timer == 0x1000U &&
                no_target_record.behavior_defaults.step_interval == 0x4000U &&
                no_target_record.heading_offset_x == 0 &&
                no_target_record.heading_offset_y == -0x2000)) {
        return 2;
    }
    if (!expect(effect_slots[0] == 1U &&
                std::all_of(effect_slots.begin() + 1, effect_slots.end(),
                            [](const std::uint8_t value) { return value == 0U; }))) {
        return 3;
    }

    EntityEffectSpawnCallbacks linked_callbacks{
        []() -> std::optional<std::uint16_t> { return 4U; },
        {},
        {},
        [](const EntityEffectSpawnTarget& target, const std::uint16_t heading)
            -> std::optional<EntityEffectSpawnTargetResolution> {
            if (target.world_x != 0x1800U || target.world_y != 0x2000U ||
                heading != 0U) {
                return std::nullopt;
            }
            return EntityEffectSpawnTargetResolution{
                EntityEffectSpawnTarget{0x1800U, 0x2000U}, 0x77U};
        }};
    std::array<std::uint8_t, 18> linked_slots{};
    const auto linked = spawn_entity_effect_record(
        parent, EntityEffectSpawnTarget{0x1800U, 0x2000U}, linked_slots,
        linked_callbacks);
    if (!expect(linked.action ==
                    EntityEffectSpawnAction::spawned_toward_target &&
                !linked.source_carry && linked.record.has_value())) {
        return 4;
    }
    const auto& linked_record = *linked.record;
    // 0x15DB0 basis (630a4e97): +x is heading 0x4000.
    if (!expect(linked_record.source_marker == 0x77U &&
                linked_record.heading == 0x4000U &&
                linked_record.effect_distance_steps == 0x20U &&
                linked_record.movement_offset == -0x200 &&
                linked_record.heading_offset_x == 0x3FFF &&
                linked_record.heading_offset_y == 0 &&
                linked_record.world_x == 0x1000U &&
                linked_record.world_y == 0x1F50U)) {
        return 5;
    }

    EntityEffectSpawnParent expired_parent = parent;
    expired_parent.animation_counter = 0x003DU;
    bool called = false;
    EntityEffectSpawnCallbacks expired_callbacks{
        [&called]() -> std::optional<std::uint16_t> {
            called = true;
            return 1U;
        },
        [&called](std::int16_t) { called = true; },
        [&called]() { called = true; },
        {}};
    std::array<std::uint8_t, 18> expired_slots{};
    const auto expired = spawn_entity_effect_record(
        expired_parent, std::nullopt, expired_slots, expired_callbacks);
    if (!expect(expired.action ==
                    EntityEffectSpawnAction::animation_window_expired &&
                expired.source_carry && !expired.record.has_value() &&
                !called)) {
        return 6;
    }

    EntityEffectSpawnCallbacks empty_allocator{
        []() -> std::optional<std::uint16_t> { return std::nullopt; }, {}, {}, {}};
    const auto no_entity = spawn_entity_effect_record(
        parent, std::nullopt, expired_slots, empty_allocator);
    if (!expect(no_entity.action == EntityEffectSpawnAction::no_entity_slot &&
                no_entity.source_carry && !no_entity.record.has_value())) {
        return 7;
    }

    std::array<std::uint8_t, 18> full_slots{};
    full_slots.fill(1U);
    const auto full = spawn_entity_effect_record(
        parent, std::nullopt, full_slots, callbacks);
    if (!expect(full.action == EntityEffectSpawnAction::effect_table_full &&
                full.source_carry && full.record.has_value() &&
                !full.record->active && full.record->effect_slot == 0U &&
                full.record->effect_record_parameter == 0U)) {
        return 8;
    }

    EntityEffectSpawnCallbacks unresolved_callbacks{
        []() -> std::optional<std::uint16_t> { return 2U; }, {}, {},
        [](const EntityEffectSpawnTarget&, std::uint16_t)
            -> std::optional<EntityEffectSpawnTargetResolution> {
            return std::nullopt;
        }};
    std::array<std::uint8_t, 18> unresolved_slots{};
    const auto unresolved = spawn_entity_effect_record(
        parent, EntityEffectSpawnTarget{0x1800U, 0x2000U}, unresolved_slots,
        unresolved_callbacks);
    if (!expect(unresolved.action ==
                    EntityEffectSpawnAction::target_transform_unresolved &&
                unresolved.source_carry && unresolved.record.has_value() &&
                unresolved.record->active)) {
        return 9;
    }

    return 0;
}

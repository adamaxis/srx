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

shadowrun::ghidra_conversion::EntityEffectSpawnRecord north_record() {
    using namespace shadowrun::ghidra_conversion;
    EntityEffectSpawnRecord record{};
    record.world_x = 0x1000U;
    record.world_y = 0x1F50U;
    record.heading = 0U;
    record.movement_scale = 0x2000U;
    record.animation_resource = 0U;
    record.effect_distance_steps = 0x00B4U;
    record.movement_offset = -0x0500;
    record.vertical_velocity = 0xF400U;
    record.lifetime = 0x0013U;
    record.directional_probe_x = 0x1040U;
    record.directional_probe_y = 0x1F00U;
    record.heading_offset_x = 0;
    record.heading_offset_y = -0x2000;
    record.behavior_type = 2U;
    record.behavior = EntityEffectSpawnBehaviorKind::thrown_item_effect;
    record.field_0x56 = 0x12U;
    record.field_0x57 = 0x0DU;
    record.field_0x9D = 0x9DU;
    record.source_marker = 0x22U;
    return record;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityEffectSpawnParent parent{
        0x1000U, 0x2000U, 0x0000U, 0x0010U,
        0x12U, 0x0DU, 0x44U, 0x9DU};
    std::vector<int> events;
    EntityEffectSpawnCallbacks callbacks{
        [&events]() -> std::optional<std::uint16_t> {
            events.push_back(3);
            return 7U;
        },
        [&events](const std::int16_t amount) {
            events.push_back(amount == -1 ? 2 : 9);
        },
        [&events]() { events.push_back(4); },
        {},
        [&events](const std::uint16_t counter) {
            events.push_back(1);
            return counter == 0x0010U;
        },
        [&events](const EntityEffectSpawnParent& source_parent,
                  const std::optional<EntityEffectSpawnTargetResolution>& target)
            -> std::optional<EntityEffectSpawnRecord> {
            events.push_back(target.has_value() ? 6 : 5);
            if (source_parent.world_x != 0x1000U) return std::nullopt;
            if (target.has_value()) return std::nullopt;
            return north_record();
        }};

    std::array<std::uint8_t, 18> effect_slots{};
    const auto no_target = spawn_entity_effect_record(
        parent, std::nullopt, effect_slots, callbacks);
    if (!expect(no_target.action ==
                    EntityEffectSpawnAction::spawned_without_target &&
                !no_target.source_carry && no_target.record.has_value() &&
                events == std::vector<int>({1, 2, 3, 4, 5}))) {
        return 1;
    }
    const auto& record = *no_target.record;
    if (!expect(record.entity_slot == 7U && record.world_x == 0x1000U &&
                record.world_y == 0x1F50U &&
                record.directional_probe_x == 0x1040U &&
                record.directional_probe_y == 0x1F00U &&
                record.effect_slot == 0U &&
                record.effect_record_parameter == 0x0049U &&
                record.effect_auxiliary_offset == 0x0920U &&
                record.behavior_defaults.vertical_offset == -12 &&
                record.behavior_defaults.animation_state == 1U &&
                record.behavior_defaults.behavior_timer == 0x1000U &&
                record.behavior_defaults.step_interval == 0x4000U &&
                record.behavior ==
                    EntityEffectSpawnBehaviorKind::thrown_item_effect &&
                record.active && record.heading_offset_y == -0x2000)) {
        return 2;
    }
    if (!expect(effect_slots[0] == 1U &&
                std::all_of(effect_slots.begin() + 1, effect_slots.end(),
                            [](const std::uint8_t value) { return value == 0U; }))) {
        return 3;
    }

    events.clear();
    EntityEffectSpawnParent expired_parent = parent;
    expired_parent.animation_counter = 0x003DU;
    auto expired_callbacks = callbacks;
    expired_callbacks.animation_window_open = [&events](std::uint16_t) {
        events.push_back(1);
        return false;
    };
    std::array<std::uint8_t, 18> expired_slots{};
    const auto expired = spawn_entity_effect_record(
        expired_parent, std::nullopt, expired_slots, expired_callbacks);
    if (!expect(expired.action ==
                    EntityEffectSpawnAction::animation_window_expired &&
                expired.source_carry && !expired.record.has_value() &&
                events == std::vector<int>({1}))) {
        return 4;
    }

    events.clear();
    auto empty_allocator = callbacks;
    empty_allocator.allocate_entity_slot = [&events]()
        -> std::optional<std::uint16_t> {
        events.push_back(3);
        return std::nullopt;
    };
    std::array<std::uint8_t, 18> no_entity_slots{};
    const auto no_entity = spawn_entity_effect_record(
        parent, std::nullopt, no_entity_slots, empty_allocator);
    if (!expect(no_entity.action == EntityEffectSpawnAction::no_entity_slot &&
                no_entity.source_carry && !no_entity.record.has_value() &&
                events == std::vector<int>({1, 2, 3}))) {
        return 5;
    }

    events.clear();
    std::array<std::uint8_t, 18> full_slots{};
    full_slots.fill(1U);
    const auto full = spawn_entity_effect_record(
        parent, std::nullopt, full_slots, callbacks);
    if (!expect(full.action == EntityEffectSpawnAction::effect_table_full &&
                full.source_carry && full.record.has_value() &&
                !full.record->active && full.record->effect_slot == 0U &&
                events == std::vector<int>({1, 2, 3, 4, 5}))) {
        return 6;
    }

    events.clear();
    auto linked_callbacks = callbacks;
    linked_callbacks.transform_target = [&events](
        const EntityEffectSpawnTarget& target, const std::uint16_t heading)
        -> std::optional<EntityEffectSpawnTargetResolution> {
        events.push_back(6);
        if (target.world_x != 0x1800U || target.world_y != 0x2000U ||
            heading != 0U) {
            return std::nullopt;
        }
        return EntityEffectSpawnTargetResolution{
            0x4000U, 0x0800U, 0x77U};
    };
    linked_callbacks.build_record = [&events](
        const EntityEffectSpawnParent&,
        const std::optional<EntityEffectSpawnTargetResolution>& target)
        -> std::optional<EntityEffectSpawnRecord> {
        events.push_back(target.has_value() ? 7 : 5);
        auto result = north_record();
        if (!target.has_value()) return result;
        if (target->heading != 0x4000U || target->distance != 0x0800U ||
            target->source_marker != 0x77U) {
            return std::nullopt;
        }
        result.heading = 0x4000U;
        result.effect_distance_steps = 0x20U;
        result.movement_offset = -0x200;
        result.heading_offset_x = 0x3FFF;
        result.heading_offset_y = 0;
        result.source_marker = target->source_marker;
        return result;
    };
    std::array<std::uint8_t, 18> linked_slots{};
    const auto linked = spawn_entity_effect_record(
        parent, EntityEffectSpawnTarget{0x1800U, 0x2000U}, linked_slots,
        linked_callbacks);
    if (!expect(linked.action ==
                    EntityEffectSpawnAction::spawned_toward_target &&
                !linked.source_carry && linked.record.has_value() &&
                linked.record->entity_slot == 7U &&
                linked.record->effect_slot == 0U &&
                linked.record->heading == 0x4000U &&
                linked.record->source_marker == 0x77U &&
                linked.record->effect_distance_steps == 0x20U &&
                linked.record->movement_offset == -0x200 &&
                linked.record->heading_offset_x == 0x3FFF &&
                linked.record->heading_offset_y == 0 &&
                events == std::vector<int>({1, 2, 3, 4, 5, 6, 7}))) {
        return 7;
    }

    events.clear();
    auto unresolved_callbacks = callbacks;
    unresolved_callbacks.transform_target = [](
        const EntityEffectSpawnTarget&, std::uint16_t)
        -> std::optional<EntityEffectSpawnTargetResolution> {
        return std::nullopt;
    };
    std::array<std::uint8_t, 18> unresolved_slots{};
    const auto unresolved = spawn_entity_effect_record(
        parent, EntityEffectSpawnTarget{0x1800U, 0x2000U}, unresolved_slots,
        unresolved_callbacks);
    if (!expect(unresolved.action ==
                    EntityEffectSpawnAction::target_transform_unresolved &&
                unresolved.source_carry && unresolved.record.has_value() &&
                unresolved.record->active)) {
        return 8;
    }

    return 0;
}

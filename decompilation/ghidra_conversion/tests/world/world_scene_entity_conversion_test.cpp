#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const SceneEntityRecord source_record{0x0120U, 0x0340U, 0U, 0U};
    if (spawn_scene_record_offset(source_record, 0x0065U, true).has_value()) {
        return 1;
    }
    const auto offset_record =
        spawn_scene_record_offset(source_record, 0x0065U, false);
    if (!offset_record.has_value() || offset_record->x != 0x01A0U ||
        offset_record->y != 0x03C0U || offset_record->kind != 0x0065U ||
        offset_record->flags != 1U) {
        return 1;
    }

    WorldMapEffectInput special{};
    special.world_mode = 1U;
    special.scene_id = 0x14U;
    special.action_requested = true;
    special.row_stride = 20;
    special.map_value_a = 0x1234U;
    special.map_value_b = 0x5678U;
    special.coordinate = {0x4000U, 0x2000U, 0x0000U};
    const auto special_result = apply_world_map_effect(special);
    if (!expect(special_result.action ==
                    WorldMapEffectAction::special_scene_map_updated &&
                special_result.map_marker_cleared &&
                special_result.world_effect_x_dirty &&
                special_result.map_writes.size() == 2U &&
                special_result.map_writes[0].value == 0x1234U &&
                special_result.map_writes[1].offset -
                        special_result.map_writes[0].offset ==
                    20)) {
        return 1;
    }

    WorldMapEffectInput horizontal = special;
    horizontal.scene_id = 0x10U;
    horizontal.world_mode = 2U;
    horizontal.entity_flags = 0x00A0U;
    horizontal.coordinate = {0x4000U, 0x2000U, 0x8000U};
    const auto horizontal_result = apply_world_map_effect(horizontal);
    if (!expect(horizontal_result.action ==
                    WorldMapEffectAction::horizontal_effect_map_updated &&
                horizontal_result.world_effect_y_dirty &&
                horizontal_result.map_writes.size() == 3U &&
                horizontal_result.map_writes[0].value == 3U &&
                horizontal_result.map_writes[1].value == 4U &&
                horizontal_result.map_writes[2].value == 5U)) {
        return 2;
    }

    WorldMapEffectInput auxiliary{};
    auxiliary.scene_id = 4U;
    auxiliary.entity_flags = 0x00C0U;
    auxiliary.auxiliary_world_y = 0x1000U;
    auxiliary.auxiliary_world_y_delta = 0x20;
    auxiliary.auxiliary_entity_slot_available = true;
    const auto auxiliary_result = apply_world_map_effect(auxiliary);
    if (!expect(auxiliary_result.action ==
                    WorldMapEffectAction::auxiliary_entity_initialized &&
                auxiliary_result.auxiliary_entity.has_value() &&
                auxiliary_result.auxiliary_entity->world_y == 0x1020 &&
                auxiliary_result.auxiliary_entity->handler == 0x1669AU)) {
        return 3;
    }

    SceneEntitySpawnInput scene_record{};
    scene_record.record_gate = 1;
    scene_record.record_kind = 0x0CU;
    scene_record.scene_event_flags = 0x02U;
    scene_record.source_entity_slot = 7U;
    scene_record.source_x = 0x1000U;
    scene_record.source_y = 0x2000U;
    const auto scene_result = spawn_world_scene_entity(scene_record);
    if (!expect(scene_result.action ==
                    SceneEntitySpawnAction::scene_record_spawned &&
                scene_result.record.has_value() &&
                scene_result.record->kind == 0x65U &&
                scene_result.record->source_entity_slot == 7U &&
                scene_result.record->behavior_handler == 0x49A0U)) {
        return 4;
    }

    SceneEntitySpawnInput weighted{};
    weighted.record_kind = 7U;
    weighted.world_mode = 0U;
    weighted.scene_record_slots_available = true;
    weighted.source_x = 0x10U;
    weighted.source_y = 0x20U;
    weighted.random_value = 8U;
    weighted.default_selections = {{0x5AU, 10U}};
    weighted.equipped_slot = 2;
    weighted.equipped_item_code = 4U;
    const auto weighted_result = spawn_world_scene_entity(weighted);
    if (!expect(weighted_result.action ==
                    SceneEntitySpawnAction::scene_record_spawned &&
                weighted_result.record.has_value() &&
                weighted_result.record->kind == 4U &&
                weighted_result.record->x == 0x90U)) {
        return 5;
    }

    weighted.equipped_item_code = 0U;
    const auto cleared_result = spawn_world_scene_entity(weighted);
    if (!expect(cleared_result.action ==
                SceneEntitySpawnAction::weighted_selection_cleared)) {
        return 6;
    }

    weighted.default_selections = {{1U, 2U}, {0x5BU, 8U}};
    weighted.random_value = 3U;
    const auto sentinel_result = spawn_world_scene_entity(weighted);
    if (!expect(sentinel_result.action ==
                SceneEntitySpawnAction::weighted_selection_cleared)) {
        return 7;
    }

    return 0;
}

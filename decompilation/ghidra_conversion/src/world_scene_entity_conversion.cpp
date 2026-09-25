#include "shadowrun_ghidra/world_scene_entity_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

std::optional<std::uint16_t> select_weighted(
    const std::vector<WeightedWorldSelection>& selections,
    std::uint16_t random_value) {
    for (const auto& selection : selections) {
        if (random_value < selection.weight) {
            return selection.result;
        }
        random_value = static_cast<std::uint16_t>(random_value -
                                                  selection.weight);
    }
    return std::nullopt;
}

} // namespace

WorldMapEffectResult apply_world_map_effect(
    const WorldMapEffectInput& input) {
    WorldMapEffectResult result{};
    if (input.world_mode == 0U) {
        if (input.scene_id == 4U && (input.entity_flags & 0x001FU) == 6U) {
            result.special_resource_selected = true;
            result.action = WorldMapEffectAction::special_resource_selected;
        }
        if ((input.entity_flags & 0x00E0U) == 0x00C0U &&
            input.auxiliary_entity_slot_available) {
            result.action = WorldMapEffectAction::auxiliary_entity_initialized;
            result.auxiliary_entity = AuxiliaryWorldEntity{
                -1,
                static_cast<std::int16_t>(input.auxiliary_world_y +
                                          input.auxiliary_world_y_delta),
                3U,
                3U,
                0x1669AU};
        } else if ((input.entity_flags & 0x00E0U) == 0x00C0U) {
            result.action =
                WorldMapEffectAction::auxiliary_entity_slot_unavailable;
        }
        return result;
    }

    if (!input.action_requested) {
        return result;
    }

    if (input.scene_id == 0x14U) {
        result.map_marker_cleared = true;
        result.cleared_map_marker = input.map_marker_index;
        const auto projected = project_entity_y_axis(input.coordinate,
                                                     input.row_stride);
        result.map_writes.push_back({projected, input.map_value_a});
        result.map_writes.push_back(
            {projected + input.row_stride, input.map_value_b});
        result.world_effect_x_dirty = true;
        result.action = WorldMapEffectAction::special_scene_map_updated;
        return result;
    }

    if (input.world_mode != 2U) {
        return result;
    }

    result.map_marker_cleared = true;
    result.cleared_map_marker = input.map_marker_index;
    if ((input.entity_flags & 0x00E0U) == 0x00A0U) {
        const auto projected =
            project_entity_x_axis(input.coordinate, input.row_stride);
        result.map_writes.push_back(
            {projected - input.row_stride, 3U});
        result.map_writes.push_back({projected, 4U});
        result.map_writes.push_back(
            {projected + input.row_stride, 5U});
        result.world_effect_y_dirty = true;
        result.action = WorldMapEffectAction::horizontal_effect_map_updated;
        return result;
    }

    const auto projected =
        project_entity_y_axis(input.coordinate, input.row_stride);
    result.map_writes.push_back({projected, 6U});
    result.map_writes.push_back({projected + input.row_stride, 7U});
    result.world_effect_x_dirty = true;
    result.action = WorldMapEffectAction::vertical_effect_map_updated;
    return result;
}

std::optional<SceneEntityRecord> spawn_scene_record_offset(
    const SceneEntityRecord& source,
    const std::uint16_t selected_kind,
    const bool all_slots_active) {
    if (all_slots_active) {
        return std::nullopt;
    }

    SceneEntityRecord record{};
    record.x = static_cast<std::uint16_t>(source.x + 0x80U);
    record.y = static_cast<std::uint16_t>(source.y + 0x80U);
    record.kind = selected_kind;
    record.flags = 0x01U;
    return record;
}

SceneEntitySpawnResult spawn_world_scene_entity(
    const SceneEntitySpawnInput& input) {
    SceneEntitySpawnResult result{};
    const bool special_kind_path = input.record_gate != 0 ||
                                   input.record_kind != 7U;

    if (special_kind_path && input.record_kind == 0x0DU) {
        if ((input.scene_event_flags & 0x01U) == 0U) {
            result.action = SceneEntitySpawnAction::blocked_by_scene_flag;
            return result;
        }
        if (!input.scene_record_slots_available) {
            result.action = SceneEntitySpawnAction::no_scene_record_slot;
            return result;
        }
        result.record = SceneEntityRecord{
            static_cast<std::uint16_t>(input.source_x + 0x80U),
            static_cast<std::uint16_t>(input.source_y + 0x80U),
            100U,
            0x01U,
            std::nullopt,
            0U,
            0U};
        result.action = SceneEntitySpawnAction::scene_record_spawned;
        return result;
    }

    if (special_kind_path && input.record_kind == 0x0CU) {
        if ((input.scene_event_flags & 0x02U) == 0U) {
            result.action = SceneEntitySpawnAction::blocked_by_scene_flag;
            return result;
        }
        if (!input.scene_record_slots_available) {
            result.action = SceneEntitySpawnAction::no_scene_record_slot;
            return result;
        }
        result.record = SceneEntityRecord{
            static_cast<std::uint16_t>(input.source_x + 0x80U),
            static_cast<std::uint16_t>(input.source_y + 0x80U),
            0x65U,
            0x01U,
            input.source_entity_slot,
            0x80U,
            0x49A0U};
        result.action = SceneEntitySpawnAction::scene_record_spawned;
        return result;
    }

    if ((input.world_mode != 2U && input.world_mode != 0U) ||
        (special_kind_path && input.record_kind == 0x0EU)) {
        result.action = SceneEntitySpawnAction::filtered;
        return result;
    }
    if (input.pending_scene_record) {
        result.action = SceneEntitySpawnAction::pending_scene_record;
        return result;
    }
    if (!input.scene_record_slots_available) {
        result.action = SceneEntitySpawnAction::no_scene_record_slot;
        return result;
    }

    const std::vector<WeightedWorldSelection>* selections =
        &input.default_selections;
    if (input.world_mode == 2U) {
        selections = &input.mode_two_selections;
    } else if (input.record_gate != 0 || input.record_kind != 7U) {
        const auto scene_index = static_cast<std::size_t>(input.scene_id);
        if (scene_index >= input.scene_selections.size()) {
            result.action = SceneEntitySpawnAction::weighted_selection_empty;
            return result;
        }
        selections = &input.scene_selections[scene_index];
    }

    const auto selected = select_weighted(*selections, input.random_value);
    if (!selected.has_value()) {
        result.action = SceneEntitySpawnAction::weighted_selection_empty;
        return result;
    }
    if (*selected == 0x5BU) {
        result.action = SceneEntitySpawnAction::weighted_selection_cleared;
        return result;
    }

    auto selected_kind = *selected;
    if (selected_kind == 0x5AU) {
        if (input.equipped_slot < 0 || input.equipped_item_code == 0U ||
            input.equipped_item_code > 0x0EU) {
            result.action = SceneEntitySpawnAction::weighted_selection_cleared;
            return result;
        }
        selected_kind = input.equipped_item_code;
    }

    result.record = SceneEntityRecord{
        static_cast<std::uint16_t>(input.source_x + 0x80U),
        static_cast<std::uint16_t>(input.source_y + 0x80U),
        selected_kind,
        0x01U,
        std::nullopt,
        0U,
        0U};
    result.action = SceneEntitySpawnAction::scene_record_spawned;
    return result;
}

} // namespace shadowrun::ghidra_conversion

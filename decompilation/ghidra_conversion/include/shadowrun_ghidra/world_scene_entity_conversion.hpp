#pragma once

#include "shadowrun_ghidra/menu_transfer_batch_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MapWordWrite {
    std::int32_t offset = 0;
    std::uint16_t value = 0;
};

struct AuxiliaryWorldEntity {
    std::int16_t state = -1;
    std::int16_t world_y = 0;
    std::uint16_t behavior = 3;
    std::uint16_t mode = 3;
    std::uint32_t handler = 0x1669AU;
};

struct WorldMapEffectInput {
    std::uint16_t world_mode = 0;
    std::uint16_t scene_id = 0;
    std::uint16_t entity_flags = 0;
    bool action_requested = false;
    std::uint16_t map_marker_index = 0;
    std::int32_t row_stride = 0;
    std::uint16_t map_value_a = 0;
    std::uint16_t map_value_b = 0;
    std::uint16_t auxiliary_world_y = 0;
    std::int16_t auxiliary_world_y_delta = 0;
    CoordinateProjectionInput coordinate{};
    bool auxiliary_entity_slot_available = false;
};

enum class WorldMapEffectAction {
    no_action,
    special_resource_selected,
    auxiliary_entity_initialized,
    auxiliary_entity_slot_unavailable,
    special_scene_map_updated,
    horizontal_effect_map_updated,
    vertical_effect_map_updated,
};

struct WorldMapEffectResult {
    WorldMapEffectAction action = WorldMapEffectAction::no_action;
    bool map_marker_cleared = false;
    bool world_effect_x_dirty = false;
    bool world_effect_y_dirty = false;
    bool special_resource_selected = false;
    std::optional<std::uint16_t> cleared_map_marker;
    std::vector<MapWordWrite> map_writes;
    std::optional<AuxiliaryWorldEntity> auxiliary_entity;
};

// ROM 0x1651E: handle the mode-14/mode-2 projected map effects and the mode-0
// auxiliary entity setup. The source's A0/A2 records become typed inputs and
// indexed map writes; no VDP/RAM address is part of the result.
WorldMapEffectResult apply_world_map_effect(
    const WorldMapEffectInput& input);

struct WeightedWorldSelection {
    std::uint16_t result = 0;
    std::uint16_t weight = 0;
};

struct SceneEntityRecord {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t kind = 0;
    std::uint8_t flags = 0;
    std::optional<std::uint16_t> source_entity_slot;
    std::uint16_t source_entity_state = 0;
    std::uint16_t behavior_handler = 0;
};

struct SceneEntitySpawnInput {
    std::int16_t record_gate = 0;
    std::uint16_t record_kind = 0;
    std::uint16_t world_mode = 0;
    std::uint16_t scene_id = 0;
    std::uint8_t scene_event_flags = 0;
    bool pending_scene_record = false;
    bool scene_record_slots_available = true;
    std::uint16_t source_entity_slot = 0;
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
    std::int8_t equipped_slot = -1;
    std::uint8_t equipped_item_code = 0;
    std::uint16_t random_value = 0;
    std::vector<WeightedWorldSelection> mode_two_selections;
    std::vector<WeightedWorldSelection> default_selections;
    std::vector<std::vector<WeightedWorldSelection>> scene_selections;
};

enum class SceneEntitySpawnAction {
    filtered,
    blocked_by_scene_flag,
    pending_scene_record,
    no_scene_record_slot,
    scene_record_spawned,
    weighted_selection_empty,
    weighted_selection_cleared,
};

struct SceneEntitySpawnResult {
    SceneEntitySpawnAction action = SceneEntitySpawnAction::filtered;
    std::optional<SceneEntityRecord> record;
};

// ROM 0x16D8A: reserve a scene record unless all ten slots are active, then
// publish the source position offset by one scene tile and the selected kind.
std::optional<SceneEntityRecord> spawn_scene_record_offset(
    const SceneEntityRecord& source,
    std::uint16_t selected_kind,
    bool all_slots_active);

// ROM 0x16B98: apply the scene-record gates, offset the source position,
// choose the mode/scene weighted table, and handle item-driven 0x5A/0x5B
// results. The weighted tables are extracted host data, not ROM pointers.
SceneEntitySpawnResult spawn_world_scene_entity(
    const SceneEntitySpawnInput& input);

} // namespace shadowrun::ghidra_conversion

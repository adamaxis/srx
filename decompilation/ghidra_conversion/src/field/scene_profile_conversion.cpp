#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool scene_uses_alternate_profile(const std::int16_t scene_id) {
    return (scene_id >= 6 && scene_id <= 9) ||
           (scene_id >= 0x2C && scene_id <= 0x31);
}

std::uint32_t field_scene_profile_table_entry(const std::uint16_t scene_id) {
    if (scene_id >= kFieldSceneProfileCount) {
        return 0U;
    }
    return kFieldSceneProfileTable[scene_id];
}

void run_field_loop(
    FieldSceneLoopState& state,
    const std::array<std::uint16_t, kFieldSceneProfileWordCount>& source) {
    state.profile = source;
    state.profile[46] = state.profile[12];
    state.profile[47] = state.profile[13];
    if (state.alternate_profile != 0U && state.override_lock == 0U) {
        state.profile[33] = state.override_x;
        state.profile[35] = state.override_x;
        state.profile[37] = state.override_x;
        state.profile[34] = state.override_y;
        state.profile[36] = state.override_y;
        state.profile[38] = state.override_y;
        state.profile[0] =
            state.override_x < 0x0A00U
                ? 0U
                : static_cast<std::uint16_t>(state.override_x - 0x0A00U);
        state.profile[1] =
            state.override_y < 0x0700U
                ? 0U
                : static_cast<std::uint16_t>(state.override_y - 0x0700U);
    }
    state.override_lock = 0U;
}

void initialize_field_scene_state(
    FieldSceneRuntimeState& state,
    const std::array<std::uint16_t, kFieldSceneRuntimeStateWordCount>& backup,
    FieldSceneRuntimeInput& input,
    const FieldSceneRuntimeStateCallbacks& callbacks) {
    if (callbacks.prepare_field_scene_runtime) {
        callbacks.prepare_field_scene_runtime();
    }
    state.scene_gate = 0U;
    state.fade_marker = -1;
    state.profile = backup;
    state.scene_byte = static_cast<std::uint8_t>(state.profile[0]);
    state.alternate_profile =
        scene_uses_alternate_profile(
            static_cast<std::int16_t>(state.profile[0]))
            ? 1U
            : 0U;
    state.ready_entity_repositioned = false;
    state.ready_entity_slot.reset();
    state.pending_profile_consumed = false;
    state.camera_x = input.prepared_camera_x;
    state.camera_y = input.prepared_camera_y;
    if (callbacks.advance_field_scene_transition) {
        callbacks.advance_field_scene_transition();
    }
    if (callbacks.upload_scene_profile_block) {
        callbacks.upload_scene_profile_block();
    }

    EntityAnchorSnapshotState snapshot{};
    snapshot.camera_x = state.camera_x;
    snapshot.camera_y = state.camera_y;
    snapshot.camera_transition_x = state.camera_transition_x;
    snapshot.camera_transition_y = state.camera_transition_y;
    snapshot.world_effect_player_spawn_pending =
        input.world_effect_player_spawn_pending;
    snapshot.pending_profile = input.pending_profile;

    const auto snapshot_result = snapshot_entity_anchors(
        snapshot,
        {{{static_cast<std::int16_t>(input.source_runner_positions[0].x),
           static_cast<std::int16_t>(input.source_runner_positions[0].y)},
          {static_cast<std::int16_t>(input.source_runner_positions[1].x),
           static_cast<std::int16_t>(input.source_runner_positions[1].y)},
          {static_cast<std::int16_t>(input.source_runner_positions[2].x),
           static_cast<std::int16_t>(input.source_runner_positions[2].y)}}},
        callbacks.find_ready_entity_slot,
        [&callbacks](const std::size_t slot, const EntityAnchor position) {
            if (callbacks.publish_ready_entity_position) {
                callbacks.publish_ready_entity_position(
                    slot,
                    SceneRunnerPosition{
                        static_cast<std::uint16_t>(position.x),
                        static_cast<std::uint16_t>(position.y)});
            }
        });

    state.camera_x = snapshot.camera_x;
    state.camera_y = snapshot.camera_y;
    state.camera_transition_x = snapshot.camera_transition_x;
    state.camera_transition_y = snapshot.camera_transition_y;
    for (std::size_t index = 0; index < state.runner_positions.size();
         ++index) {
        state.runner_positions[index] = {
            static_cast<std::uint16_t>(snapshot.anchors[index].x),
            static_cast<std::uint16_t>(snapshot.anchors[index].y)};
    }
    state.world_effect_player_spawn_pending =
        input.world_effect_player_spawn_pending;
    state.ready_entity_repositioned =
        snapshot_result.ready_entity_repositioned;
    state.ready_entity_slot = snapshot_result.ready_entity_slot;
    state.pending_profile_consumed = snapshot_result.pending_profile_consumed;
    input.pending_profile = snapshot.pending_profile;
}

std::size_t copy_terrain_layers(
    const std::uint32_t source_a,
    const std::uint32_t source_b,
    const FieldTerrainLayerCopy& copy_block) {
    if (!copy_block) {
        return 0U;
    }
    copy_block(source_a, 0x00FF2700U, 0x0E10U);
    copy_block(source_b, 0x00FF4700U, 0x0E10U);
    return 0x1C20U;
}

bool toggle_field_scene_variant(FieldSceneVariantState& state) {
    if (state.field_mode != 0x10U) {
        return false;
    }
    if (state.scene_id == 0U) {
        if (state.camera_x < 0x1400U) {
            state.scene_id = 3U;
            state.pending_profile = kFieldSceneVariantProfile0x3222;
        } else {
            state.scene_id = 5U;
            state.pending_profile = kFieldSceneVariantProfile0x3212;
        }
        return true;
    }
    if (state.scene_id == 5U) {
        state.scene_id = 0U;
        state.pending_profile = kFieldSceneVariantProfile0x31F2;
        return true;
    }
    state.scene_id = 0U;
    state.pending_profile = kFieldSceneVariantProfile0x3202;
    return true;
}

void apply_scene_profile_selection(
    SceneProfileState& state,
    std::optional<std::array<std::uint16_t, 8>>& pending_profile,
    const SceneProfileDefaults& defaults) {
    if (!pending_profile.has_value()) {
        state.camera_x = defaults.camera_x;
        state.camera_y = defaults.camera_y;
        state.runner_positions = defaults.runner_positions;
        return;
    }

    const auto& words = *pending_profile;
    state.camera_x = static_cast<std::uint16_t>(words[0] & 0xFF80U);
    state.camera_y = static_cast<std::uint16_t>(words[1] & 0xFF80U);
    for (std::size_t index = 0; index < state.runner_positions.size();
         ++index) {
        state.runner_positions[index] = {words[2U + index * 2U],
                                         words[3U + index * 2U]};
    }
    state.camera_transition_x = -1;
    state.camera_transition_y = -1;
    pending_profile.reset();
}

void copy_scene_resource_cursor(
    SceneResourceCursorState& state,
    const SceneResourceProfileFields& source) {
    state.cursor = source.cursor;
    state.resource_index = source.resource_index;
}

bool spawn_scene_record_offset(
    SceneRecordPlacement& destination,
    const SceneRecordPlacement& source,
    const std::int16_t kind,
    const std::function<bool()>& all_slots_active) {
    if (all_slots_active && all_slots_active()) {
        return false;
    }

    destination.x = static_cast<std::int16_t>(source.x + 0x80);
    destination.y = static_cast<std::int16_t>(source.y + 0x80);
    destination.flags = static_cast<std::uint8_t>(destination.flags | 1U);
    destination.kind = kind;
    return true;
}

} // namespace shadowrun::ghidra_conversion

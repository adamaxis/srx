#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include <variant>

#include "enums.hpp"
#include "shadowrun_ghidra/scripting/run_location_catalog.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"
#include "numeric_visual_renderer.hpp"
#include "field_map_composition.hpp"
#include "matrix_map_presenter.hpp"
#include "matrix_datastore_facts.hpp"

#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

using field_scene_detail::kInitialFieldSceneReference;
using field_scene_detail::kSceneProfileEntityPositionWord;
using field_scene_detail::kFieldModalReturnTransitionFrames;
using field_scene_detail::magic_action_record_from_runner;

namespace {
constexpr std::array<std::array<int, 2>, 12> kHospitalRandomGridOrigins{{
    {{40, 200}}, {{72, 200}}, {{184, 200}}, {{216, 200}},
    {{40, 208}}, {{72, 208}}, {{184, 208}}, {{216, 208}},
    {{56, 200}}, {{200, 200}}, {{56, 208}}, {{200, 208}},
}};

using ScriptValue = scripting::ScriptPersistentValue;

ScriptValue script_integer_value(const std::int64_t value) {
    ScriptValue result;
    result.value = value;
    return result;
}

ScriptValue script_number_value(const double value) {
    ScriptValue result;
    result.value = value;
    return result;
}

ScriptValue script_boolean_value(const bool value) {
    ScriptValue result;
    result.value = value;
    return result;
}

ScriptValue script_text_value(const std::string_view value) {
    ScriptValue result;
    result.value = std::string(value);
    return result;
}

ScriptValue script_array_value(ScriptValue::Array value) {
    ScriptValue result;
    result.value = std::move(value);
    return result;
}

ScriptValue script_object_value(ScriptValue::Object value) {
    ScriptValue result;
    result.value = std::move(value);
    return result;
}

const ScriptValue* script_member(const ScriptValue& value,
                                const std::string_view name) {
    const auto* object = std::get_if<ScriptValue::Object>(&value.value);
    if (object == nullptr) return nullptr;
    const auto found = object->find(std::string(name));
    return found == object->end() ? nullptr : &found->second;
}

std::optional<std::int64_t> script_integer(const ScriptValue* value) {
    if (value == nullptr) return std::nullopt;
    const auto* integer = std::get_if<std::int64_t>(&value->value);
    return integer == nullptr ? std::nullopt
                              : std::optional<std::int64_t>(*integer);
}

std::optional<double> script_number(const ScriptValue* value) {
    if (value == nullptr) return std::nullopt;
    if (const auto* number = std::get_if<double>(&value->value)) {
        return *number;
    }
    if (const auto* integer = std::get_if<std::int64_t>(&value->value)) {
        return static_cast<double>(*integer);
    }
    return std::nullopt;
}

std::optional<bool> script_boolean(const ScriptValue* value) {
    if (value == nullptr) return std::nullopt;
    const auto* boolean = std::get_if<bool>(&value->value);
    return boolean == nullptr ? std::nullopt
                              : std::optional<bool>(*boolean);
}

std::optional<std::string_view> script_text(const ScriptValue* value) {
    if (value == nullptr) return std::nullopt;
    const auto* string = std::get_if<std::string>(&value->value);
    return string == nullptr ? std::nullopt
                             : std::optional<std::string_view>(*string);
}

const ScriptValue::Array* script_array(const ScriptValue* value) {
    return value == nullptr
        ? nullptr
        : std::get_if<ScriptValue::Array>(&value->value);
}

struct DoorReturnSide {
    std::string_view id;
    std::uint8_t source_index;
};

DoorReturnSide door_return_side_from_collision_samples(
    const std::array<std::uint8_t, 3>& samples) {
    // The collision adapter translates ROM 0xB7B8/0xBC52 into a named side
    // here. Gameplay Lua receives only that side, never collision bytes.
    auto selected = samples.back();
    for (const auto sample : samples) {
        if ((sample & 0x80U) != 0U) {
            selected = sample;
            break;
        }
    }
    constexpr std::array<std::string_view, 4> sides{
        "north", "east", "south", "west"};
    const auto index = static_cast<std::uint8_t>((selected >> 5U) & 0x03U);
    return DoorReturnSide{sides[index], index};
}

} // namespace

bool FieldSceneRuntime::prepare_runtime_field_scene(
    const std::uint16_t scene_reference,
    const std::optional<SceneRunnerPosition> requested_spawn) {
    if (!field_session_source_assets_ || !field_asset_catalog_ ||
        scene_reference >= kFieldSceneProfileCount) {
        return false;
    }

    auto input = integration::make_field_scene_input(
        *field_session_source_assets_, scene_reference);
    if (!input.has_value()) {
        return false;
    }
    const bool apply_mode_transition_spawn =
        field_pending_mode_transition_scene_ ==
            std::optional<std::uint16_t>{scene_reference} &&
        field_pending_mode_transition_spawn_.has_value();
    if (requested_spawn.has_value()) {
        input->pending_position_available = true;
        input->pending_position = *requested_spawn;
    } else if (apply_mode_transition_spawn) {
        input->pending_position_available = true;
        input->pending_position = *field_pending_mode_transition_spawn_;
    }
    const auto alias =
        std::string("field.maps.mode.") + std::to_string(scene_reference);
    const auto priority_overlay_alias = alias + ".priority.overlay";
    QImage next_field_map;
    QImage next_field_map_priority_overlay;
    if (!load_image(*field_asset_catalog_, alias, next_field_map) ||
        !load_image(*field_asset_catalog_, priority_overlay_alias,
                    next_field_map_priority_overlay) ||
        next_field_map_priority_overlay.size() != next_field_map.size()) {
        if (failure_.empty()) {
            failure_ = "field map priority overlay dimensions do not match map";
        }
        return false;
    }

    // The world-activity routine prepares its destination scene before the
    // outer scene-entry pass completes. That pass sets the pending marker and
    // may ask for one more buffer refresh of the already prepared destination.
    // Keep this activity's just-published actor records during that refresh.
    const bool same_scene = state_.field_scene_buffers.initialized &&
                            state_.field_scene_buffers.scene_reference ==
                                scene_reference;
    const bool force_scene_entry = requested_spawn.has_value();
    const bool preserve_scene_entry_position =
        same_scene && !field_world_activity_scene_entry_pending_ &&
        !force_scene_entry;
    const auto prior_world_mode = field_movement_transition_state_.world_mode;
    const auto prior_camera_x = state_.field_scene_buffers.camera_x;
    const auto prior_camera_y = state_.field_scene_buffers.camera_y;
    const auto prior_runner_position = field_entity_.world_position;
    auto next_buffers = state_.field_scene_buffers;
    next_buffers.initialized = false;
    if (!prepare_field_scene_buffers(next_buffers, *input, {})) {
        return false;
    }

    if (preserve_scene_entry_position) {
        // A 0x2BD2 same-scene refresh redraws the active scene without
        // resetting the controlled runner to its original entry position.
        // The immediately repeated load after a world-activity selector is
        // different: it is still the destination entry pass, and must retain
        // the new scene's profile placements rather than the caller's map
        // coordinates. The world-activity entry marker distinguishes those
        // two cases while allowing already-published actor records to remain
        // in place.
        next_buffers.camera_x = prior_camera_x;
        next_buffers.camera_y = prior_camera_y;
        if (field_entity_.active) {
            next_buffers.profile[kSceneProfileEntityPositionWord] =
                prior_runner_position.x;
            next_buffers.profile[kSceneProfileEntityPositionWord + 1U] =
                prior_runner_position.y;
            next_buffers.source_metadata.initial_field_records[0] =
                prior_runner_position;
        }
    }

    const auto interaction_mode =
        select_scene_interaction_mode(scene_reference);
    if (!interaction_mode.has_value()) {
        return false;
    }

    if (state_.field_scene_buffers.initialized &&
        (!same_scene || force_scene_entry)) {
        // ROM 0x167AE clears the field alarm/security latches on scene entry.
        // Route that reset through Lua's shared 0x1E09A cleanup too, stopping
        // the alarm cue and marking published alarm responders to despawn.
        advance_field_alarm(false, false, false, false, true);
    }

    state_.field_scene_buffers = std::move(next_buffers);
    field_map_ = std::move(next_field_map);
    field_map_priority_overlay_ =
        std::move(next_field_map_priority_overlay);
    reset_field_map_cell_words();
    // ROM 0x167AE selects FFDF40 before 0xA800's conditional actor decision.
    // Install the destination mode before the native actor population pass so
    // its Lua selector sees the same source state on resource-scene entry.
    field_movement_transition_state_.world_mode =
        same_scene && !force_scene_entry ? prior_world_mode : *interaction_mode;
    if (!same_scene || field_world_activity_scene_entry_pending_ ||
        force_scene_entry) {
        initialize_default_field_actors(scene_reference);
    }
    if (!same_scene || force_scene_entry) {
        field_weapon_projectiles_.clear();
        field_hit_sparks_.clear();
        // ROM scene load 0x16AB8 calls 0x4B6C before installing the new map [C].
        clear_field_barrier_collision_overlay();
        field_weapon_slots_occupied_.fill(false);
        // 0x167AE may arm FFDF84/FFDF86 while selecting a new field scene.
        // Its later 0x2BD2 re-entry prepares that scene again, but the timeout
        // words survive in the ROM. The pending-scene marker distinguishes
        // that final presentation pass from a fresh unrelated scene load.
        if (!field_world_activity_scene_entry_pending_) {
            field_world_activity_timeout_state_ =
                WorldActivityTimeoutState{};
        }
    }
    if ((!same_scene || force_scene_entry) &&
        field_palette_fade_steps_ != 0U) {
        // The source scene-buffer load replaces the palette after the
        // field-transition fade. The Qt fade is a presentation overlay, so
        // release it only after the new scene assets are committed; otherwise
        // the replacement scene remains hidden behind the old scene's black
        // transition layer.
        field_palette_fade_steps_ = 0U;
        source_boundary("field.scene.transition.clear_old_fade_overlay");
    }
    state_.field_resource_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.scene_id = scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_movement_transition_state_.scene_id = scene_reference;
    field_world_activity_timeout_state_.scene_id = scene_reference;
    // A same-scene 0x2BD2 redraw preserves FFDF40; the value was chosen above
    // before actor population to match the 0x167AE -> 0xA800 source order.
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    world_frame_logic_state_.world_mode = scene_reference;
    field_terrain_sample_state_ = {};
    field_collision_marker_index_ = 0U;
    field_camera_alignment_ready_ = false;
    field_runtime_state_.display_dirty_flags = static_cast<std::uint8_t>(
        field_runtime_state_.display_dirty_flags | 0x0EU);
    state_.display_dirty_flags = static_cast<std::uint8_t>(
        state_.display_dirty_flags | 0x0EU);
    if (!same_scene || force_scene_entry) {
        // A scene change goes through 0x2822/0x2830 (FFE172 = FFFF); a
        // same-scene 0x2BD2 redraw does not restart the song [C]. The song
        // starts on the next field tick (0x2900).
        scene_music_latch_ = true;
    }
    if (apply_mode_transition_spawn) {
        field_pending_mode_transition_spawn_.reset();
    }
    source_boundary("field.world_activity.scene_prepared");
    return true;
}

void FieldSceneRuntime::snapshot_world_activity_source_scene() {
    WorldActivityReturnSnapshot snapshot{};
    snapshot.scene_buffers = state_.field_scene_buffers;
    snapshot.scene_entry = state_.field_scene_entry;
    snapshot.controlled_actor = field_entity_;
    snapshot.movement = field_movement_state_;
    snapshot.movement_transition = field_movement_transition_state_;
    snapshot.camera_follow = field_camera_follow_;
    snapshot.default_actors = field_default_actors_;
    snapshot.render_entities = state_.render_entities;
    snapshot.activity_state = field_world_activity_state_;
    snapshot.activity_timeout = field_world_activity_timeout_state_;
    snapshot.world_effect_slots_occupied =
        field_world_effect_slots_occupied_;
    snapshot.spell_effect_slot_target_ids =
        field_spell_effect_slot_target_ids_;
    snapshot.entity_target_owner = field_entity_target_owner_;
    snapshot.entity_target_records = field_entity_target_records_;
    snapshot.player_animation = field_entity_animation_;
    snapshot.player_animation_id = field_entity_animation_id_;
    snapshot.player_animation_clock_reset_pending =
        field_player_animation_clock_reset_pending_;
    snapshot.player_walking = field_entity_walking_;
    snapshot.player_attack_pose = field_entity_attack_pose_active_;
    snapshot.player_attack_pose_kind = field_entity_attack_pose_kind_;
    snapshot.player_attack_direction = field_entity_attack_direction_;
    snapshot.target_heading = field_target_heading_;
    snapshot.return_anchor = pending_field_return_anchor_;
    snapshot.return_facing = pending_field_return_facing_;

    if (snapshot.return_anchor ==
        FieldLocationReturnAnchor::collision_probe_lead) {
        if (!field_collision_probe_state_.initialized) {
            source_boundary("field.world_activity.return_probe_missing");
            snapshot.return_anchor = FieldLocationReturnAnchor::none;
        } else {
            // ROM 0xBC36 writes the selected offset-bank lead point to
            // FFDF44; 0x2F32/0x2FA4 copy that anchor into the runner records
            // when the source field map is rebuilt.
            const auto& anchor = field_collision_probe_state_.points[0U];
            // 0x2F32 copies all three FFDF44.. points into the runner
            // records, so runners 2 and 3 return beside the lead.
            auto& return_records =
                snapshot.scene_buffers.source_metadata.initial_field_records;
            for (std::size_t index = 0U;
                 index < field_collision_probe_state_.points.size() &&
                 index < return_records.size();
                 ++index) {
                return_records[index] = SceneRunnerPosition{
                    field_collision_probe_state_.points[index].x,
                    field_collision_probe_state_.points[index].y};
            }
            snapshot.controlled_actor.world_position =
                SceneRunnerPosition{anchor.x, anchor.y};
            snapshot.controlled_actor.projection.world_x = anchor.x;
            snapshot.controlled_actor.projection.world_y = anchor.y;
            snapshot.movement.position_x = anchor.x;
            snapshot.movement.position_y = anchor.y;
            snapshot.scene_buffers.camera_x =
                field_collision_probe_state_.camera_x;
            snapshot.scene_buffers.camera_y =
                field_collision_probe_state_.camera_y;
            snapshot.camera_follow.camera_x =
                field_collision_probe_state_.camera_x;
            snapshot.camera_follow.camera_y =
                field_collision_probe_state_.camera_y;
            snapshot.camera_follow.aligned_x =
                static_cast<std::uint16_t>(
                    field_collision_probe_state_.camera_x & 0xFF80U);
            snapshot.camera_follow.aligned_y =
                static_cast<std::uint16_t>(
                    field_collision_probe_state_.camera_y & 0xFF80U);

            if (snapshot.return_facing ==
                FieldLocationReturnFacing::face_away_from_entry) {
                const auto return_heading =
                    field_door_exit_heading(snapshot.movement.heading);
                snapshot.movement.heading = return_heading;
                snapshot.movement.animation_heading = return_heading;
                snapshot.target_heading = return_heading;
                snapshot.controlled_actor.direction =
                    static_cast<std::uint8_t>(
                        ((return_heading >> 14U) & 3U) * 2U);
                snapshot.player_walking = false;
                snapshot.player_attack_pose = false;
                snapshot.player_animation_clock_reset_pending = true;
            }
        }
    }
    world_activity_return_snapshot_ = std::move(snapshot);
    pending_field_return_anchor_ = FieldLocationReturnAnchor::none;
    pending_field_return_facing_ =
        FieldLocationReturnFacing::preserve_entry_heading;
    pending_field_return_music_policy_ =
        FieldLocationMusicReturnPolicy::none;

    // ROM 0x2934's frame path when FFDF8A != 0 calls 0x4B6C [C].
    // ROM 0x16E30 retires pending entity effects and clears effect-slot
    // occupancy before the resource scene is prepared. Gun projectiles use
    // those typed host slots in this field presenter.
    field_weapon_projectiles_.clear();
    field_hit_sparks_.clear();
    clear_field_barrier_collision_overlay();
    field_weapon_slots_occupied_.fill(false);
    source_boundary("field.world_activity.source_scene_snapshotted");
}

bool FieldSceneRuntime::restore_world_activity_source_scene(
    integration::FieldRuntimeCoordinatorState& runtime_state) {
    if (!world_activity_return_snapshot_.has_value()) {
        source_boundary("field.world_activity.return_snapshot_missing");
        return false;
    }

    const auto& snapshot = *world_activity_return_snapshot_;
    const auto source_scene = snapshot.scene_buffers.scene_reference;
    if (!snapshot.scene_buffers.initialized ||
        source_scene >= kFieldSceneProfileCount) {
        source_boundary("field.world_activity.return_snapshot_invalid");
        return false;
    }

    // Restore the source scene before the common scene-entry path runs. That
    // lets prepare_runtime_field_scene recognize this as a same-scene rebuild
    // and retain its camera and runner position while replacing the complete
    // resource-scene image.
    state_.field_scene_buffers = snapshot.scene_buffers;
    state_.field_scene_entry = snapshot.scene_entry;
    state_.field_resource_mode = snapshot.scene_buffers.field_resource_mode;
    state_.render_entities = snapshot.render_entities;
    field_entity_ = snapshot.controlled_actor;
    field_movement_state_ = snapshot.movement;
    field_movement_transition_state_ = snapshot.movement_transition;
    field_camera_follow_ = snapshot.camera_follow;
    field_default_actors_ = snapshot.default_actors;
    field_world_activity_state_ = snapshot.activity_state;
    field_world_activity_timeout_state_ = snapshot.activity_timeout;
    field_world_effect_slots_occupied_ =
        snapshot.world_effect_slots_occupied;
    field_spell_effect_slot_target_ids_ =
        snapshot.spell_effect_slot_target_ids;
    field_entity_target_owner_ = snapshot.entity_target_owner;
    field_entity_target_records_ = snapshot.entity_target_records;
    field_entity_animation_ = snapshot.player_animation;
    field_entity_animation_id_ = snapshot.player_animation_id;
    field_player_animation_clock_reset_pending_ =
        snapshot.player_animation_clock_reset_pending;
    field_entity_walking_ = snapshot.player_walking;
    field_entity_attack_pose_active_ = snapshot.player_attack_pose;
    field_entity_attack_pose_kind_ = snapshot.player_attack_pose_kind;
    field_entity_attack_direction_ = snapshot.player_attack_direction;
    field_target_heading_ = snapshot.target_heading;

    field_world_activity_scene_entry_pending_ = false;
    field_security_floor_index_.reset();
    field_world_activity_timeout_state_.scene_id = source_scene;
    field_movement_transition_state_.scene_id = source_scene;
    field_movement_transition_state_.world_interaction_lock = false;
    field_movement_transition_state_.interaction_transition_pending = false;
    field_movement_transition_state_.interaction_transition_timer = 0;
    field_palette_fade_steps_ = 0U;
    field_runtime_state_.scene_id = source_scene;
    field_runtime_state_.field_mode =
        snapshot.scene_buffers.field_resource_mode;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    field_runtime_state_.pending_field_scene_handler = false;
    field_runtime_state_.world_activity_countdown = 0U;
    field_runtime_state_.scene_refresh_marker = -1;
    runtime_state.scene_id = source_scene;
    runtime_state.field_mode = snapshot.scene_buffers.field_resource_mode;
    runtime_state.field_scene_gate = 0U;
    runtime_state.field_transition_requested = false;
    runtime_state.pending_field_scene_handler = false;
    runtime_state.world_activity_countdown = 0U;
    world_frame_logic_state_.world_mode = source_scene;

    advance_field_scene_transition_after_modal();
    const bool scene_load_deferred_for_building_fade =
        field_building_scene_transition_active_ &&
        active_field_script_route_.has_value() &&
        active_field_script_route_->scene_transition_policy ==
            FieldSceneTransitionPolicy::building_fade;
    if (!state_.field_scene_buffers.initialized ||
        (state_.field_scene_buffers.scene_reference != source_scene &&
         !scene_load_deferred_for_building_fade)) {
        source_boundary("field.world_activity.return_scene_prepare_failed");
        return false;
    }

    field_runtime_state_.field_transition_requested = false;
    runtime_state.field_transition_requested = false;
    source_boundary("field.world_activity.source_scene_restored");
    world_activity_return_snapshot_.reset();
    return true;
}

void FieldSceneRuntime::advance_field_scene_transition_after_modal() {
    // The modal Qt loop is non-blocking. Keep the 0x2BD2 transition pending
    // while the page owns the screen; once it closes, run the ROM-selected
    // same-scene refresh or nonzero scene/profile re-entry in source order.

    if (!field_building_transition_handoff_ &&
        active_field_script_route_.has_value() &&
        active_field_script_route_->scene_transition_policy ==
            FieldSceneTransitionPolicy::building_fade) {
        if (active_field_script_route_->scene_transition_vblanks !=
            kRomBuildingTransitionVBlanks) {
            source_boundary("field.building_transition.invalid_lua_timing");
            return;
        }
        if (field_building_scene_transition_active_) {
            return;
        }
        if (field_scene_transition_.is_active()) {
            source_boundary("field.building_transition.transition_busy");
            return;
        }
        if (field_building_transition_source_image_.isNull()) {
            capture_field_building_transition_source_image();
        }
        if (!field_scene_transition_.begin_fade_out()) {
            source_boundary("field.building_transition.fade_out_unresolved");
            return;
        }
        field_building_scene_transition_active_ = true;
        suppress_field_modal_close_input();
        source_boundary("field.building_transition.fade_out_started");
        return;
    }

    FieldSceneTransitionState transition{};
    transition.field_transition_marker = static_cast<std::uint16_t>(
        field_runtime_state_.scene_refresh_marker);
    transition.scene_refresh_marker =
        state_.field_scene_entry.scene_refresh_marker;
    transition.pending_scene_marker =
        state_.field_scene_entry.pending_scene_marker;
    transition.camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    transition.camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    transition.camera_transition_x = static_cast<std::uint16_t>(
        state_.field_scene_entry.camera_transition_x);
    transition.camera_transition_y = static_cast<std::uint16_t>(
        state_.field_scene_entry.camera_transition_y);
    const bool one_shot_scene_profile_pending =
        field_pending_scene_profile_.has_value();

    const auto& collision_map =
        state_.field_scene_buffers.source_metadata.terrain_collision_map;
    std::vector<std::uint8_t> event_counters(collision_map.begin(),
                                             collision_map.end());
    FieldSceneEntryCallbacks scene_callbacks{};
    scene_callbacks.step_palette_transition =
        callbacks_.advance_title_palette_fade;
    scene_callbacks.run_world_frame_services =
        callbacks_.run_world_frame_services;
    scene_callbacks.guard_world_effect_player_spawn =
        callbacks_.guard_world_effect_player_spawn;
    scene_callbacks.prepare_field_viewports = [this] {
        prepare_field_viewports(
            FieldViewportPreparationState{state_.field_resource_mode},
            callbacks_.field_viewports);
    };
    scene_callbacks.prepare_field_scene_buffers = [this] {
        const auto destination_scene =
            field_pending_mode_transition_scene_.value_or(
                state_.field_scene_buffers.scene_reference);
        if (!prepare_runtime_field_scene(
                destination_scene)) {
            source_boundary("field.scene.transition.scene_prepare_unresolved");
        } else if (field_pending_mode_transition_scene_ ==
                   std::optional<std::uint16_t>{destination_scene}) {
            field_pending_mode_transition_scene_.reset();
        }
    };
    scene_callbacks.apply_scene_profile_selection =
        callbacks_.apply_scene_profile_selection;
    scene_callbacks.record_pending_event = [this](
                                               const std::uint16_t index) {
        source_boundary("field.scene.transition.pending_event_recorded");
        (void)index;
    };
    if (transition.field_transition_marker == 0U) {
        transition.pending_event_index =
            field_movement_transition_state_.activity_source_word;
        transition.event_state_negative =
            static_cast<std::int8_t>(
                field_movement_transition_state_.activity_selection) < 0;
    }
    scene_callbacks.upload_scene_profile_block =
        callbacks_.upload_scene_profile_block;

    ::shadowrun::ghidra_conversion::advance_field_scene_transition(
        transition, event_counters, scene_callbacks);
    if (one_shot_scene_profile_pending &&
        !field_pending_scene_profile_.has_value()) {
        // ROM 0x2B78 replaces the camera and clears its aligned-edge cache.
        // Keep the conversion's local transition record in sync with the
        // one-shot callback before its final state publication below.
        transition.camera_x = static_cast<std::uint16_t>(
            state_.field_scene_buffers.camera_x);
        transition.camera_y = static_cast<std::uint16_t>(
            state_.field_scene_buffers.camera_y);
        transition.camera_transition_x = static_cast<std::uint16_t>(
            state_.field_scene_entry.camera_transition_x);
        transition.camera_transition_y = static_cast<std::uint16_t>(
            state_.field_scene_entry.camera_transition_y);
    }
    field_world_activity_scene_entry_pending_ = false;

    auto& updated_collision_map =
        state_.field_scene_buffers.source_metadata.terrain_collision_map;
    std::copy(event_counters.begin(), event_counters.end(),
              updated_collision_map.begin());
    if (transition.field_transition_marker == 0U &&
        transition.pending_event_index == 0U) {
        field_movement_transition_state_.activity_source_word = 0U;
    }

    state_.field_scene_entry.field_transition_marker =
        transition.field_transition_marker;
    state_.field_scene_entry.scene_refresh_marker =
        transition.scene_refresh_marker;
    state_.field_scene_entry.pending_scene_marker =
        transition.pending_scene_marker;
    state_.field_scene_entry.camera_x = transition.camera_x;
    state_.field_scene_entry.camera_y = transition.camera_y;
    state_.field_scene_entry.camera_transition_x = static_cast<std::int16_t>(
        transition.camera_transition_x);
    state_.field_scene_entry.camera_transition_y = static_cast<std::int16_t>(
        transition.camera_transition_y);
    state_.field_scene_buffers.camera_x = transition.camera_x;
    state_.field_scene_buffers.camera_y = transition.camera_y;
    field_runtime_state_.scene_refresh_marker = static_cast<std::int16_t>(
        transition.field_transition_marker);
    field_runtime_state_.display_dirty_flags = static_cast<std::uint8_t>(
        field_runtime_state_.display_dirty_flags | 0x0EU);
    state_.display_dirty_flags = static_cast<std::uint8_t>(
        state_.display_dirty_flags | 0x0EU);
    // ROM 0x08C4 clears FFDFE8 after the scene buffers and profile are
    // rebuilt. The previous projected interaction owns this display gate
    // through the fade, but the replacement field must release it before a
    // later interaction handler can dispatch.
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    source_boundary("field.scene.transition.display_service_released");
    field_camera_alignment_ready_ = false;
    update_source_field_camera();
    update_entity_render_pass(state_.render_entities,
                              callbacks_.entity_render_pass);
    dispatch_field_map_scene_entry(
        state_.field_scene_buffers.scene_reference);
    source_boundary("field.scene.transition.same_scene_reentered");
}

void FieldSceneRuntime::capture_field_building_transition_source_image() {
    QImage image(320, 224, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    QPainter painter(&image);
    paint(painter);
    painter.end();
    field_building_transition_source_image_ = std::move(image);
}

void FieldSceneRuntime::advance_field_building_scene_transition() {
    if (!field_building_scene_transition_active_) {
        return;
    }
    const auto update = field_scene_transition_.advance();
    if (update.palette_step_changed) {
        source_boundary("field.building_transition.palette_step");
    }
    if (update.destination_load_requested) {
        // The destination is committed only after the full ROM-sized fade to
        // black. Keep this handoff in the same scene re-entry code used by the
        // ordinary field route, then reveal the new scene from black.
        field_building_transition_source_image_ = QImage{};
        field_building_transition_handoff_ = true;
        advance_field_scene_transition_after_modal();
        field_building_transition_handoff_ = false;
        if (!field_scene_transition_.begin_fade_in()) {
            field_scene_transition_.cancel();
            field_building_scene_transition_active_ = false;
            source_boundary("field.building_transition.fade_in_unresolved");
            return;
        }
        source_boundary("field.building_transition.fade_in_started");
        return;
    }
    if (update.completed) {
        field_building_scene_transition_active_ = false;
        field_building_transition_source_image_ = QImage{};
        active_field_script_route_.reset();
        source_boundary("field.building_transition.completed");
    }
}

bool FieldSceneRuntime::resume_field_mode_entry_route() {
    if (!field_mode_entry_transition_pending_ ||
        !field_mode_entry_destination_scene_.has_value()) {
        return false;
    }
    field_mode_entry_transition_pending_ = false;
    const auto expected_scene = *field_mode_entry_destination_scene_;
    field_mode_entry_destination_scene_.reset();

    if (!active_field_script_route_.has_value() ||
        active_field_script_route_->kind != FieldScriptRouteKind::area_event ||
        active_field_script_route_->area_event_target !=
            std::optional<std::uint8_t>{
                static_cast<std::uint8_t>(expected_scene)}) {
        field_mode_entry_run_target_actor_group_.reset();
        source_boundary("field.entry.area_event_route_lost");
        return false;
    }

    const bool run_target_actor_group_pending =
        field_mode_entry_run_target_actor_group_.has_value();
    const auto expected_activity_scene = run_target_actor_group_pending
        ? static_cast<std::uint16_t>(15U)
        : expected_scene;
    if (run_target_actor_group_pending) {
        const auto expected_group =
            scripting::run_target_actor_group_id_for_source_selector(
                state_.field_scene_buffers.scene_reference,
                active_field_script_route_->event_code);
        if (!expected_group.has_value() ||
            *expected_group !=
                field_mode_entry_run_target_actor_group_->actor_group_id) {
            field_mode_entry_run_target_actor_group_.reset();
            source_boundary(
                "field.entry.run_target_actor_group_route_mismatch");
            return false;
        }
    }

    const auto initialize =
        field_runtime_callbacks_.initialize_world_activity_mode;
    if (!initialize) {
        field_mode_entry_run_target_actor_group_.reset();
        source_boundary("field.entry.area_event_route_resume_unavailable");
        return false;
    }

    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.scene_runtime_state = 0U;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    initialize(field_runtime_state_);
    field_mode_entry_run_target_actor_group_.reset();

    const bool scene_load_deferred_for_building_fade =
        field_building_scene_transition_active_ &&
        active_field_script_route_.has_value() &&
        active_field_script_route_->scene_transition_policy ==
            FieldSceneTransitionPolicy::building_fade;
    if (field_runtime_state_.field_transition_requested ||
        field_runtime_state_.scene_id != expected_activity_scene ||
        (state_.field_scene_buffers.scene_reference !=
             expected_activity_scene &&
         !scene_load_deferred_for_building_fade)) {
        source_boundary("field.entry.area_event_scene_prepare_failed");
        return false;
    }

    field_runtime_state_.scene_refresh_marker = -1;
    field_movement_transition_state_.scene_id = static_cast<std::uint8_t>(
        field_runtime_state_.scene_id);
    world_frame_logic_state_.world_mode =
        static_cast<std::uint8_t>(field_runtime_state_.scene_id);
    advance_field_scene_transition_after_modal();
    if (state_.field_scene_buffers.scene_reference !=
        expected_activity_scene) {
        source_boundary("field.entry.area_event_scene_reentry_failed");
        return false;
    }
    source_boundary("field.entry.area_event_scene_entered");
    return true;
}

void FieldSceneRuntime::begin_field_modal_return_transition(
    const std::uint8_t duration_frames) {
    if (field_building_scene_transition_active_) {
        field_modal_return_transition_frames_remaining_ = 0U;
        field_palette_fade_steps_ = 0U;
        suppress_field_modal_close_input();
        source_boundary("field.modal_return.building_transition_owns_fade");
        return;
    }
    // ROM 0x2BD2 begins with a 17-pass 0x131C fade-out before its field
    // refresh; each palette pass waits two VBlanks through 0x1342 [C]. This
    // additional native 60-frame fade-in and input guard are host presentation
    // choices, not timing copied from that path [H].
    field_modal_return_transition_duration_frames_ =
        duration_frames == 0U ? kFieldModalReturnTransitionFrames
                              : duration_frames;
    field_modal_return_transition_frames_remaining_ =
        field_modal_return_transition_duration_frames_;
    field_palette_fade_steps_ = 9U;
    suppress_field_modal_close_input();
    field_entity_animation_.redraw_requested = true;
    source_boundary("field.modal_return.transition_started");
}

void FieldSceneRuntime::suppress_field_modal_close_input() {
    field_modal_return_blocked_inputs_ = static_cast<std::uint8_t>(
        (field_controller_sample_state_.held & 0x00FFU) | 0x0040U);
    field_combat_input_suppressed_until_release_ =
        (field_modal_return_blocked_inputs_ & 0x40U) != 0U;
    // The A edge that dismissed the modal belongs to that modal. Keep the
    // held snapshot so a still-held key cannot become a fresh edge when the
    // field loop resumes, but discard its queued high-byte press edge.
    field_controller_sample_state_.events =
        static_cast<std::uint16_t>(field_controller_sample_state_.held &
                                   0x00FFU);
    field_controller_latch_state_ = ControllerLatchState{};
}

void FieldSceneRuntime::dispatch_field_map_scene_entry(
    const std::uint16_t scene_id) {
    // A saved door return belongs to the scene it was recorded in. A linked
    // same-scene refresh can happen before the modal closes, so retain the
    // point for any route carrying the shared return policy.
    const bool pending_same_scene_exit_return =
        field_door_return_point_.valid &&
        field_door_return_point_.scene_id == scene_id &&
        active_field_script_route_.has_value() &&
        active_field_script_route_->return_anchor !=
            FieldLocationReturnAnchor::none;
    if (!field_door_return_active_ &&
        !pending_same_scene_exit_return) {
        field_door_return_point_.valid = false;
    }
    field_map_event_system_.begin_scene_visit(scene_id);
    handle_field_map_event_activations(
        field_map_event_system_.dispatch_scene_entry(
            FieldMapEventTriggerContext{
                scene_id,
                field_entity_.world_position.x,
                field_entity_.world_position.y,
                field_map_event_state_flags_,
                field_entity_.active}));
}

void FieldSceneRuntime::handle_field_map_event_activations(
    const std::vector<FieldMapEventActivation>& activations) {
    for (const auto& activation : activations) {
        switch (activation.action) {
        case FieldMapEventScriptAction::run_lua_handler:
            handle_game_script_task_step(
                game_scripts_.start_task("field_events", activation.handler));
            break;
        case FieldMapEventScriptAction::dispatch_native_script:
            source_boundary("field.map_event.native_script_handler_unresolved");
            break;
        }
    }
}

void FieldSceneRuntime::handle_game_script_task_step(
    const scripting::ScriptTaskStep& step) {
    using ScriptTaskStatus = scripting::ScriptTaskStep::Status;
    if (step.status == ScriptTaskStatus::failed) {
        failure_ = game_scripts_.failure();
        source_boundary("field.map_event.lua_handler_failed");
        return;
    }
    if (step.status == ScriptTaskStatus::completed) {
        source_boundary("field.map_event.lua_handler_completed");
        return;
    }
    if (!step.operation.has_value()) {
        failure_ = "Lua gameplay task yielded without a typed operation";
        source_boundary("field.map_event.lua_handler_failed");
        return;
    }

    const auto& operation = *step.operation;
    if (operation.type !=
            scripting::ScriptNativeOperation::Type::actor_walk_direction ||
        operation.actor_id != "player" ||
        field_opening_auto_walk_.active) {
        failure_ = "Lua requested an unsupported native actor operation";
        source_boundary("field.map_event.lua_operation_unresolved");
        return;
    }

    field_opening_auto_walk_.active = true;
    field_opening_auto_walk_.target_direction = operation.direction;
    source_boundary("field.map_event.new_game_opening_walk_started");
}

void FieldSceneRuntime::record_field_door_contact(
    const std::array<std::uint8_t, 3>& collision_samples) {
    const auto side = door_return_side_from_collision_samples(collision_samples);
    const auto result = game_scripts_.call_value(
        "world_links", "door_contact",
        {script_object_value({
            {"outdoor_field", script_boolean_value(
                                  field_movement_transition_state_.world_mode ==
                                  0U)},
            {"return_side", script_text_value(side.id)},
            {"lead", script_object_value({
                {"x", script_integer_value(field_entity_.world_position.x)},
                {"y", script_integer_value(field_entity_.world_position.y)}})},
            {"camera", script_object_value({
                {"x", script_integer_value(static_cast<std::uint16_t>(
                             state_.field_scene_buffers.camera_x))},
                {"y", script_integer_value(static_cast<std::uint16_t>(
                             state_.field_scene_buffers.camera_y))}})},
        })});
    const auto saved = result.has_value()
        ? script_boolean(script_member(*result, "saved"))
        : std::nullopt;
    if (!result.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.door.contact_script_failed");
        return;
    }
    if (!saved.has_value()) {
        failure_ = "world_links.door_contact returned no saved flag";
        source_boundary("field.door.contact_script_failed");
        return;
    }
    field_collision_probe_state_.selected_offset_bank =
        side.source_index;
    if (!*saved) {
        return;
    }
    const auto* party = script_array(script_member(*result, "party"));
    const auto* camera = script_member(*result, "camera");
    const auto camera_x = camera == nullptr
        ? std::nullopt : script_integer(script_member(*camera, "x"));
    const auto camera_y = camera == nullptr
        ? std::nullopt : script_integer(script_member(*camera, "y"));
    if (party == nullptr || party->size() !=
            field_collision_probe_state_.points.size() ||
        !camera_x.has_value() || !camera_y.has_value()) {
        failure_ = "world_links.door_contact returned an invalid placement";
        source_boundary("field.door.contact_script_failed");
        return;
    }
    for (std::size_t index = 0U;
         index < field_collision_probe_state_.points.size(); ++index) {
        const auto x = script_integer(script_member(party->at(index), "x"));
        const auto y = script_integer(script_member(party->at(index), "y"));
        if (!x.has_value() || !y.has_value()) {
            failure_ = "world_links.door_contact returned an invalid party point";
            source_boundary("field.door.contact_script_failed");
            return;
        }
        field_collision_probe_state_.points[index] = CollisionProbePoint{
            static_cast<std::uint16_t>(*x),
            static_cast<std::uint16_t>(*y)};
    }
    field_collision_probe_state_.camera_x =
        static_cast<std::uint16_t>(*camera_x);
    field_collision_probe_state_.camera_y =
        static_cast<std::uint16_t>(*camera_y);
    field_collision_probe_state_.initialized = true;
}

std::uint16_t FieldSceneRuntime::field_door_exit_heading(
    const std::uint16_t entry_heading) {
    const std::string_view facing_policy =
        pending_field_return_facing_ ==
                FieldLocationReturnFacing::face_away_from_entry
            ? "face_away_from_entry"
            : "preserve_entry";
    constexpr double kDegreesPerTurn = 360.0;
    constexpr double kUnitsPerTurn = 65536.0;
    const auto entry_bearing =
        static_cast<double>(entry_heading) * kDegreesPerTurn / kUnitsPerTurn;
    const auto result = game_scripts_.call_value(
        "world_links", "exit_bearing",
        {script_object_value({
            {"facing_policy", script_text_value(facing_policy)},
            {"entry_bearing_degrees", script_number_value(entry_bearing)},
        })});
    const auto bearing = result.has_value()
        ? script_number(script_member(*result, "bearing_degrees"))
        : std::nullopt;
    if (!bearing.has_value() || !std::isfinite(*bearing) ||
        *bearing < 0.0 || *bearing >= kDegreesPerTurn) {
        failure_ = result.has_value()
            ? "world_links.exit_bearing returned an invalid bearing"
            : game_scripts_.failure();
        source_boundary("field.door.exit_bearing_script_failed");
        return entry_heading;
    }
    const auto host_heading = static_cast<std::int64_t>(
        std::llround(*bearing * kUnitsPerTurn / kDegreesPerTurn));
    return static_cast<std::uint16_t>(host_heading);
}

void FieldSceneRuntime::save_field_door_return_point() {
    // ROM 0xBC36 has already expanded the selected 0xBD34 bank around the
    // contact position. Those three positions, rather than an inferred
    // heading-to-distance rule, are the source return placement.
    if (pending_field_return_anchor_ !=
            FieldLocationReturnAnchor::collision_probe_lead ||
        !field_collision_probe_state_.initialized) {
        field_door_return_point_ = FieldDoorReturnPoint{};
        source_boundary("field.door.return_policy_or_source_position_missing");
        return;
    }
    field_door_return_point_.valid = true;
    field_door_return_point_.scene_id = static_cast<std::uint16_t>(
        state_.field_scene_buffers.scene_reference);
    for (std::size_t index = 0U;
         index < field_door_return_point_.party.size(); ++index) {
        field_door_return_point_.party[index] = SceneRunnerPosition{
            field_collision_probe_state_.points[index].x,
            field_collision_probe_state_.points[index].y};
    }
    field_door_return_point_.lead = field_door_return_point_.party[0U];
    field_door_return_point_.entry_heading = field_movement_state_.heading;
    field_door_return_point_.return_anchor = pending_field_return_anchor_;
    field_door_return_point_.return_facing = pending_field_return_facing_;
    field_door_return_point_.return_music_policy =
        pending_field_return_music_policy_;
    field_door_return_point_.camera_x =
        field_collision_probe_state_.camera_x;
    field_door_return_point_.camera_y =
        field_collision_probe_state_.camera_y;
    source_boundary("field.door.return_point_saved");
}

void FieldSceneRuntime::place_party_at_field_door_return_point() {
    const auto& point = field_door_return_point_;
    constexpr double kDegreesPerTurn = 360.0;
    constexpr double kUnitsPerTurn = 65536.0;
    const auto facing_policy = point.return_facing ==
            FieldLocationReturnFacing::face_away_from_entry
        ? std::string_view("face_away_from_entry")
        : std::string_view("preserve_entry");
    const auto return_anchor = point.return_anchor ==
            FieldLocationReturnAnchor::collision_probe_lead
        ? std::string_view("collision_probe_lead")
        : std::string_view{};
    ScriptValue::Array party_input;
    party_input.reserve(point.party.size());
    for (const auto& runner : point.party) {
        party_input.push_back(script_object_value({
            {"x", script_integer_value(runner.x)},
            {"y", script_integer_value(runner.y)},
        }));
    }
    const auto entry_bearing = static_cast<double>(point.entry_heading) *
        kDegreesPerTurn / kUnitsPerTurn;
    const auto result = game_scripts_.call_value(
        "world_links", "building_service_exit_return",
        {script_object_value({
            {"point", script_object_value({
                {"party", script_array_value(std::move(party_input))},
                {"camera", script_object_value({
                    {"x", script_integer_value(point.camera_x)},
                    {"y", script_integer_value(point.camera_y)},
                })},
            })},
            {"entry_bearing_degrees", script_number_value(entry_bearing)},
            {"return_anchor", script_text_value(return_anchor)},
            {"return_facing", script_text_value(facing_policy)},
            {"return_music_policy", script_text_value(
                point.return_music_policy ==
                        FieldLocationMusicReturnPolicy::resume_scene_profile
                    ? "resume_scene_profile"
                    : "none")},
        })});
    const auto* returned_party_value = result.has_value()
        ? script_member(*result, "party") : nullptr;
    const auto* returned_party = script_array(returned_party_value);
    const auto* returned_camera = result.has_value()
        ? script_member(*result, "camera") : nullptr;
    const auto camera_x = returned_camera == nullptr
        ? std::nullopt : script_integer(script_member(*returned_camera, "x"));
    const auto camera_y = returned_camera == nullptr
        ? std::nullopt : script_integer(script_member(*returned_camera, "y"));
    const auto bearing = result.has_value()
        ? script_number(script_member(*result, "bearing_degrees"))
        : std::nullopt;
    if (returned_party == nullptr || returned_party->size() != point.party.size() ||
        !camera_x.has_value() || !camera_y.has_value() ||
        !bearing.has_value() || !std::isfinite(*bearing) ||
        *bearing < 0.0 || *bearing >= kDegreesPerTurn) {
        failure_ = result.has_value()
            ? "world_links.building_service_exit_return returned an invalid placement"
            : game_scripts_.failure();
        source_boundary("field.door.exit_return_script_failed");
        return;
    }
    std::array<SceneRunnerPosition, 3> returned_party_positions{};
    for (std::size_t index = 0U; index < returned_party->size(); ++index) {
        const auto x = script_integer(script_member(returned_party->at(index), "x"));
        const auto y = script_integer(script_member(returned_party->at(index), "y"));
        if (!x.has_value() || !y.has_value() || *x < 0 || *x > 0xFFFF ||
            *y < 0 || *y > 0xFFFF) {
            failure_ = "world_links.building_service_exit_return returned an invalid runner point";
            source_boundary("field.door.exit_return_script_failed");
            return;
        }
        returned_party_positions[index] = SceneRunnerPosition{
            static_cast<std::uint16_t>(*x), static_cast<std::uint16_t>(*y)};
    }
    if (*camera_x < 0 || *camera_x > 0xFFFF || *camera_y < 0 ||
        *camera_y > 0xFFFF) {
        failure_ = "world_links.building_service_exit_return returned an invalid camera";
        source_boundary("field.door.exit_return_script_failed");
        return;
    }
    const auto resolved_heading = static_cast<std::uint16_t>(
        std::llround(*bearing * kUnitsPerTurn / kDegreesPerTurn));
    const auto& returned_lead = returned_party_positions[0U];
    field_entity_.world_position = returned_lead;
    field_entity_.projection.world_x = returned_lead.x;
    field_entity_.projection.world_y = returned_lead.y;
    field_movement_state_.position_x = returned_lead.x;
    field_movement_state_.position_y = returned_lead.y;
    // 0x2F32 copies FFDF44..4E into all three runner records.
    auto& return_records =
        state_.field_scene_buffers.source_metadata.initial_field_records;
    for (std::size_t index = 0U;
         index < point.party.size() && index < return_records.size();
         ++index) {
        return_records[index] = returned_party_positions[index];
    }
    field_movement_state_.heading = resolved_heading;
    field_movement_state_.animation_heading = resolved_heading;
    field_target_heading_ = resolved_heading;
    field_movement_behavior_record_.current_heading = resolved_heading;
    field_movement_behavior_record_.target_heading = resolved_heading;
    // The Lua runner keeps its own heading after spawn; respawn it so the
    // restored facing is the one it turns from.
    field_player_actor_script_initialized_ = false;
    field_entity_.direction =
        static_cast<std::uint8_t>(((resolved_heading >> 14U) & 3U) * 2U);
    source_boundary("field.door.exit_return_applied");
}

bool FieldSceneRuntime::apply_field_door_exit_return() {
    // ROM 0xBC36/0xBD34 supplies the saved party point; 0x09B8/0x2FA4
    // establish the outgoing heading and restore the party anchors. Music is
    // resumed by the selected Lua dialogue-exit primitive.
    if (!field_door_return_point_.valid) {
        return false;
    }
    place_party_at_field_door_return_point();
    field_door_return_point_.valid = false;
    field_entity_animation_.redraw_requested = true;
    source_boundary("field.door.service_close_return");
    return true;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::vector<
    FieldSceneRuntime::FieldBuildingServiceExitSweepRow>>
FieldSceneRuntime::perform_building_service_exit_sweep_for_dump(
    std::function<FieldBuildingServiceExitAudioSnapshot()> audio_probe) {
    if (!field_session_source_assets_ || !audio_probe) {
        failure_ = "building/service exit audio sweep needs source profiles "
                   "and a backend probe";
        return std::nullopt;
    }
    const auto cases_result = game_scripts_.call_value(
        "world_links", "building_service_exit_cases", {});
    const auto* cases = cases_result.has_value()
        ? script_array(&*cases_result) : nullptr;
    if (cases == nullptr) {
        failure_ = cases_result.has_value()
            ? "world_links.building_service_exit_cases returned no array"
            : game_scripts_.failure();
        return std::nullopt;
    }

    const auto original_scene = static_cast<std::uint16_t>(
        state_.field_scene_buffers.scene_reference);
    const auto original_profile = state_.field_scene_buffers.profile;
    constexpr std::size_t kSceneProfileMusicWord = 0x40U / 2U;
    std::vector<FieldBuildingServiceExitSweepRow> rows;
    rows.reserve(cases->size());
    for (const auto& exit_case : *cases) {
        const auto exit_id = script_text(script_member(exit_case, "exit_id"));
        const auto scene_id = script_integer(
            script_member(exit_case, "scene_id"));
        const auto link_index = script_integer(
            script_member(exit_case, "link_index"));
        const auto return_side = script_text(
            script_member(exit_case, "return_side"));
        const auto* contact = script_member(exit_case, "contact");
        const auto contact_x = contact == nullptr
            ? std::nullopt : script_integer(script_member(*contact, "x"));
        const auto contact_y = contact == nullptr
            ? std::nullopt : script_integer(script_member(*contact, "y"));
        if (!exit_id.has_value() || !scene_id.has_value() ||
            *scene_id < 0 || *scene_id > 0xFF ||
            !link_index.has_value() || *link_index < 1 ||
            !return_side.has_value() || !contact_x.has_value() ||
            !contact_y.has_value() || *contact_x < 0 || *contact_x > 0xFFFF ||
            *contact_y < 0 || *contact_y > 0xFFFF) {
            failure_ = "world_links.building_service_exit_cases returned an invalid row";
            state_.field_scene_buffers.scene_reference = original_scene;
            return std::nullopt;
        }

        std::uint8_t bank = 0U;
        std::uint16_t entry_heading = 0U;
        if (*return_side == "north") {
            bank = 0U;
            entry_heading = 0x8000U;
        } else if (*return_side == "east") {
            bank = 1U;
            entry_heading = 0xC000U;
        } else if (*return_side == "south") {
            bank = 2U;
            entry_heading = 0x0000U;
        } else if (*return_side == "west") {
            bank = 3U;
            entry_heading = 0x4000U;
        } else {
            failure_ = "world_links.building_service_exit_cases returned an unknown side";
            state_.field_scene_buffers.scene_reference = original_scene;
            return std::nullopt;
        }

        state_.field_scene_buffers.scene_reference =
            static_cast<std::uint16_t>(*scene_id);
        const auto& source_profile =
            field_session_source_assets_->scene_profile_table_source[
                static_cast<std::size_t>(*scene_id)];
        state_.field_scene_buffers.profile = source_profile;

        // The #9 route sweep now runs the shared return close with the
        // source scene's profile. Seed its area track, execute the ROM
        // 0x53126 mode-entry fade, then measure the returned field state.
        scene_music_latch_ = true;
        service_scene_music_latch();
        request_audio_fade();

        set_field_entity_world_position_for_dump(
            static_cast<std::uint16_t>(*contact_x),
            static_cast<std::uint16_t>(*contact_y));
        field_movement_state_.heading = entry_heading;
        field_movement_state_.animation_heading = entry_heading;
        field_movement_behavior_record_.current_heading = entry_heading;
        field_movement_behavior_record_.target_heading = entry_heading;
        field_collision_probe_state_.initialized = false;
        const auto sample = static_cast<std::uint8_t>(0x80U | (bank << 5U));
        record_field_door_contact({sample, 0U, 0U});
        if (!field_collision_probe_state_.initialized) {
            state_.field_scene_buffers.scene_reference = original_scene;
            return std::nullopt;
        }

        const auto policy_result = game_scripts_.call_value(
            "world_links", "building_service_exit_policy",
            {script_object_value({
                {"exit_id", script_text_value(*exit_id)},
            })});
        if (!policy_result.has_value()) {
            failure_ = game_scripts_.failure();
            state_.field_scene_buffers.scene_reference = original_scene;
            return std::nullopt;
        }
        const auto anchor = script_text(
            script_member(*policy_result, "return_anchor"));
        const auto facing = script_text(
            script_member(*policy_result, "return_facing"));
        const auto music_policy = script_text(
            script_member(*policy_result, "return_music_policy"));
        const auto exception_rom = script_text(
            script_member(*policy_result, "rom_address"));
        if (!anchor.has_value() || !facing.has_value() ||
            !music_policy.has_value() ||
            *anchor != "collision_probe_lead" ||
            (*facing != "face_away_from_entry" &&
             *facing != "preserve_entry") ||
            *music_policy != "resume_scene_profile") {
            failure_ = "world_links.building_service_exit_policy returned an invalid policy";
            state_.field_scene_buffers.scene_reference = original_scene;
            return std::nullopt;
        }
        pending_field_return_anchor_ =
            FieldLocationReturnAnchor::collision_probe_lead;
        pending_field_return_facing_ = *facing == "face_away_from_entry"
            ? FieldLocationReturnFacing::face_away_from_entry
            : FieldLocationReturnFacing::preserve_entry_heading;
        pending_field_return_music_policy_ =
            FieldLocationMusicReturnPolicy::resume_scene_profile;
        save_field_door_return_point();
        if (!field_door_return_point_.valid ||
            !apply_field_door_exit_return()) {
            state_.field_scene_buffers.scene_reference = original_scene;
            state_.field_scene_buffers.profile = original_profile;
            return std::nullopt;
        }
        scene_music_latch_ = true;
        // 0x2900 is serviced at the start of the next field frame.
        advance_frame();

        FieldBuildingServiceExitSweepRow row{};
        row.exit_id = std::string(*exit_id);
        row.scene_id = static_cast<std::uint16_t>(*scene_id);
        row.link_index = static_cast<std::size_t>(*link_index);
        row.return_side = std::string(*return_side);
        row.contact = SceneRunnerPosition{
            static_cast<std::uint16_t>(*contact_x),
            static_cast<std::uint16_t>(*contact_y)};
        for (std::size_t index = 0U; index < row.returned_party.size(); ++index) {
            row.returned_party[index] =
                state_.field_scene_buffers.source_metadata
                    .initial_field_records[index];
        }
        row.heading = field_entity_heading_for_dump();
        row.return_anchor = std::string(*anchor);
        row.return_facing = std::string(*facing);
        if (exception_rom.has_value()) {
            row.exception_rom_address = std::string(*exception_rom);
        }
        row.expected_music_id = source_profile[kSceneProfileMusicWord];
        const auto audio = audio_probe();
        row.music_id_after_return = audio.music_id;
        row.attenuation_after_return = audio.attenuation;
        row.backend_events = audio.backend_events;
        rows.push_back(std::move(row));
    }
    state_.field_scene_buffers.scene_reference = original_scene;
    state_.field_scene_buffers.profile = original_profile;
    return rows;
}

std::optional<std::vector<std::string>>
FieldSceneRuntime::building_service_exit_exceptions_for_dump() {
    const auto result = game_scripts_.call_value(
        "world_links", "building_service_exit_exceptions", {});
    const auto* exceptions = result.has_value()
        ? script_array(&*result) : nullptr;
    if (exceptions == nullptr && result.has_value()) {
        const auto* empty_object = std::get_if<ScriptValue::Object>(
            &result->value);
        if (empty_object != nullptr && empty_object->empty()) {
            return std::vector<std::string>{};
        }
    }
    if (exceptions == nullptr) {
        failure_ = result.has_value()
            ? "world_links.building_service_exit_exceptions returned no array"
            : game_scripts_.failure();
        return std::nullopt;
    }
    std::vector<std::string> rows;
    rows.reserve(exceptions->size());
    for (const auto& exception : *exceptions) {
        const auto exit_id = script_text(script_member(exception, "exit_id"));
        const auto rom_address = script_text(
            script_member(exception, "rom_address"));
        const auto anchor = script_text(
            script_member(exception, "return_anchor"));
        const auto facing = script_text(
            script_member(exception, "return_facing"));
        if (!exit_id.has_value() || !rom_address.has_value() ||
            !anchor.has_value() || !facing.has_value()) {
            failure_ = "world_links.building_service_exit_exceptions returned an invalid row";
            return std::nullopt;
        }
        rows.push_back(std::string(*exit_id) + " rom=" +
                       std::string(*rom_address) + " anchor=" +
                       std::string(*anchor) + " facing=" +
                       std::string(*facing));
    }
    return rows;
}
#endif // SHADOWRUN_QT_QA_DRIVER

void FieldSceneRuntime::begin_field_door_return() {
    if (!field_door_return_point_.valid) {
        source_boundary("field.door.return_point_missing");
        return;
    }
    // 0xDB20 -> d6=-1 -> 0x2F32: the party is placed at the return point
    // on the confirm frame [C: emulator run5 8440].
    const auto& point = field_door_return_point_;
    place_party_at_field_door_return_point();
    // 0x2BD2 aligns the saved camera to 128 units before the reload.
    state_.field_scene_buffers.camera_x = point.camera_x & 0xFF80U;
    state_.field_scene_buffers.camera_y = point.camera_y & 0xFF80U;
    field_camera_alignment_ready_ = false;
    field_door_return_point_.valid = false;
    field_door_return_active_ = true;
    field_door_return_frame_ = 0U;
    // Keep the last checkout page paint: 0x131C fades that page, not the
    // field behind it.
    source_boundary("field.door.return_started");
}

bool FieldSceneRuntime::begin_hospital_menu_task(
    const std::string_view function) {
    if (!hospital_service_session_active_) {
        hospital_random_grid_effect_ = RandomGridEffectState{};
        hospital_random_grid_cells_.fill(-1);
        hospital_service_session_active_ = true;
    }
    field_palette_fade_steps_ = 0U;
    if (!field_dialogue_.open_gameplay_task(
            scripting::ScriptDialogueSource::building_service,
            "council_island_hospital", function)) {
        hospital_service_session_active_ = false;
        failure_ = "Council Island Hospital Lua menu task could not open";
        source_boundary("field.hospital.lua_menu_task_failed");
        return false;
    }
    source_boundary("field.hospital.lua_menu_task_opened");
    return true;
}

void FieldSceneRuntime::advance_hospital_grid_effect() {
    if (!field_dialogue_.uses_hospital_menu_layout()) {
        return;
    }
    advance_random_grid_effect(
        hospital_random_grid_effect_,
        {[](const std::uint16_t bound, const std::uint32_t rom_site) {
             const auto value = source_random_bound(bound, rom_site);
             return static_cast<std::uint16_t>(value < 0 ? 0 : value);
         },
         [this](const RandomGridEffectPlan& plan) {
             const auto location =
                 plan.primary_source ==
                         RandomGridPrimarySource::branch_zero_table
                     ? static_cast<std::size_t>(plan.primary_entry_index)
                     : 8U + static_cast<std::size_t>(
                                plan.primary_entry_index);
             const auto pattern =
                 plan.variant_source == RandomGridVariantSource::
                                           fixed_three_variant_resources
                     ? static_cast<std::size_t>(plan.variant_entry_index)
                     : 3U + static_cast<std::size_t>(
                                plan.variant_entry_index);
             if (location < hospital_random_grid_cells_.size() &&
                 pattern < dismiss_runner_random_grid_patterns_.size()) {
                 hospital_random_grid_cells_[location] =
                     static_cast<std::int8_t>(pattern);
             }
         }});
}

void FieldSceneRuntime::paint_hospital_menu(QPainter& painter) const {
    painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
    if (!pocket_secretary_background_.isNull()) {
        painter.drawImage(0, 0, pocket_secretary_background_);
    }
    if (!pocket_secretary_grid_.isNull()) {
        painter.drawImage(24, 24, pocket_secretary_grid_);
    }
    for (std::size_t location = 0;
         location < hospital_random_grid_cells_.size(); ++location) {
        const auto pattern = hospital_random_grid_cells_[location];
        if (pattern < 0 ||
            static_cast<std::size_t>(pattern) >=
                dismiss_runner_random_grid_patterns_.size()) {
            continue;
        }
        painter.drawImage(
            kHospitalRandomGridOrigins[location][0],
            kHospitalRandomGridOrigins[location][1],
            dismiss_runner_random_grid_patterns_[
                static_cast<std::size_t>(pattern)]);
    }
    painter.save();
    painter.setClipRect(QRect(272, 0, 48, 224));
    painter.drawImage(0, 0, field_hud_);
    paint_field_hud_overlays(painter);
    painter.restore();

    const auto& panel = field_dialogue_.panel();
    if (field_dialogue_.uses_hospital_medical_attention_layout()) {
        for (std::size_t row = 0U;
             row < panel.text_segment_count && row < 3U; ++row) {
            const auto y = row < 2U
                               ? 48 + static_cast<int>(row) * 8
                               : 64;
            const auto text = panel.text_segments[row];
            const auto x = row < 2U
                               ? 72
                               : std::max(72, 232 -
                                                   static_cast<int>(text.size()) * 8);
            draw_hotel_menu_text(
                painter, x, y, text,
                HotelMenuPaletteLine::status_and_options);
        }
        for (std::size_t row = 0U; row < panel.option_count; ++row) {
            const auto y = 80 + static_cast<int>(row) * 16;
            draw_hotel_menu_text(
                painter, 72, y, panel.options[row].source_label,
                HotelMenuPaletteLine::status_and_options);
            if (panel.cursor_selection && panel.selected_option == row &&
                !hotel_options_cursor_.isNull()) {
                painter.drawImage(72, y, hotel_options_cursor_);
            }
        }
        return;
    }

    if (panel.option_count == 2U) {
        for (std::size_t row = 0U;
             row < panel.text_segment_count && row < 2U; ++row) {
            draw_hotel_menu_text(
                painter, 80, 72 + static_cast<int>(row) * 8,
                panel.text_segments[row], HotelMenuPaletteLine::checkout);
        }
        for (std::size_t row = 0U; row < panel.option_count; ++row) {
            draw_hotel_menu_text(
                painter, 128, 96 + static_cast<int>(row) * 16,
                panel.options[row].source_label,
                HotelMenuPaletteLine::checkout);
        }
        if (panel.cursor_selection &&
            !hotel_checkout_cursor_.isNull()) {
            painter.drawImage(
                104, 96 + static_cast<int>(panel.selected_option) * 16,
                hotel_checkout_cursor_);
        }
        return;
    }

    if (panel.text_segment_count != 0U) {
        draw_hotel_menu_text(painter, 72, 48, panel.text_segments[0],
                             HotelMenuPaletteLine::status_and_options);
    }
    const auto* separator = game_scripts_.dialogue_text(
        "hospital.options.separator");
    if (separator != nullptr) {
        draw_hotel_menu_text(painter, 72, 56, *separator,
                             HotelMenuPaletteLine::status_and_options);
    }
    for (std::size_t row = 0U; row < panel.option_count; ++row) {
        const auto y = 72 + static_cast<int>(row) * 16;
        draw_hotel_menu_text(
            painter, 72, y, panel.options[row].source_label,
            HotelMenuPaletteLine::status_and_options);
        if (panel.cursor_selection && panel.selected_option == row &&
            !hotel_options_cursor_.isNull()) {
            painter.drawImage(72, y, hotel_options_cursor_);
        }
    }
}

bool FieldSceneRuntime::advance_field_door_return() {
    ++field_door_return_frame_;
    // Inputs are not sampled while the scene reloads.
    field_controller_sample_state_.events = 0U;
    if (field_door_return_frame_ >= checkout_camera_settle_start_frame_ &&
        field_door_return_frame_ < checkout_fade_in_start_frame_) {
        // The reloaded scene runs its camera pass while the display is still
        // black (emulator run5: 8576 -> 9568 over +83..+97, about 64 units a
        // frame), then the camera stays put until walking resumes. Two follow
        // passes per frame approximate that rate [H: the reload scroll
        // routine itself is not traced].
        update_source_field_camera();
        update_source_field_camera();
        field_entity_animation_.redraw_requested = true;
    }
    // The reloaded scene draws the restored party; project the runner so it
    // is on screen when the field fades in (the render pass that normally
    // projects it is skipped while input is held).
    field_entity_.projection.world_x = field_entity_.world_position.x;
    field_entity_.projection.world_y = field_entity_.world_position.y;
    field_entity_.visible = project_entity_screen_position_standard(
        field_entity_.projection,
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y))
                                .visible;
    if (field_door_return_frame_ >= checkout_input_resume_frame_) {
        field_door_return_active_ = false;
        field_door_return_modal_frame_ = QImage{};
        field_modal_return_blocked_inputs_ = 0U;
        // The checkout return has restored the destination scene and is about
        // to rejoin the recurring field coordinator. Release FFDFE8 here so
        // its 0x0AEA Start gate can accept the next edge.
        field_runtime_state_.display_service_word = 0U;
        state_.display_service_word = 0U;
        source_boundary("field.door.return_display_service_released");
        source_boundary("field.door.return_completed");
        return false;
    }
    return true;
}

std::uint8_t FieldSceneRuntime::field_door_return_palette_level()
    const noexcept {
    const auto frame = field_door_return_frame_;
    if (frame < checkout_fade_in_start_frame_) {
        // 0x131C: one CRAM level per two frames from the fade start.
        if (frame <= checkout_fade_out_start_frame_) {
            return 0U;
        }
        return static_cast<std::uint8_t>(std::min<std::uint32_t>(
            (frame - checkout_fade_out_start_frame_) / 2U, 7U));
    }
    // Generic fade-in (ordinary fade): four levels, one per frame.
    const auto since = frame - checkout_fade_in_start_frame_;
    return static_cast<std::uint8_t>(since >= 4U ? 0U : 4U - since);
}


} // namespace shadowrun::ghidra_conversion::qt_runtime

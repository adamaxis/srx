#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"
#include "enums.hpp"
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
using field_scene_detail::kFieldHudAlias;
using field_scene_detail::kFieldHudFaceAliasPrefix;
using field_scene_detail::kFieldHudItemAliasPrefix;
using field_scene_detail::kFieldHudUnarmedItemAliasPrefix;
using field_scene_detail::kFieldHudPartyFaceAliasPrefix;
using field_scene_detail::kFieldHudPartyItemAliasPrefix;
using field_scene_detail::kFieldHudPartyUnarmedItemAliasPrefix;
using field_scene_detail::kFieldHudMagicItemAliasPrefix;
using field_scene_detail::kFieldHudDelayMeterAliasPrefix;
using field_scene_detail::kFieldHudBehaviorMeterPrimaryAliasPrefix;
using field_scene_detail::kFieldHudBehaviorMeterCompanionAliasPrefix;
using field_scene_detail::kFieldHudAmmoCounterAliasPrefix;
using field_scene_detail::kFieldEntityIdleAliasPrefix;
using field_scene_detail::kRunnerFlagsDown;
using field_scene_detail::kFieldPlayerPoseFirearm;
using field_scene_detail::kFieldPlayerPoseStrike;
using field_scene_detail::kFieldPlayerPoseCast;
using field_scene_detail::kFieldPlayerBarrierEffectOwner;
using field_scene_detail::kFieldSpellEffectProfiles;
using field_scene_detail::kFieldSpellEffectPhaseCapacity;
using field_scene_detail::kFieldWeaponProjectileAliasPrefix;
using field_scene_detail::kFieldHitSparkAliasPrefix;
using field_scene_detail::kFieldHitSparkKind;
using field_scene_detail::kFieldHitSparkSelector;
using field_scene_detail::kFieldHitSparkPalette;
using field_scene_detail::kFieldHitSparkFrameCount;
using field_scene_detail::kFieldHitSparkFrameIncrement;
using field_scene_detail::kFieldHitSparkFramePeriod;
using field_scene_detail::kFieldHitSparkTargetYOffset;
using field_scene_detail::kElevatorAnimationAliasPrefix;
using field_scene_detail::kElevatorCurrentFloorAliasPrefix;
using field_scene_detail::kElevatorSelectedFloorAliasPrefix;
using field_scene_detail::kElevatorFeedbackAliasPrefix;
using field_scene_detail::kElevatorFeedbackAssetNames;
using field_scene_detail::kGenesisSpriteOrigin;

bool FieldSceneRuntime::load_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field asset alias: " + std::string(alias);
        return false;
    }
    destination = QImage(QString::fromStdWString(
        record->absolute_path.wstring()));
    if (destination.isNull()) {
        failure_ = "cannot load direct field asset alias: " +
                   std::string(alias);
        return false;
    }
    destination = destination.convertToFormat(
        QImage::Format_ARGB32_Premultiplied);
    return true;
}


bool FieldSceneRuntime::load_field_elevator_selector_assets(
    const ConversionAssetCatalog& catalog) {
    for (std::size_t frame = 0U;
         frame < field_elevator_selector_backgrounds_.size(); ++frame) {
        const auto alias = std::string(kElevatorAnimationAliasPrefix) +
                           std::to_string(frame);
        if (!load_image(catalog, alias,
                        field_elevator_selector_backgrounds_[frame]) ||
            field_elevator_selector_backgrounds_[frame].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.animation_asset_missing");
            return false;
        }
    }
    for (std::size_t floor = 0U;
         floor < kElevatorSelectorMaximumFloors; ++floor) {
        const auto current_alias =
            std::string(kElevatorCurrentFloorAliasPrefix) +
            std::to_string(floor);
        const auto selected_alias =
            std::string(kElevatorSelectedFloorAliasPrefix) +
            std::to_string(floor);
        if (!load_image(
                catalog, current_alias,
                field_elevator_selector_current_floor_layers_[floor]) ||
            !load_image(
                catalog, selected_alias,
                field_elevator_selector_selected_floor_layers_[floor]) ||
            field_elevator_selector_current_floor_layers_[floor].size() !=
                QSize(320, 224) ||
            field_elevator_selector_selected_floor_layers_[floor].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.floor_label_asset_missing");
            return false;
        }
    }
    for (std::size_t index = 0U;
         index < field_elevator_selector_feedback_layers_.size(); ++index) {
        const auto alias = std::string(kElevatorFeedbackAliasPrefix) +
                           std::string(kElevatorFeedbackAssetNames[index]);
        if (!load_image(catalog, alias,
                        field_elevator_selector_feedback_layers_[index]) ||
            field_elevator_selector_feedback_layers_[index].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.navigation_feedback_asset_missing");
            return false;
        }
    }
    return true;
}


bool FieldSceneRuntime::load_field_taxi_selector_assets(
    const ConversionAssetCatalog& catalog) {
    if (!load_image(catalog, "intro.cab.map", field_taxi_map_) ||
        field_taxi_map_.size() != QSize(208, 136)) {
        source_boundary("field.taxi.map_asset_missing");
        return false;
    }

    constexpr std::array<std::string_view, 8> kCursorTileNames{{
        "top.left", "top.middle", "top.right", "left", "right",
        "bottom.left", "bottom.middle", "bottom.right"}};
    for (std::size_t index = 0U; index < kCursorTileNames.size(); ++index) {
        const auto alias = std::string("intro.cab.cursor.") +
                           std::string(kCursorTileNames[index]);
        if (!load_image(catalog, alias, field_taxi_cursor_tiles_[index]) ||
            field_taxi_cursor_tiles_[index].size() != QSize(8, 8)) {
            source_boundary("field.taxi.cursor_asset_missing");
            return false;
        }
    }
    return true;
}


bool FieldSceneRuntime::open_field_elevator_selector(
    const FieldCollisionEventCode event_code) {
    if (!field_session_source_assets_ || field_elevator_selector_.is_open()) {
        return false;
    }
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    std::vector<std::vector<std::uint16_t>> source_elevator_groups;
    source_elevator_groups.reserve(
        field_session_source_assets_->field_elevator_scene_groups.size());
    for (const auto& source_group :
         field_session_source_assets_->field_elevator_scene_groups) {
        if (source_group.count > source_group.scene_ids.size()) {
            source_boundary("field.elevator.source_group_invalid");
            return false;
        }
        auto& group = source_elevator_groups.emplace_back();
        group.insert(group.end(), source_group.scene_ids.begin(),
                     source_group.scene_ids.begin() +
                         static_cast<std::ptrdiff_t>(source_group.count));
    }

    const auto entry_decision = game_scripts_.resolve_field_location_entry(
        scene_reference, event_code, source_elevator_groups);
    if (!entry_decision.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.elevator.entry_script_failed");
        return false;
    }
    if (entry_decision->action !=
            scripting::ScriptLocationEntryAction::open_elevator_selector ||
        entry_decision->elevator_group.scene_ids.size() < 2U ||
        entry_decision->elevator_group.scene_ids.size() >
            kElevatorSelectorMaximumFloors) {
        failure_ = "Lua declined elevator selector entry for scene " +
                   std::to_string(scene_reference) + " and event code " +
                   std::to_string(field_collision_event_value(event_code));
        source_boundary("field.elevator.entry_script_no_selector");
        return false;
    }

    field_elevator_floor_group_ = entry_decision->elevator_group;
    const auto floor_count = static_cast<std::uint8_t>(
        field_elevator_floor_group_->scene_ids.size());
    if (!field_elevator_selector_.open(
            floor_count, field_elevator_floor_group_->entry_floor)) {
        field_elevator_floor_group_.reset();
        return false;
    }
    field_elevator_script_destination_ =
        game_scripts_.resolve_elevator_destination(
            *field_elevator_floor_group_,
            field_elevator_selector_.selected_floor());
    if (!field_elevator_script_destination_.has_value()) {
        failure_ = game_scripts_.failure();
        field_elevator_selector_.close();
        field_elevator_floor_group_.reset();
        source_boundary("field.elevator.destination_script_failed");
        return false;
    }
    source_boundary("field.elevator.selector.opened");
    return true;
}


bool FieldSceneRuntime::transition_to_field_destination(
    const std::uint16_t scene_reference,
    const FieldDestinationTransitionRoute route,
    const std::optional<SceneRunnerPosition> requested_spawn) {
    const auto source_prefix =
        route == FieldDestinationTransitionRoute::dialogue_scene_entry
            ? "field.dialogue.scene_entry."
            : route == FieldDestinationTransitionRoute::taxi
                  ? "field.taxi."
                  : route == FieldDestinationTransitionRoute::vid_phone
                        ? "field.vid_phone."
                        : "field.elevator.";
    if (!field_session_source_assets_ ||
        scene_reference >=
            field_session_source_assets_->field_scene_resources.size() ||
        !prepare_runtime_field_scene(scene_reference, requested_spawn)) {
        source_boundary(std::string(source_prefix) +
                        "destination_prepare_unresolved");
        return false;
    }

    // ROM 0x16A8E reloads the destination field profile through 0x03074/0x03084.
    // The first profile record is the arrival point for both cab travel and
    // elevator floor changes; coordinates from another map are map-local.
    const auto arrival = requested_spawn.value_or(
        state_.field_scene_buffers.source_metadata.initial_field_records[0U]);
    field_entity_.world_position = arrival;
    field_entity_.projection = EntityProjectionState{};
    field_entity_.visible = false;
    field_entity_.direction = 0U;
    field_movement_state_.position_x = arrival.x;
    field_movement_state_.position_y = arrival.y;
    field_movement_state_.heading = 0U;
    field_movement_state_.movement_substate = 0U;
    field_target_heading_ = 0U;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
    field_entity_animation_ = EntityAnimationState{};
    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_id_ = "idle";
    field_player_actor_script_initialized_ = false;
    field_player_animation_reset_pending_ = false;
    field_entity_animation_selector_ = 0U;
    field_movement_behavior_record_.current_heading = 0U;
    field_movement_behavior_record_.target_heading = 0U;
    field_movement_transition_state_.terrain_collision_code = 0U;
    field_movement_transition_state_.mode_event_code = 0U;
    field_movement_transition_state_.activity_sequence_active = false;
    field_movement_transition_state_.terrain_event_suppressed = false;
    field_movement_transition_state_.scene_id = scene_reference;
    // ROM 0x136DE / 0x2A94 seed the FFFC10 cave room for scenes entered from
    // outside the 0x2CE0 graph; Lua world_links owns which scenes do.
    if (const auto entry_room = game_scripts_.call_integers(
            "world_links", "entry_room_flat",
            {static_cast<std::int64_t>(scene_reference)});
        entry_room.has_value() && entry_room->size() == 1U &&
        (*entry_room)[0] >= 0) {
        field_movement_transition_state_.field_transition_context =
            static_cast<std::uint16_t>((*entry_room)[0]);
        source_boundary(std::string(source_prefix) + "entry_room_seeded");
    }
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    field_movement_transition_state_.activity_source_word = 0U;
    field_runtime_state_.scene_id = scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    field_runtime_state_.pending_field_scene_handler = false;
    field_runtime_state_.field_activity_lock = false;
    field_runtime_state_.world_activity_countdown = 0U;
    world_frame_logic_state_.world_mode = scene_reference;
    field_world_activity_scene_entry_pending_ = false;
    field_world_activity_state_ = WorldActivityState{};
    field_think_token_claimed_ = false;
    // FFE188 is not cleared on a scene change; only boot clears it [C].
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    field_world_activity_timeout_state_.scene_id = scene_reference;
    world_activity_routine_state_ = WorldActivityRoutineState{};
    world_activity_request_state_ = WorldActivityRoutineRequestState{};
    world_activity_request_start_state_ = WorldActivityStartState{};
    world_activity_return_snapshot_.reset();
    field_weapon_projectiles_.clear();
    field_hit_sparks_.clear();
    field_weapon_slots_occupied_.fill(false);
    last_field_weapon_spawn_result_ = EntitySpawnCycleResult{};
    field_camera_alignment_ready_ = false;
    field_palette_fade_steps_ = 0U;
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    // ROM 0x08C0, before 0x12F6: a new field mode runs 31 camera and entity
    // projection passes with FFDFE8 clear, then fades in. The fade at 0x12F6
    // only waits on 0x1342. Scene 9's profile spawn (0x0D80, 0x1100) projects
    // to pixel y 232 under camera (0x0A00, 0x0280), so those passes have to
    // finish while the destination is still black.
    const auto restore_interaction_lock =
        field_movement_transition_state_.world_interaction_lock;
    const bool mode_entry_route =
        route == FieldDestinationTransitionRoute::vid_phone ||
        route == FieldDestinationTransitionRoute::dialogue_scene_entry;
    if (mode_entry_route) {
        field_movement_transition_state_.world_interaction_lock = false;
        field_camera_follow_.layer_x_dirty = false;
        field_camera_follow_.layer_y_dirty = false;
    }
    constexpr int kModeEntryCameraPasses = 31;
    const int camera_passes =
        mode_entry_route ? kModeEntryCameraPasses : 1;
    for (int pass = 0; pass < camera_passes; ++pass) {
        update_source_field_camera();
        update_entity_render_pass(state_.render_entities,
                                  callbacks_.entity_render_pass);
    }
    if (mode_entry_route) {
        field_movement_transition_state_.world_interaction_lock =
            restore_interaction_lock;
    }
    dispatch_field_map_scene_entry(scene_reference);
    source_boundary(std::string(source_prefix) + "destination_loaded");
    return true;
}


void FieldSceneRuntime::advance_field_elevator_selector() {
    field_controller_latch_state_.events =
        field_controller_sample_state_.events;
    field_controller_latch_state_.extra_events = 0U;
    (void)::shadowrun::ghidra_conversion::latch_controller_events(
        field_controller_latch_state_);
    ::shadowrun::ghidra_conversion::sample_controller_input(
        field_controller_sample_state_, field_controller_port_byte_);
    const auto update = field_elevator_selector_.advance(
        field_controller_sample_state_.events);
    if (update.selection_moved) {
        if (!field_elevator_floor_group_.has_value()) {
            source_boundary("field.elevator.floor_group_missing");
            return;
        }
        field_elevator_script_destination_ =
            game_scripts_.resolve_elevator_destination(
                *field_elevator_floor_group_,
                field_elevator_selector_.selected_floor());
        if (!field_elevator_script_destination_.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.elevator.destination_script_failed");
            return;
        }
        source_boundary("field.elevator.selector.selection_moved");
    }
    if (update.navigation_feedback_started) {
        source_boundary("field.elevator.selector.navigation_feedback_started");
    }
    if (update.navigation_feedback_finished) {
        source_boundary("field.elevator.selector.navigation_feedback_finished");
    }
    if (update.sound_effect_requested) {
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        source_boundary("field.elevator.selector.source_sound_25");
    }
    if (update.animation_started) {
        source_boundary("field.elevator.animation.started");
    } else if (update.animation_frame_changed) {
        source_boundary("field.elevator.animation.frame_changed");
    } else if (update.animation_finished) {
        source_boundary("field.elevator.animation.finished");
    }
    if (!update.confirmed) {
        return;
    }

    const auto current_scene = state_.field_scene_buffers.scene_reference;
    source_boundary("field.elevator.selector.confirmed");
    if (field_elevator_selector_.destination_changed()) {
        if (!field_elevator_floor_group_.has_value()) {
            source_boundary("field.elevator.floor_group_missing");
            return;
        }
        const auto destination = game_scripts_.resolve_elevator_destination(
            *field_elevator_floor_group_,
            field_elevator_selector_.current_floor());
        if (!destination.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.elevator.destination_script_failed");
            return;
        }
        field_elevator_script_destination_ = destination;
        if (*destination == current_scene) {
            field_elevator_selector_.close();
            field_elevator_floor_group_.reset();
            field_runtime_state_.display_service_word = 0U;
            state_.display_service_word = 0U;
            source_boundary("field.elevator.selector.returned_to_current_scene");
            return;
        }
        field_elevator_pending_destination_ = destination;
        if (field_elevator_scene_transition_.begin_fade_out()) {
            source_boundary("field.elevator.destination_fade_out_started");
        } else {
            field_elevator_pending_destination_.reset();
            source_boundary("field.elevator.destination_transition_busy");
        }
        return;
    }
    field_elevator_selector_.close();
    field_elevator_floor_group_.reset();
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    source_boundary("field.elevator.selector.returned_to_current_scene");
}


void FieldSceneRuntime::advance_field_elevator_scene_transition() {
    const auto update = field_elevator_scene_transition_.advance();
    const bool dialogue_scene_route =
        field_dialogue_scene_transition_active_;
    const bool phone_route = field_phone_transition_active_;
    const bool taxi_route = field_taxi_transition_active_;
    const auto route = dialogue_scene_route
                           ? FieldDestinationTransitionRoute::
                                 dialogue_scene_entry
                           : phone_route
                                 ? FieldDestinationTransitionRoute::vid_phone
                                 : taxi_route
                                       ? FieldDestinationTransitionRoute::taxi
                                       : FieldDestinationTransitionRoute::
                                             elevator;
    const auto source_prefix =
        dialogue_scene_route
            ? "field.dialogue.scene_entry."
            : phone_route ? "field.vid_phone."
                          : taxi_route ? "field.taxi." : "field.elevator.";
    if (update.palette_step_changed) {
        source_boundary(std::string(source_prefix) +
                        "destination_fade.palette_step");
    }
    if (update.destination_load_requested) {
        const auto destination =
            dialogue_scene_route
                ? field_dialogue_scene_pending_destination_
                : phone_route ? field_phone_pending_destination_
                              : taxi_route
                                    ? field_taxi_pending_destination_
                                    : field_elevator_pending_destination_;
        const auto requested_spawn = dialogue_scene_route
            ? field_dialogue_scene_pending_spawn_
            : std::optional<SceneRunnerPosition>{};
        if (!destination.has_value() ||
            !transition_to_field_destination(*destination, route,
                                             requested_spawn)) {
            field_elevator_scene_transition_.cancel();
            field_elevator_pending_destination_.reset();
            field_taxi_pending_destination_.reset();
            field_phone_pending_destination_.reset();
            field_dialogue_scene_pending_destination_.reset();
            field_dialogue_scene_pending_spawn_.reset();
            field_taxi_transition_active_ = false;
            field_phone_transition_active_ = false;
            field_dialogue_scene_transition_active_ = false;
            source_boundary(std::string(source_prefix) +
                            "destination_transition_reopened");
            return;
        }
        if (phone_route) {
            field_phone_pending_destination_.reset();
        } else if (taxi_route) {
            field_taxi_pending_destination_.reset();
        } else if (dialogue_scene_route) {
            field_dialogue_scene_pending_destination_.reset();
            field_dialogue_scene_pending_spawn_.reset();
        } else {
            field_elevator_selector_.close();
            field_elevator_floor_group_.reset();
            field_elevator_pending_destination_.reset();
        }
        if (!field_elevator_scene_transition_.begin_fade_in()) {
            field_elevator_scene_transition_.cancel();
            field_taxi_transition_active_ = false;
            field_phone_transition_active_ = false;
            field_dialogue_scene_transition_active_ = false;
            source_boundary(std::string(source_prefix) +
                            "destination_fade_in_unresolved");
            return;
        }
        source_boundary(std::string(source_prefix) +
                        "destination_fade_in_started");
    } else if (update.completed) {
        source_boundary(std::string(source_prefix) +
                        "destination_fade_finished");
        field_taxi_transition_active_ = false;
        field_phone_transition_active_ = false;
        field_dialogue_scene_transition_active_ = false;
    }
}


bool FieldSceneRuntime::load_sprite_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination,
    int& origin_x,
    int& origin_y) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field sprite alias: " + std::string(alias);
        return false;
    }
    if (!load_image(catalog, alias, destination)) {
        return false;
    }
    origin_x = record->origin_x;
    origin_y = record->origin_y;
    // The extraction bakes color-zero as alpha 0 and loads it through the
    // V4 BMP alpha mask. Re-deriving transparency from black pixels here
    // would punch holes in the sprites' black outlines.
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

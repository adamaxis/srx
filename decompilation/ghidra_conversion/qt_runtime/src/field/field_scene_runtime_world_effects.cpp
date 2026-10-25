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

void FieldSceneRuntime::initialize_field_entity_selection_state() {
    // ROM 0x42B0 publishes the three field-record types in slot order. Seed
    // their selection flags from the same typed records used by the field HUD
    // so the C-button selection action and combat gate share authoritative
    // runner state.
    field_entity_selection_state_ = RunnerSwitchState{};
    const auto& records = field_party_roster_.records();
    field_entity_selection_state_.records.reserve(records.size());
    for (std::size_t slot = 0; slot < records.size(); ++slot) {
        auto selection_flags = static_cast<std::uint8_t>(
            records[slot].flags >> 8U);
        if (!field_party_roster_.slot_active(slot)) {
            selection_flags = static_cast<std::uint8_t>(
                selection_flags & ~0x01U);
        }
        field_entity_selection_state_.records.push_back(
            RunnerSwitchRecord{static_cast<std::uint16_t>(slot),
                               selection_flags});
    }
    field_entity_selection_state_.active_index =
        field_party_roster_.active_slot();
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_runner_refresh_state_ = FieldRunnerRefreshState{};
    field_mode_exit_transition_state_ = FieldModeExitTransitionState{};
    field_mode_exit_transition_state_.action_counter = field_action_counter_;
}

void FieldSceneRuntime::refresh_field_runner_state_and_assets() {
    const auto result =
        ::shadowrun::ghidra_conversion::refresh_field_runner_state_and_assets(
            field_runner_refresh_state_, 0U,
            {[this] {
                 source_boundary("field.runners.serialize_population");
             },
             [this](const FieldRunnerSlot slot) {
                 source_boundary(slot == FieldRunnerSlot::primary
                                     ? "field.runners.clear_primary_effect"
                                     : "field.runners.clear_secondary_effect");
             },
             [this](const FieldRunnerSlot) {
                 handle_field_mode_action_event();
             },
             [this] { source_boundary("field.resources.upload_asset_setup"); },
             [this] {
                 source_boundary("field.resources.run_animation_triplet");
             }});
    if (result.processed[0] || result.processed[1]) {
        source_boundary("field.runners.refresh_processed");
    }
}

void FieldSceneRuntime::handle_field_mode_action_event() {
    field_entity_selection_state_.active_index =
        field_party_roster_.active_slot();
    const auto& runner_records = field_party_roster_.records();
    for (std::size_t slot = 0;
         slot < field_entity_selection_state_.records.size() &&
         slot < runner_records.size();
         ++slot) {
        auto selection_flags = static_cast<std::uint8_t>(
            runner_records[slot].flags >> 8U);
        if (!field_party_roster_.slot_active(slot)) {
            selection_flags = static_cast<std::uint8_t>(
                selection_flags & ~0x01U);
        }
        field_entity_selection_state_.records[slot].flags = selection_flags;
    }
    const auto decision = advance_runner_switch(field_entity_selection_state_);
    const auto selected_slot = field_entity_selection_state_.active_index;
    if (selected_slot < runner_records.size() &&
        field_party_roster_.slot_active(selected_slot)) {
        field_party_roster_.set_active_slot(selected_slot);
    }
    auto& mutable_runner_records = field_party_roster_.records();
    for (std::size_t slot = 0;
         slot < field_entity_selection_state_.records.size() &&
         slot < mutable_runner_records.size();
         ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        auto& runner = mutable_runner_records[slot];
        runner.flags = static_cast<std::uint16_t>(
            (runner.flags & 0x00FFU) |
            (static_cast<std::uint16_t>(field_entity_selection_state_
                                            .records[slot]
                                            .flags)
             << 8U));
    }
    if (field_entity_selection_state_.active_index <
        field_entity_selection_state_.records.size()) {
        field_entity_behavior_flags_ = static_cast<std::uint8_t>(
            field_entity_selection_state_
                .records[field_entity_selection_state_.active_index]
                .flags);
    }
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_entity_.projection.sprite_offset_y =
        active_runner_record().projection_word_08;
    refresh_field_hud_runner_slot();
    source_boundary("field.entities.handle_mode_action_event");
    if (decision.result == RunnerSwitchResult::switched) {
        source_boundary("field.entities.active_runner_changed");
    }
}

void FieldSceneRuntime::run_field_mode_exit_transition() {
    ::shadowrun::ghidra_conversion::finalize_field_mode_exit_transition(
        field_mode_exit_transition_state_,
        {[this](const ExitEffectSlotGroup) {
             source_boundary("field.effects.clear_high_slots");
         },
         [this] { source_boundary("field.world.reset_effect_globals"); },
         [this](const FieldModeExitServicePhase phase) {
             source_boundary(phase == FieldModeExitServicePhase::
                                         mode_specific_cleanup
                                     ? "field.mode_exit.service_cleanup"
                                     : "field.mode_exit.service_refresh");
         },
         [this] { refresh_field_runner_state_and_assets(); },
         [this] { source_boundary("field.resources.run_animation_triplet"); },
         [this] { handle_field_mode_action_event(); },
         [this] { source_boundary("field.mode_exit.initialize_profile_a"); },
         [this] { source_boundary("field.mode_exit.initialize_profile_b"); }});
    field_action_counter_ = field_mode_exit_transition_state_.action_counter;
}

bool FieldSceneRuntime::project_field_world_interaction() {
    const auto& profile = state_.field_scene_buffers.profile;
    WorldMapEffectInput input{};
    input.world_mode = field_movement_transition_state_.world_mode;
    input.scene_id = field_movement_transition_state_.scene_id;
    input.entity_flags = field_movement_behavior_record_.projected_field_code;
    input.action_requested =
        field_movement_transition_state_.mode_event_code != 0U;
    input.map_marker_index = field_collision_marker_index_;
    input.row_stride = profile.size() > 18U ? profile[18U] : 0;
    input.map_value_a = kSceneFourteenMapWordA;
    input.map_value_b = kSceneFourteenMapWordB;
    // FFDD16 is profile word 40. The 48-word block starts at FFDDC6.
    input.auxiliary_world_y = profile.size() > 40U ? profile[40U] : 0U;
    const auto rotated_slot =
        field_movement_behavior_record_.action_heading_source;
    input.auxiliary_world_y_delta = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(rotated_slot << 2U) |
        static_cast<std::uint16_t>(rotated_slot >> 14U));
    input.coordinate = CoordinateProjectionInput{
        field_entity_.world_position.x, field_entity_.world_position.y,
        field_movement_state_.heading};
    input.auxiliary_entity_slot_available =
        entity_table_has_free_slot(field_entity_table_occupancy_);

    const auto effect = apply_world_map_effect(input);
    if (effect.map_marker_cleared) {
        auto& collision_map =
            state_.field_scene_buffers.source_metadata.terrain_collision_map;
        if (field_collision_marker_index_ < collision_map.size()) {
            collision_map[field_collision_marker_index_] = 0U;
        } else {
            source_boundary("field.world.interaction_marker_out_of_range");
        }
    }
    if (effect.special_resource_selected) {
        field_pending_scene_profile_ = kSceneFourInteractionProfile;
    }
    if (effect.world_effect_x_dirty) {
        field_camera_follow_.layer_x_dirty = true;
        viewport_state_.normal_render_pending = 1U;
    }
    if (effect.world_effect_y_dirty) {
        field_camera_follow_.layer_y_dirty = true;
        viewport_state_.alternate_render_pending = 1U;
    }
    if (effect.world_effect_x_dirty || effect.world_effect_y_dirty) {
        apply_world_effect_layer_writes(effect.world_effect_y_dirty,
                                        effect.map_writes);
    }
    if (effect.auxiliary_entity.has_value()) {
        for (std::size_t index = 0; index < field_entity_table_occupancy_.size();
             ++index) {
            if (field_entity_table_occupancy_[index] != 0U) {
                continue;
            }
            // ROM writes -1 into the free slot's leading word.
            field_entity_table_occupancy_[index] = 0xFFFFU;
            field_entity_table_[index] = *effect.auxiliary_entity;
            break;
        }
    }
    return world_map_effect_sets_carry(input);
}

std::int16_t FieldSceneRuntime::select_field_world_effect_sequence() {
    const auto first = sample_source_random_bound(field_random_seed_, 5U);
    const auto second = sample_source_random_bound(field_random_seed_, 5U);
    if (first < 0 || first > 4 || second < 0 || second > 4) {
        pending_field_encounter_outcome_.reset();
        failure_ = "ROM field encounter selector produced an invalid bounded draw";
        source_boundary("field.random_encounter.invalid_selector_draw");
        return 1;
    }
    auto outcome = game_scripts_.resolve_field_encounter(
        state_.field_scene_buffers.scene_reference,
        static_cast<std::uint8_t>(first),
        static_cast<std::uint8_t>(second));
    if (!outcome.has_value()) {
        pending_field_encounter_outcome_.reset();
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.selector_row_unresolved");
        return 1;
    }
    pending_field_encounter_outcome_ = std::move(*outcome);
    return pending_field_encounter_outcome_->kind ==
                   scripting::ScriptFieldEncounterOutcomeKind::dialogue_event
               ? 1
               : 0;
}

void FieldSceneRuntime::trigger_field_world_effect_sequence() {
    if (process_pending_story_world_effect_sequence()) {
        return;
    }
    scripting::ScriptFieldEncounterTriggerInput trigger{};
    trigger.scene_id = state_.field_scene_buffers.scene_reference;
    trigger.frame_counter = field_world_frame_counter_;
    trigger.trigger_delay_active = state_.display_service_word != 0U;
    trigger.current_entity_protected =
        (field_movement_behavior_record_.record_flags & 0x80U) != 0U;
    trigger.activity_lock = field_runtime_state_.field_activity_lock;
    trigger.sequence_mode = world_frame_logic_state_.event_lock != 0U;
    trigger.world_effects_active =
        field_world_activity_timeout_state_.timeout_marker != 0;
    trigger.placement_pending =
        field_world_activity_timeout_state_.scene_36_gate != 0U;

    auto action = game_scripts_.evaluate_field_encounter_trigger(trigger);
    if (!action.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.script_gate_failed");
        return;
    }
    if (*action !=
        scripting::ScriptFieldEncounterTriggerAction::sample_probability) {
        return;
    }

    // 0x1E166 advances the source PRNG only after its state and 0x3FF cadence
    // gates pass. Lua requests this draw, then owns the probability decision.
    trigger.probability_roll = advance_source_random_seed(field_random_seed_);
    trigger.probability_threshold = state_.field_scene_buffers.profile[
        field_scene_profile_word_index(
            FieldSceneProfileWord::random_event_probability_threshold)];
    action = game_scripts_.evaluate_field_encounter_trigger(trigger);
    if (!action.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.script_chance_failed");
        return;
    }
    switch (*action) {
    case scripting::ScriptFieldEncounterTriggerAction::blocked:
    case scripting::ScriptFieldEncounterTriggerAction::not_due:
    case scripting::ScriptFieldEncounterTriggerAction::random_miss:
        return;
    case scripting::ScriptFieldEncounterTriggerAction::forward_active_sequence:
        source_boundary("field.random_encounter.forward_existing_sequence");
        return;
    case scripting::ScriptFieldEncounterTriggerAction::select_scene_outcome:
        (void)select_field_world_effect_sequence();
        if (!pending_field_encounter_outcome_.has_value()) {
            return;
        }
        if (pending_field_encounter_outcome_->kind ==
            scripting::ScriptFieldEncounterOutcomeKind::dialogue_event) {
            failure_ = "ROM random-event dialogue handler is selected but not connected";
            source_boundary(
                "field.random_encounter.dialogue_handler_unimplemented");
            return;
        }
        spawn_field_world_effect_sequence();
        return;
    case scripting::ScriptFieldEncounterTriggerAction::sample_probability:
        failure_ = "Lua field encounter gate requested a second probability draw";
        source_boundary("field.random_encounter.repeated_probability_request");
        return;
    }
}

std::optional<std::size_t> FieldSceneRuntime::projected_terrain_cell() const {
    const auto& profile = state_.field_scene_buffers.profile;
    const auto row_stride_bytes = profile[18U];
    if (row_stride_bytes == 0U) {
        return std::nullopt;
    }
    const auto offset = project_entity_y_axis(
        CoordinateProjectionInput{field_entity_.world_position.x,
                                   field_entity_.world_position.y,
                                   field_movement_state_.heading},
        row_stride_bytes);
    const auto byte_offset = offset + static_cast<std::int32_t>(row_stride_bytes);
    if (byte_offset < 0 || (byte_offset & 1) != 0) {
        return std::nullopt;
    }
    const auto word_index = static_cast<std::size_t>(byte_offset / 2);
    if (word_index >= state_.field_scene_buffers.layer_a.size()) {
        return std::nullopt;
    }
    return word_index;
}

std::size_t FieldSceneRuntime::apply_world_effect_layer_writes(
    const bool layer_b_target, const std::vector<MapWordWrite>& writes) {
    // World-effect writes land in FF4700 when the effect marks the Y layer
    // dirty, otherwise in FF2700 (caller sets FFE100/FFE102). The Qt map is
    // pre-rendered, so each changed cell is repainted from a donor cell that
    // already shows the new (FF2700, FF4700) pair, exactly as R3 does for
    // the 0x16760 terrain mark.
    auto& layer = layer_b_target ? state_.field_scene_buffers.layer_b
                                 : state_.field_scene_buffers.layer_a;
    std::size_t repainted = 0U;
    for (const auto& write : writes) {
        if (write.offset < 0 || (write.offset & 1) != 0) {
            source_boundary("field.world.layer_write_misaligned");
            continue;
        }
        const auto word_index = static_cast<std::size_t>(write.offset / 2);
        if (word_index >= layer.size()) {
            source_boundary("field.world.layer_write_out_of_range");
            continue;
        }
        layer[word_index] = write.value;
        if (word_index >= field_map_cell_words_.size()) {
            continue;   // outside the pre-rendered image (or no geometry)
        }
        const std::pair<std::uint16_t, std::uint16_t> shown =
            field_map_cell_words_[word_index];
        if (shown.first == state_.field_scene_buffers.layer_a[word_index] &&
            shown.second == state_.field_scene_buffers.layer_b[word_index]) {
            continue;   // the image already shows this pair
        }
        if (repaint_field_map_cell(word_index)) {
            ++repainted;
        } else {
            source_boundary("field.world.map_write_without_donor");
        }
    }
    return repainted;
}

void FieldSceneRuntime::reset_field_map_cell_words() {
    constexpr int kCellPixels = 32;
    field_map_cell_words_.clear();
    field_map_cell_columns_ = 0U;
    const auto& layer_a = state_.field_scene_buffers.layer_a;
    const auto& layer_b = state_.field_scene_buffers.layer_b;
    if (field_map_.isNull() || field_map_.width() % kCellPixels != 0 ||
        field_map_.height() % kCellPixels != 0) {
        return;
    }
    const auto columns =
        static_cast<std::size_t>(field_map_.width() / kCellPixels);
    const auto cells =
        columns * static_cast<std::size_t>(field_map_.height() / kCellPixels);
    const auto& profile = state_.field_scene_buffers.profile;
    const auto row_stride_bytes = profile.size() > 18U ? profile[18U] : 0U;
    if (cells > layer_a.size() || cells > layer_b.size() ||
        (row_stride_bytes != 0U && row_stride_bytes != columns * 2U)) {
        source_boundary("field.terrain.map_cell_geometry_mismatch");
        return;
    }
    field_map_cell_columns_ = columns;
    field_map_cell_words_.reserve(cells);
    for (std::size_t index = 0U; index < cells; ++index) {
        field_map_cell_words_.emplace_back(layer_a[index], layer_b[index]);
    }
}

bool FieldSceneRuntime::repaint_field_map_cell(const std::size_t cell) {
    constexpr int kCellPixels = 32;
    if (field_map_cell_columns_ == 0U || cell >= field_map_cell_words_.size()) {
        return false;
    }
    // Both words are read live: 0x16760 changes FF2700 and world effects can
    // change either layer.
    const std::pair<std::uint16_t, std::uint16_t> wanted{
        state_.field_scene_buffers.layer_a[cell],
        state_.field_scene_buffers.layer_b[cell]};
    std::optional<std::size_t> donor;
    for (std::size_t index = 0U; index < field_map_cell_words_.size();
         ++index) {
        if (index != cell && field_map_cell_words_[index] == wanted) {
            donor = index;
            break;
        }
    }
    if (!donor.has_value()) {
        return false;
    }
    const auto cell_rect = [this](const std::size_t index) {
        return QRect(
            static_cast<int>(index % field_map_cell_columns_) * kCellPixels,
            static_cast<int>(index / field_map_cell_columns_) * kCellPixels,
            kCellPixels, kCellPixels);
    };
    const auto source = cell_rect(*donor);
    const auto target = cell_rect(cell);
    for (QImage* image : {&field_map_, &field_map_priority_overlay_}) {
        if (image->isNull()) {
            continue;
        }
        if (image->format() == QImage::Format_Indexed8 ||
            image->format() == QImage::Format_Mono ||
            image->format() == QImage::Format_MonoLSB) {
            // QPainter cannot target palette images; keep the pixels.
            *image = image->convertToFormat(image->hasAlphaChannel()
                                                ? QImage::Format_ARGB32
                                                : QImage::Format_RGB32);
        }
        const auto block = image->copy(source);
        QPainter painter(image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(target.topLeft(), block);
    }
    field_map_cell_words_[cell] = wanted;
    return true;
}

void FieldSceneRuntime::advance_tarislar_tomb_outro() {
    const auto scene_id = state_.field_scene_buffers.scene_reference;
    std::array<std::string, 6> actor_ids{};
    if (scene_id == 55U) {
        for (const auto& actor : field_default_actors_) {
            if (!actor.active || actor.scene_reference != scene_id ||
                !actor.source_record_slot.has_value()) {
                continue;
            }
            const auto slot = static_cast<std::uint8_t>(
                *actor.source_record_slot);
            if (slot < 4U || slot > 9U) {
                continue;
            }
            auto& actor_id = actor_ids[static_cast<std::size_t>(slot - 4U)];
            if (!actor_id.empty()) {
                source_boundary(
                    "field.scene55.tomb_outro_duplicate_actor_slot");
                return;
            }
            actor_id = actor.actor_instance_id;
        }
    }

    const auto step = game_scripts_.advance_tarislar_tomb_outro(
        scene_id, world_frame_logic_state_.scene_state, actor_ids, false);
    if (!step.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.scene55.tomb_outro_lua_failed");
        return;
    }
    if (*step == scripting::ScriptTarislarTombOutroStep::unresolved) {
        source_boundary("field.scene55.tomb_outro_actor_state_unresolved");
        return;
    }
    if (*step != scripting::ScriptTarislarTombOutroStep::completion_ready ||
        field_activity_continuation_ ==
            MovementModeTransitionContinuation::
                scene_55_tomb_credits_callback) {
        return;
    }

    const auto continuation =
        MovementModeTransitionContinuation::scene_55_tomb_credits_callback;
    if (!begin_world_activity_if_idle(
            field_world_activity_state_,
            static_cast<std::uint16_t>(continuation))) {
        return;
    }
    world_frame_logic_state_.resource_countdown =
        field_world_activity_state_.timeout;
    record_field_activity_start(continuation);
    source_boundary("field.scene55.tomb_outro_callback_queued");
}

void FieldSceneRuntime::update_source_field_camera() {
    const auto& profile = state_.field_scene_buffers.profile;
    if (profile.size() < 6U) {
        source_boundary("field.camera.bounds_missing");
        return;
    }
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    if (!field_camera_alignment_ready_) {
        field_camera_follow_.aligned_x =
            static_cast<std::uint16_t>(camera_x & 0xFF80U);
        field_camera_follow_.aligned_y =
            static_cast<std::uint16_t>(camera_y & 0xFF80U);
        field_camera_alignment_ready_ = true;
    }
    field_camera_follow_.camera_x = camera_x;
    field_camera_follow_.camera_y = camera_y;
    field_camera_follow_.minimum_x = profile[2];
    field_camera_follow_.minimum_y = profile[3];
    field_camera_follow_.maximum_x = profile[4];
    field_camera_follow_.maximum_y = profile[5];
    field_camera_follow_.interaction_locked =
        field_movement_transition_state_.world_interaction_lock;
    field_camera_follow_ = follow_field_camera(
        field_camera_follow_, field_entity_.world_position.x,
        field_entity_.world_position.y, field_movement_state_.heading, false);
    state_.field_scene_buffers.camera_x = field_camera_follow_.camera_x;
    state_.field_scene_buffers.camera_y = field_camera_follow_.camera_y;
    const auto row_stride = profile.size() > 18U ? profile[18U] : 0U;
    if (field_camera_follow_.horizontal_edge) {
        FieldTileRowEdgeState row_edge{};
        row_edge.horizontal_direction = field_camera_follow_.horizontal_step;
        row_edge.camera_x = field_camera_follow_.camera_x;
        row_edge.camera_x_limit = static_cast<std::int16_t>(
            field_camera_follow_.maximum_x);
        row_edge.camera_y = field_camera_follow_.camera_y;
        row_edge.field_y = field_camera_follow_.camera_y;
        row_edge.row_stride_bytes = row_stride;
        render_field_tile_row_edge(
            row_edge, {[this](const FieldTileRowEdgeRequest&) {
                source_boundary("field.camera.row_edge");
            }});
    }
    if (field_camera_follow_.vertical_edge) {
        FieldTileColumnEdgeState column_edge{};
        column_edge.vertical_direction = field_camera_follow_.vertical_step;
        column_edge.field_y = field_camera_follow_.camera_y;
        column_edge.field_y_limit = static_cast<std::int16_t>(
            field_camera_follow_.maximum_y >> 4U);
        column_edge.camera_x = field_camera_follow_.camera_x;
        column_edge.camera_y = field_camera_follow_.camera_y;
        column_edge.row_stride_bytes = row_stride;
        render_field_tile_column_edge(
            column_edge, {[this](const FieldTileColumnEdgeRequest&) {
                source_boundary("field.camera.column_edge");
            }});
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

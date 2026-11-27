#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include <iomanip>
#include <sstream>

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
namespace {

// ROM 0x1E1A8..0x1E1B6 checks one encounter opportunity per 1024 field
// frames, at phase 10. Keep that schedule in the native clock adapter; Lua
// receives only whether an opportunity is due.
constexpr std::uint16_t kEncounterCheckPeriodFrames = 1024U;
constexpr std::uint16_t kEncounterCheckPhase = 10U;
constexpr double kSourceRandomRange = 65536.0;

bool encounter_check_is_due(const std::uint16_t field_frame_number) {
    return field_frame_number % kEncounterCheckPeriodFrames ==
           kEncounterCheckPhase;
}

} // namespace

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
    auto service_progress_counters = std::move(
        field_runner_refresh_state_.effect_counters);
    const auto compact_record_markers =
        field_runner_refresh_state_.compact_record_markers;
    field_runner_refresh_state_ = FieldRunnerRefreshState{};
    field_runner_refresh_state_.effect_counters =
        std::move(service_progress_counters);
    field_runner_refresh_state_.compact_record_markers =
        compact_record_markers;
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
         [this] {
             // ROM 0x1E68C clears FFFC1C, including its bit-7 mirror.
             field_runtime_state_.world_activity_scene_reset_bit = false;
             source_boundary("field.world.reset_effect_globals");
         },
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
         [this] { source_boundary("field.mode_exit.initialize_profile_b"); },
         [this] {
             if (!game_scripts_.reset_conditional_actor_selection()) {
                 failure_ = game_scripts_.failure();
                 source_boundary(
                     "field.mode_exit.conditional_actor_choice_reset_failed");
                 return;
             }
             source_boundary("field.mode_exit.conditional_actor_choice_cleared");
         }});
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
    // ROM 0x59986..0x59992: two bound-5 draws from shared FFF9F8.
    const auto first = source_random_bound(5U, source_random_site::world_effect_sequence_first);
    const auto second = source_random_bound(5U, source_random_site::world_effect_sequence_second);
    std::optional<double> weighted_selection;
    if (first >= 0 && second >= 0) {
        weighted_selection = field_scene_detail::
            field_encounter_selection_fraction(
                static_cast<std::uint16_t>(first),
                static_cast<std::uint16_t>(second));
    }
    if (!weighted_selection.has_value()) {
        pending_field_encounter_outcome_.reset();
        failure_ = "ROM field encounter selector produced an invalid bounded draw";
        source_boundary("field.random_encounter.invalid_selector_draw");
        return 1;
    }
    // Lua receives a normalized position in the content weights. Adjacent
    // weights preserve the nine source-sum outcomes from 0x59986 [C].
    auto outcome = game_scripts_.resolve_field_encounter(
        state_.field_scene_buffers.scene_reference,
        *weighted_selection);
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

void FieldSceneRuntime::advance_field_alarm(const bool direct_raise,
                                           const bool clear_alarm,
                                           const bool disable_alarm,
                                           const bool reset_alarm,
                                           const bool scene_exit) {
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    auto profile = game_scripts_.field_alarm_profile(scene_reference);
    if (!profile.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.alarm.profile_unresolved");
        return;
    }

    scripting::ScriptFieldAlarmInput input{};
    // ROM 0x1DFC4 samples on the 13th frame in each 64-frame period. Keep
    // that source counter gate at the host boundary; scripts receive a due fact.
    input.alarm_check_due =
        (field_world_frame_counter_ & static_cast<std::uint16_t>(
            srx::FieldAlarm::FRAME_MASK)) == static_cast<std::uint16_t>(
                srx::FieldAlarm::FRAME_VALUE);
    input.building_scene = field_movement_transition_state_.world_mode ==
        static_cast<std::uint16_t>(srx::SceneMode::BUILDING);
    input.alarm_timeout_scene = scene_reference ==
        static_cast<std::uint16_t>(srx::SceneId::ALARM_COMBAT);
    const auto tile_shift = static_cast<unsigned>(srx::FieldAlarm::TILE_SHIFT);
    input.player_tile_x = field_entity_.world_position.x >> tile_shift;
    input.player_tile_y = field_entity_.world_position.y >> tile_shift;
    input.security_disabled =
        field_matrix_field_state_.security_off.value_or(false);
    input.alarm_active = field_matrix_field_state_.alarm.value_or(false);
    input.alarm_time_remaining_frames = alarm_time_remaining_frames_;
    input.direct_raise = direct_raise;
    input.clear_alarm = clear_alarm;
    input.disable_alarm = disable_alarm;
    input.reset = reset_alarm;
    input.scene_exit = scene_exit;
    input.alarm_regions = std::move(profile->alarm_regions);
    input.reinforcement_activity_id =
        profile->reinforcement_activity_id;
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != scene_reference ||
            !actor.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::uint16_t>(
            *actor.source_record_slot);
        if (slot < static_cast<std::uint16_t>(FieldWorldActorSlot::slot_3) ||
            slot > static_cast<std::uint16_t>(FieldWorldActorSlot::slot_9)) {
            continue;
        }
        input.actors.push_back(scripting::ScriptFieldAlarmActorFacts{
            actor.actor_instance_id,
            field_scene_detail::actor_roles_from_source_byte(
                actor.source_spawn_state_flags)});
    }

    auto result = game_scripts_.evaluate_field_alarm(input);
    if (!result.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.alarm.script_tick_failed");
        return;
    }
    if (result->request_probability_sample) {
        // ROM 0x1DFC4 calls 0xD44 only after its cadence and state gates. The
        // native adapter preserves PRNG order and translates the unsigned word
        // to a probability in [0, 1); Lua applies the named half-chance rule. [C]
        const auto random_word = next_source_random_word(
            source_random_site::field_alarm_half_chance);
        input.probability_sample =
            static_cast<double>(random_word) / 65536.0;
        result = game_scripts_.evaluate_field_alarm(input);
        if (!result.has_value() || result->request_probability_sample) {
            failure_ = game_scripts_.failure().empty()
                ? "field alarm requested more than one random word"
                : game_scripts_.failure();
            source_boundary("field.alarm.probability_sample_failed");
            return;
        }
    }
    apply_field_alarm_result(*result, scene_reference);
}

void FieldSceneRuntime::apply_field_alarm_result(
    const scripting::ScriptFieldAlarmResult& result,
    const std::uint16_t scene_reference) {
    field_matrix_field_state_.alarm = result.alarm_active;
    field_matrix_field_state_.security_off = result.security_disabled;
    alarm_time_remaining_frames_ = result.alarm_time_remaining_frames;

    for (const auto& source : result.actors) {
        const auto actor = std::find_if(
            field_default_actors_.begin(), field_default_actors_.end(),
            [scene_reference, &source](const auto& candidate) {
                return candidate.active &&
                    candidate.scene_reference == scene_reference &&
                    candidate.source_record_slot.has_value() &&
                    candidate.actor_instance_id == source.actor_id;
            });
        if (actor != field_default_actors_.end()) {
            actor->source_spawn_state_flags =
                field_scene_detail::merge_actor_role_facts(
                    actor->source_spawn_state_flags, source.roles);
        }
    }
    if (result.sound_start.has_value() && source_audio_event_callback_) {
        source_audio_event_callback_(*result.sound_start);
    }
    if (result.sound_stop.has_value() && source_audio_stop_callback_) {
        source_audio_stop_callback_(*result.sound_stop);
    }
    if (result.schedule_timeout) {
        if (schedule_world_activity_timeout(
                field_world_activity_timeout_state_)) {
            world_frame_logic_state_.world_active = static_cast<std::uint16_t>(
                field_world_activity_timeout_state_.timeout_marker);
            world_frame_logic_state_.cleanup_countdown =
                static_cast<std::int16_t>(
                    field_world_activity_timeout_state_.timeout_frames);
            source_boundary("field.alarm.scene14_timeout_scheduled");
        } else {
            source_boundary("field.alarm.scene14_timeout_gated");
        }
    }
    if (!result.spawn_reinforcements) {
        return;
    }
    if (!result.reinforcement_activity_id.has_value()) {
        source_boundary("field.alarm.reinforcement_data_unresolved");
        return;
    }
    if (!pending_world_activity_script_actors_.empty() ||
        pending_world_activity_script_actor_scene_.has_value() ||
        field_pending_activity_actor_group_.has_value() ||
        field_route_activity_members_remaining_.has_value()) {
        source_boundary("field.alarm.reinforcement_list_conflict");
        return;
    }
    auto actors = game_scripts_.world_activity_actors_for_activity_id(
        scene_reference, *result.reinforcement_activity_id);
    if (!actors.has_value() || actors->empty()) {
        failure_ = game_scripts_.failure().empty()
            ? "alarm reinforcement list has no extracted actors"
            : game_scripts_.failure();
        source_boundary("field.alarm.reinforcement_list_unresolved");
        return;
    }
    for (auto& actor : *actors) {
        auto roles = field_scene_detail::actor_roles_from_source_byte(
            actor.source_spawn_state_flags);
        roles.reinforcement = true;
        actor.source_spawn_state_flags =
            field_scene_detail::merge_actor_role_facts(
                actor.source_spawn_state_flags, roles);
    }
    pending_world_activity_script_actor_scene_ = scene_reference;
    pending_world_activity_script_actors_ = std::move(*actors);
    publish_pending_world_activity_script_actors(
        scene_reference, "alarm_reinforcements");
    source_boundary("field.alarm.reinforcements_published");
}

bool FieldSceneRuntime::present_field_dialogue_event() {
    if (!pending_field_encounter_outcome_.has_value() ||
        pending_field_encounter_outcome_->kind !=
            scripting::ScriptFieldEncounterOutcomeKind::dialogue_event) {
        return false;
    }
    const auto event_id = pending_field_encounter_outcome_->event_id;
    const char* task_name =
        field_scene_detail::field_dialogue_task_for_event(event_id);
    if (task_name == nullptr) {
        failure_ = "field event has no Lua behavior: " +
            event_id;
        source_boundary(
            "field.random_encounter.dialogue_handler_unimplemented");
        pending_field_encounter_outcome_.reset();
        return false;
    }

    const auto scene_id = static_cast<std::uint8_t>(
        state_.field_scene_buffers.scene_reference);
    game_scripts_.set_location_context(
        "field_map", "random_field_event", "field.world_event",
        "field_narrator", event_id, scene_id);
    if (!field_dialogue_.open_gameplay_task(
            scripting::ScriptDialogueSource::random_encounter,
            "field_random_events", task_name)) {
        failure_ = "could not open field event " + event_id + ": " +
            field_dialogue_.failure();
        source_boundary(
            "field.random_encounter.dialogue_open_failed");
        pending_field_encounter_outcome_.reset();
        return false;
    }
    source_boundary("field.random_encounter." + event_id +
                    ".prompt_opened");
    return true;
}

void FieldSceneRuntime::trigger_field_world_effect_sequence() {
    if (process_pending_story_world_effect_sequence()) {
        return;
    }
    scripting::ScriptFieldEncounterTriggerInput trigger{};
    trigger.scene_id = state_.field_scene_buffers.scene_reference;
    // FFDF88 low 10 bits == 10 at ROM 0x1E1A8..0x1E1B6 [C]. The host's
    // field-frame clock is the source-counter mapping [H].
    trigger.encounter_check_due =
        encounter_check_is_due(field_world_frame_counter_);
    trigger.display_busy = field_runtime_state_.display_service_word != 0U;
    const auto leader_status_ex = static_cast<std::uint8_t>(
        active_runner_record().flags & 0x00FFU); // FFFC04 +0x11 (low byte) [C]
    trigger.leader_unseen =
        (leader_status_ex & static_cast<std::uint8_t>(
             srx::v(srx::ActorStatusEx::UNSEEN))) != 0U; // bit 7 [C]
    trigger.world_activity_locked =
        field_runtime_state_.field_activity_lock; // FFE18B [C]
    trigger.alarm_active =
        field_matrix_field_state_.alarm.value_or(false); // FFE16F [C]
    trigger.combat_active =
        world_frame_logic_state_.world_active != 0U; // FFDF84 [C]
    trigger.encounter_in_progress =
        field_world_activity_timeout_state_.encounter_actor_count != 0U;
    const auto source_probability_threshold = state_.field_scene_buffers.profile[
        field_scene_profile_word_index(
            FieldSceneProfileWord::random_event_probability_threshold)];
    // ROM 0x1E1BC..0x1E1CA accepts the unsigned PRNG result below FFDE18.
    // Scaling both operands by 2^-16 preserves that comparison exactly [C].
    trigger.encounter_probability =
        static_cast<double>(source_probability_threshold) /
        kSourceRandomRange;

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

    // Lua requests this draw only after all state and cadence gates pass.
    // 0x1E1BC..0x1E1CA compares the source unsigned result with FFDE18 [C].
    trigger.probability_sample =
        static_cast<double>(next_source_random_word(
            source_random_site::world_effect_probability)) /
        kSourceRandomRange;
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
    case scripting::ScriptFieldEncounterTriggerAction::select_alarm_sequence: {
        auto outcome = game_scripts_.resolve_field_actor_sequence(
            trigger.scene_id, "field_alarm_reinforcements");
        if (!outcome.has_value() ||
            outcome->kind != scripting::
                ScriptFieldEncounterOutcomeKind::dynamic_world_effect_sequence ||
            outcome->placement_policy != scripting::
                ScriptFieldEncounterPlacementPolicy::screen_edge_candidates) {
            failure_ = game_scripts_.failure().empty()
                ? "ROM alarm alternate actor list is unavailable"
                : game_scripts_.failure();
            source_boundary("field.random_encounter.alarm_list_unresolved");
            return;
        }
        pending_field_encounter_outcome_ = std::move(*outcome);
        source_boundary("field.random_encounter.alarm_sequence_resolved");
        spawn_field_world_effect_sequence();
        return;
    }
    case scripting::ScriptFieldEncounterTriggerAction::select_scene_outcome:
        (void)select_field_world_effect_sequence();
        if (!pending_field_encounter_outcome_.has_value()) {
            return;
        }
        if (pending_field_encounter_outcome_->kind ==
            scripting::ScriptFieldEncounterOutcomeKind::dialogue_event) {
            (void)present_field_dialogue_event();
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
#ifdef SHADOWRUN_QT_QA_DRIVER
        rebuild_field_vdp_mirror();
#endif
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
#ifdef SHADOWRUN_QT_QA_DRIVER
        rebuild_field_vdp_mirror();
#endif
        return;
    }
    field_map_cell_columns_ = columns;
    field_map_cell_words_.reserve(cells);
    for (std::size_t index = 0U; index < cells; ++index) {
        field_map_cell_words_.emplace_back(layer_a[index], layer_b[index]);
    }
#ifdef SHADOWRUN_QT_QA_DRIVER
    rebuild_field_vdp_mirror();
#endif
}

#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::rebuild_field_vdp_mirror() {
    if (!field_vdp_mirror_enabled_) {
        field_vdp_mirror_.reset();
        return;
    }
    if (!field_asset_catalog_) {
        field_vdp_mirror_.reset();
        source_boundary("field.vdp_mirror.asset_catalog_missing");
        return;
    }
    auto next_mirror = std::make_unique<FieldVdpMirror>();
    if (!next_mirror->rebuild(
            *field_asset_catalog_,
            state_.field_scene_buffers.scene_reference, field_map_,
            field_map_priority_overlay_,
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y),
            static_cast<std::int16_t>(
                field_camera_follow_.vertical_bias_b -
                field_camera_follow_.vertical_bias_a))) {
        field_vdp_mirror_.reset();
        source_boundary("field.vdp_mirror.scene_build_failed");
        return;
    }
    field_vdp_mirror_ = std::move(next_mirror);
}

void FieldSceneRuntime::stage_field_sat_pieces(
    std::vector<VdpSpritePiece> pieces) {
    if (field_vdp_mirror_) {
        field_vdp_mirror_->stage_sat_pieces(std::move(pieces));
    }
}

void FieldSceneRuntime::stage_field_vdp_frame() {
    if (!field_vdp_mirror_) {
        return;
    }
    field_vdp_mirror_->stage_frame(
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y),
        static_cast<std::int16_t>(
            field_camera_follow_.vertical_bias_b -
            field_camera_follow_.vertical_bias_a));
}
#endif

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
#ifdef SHADOWRUN_QT_QA_DRIVER
    if (field_vdp_mirror_ &&
        !field_vdp_mirror_->write_metatile_cell(
            cell, *donor, field_map_cell_columns_)) {
        source_boundary("field.vdp_mirror.metatile_write_failed");
    }
#endif
    return true;
}

void FieldSceneRuntime::advance_tarislar_tomb_outro() {
    const auto scene_id = state_.field_scene_buffers.scene_reference;
    std::vector<std::pair<std::uint8_t, std::string>> selected_actors;
    if (scene_id == 55U) {
        // ROM 0x5D29C scans source records in slots 4..9. Resolve that
        // selection here; Lua receives stable identities, not slot numbers.
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
            selected_actors.emplace_back(slot, actor.actor_instance_id);
        }
    }
    std::sort(selected_actors.begin(), selected_actors.end(),
              [](const auto& left, const auto& right) {
                  return left.first < right.first;
              });
    for (std::size_t index = 1U; index < selected_actors.size(); ++index) {
        if (selected_actors[index - 1U].first == selected_actors[index].first) {
            source_boundary(
                "field.scene55.tomb_outro_duplicate_actor_slot");
            return;
        }
    }
    std::vector<std::string> actor_ids;
    actor_ids.reserve(selected_actors.size());
    for (const auto& [slot, actor_id] : selected_actors) {
        static_cast<void>(slot);
        actor_ids.push_back(actor_id);
    }

    const auto step = game_scripts_.advance_tarislar_tomb_outro(
        scene_id == 55U && world_frame_logic_state_.scene_state == 3U,
        actor_ids, false);
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

bool FieldSceneRuntime::start_tarislar_tomb_ending_presentation() {
    if (!ready_ || tarislar_tomb_ending_presentation_active_) {
        return false;
    }
    if (!load_tarislar_tomb_ending_sprite_lists()) {
        return false;
    }
    const auto iteration_count = game_scripts_.call_integers(
        "tarislar_tomb_ending", "iteration_count", {});
    if (!iteration_count.has_value() || iteration_count->size() != 1U ||
        iteration_count->front() < 1 ||
        iteration_count->front() > 0xFFFF) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb ending iteration count is invalid"
            : game_scripts_.failure();
        source_boundary("ending.presentation.iteration_count_failed:0x5BB5A");
        return false;
    }

    const auto reset = game_scripts_.call_integers(
        "tarislar_tomb_ending", "reset", {});
    const auto initial_flash = game_scripts_.call_integers(
        "tarislar_tomb_ending", "initial_flash_state", {});
    if (!reset.has_value() || reset->size() != 1U || reset->front() != 0 ||
        !initial_flash.has_value() || initial_flash->size() != 4U) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb ending initial state is invalid"
            : game_scripts_.failure();
        source_boundary("ending.presentation.initial_state_failed:0x5C2FE");
        return false;
    }
    for (const auto value : *initial_flash) {
        if (value < 0 || value > 0xFFFF) {
            source_boundary("ending.presentation.invalid_initial_flash:0x5C31C");
            return false;
        }
    }
    tarislar_tomb_ending_fff150_ =
        static_cast<std::uint16_t>(initial_flash->at(0U));
    tarislar_tomb_ending_fff14e_ =
        static_cast<std::uint16_t>(initial_flash->at(1U));
    tarislar_tomb_ending_fff152_ =
        static_cast<std::uint16_t>(initial_flash->at(2U));
    tarislar_tomb_ending_fff154_ =
        static_cast<std::uint16_t>(initial_flash->at(3U));
    tarislar_tomb_ending_t_ = 0U;
    tarislar_tomb_ending_now_ = 0U;
    tarislar_tomb_ending_fff1b8_ = 0U;
    tarislar_tomb_ending_loop_iteration_ = 0U;
    tarislar_tomb_ending_fff1e2_ = 0U;
    tarislar_tomb_ending_fff1e6_ = 0U;
    tarislar_tomb_ending_name_map_columns_ = 0U;
    tarislar_tomb_ending_name_map_rows_ = 0U;
    tarislar_tomb_ending_plane_b_cleared_ = false;
    tarislar_tomb_ending_scroll_words_zero_ = false;
    tarislar_tomb_ending_palette_line_0_.fill(0U);
    tarislar_tomb_ending_palette_line_3_.fill(0U);

    tarislar_tomb_ending_presentation_active_ = true;
    // Process 0x5BAC4's t=0 music at the first active host frame, before its
    // first 0x0BD2 loop wait, while the modal path still owns that frame.
    tarislar_tomb_ending_music_pending_ = true;
    tarislar_tomb_ending_iteration_count_ =
        static_cast<std::uint16_t>(iteration_count->front());
    if (!emit_tarislar_tomb_ending_sprite_frame(0U, 0U)) {
        tarislar_tomb_ending_presentation_active_ = false;
        return false;
    }
    source_boundary("ending.presentation.started:0x5BAC4");
    return true;
}

bool FieldSceneRuntime::load_tarislar_tomb_ending_sprite_lists() {
    constexpr std::string_view kSpritePieceTableAlias =
        "cinematics.tarislar.tomb.sprites.pieces";
    if (!field_asset_catalog_) {
        failure_ = "Tarislar Tomb ROM sprite-piece asset catalog is missing";
        source_boundary("ending.sprite_table.catalog_missing:0x1EFB5C");
        return false;
    }
    const auto* record = field_asset_catalog_->find(kSpritePieceTableAlias);
    if (record == nullptr || record->kind != "rom_sprite_piece_table") {
        failure_ = "Tarislar Tomb ROM sprite-piece asset is missing";
        source_boundary("ending.sprite_table.asset_missing:0x1EFB5C");
        return false;
    }
    std::ifstream input(record->absolute_path);
    std::string line;
    if (!input || !std::getline(input, line) ||
        line != "list_address\tw0\tw1\tw2\tw3") {
        failure_ = "Tarislar Tomb ROM sprite-piece asset has an invalid header";
        source_boundary("ending.sprite_table.header_invalid:0x1EFB5C");
        return false;
    }
    std::unordered_map<std::uint32_t,
        std::vector<TarislarTombEndingSpritePiece>> lists;
    std::size_t line_number = 1U;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty()) continue;
        std::uint32_t address = 0U;
        int x_offset = 0;
        int y_offset = 0;
        unsigned int size = 0U;
        unsigned int tile = 0U;
        std::istringstream row(line);
        if (!(row >> address >> x_offset >> y_offset >> size >> tile) ||
            address == 0U || x_offset < -0x8000 || x_offset > 0x7FFF ||
            y_offset < -0x8000 || y_offset > 0x7FFF || size > 0xFFFFU ||
            tile > 0xFFFFU) {
            failure_ = "Tarislar Tomb ROM sprite-piece asset row is invalid at " +
                std::to_string(line_number);
            source_boundary("ending.sprite_table.row_invalid:0x1EFB5C");
            return false;
        }
        lists[address].push_back(TarislarTombEndingSpritePiece{
            static_cast<std::int16_t>(x_offset),
            static_cast<std::int16_t>(y_offset),
            static_cast<std::uint16_t>(size),
            static_cast<std::uint16_t>(tile)});
    }
    if (lists.empty() || !input.eof()) {
        failure_ = "Tarislar Tomb ROM sprite-piece asset is incomplete";
        source_boundary("ending.sprite_table.read_failed:0x1EFB5C");
        return false;
    }
    tarislar_tomb_ending_sprite_lists_ = std::move(lists);
    return true;
}

bool FieldSceneRuntime::emit_tarislar_tomb_ending_sprite_frame(
    const std::uint16_t t, const std::uint16_t now) {
    const auto random_bound = game_scripts_.call_integers(
        "tarislar_tomb_ending", "sprite_random_bound", {t});
    if (!random_bound.has_value() || random_bound->size() != 1U ||
        random_bound->front() < 0 || random_bound->front() > 0xFFFF) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb sprite random bound is invalid"
            : game_scripts_.failure();
        source_boundary("ending.sprite.random_bound_failed:0x5BD6A");
        return false;
    }
    std::int64_t scatter_roll = -1;
    if (random_bound->front() > 0) {
        scatter_roll = source_random_bound(
            static_cast<std::uint16_t>(random_bound->front()),
            source_random_site::unknown);
    }
    const auto requests = game_scripts_.call_integers(
        "tarislar_tomb_ending", "sprite_frame",
        {t, now, scatter_roll});
    if (!requests.has_value() || requests->empty() ||
        (requests->size() - 1U) % 4U != 0U) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb sprite frame is malformed"
            : game_scripts_.failure();
        source_boundary("ending.sprite.frame_failed:0x5BD2C");
        return false;
    }
    if (requests->front() < 0 || requests->front() > 0xFFFF) {
        source_boundary("ending.sprite.invalid_fff1b8:0x5BEA2");
        return false;
    }
    tarislar_tomb_ending_fff1b8_ =
        static_cast<std::uint16_t>(requests->front());

    std::vector<TarislarTombEndingSatEntry> sat_entries;
    std::vector<std::uint32_t> list_addresses;
#ifdef SHADOWRUN_QT_QA_DRIVER
    std::vector<VdpSpritePiece> vdp_pieces;
#endif
    std::uint16_t link = 2U;
    const auto word = [](const std::int64_t value) {
        return static_cast<std::uint16_t>(
            static_cast<std::uint64_t>(value) & 0xFFFFU);
    };
    for (std::size_t index = 1U; index < requests->size(); index += 4U) {
        const auto address_value = requests->at(index);
        const auto x_value = requests->at(index + 1U);
        const auto y_value = requests->at(index + 2U);
        const auto attribute_value = requests->at(index + 3U);
        if (address_value <= 0 || address_value > 0xFFFFFFFFLL ||
            x_value < -0x8000 || x_value > 0x7FFF ||
            y_value < -0x8000 || y_value > 0x7FFF ||
            attribute_value < 0 || attribute_value > 0xFFFF) {
            source_boundary("ending.sprite.invalid_request:0x1378");
            return false;
        }
        const auto address = static_cast<std::uint32_t>(address_value);
        const auto list = tarislar_tomb_ending_sprite_lists_.find(address);
        if (list == tarislar_tomb_ending_sprite_lists_.end()) {
            failure_ = "Tarislar Tomb ROM sprite-piece list is missing: " +
                std::to_string(address);
            source_boundary("ending.sprite.list_missing:0x1378");
            return false;
        }
        list_addresses.push_back(address);
        for (const auto& piece : list->second) {
            // ROM 0x1378/0x1384 adds D1 to signed list word w0, then writes
            // the wrapped word at SAT +6; 0x13BC returns to the next piece.
            const auto x = word(x_value + piece.x_offset);
            const auto y = word(y_value + piece.y_offset);
            const auto size = static_cast<std::uint8_t>(piece.size & 0xFFU);
            const auto tile = static_cast<std::uint16_t>(
                piece.tile | static_cast<std::uint16_t>(attribute_value));
            sat_entries.push_back(TarislarTombEndingSatEntry{
                y, size, static_cast<std::uint8_t>(link), tile, x});
            ++link;

#ifdef SHADOWRUN_QT_QA_DRIVER
            const auto sat_x = x < 0x8000U
                ? static_cast<int>(x) : static_cast<int>(x) - 0x10000;
            const auto sat_y = y < 0x8000U
                ? static_cast<int>(y) : static_cast<int>(y) - 0x10000;
            const auto width =
                (static_cast<int>((size >> 2U) & 0x03U) + 1) * 8;
            const auto height =
                (static_cast<int>(size & 0x03U) + 1) * 8;
            vdp_pieces.push_back(VdpSpritePiece{
                sat_x - 0x80, sat_y - 0x80, width, height});
#endif
        }
    }
    if (!sat_entries.empty()) {
        sat_entries.back().link = 0U;
    }
    tarislar_tomb_ending_sat_entries_ = std::move(sat_entries);
    tarislar_tomb_ending_sat_list_addresses_ = std::move(list_addresses);
#ifdef SHADOWRUN_QT_QA_DRIVER
    stage_field_sat_pieces(std::move(vdp_pieces));
#endif
    return true;
}

void FieldSceneRuntime::advance_tarislar_tomb_ending_presentation() {
    if (tarislar_tomb_ending_music_pending_) {
        const auto initial_events = game_scripts_.call_integers(
            "tarislar_tomb_ending", "step",
            {0, -1, field_camera_follow_.vertical_bias_b});
        if (!initial_events.has_value() || initial_events->size() != 2U ||
            initial_events->at(0U) != static_cast<std::int64_t>(
                TarislarTombEndingEventKind::music) ||
            initial_events->at(1U) < 0 || initial_events->at(1U) > 0xFFFF) {
            failure_ = game_scripts_.failure().empty()
                ? "Lua Tarislar Tomb ending music event is invalid"
                : game_scripts_.failure();
            tarislar_tomb_ending_presentation_active_ = false;
            source_boundary("ending.presentation.music_start_failed:0x5BB4A");
            return;
        }
        const auto music_id =
            static_cast<std::uint16_t>(initial_events->at(1U));
        field_current_music_id_ = music_id;
        field_last_music_scene_id_ =
            state_.field_scene_buffers.scene_reference;
        field_music_requests_.push_back(music_id);
        if (source_music_callback_) {
            source_music_callback_(music_id);
        }
        tarislar_tomb_ending_music_pending_ = false;
    }
    if (tarislar_tomb_ending_loop_iteration_ >=
        tarislar_tomb_ending_iteration_count_) {
        return;
    }
    if (tarislar_tomb_ending_t_ == 0xFFFFU) {
        failure_ = "Tarislar Tomb ending wait counter overflowed";
        tarislar_tomb_ending_presentation_active_ = false;
        source_boundary("ending.presentation.t_overflow:0x0BD2");
        return;
    }
    // Main-loop wait at 0x5BB60: t is FFDF88 - FFF11E, not loop index.
    ++tarislar_tomb_ending_loop_iteration_;
    ++tarislar_tomb_ending_t_;
    ++tarislar_tomb_ending_now_;
    const auto t = tarislar_tomb_ending_t_;
    const auto random_bound = game_scripts_.call_integers(
        "tarislar_tomb_ending", "random_bound", {t});
    if (!random_bound.has_value() || random_bound->size() != 1U ||
        random_bound->front() < 0 || random_bound->front() > 0xFFFF) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb ending random bound is invalid"
            : game_scripts_.failure();
        tarislar_tomb_ending_presentation_active_ = false;
        source_boundary("ending.presentation.random_bound_failed:0x5BC70");
        return;
    }
    std::int64_t shake_roll = -1;
    if (random_bound->front() > 0) {
        shake_roll = source_random_bound(
            static_cast<std::uint16_t>(random_bound->front()),
            source_random_site::unknown);
    }
    const auto events = game_scripts_.call_integers(
        "tarislar_tomb_ending", "step",
        {t, shake_roll, field_camera_follow_.vertical_bias_b});
    if (!events.has_value() || events->size() % 2U != 0U) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb ending step returned malformed events"
            : game_scripts_.failure();
        tarislar_tomb_ending_presentation_active_ = false;
        source_boundary("ending.presentation.step_failed:0x5BC70");
        return;
    }

    std::uint32_t extra_wait_count = 0U;
    for (std::size_t index = 0U; index < events->size(); index += 2U) {
        const auto kind = static_cast<TarislarTombEndingEventKind>(
            events->at(index));
        const auto value = events->at(index + 1U);
        switch (kind) {
        case TarislarTombEndingEventKind::sound:
            if (value < 0 || value > 0xFFFF) {
                source_boundary("ending.presentation.invalid_sound:0x5BBA6");
                tarislar_tomb_ending_presentation_active_ = false;
                return;
            }
            if (source_audio_event_callback_) {
                source_audio_event_callback_(
                    static_cast<std::uint16_t>(value));
            }
            source_boundary("ending.presentation.sound:" +
                            std::to_string(t) + ":" +
                            std::to_string(value));
            break;
        case TarislarTombEndingEventKind::music:
            if (value < 0 || value > 0xFFFF) {
                source_boundary("ending.presentation.invalid_music:0x5BAC4");
                tarislar_tomb_ending_presentation_active_ = false;
                return;
            }
            field_current_music_id_ = static_cast<std::uint16_t>(value);
            field_last_music_scene_id_ =
                state_.field_scene_buffers.scene_reference;
            field_music_requests_.push_back(
                static_cast<std::uint16_t>(value));
            if (source_music_callback_) {
                source_music_callback_(static_cast<std::uint16_t>(value));
            }
            break;
        case TarislarTombEndingEventKind::camera_shake:
            field_camera_follow_.horizontal_bias_a =
                static_cast<std::int16_t>(value);
            break;
        case TarislarTombEndingEventKind::camera_scroll:
            field_camera_follow_.vertical_bias_a =
                static_cast<std::int16_t>(
                    field_camera_follow_.vertical_bias_a + value);
            field_camera_follow_.vertical_bias_b =
                static_cast<std::int16_t>(
                    field_camera_follow_.vertical_bias_b + value);
            break;
        case TarislarTombEndingEventKind::clear_plane_b:
            // ROM 0x5BC70 calls 0x102A, which clears and fills plane B.
            tarislar_tomb_ending_plane_b_cleared_ = true;
#ifdef SHADOWRUN_QT_QA_DRIVER
            if (field_vdp_mirror_) {
                auto& model = field_vdp_mirror_->model();
                for (std::size_t y = 0U; y < VdpModel::kNameTableHeight; ++y) {
                    for (std::size_t x = 0U; x < VdpModel::kNameTableWidth; ++x) {
                        model.set_name_word(VdpPlane::plane_b, x, y, 0U);
                    }
                }
            }
#endif
            source_boundary("ending.presentation.plane_b_clear:0x102A");
            break;
        case TarislarTombEndingEventKind::clear_planes:
#ifdef SHADOWRUN_QT_QA_DRIVER
            if (field_vdp_mirror_) {
                auto& model = field_vdp_mirror_->model();
                for (std::size_t y = 0U; y < VdpModel::kNameTableHeight; ++y) {
                    for (std::size_t x = 0U; x < VdpModel::kNameTableWidth; ++x) {
                        model.set_name_word(VdpPlane::plane_a, x, y, 0U);
                        model.set_name_word(VdpPlane::plane_b, x, y, 0U);
                    }
                }
            }
#endif
            source_boundary("ending.presentation.clear_planes:0x0FFE,0x102A");
            break;
        case TarislarTombEndingEventKind::zero_scroll_words:
            // 0x5C744 clears FFDD08/C/0A/0E before its resource uploads.
            field_camera_follow_.horizontal_bias_a = 0;
            field_camera_follow_.horizontal_bias_b = 0;
            field_camera_follow_.vertical_bias_a = 0;
            field_camera_follow_.vertical_bias_b = 0;
            tarislar_tomb_ending_scroll_words_zero_ = true;
#ifdef SHADOWRUN_QT_QA_DRIVER
            if (field_vdp_mirror_) {
                field_vdp_mirror_->model().set_scroll(VdpScrollState{});
            }
#endif
            source_boundary("ending.presentation.scroll_words_zero:0x5C744");
            break;
        case TarislarTombEndingEventKind::rom_event: {
            std::ostringstream reason;
            reason << "ending.presentation.rom_event:0x" << std::hex
                   << std::uppercase << static_cast<std::uint32_t>(value);
            source_boundary(reason.str());
            break;
        }
        case TarislarTombEndingEventKind::extra_waits:
            if (value < 0 || value > 0xFFFF) {
                source_boundary("ending.presentation.invalid_extra_waits:0x5C744");
                tarislar_tomb_ending_presentation_active_ = false;
                return;
            }
            extra_wait_count += static_cast<std::uint32_t>(value);
            break;
        case TarislarTombEndingEventKind::flash_spawn: {
            const auto registers = game_scripts_.call_integers(
                "tarislar_tomb_ending", "flash_state", {t});
            if (!registers.has_value() || registers->size() != 4U ||
                std::any_of(registers->begin(), registers->end(),
                            [](const std::int64_t value) {
                                return value < 0 || value > 0xFFFF;
                            })) {
                failure_ = game_scripts_.failure().empty()
                    ? "Lua Tarislar Tomb flash spawn is invalid"
                    : game_scripts_.failure();
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.flash_spawn_failed:0x5C31C");
                return;
            }
            tarislar_tomb_ending_fff150_ =
                static_cast<std::uint16_t>(registers->at(0U));
            tarislar_tomb_ending_fff14e_ =
                static_cast<std::uint16_t>(registers->at(1U));
            tarislar_tomb_ending_fff152_ =
                static_cast<std::uint16_t>(registers->at(2U));
            tarislar_tomb_ending_fff154_ =
                static_cast<std::uint16_t>(registers->at(3U));
            source_boundary("ending.presentation.flash_spawn:" +
                            std::to_string(t) + ":0x5C31C");
            break;
        }
        case TarislarTombEndingEventKind::palette_line_0: {
            const auto words = game_scripts_.call_integers(
                "tarislar_tomb_ending", "palette_line_0", {});
            if (!words.has_value() || words->size() !=
                    tarislar_tomb_ending_palette_line_0_.size() ||
                std::any_of(words->begin(), words->end(),
                            [](const std::int64_t word) {
                                return word < 0 || word > 0x0FFF;
                            })) {
                failure_ = game_scripts_.failure().empty()
                    ? "Lua Tarislar Tomb palette is invalid"
                    : game_scripts_.failure();
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.palette_failed:0x5CA74");
                return;
            }
            for (std::size_t color = 0U; color < words->size(); ++color) {
                const auto word = static_cast<std::uint16_t>(words->at(color));
                tarislar_tomb_ending_palette_line_0_[color] = word;
#ifdef SHADOWRUN_QT_QA_DRIVER
                if (field_vdp_mirror_) {
                    field_vdp_mirror_->model().set_cram_word(
                        0U, static_cast<std::uint8_t>(color + 1U), word);
                }
#endif
            }
            source_boundary("ending.presentation.palette_line_0:0x5CA74");
            break;
        }
        case TarislarTombEndingEventKind::palette_line_3: {
            const auto words = game_scripts_.call_integers(
                "tarislar_tomb_ending", "palette_line_3", {});
            if (!words.has_value() || words->size() !=
                    tarislar_tomb_ending_palette_line_3_.size() ||
                std::any_of(words->begin(), words->end(),
                            [](const std::int64_t word) {
                                return word < 0 || word > 0x0FFF;
                            })) {
                failure_ = game_scripts_.failure().empty()
                    ? "Lua Tarislar Tomb line-3 palette is invalid"
                    : game_scripts_.failure();
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.palette_line_3_failed:0x5C9B4");
                return;
            }
            for (std::size_t color = 0U; color < words->size(); ++color) {
                const auto word = static_cast<std::uint16_t>(words->at(color));
                tarislar_tomb_ending_palette_line_3_[color] = word;
#ifdef SHADOWRUN_QT_QA_DRIVER
                if (field_vdp_mirror_) {
                    field_vdp_mirror_->model().set_cram_word(
                        3U, static_cast<std::uint8_t>(color + 1U), word);
                }
#endif
            }
            source_boundary("ending.presentation.palette_line_3:0x5C9B4");
            break;
        }
        case TarislarTombEndingEventKind::background_city: {
            constexpr auto kBackgroundAlias =
                "cinematics.tarislar.tomb.background.city.at.400";
            QImage background;
            if (!field_asset_catalog_ ||
                !load_image(*field_asset_catalog_, kBackgroundAlias,
                            background) ||
                background.size() != QSize(320, 224)) {
                failure_ = failure_.empty()
                    ? "Tarislar Tomb city background asset is invalid"
                    : failure_;
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.background_failed:0x1E8802");
                return;
            }
            // This named image is the already-extracted output of the ROM
            // 0x1530/0x1728 resource sequence; the runtime does not decode ROM.
            field_map_ = std::move(background);
            field_map_priority_overlay_ = QImage(
                field_map_.size(), QImage::Format_ARGB32_Premultiplied);
            field_map_priority_overlay_.fill(Qt::transparent);
            field_map_cell_words_.clear();
            field_map_cell_columns_ = 0U;
            source_boundary("ending.presentation.background_city:0x1E8802");
            break;
        }
        case TarislarTombEndingEventKind::logo_reveal_overlay: {
            constexpr auto kOverlayAlias =
                "cinematics.tarislar.tomb.logo.reveal.overlay";
            const auto* record = field_asset_catalog_
                ? field_asset_catalog_->find(kOverlayAlias)
                : nullptr;
            QImage overlay;
            if (!record || !load_image(*field_asset_catalog_, kOverlayAlias,
                                       overlay) ||
                field_map_.size() != QSize(320, 224)) {
                failure_ = failure_.empty()
                    ? "Tarislar Tomb logo overlay asset is invalid"
                    : failure_;
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.logo_overlay_failed:0x1F310E");
                return;
            }
            QPainter painter(&field_map_);
            painter.drawImage(record->origin_x, record->origin_y, overlay);
            if (!painter.end()) {
                failure_ = "Tarislar Tomb logo overlay could not be composed";
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.logo_overlay_failed:0x1F6532");
                return;
            }
            source_boundary("ending.presentation.logo_overlay:0x1F6532");
            break;
        }
        case TarislarTombEndingEventKind::name_map_begin: {
            const auto map = game_scripts_.call_integers(
                "tarislar_tomb_ending", "name_map_begin", {});
            if (!map.has_value() || map->size() != 3U || map->at(0U) < 0 ||
                map->at(0U) > 0xFFFFFFFFLL || map->at(1U) < 1 ||
                map->at(1U) > 0xFFFF || map->at(2U) < 1 ||
                map->at(2U) > 0xFFFF) {
                failure_ = game_scripts_.failure().empty()
                    ? "Lua Tarislar Tomb name-map dimensions are invalid"
                    : game_scripts_.failure();
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.name_map_begin_failed:0x5C842");
                return;
            }
            tarislar_tomb_ending_fff1e2_ =
                static_cast<std::uint32_t>(map->at(0U));
            tarislar_tomb_ending_fff1e6_ = 0U;
            tarislar_tomb_ending_name_map_columns_ =
                static_cast<std::uint16_t>(map->at(1U));
            tarislar_tomb_ending_name_map_rows_ =
                static_cast<std::uint16_t>(map->at(2U));
            source_boundary("ending.presentation.name_map_begin:0x5C842");
            break;
        }
        case TarislarTombEndingEventKind::name_map_column: {
            constexpr std::size_t kFirstPlaneAColumn = 1U;
            constexpr std::size_t kFirstPlaneARow = 2U;
            // ROM 0x0E9C encodes D4 unchanged. 0x5C8C6 starts at VRAM 0x0102
            // and adds 0x80 bytes per row, so these words land in Plane A
            // (x=1+column,y=2+row); 0x5C744's 0x1728 layout starts at 0x2680
            // in Plane B. This preserves the reveal when 0x102A clears B.
            if (value < 0 || value != tarislar_tomb_ending_fff1e6_ ||
                value >= tarislar_tomb_ending_name_map_columns_) {
                source_boundary("ending.presentation.invalid_name_map_column:0x5C88C");
                tarislar_tomb_ending_presentation_active_ = false;
                return;
            }
            const auto words = game_scripts_.call_integers(
                "tarislar_tomb_ending", "name_map_column", {value});
            if (!words.has_value() || words->size() !=
                    tarislar_tomb_ending_name_map_rows_ ||
                std::any_of(words->begin(), words->end(),
                            [](const std::int64_t word) {
                                return word < 0 || word > 0xFFFF;
                            })) {
                failure_ = game_scripts_.failure().empty()
                    ? "Lua Tarislar Tomb name-map column is invalid"
                    : game_scripts_.failure();
                tarislar_tomb_ending_presentation_active_ = false;
                source_boundary("ending.presentation.name_map_column_failed:0x1F6532");
                return;
            }
#ifdef SHADOWRUN_QT_QA_DRIVER
            if (field_vdp_mirror_) {
                auto& model = field_vdp_mirror_->model();
                for (std::size_t row = 0U; row < words->size(); ++row) {
                    model.set_name_word(
                        VdpPlane::plane_a, kFirstPlaneAColumn +
                            static_cast<std::size_t>(value),
                        kFirstPlaneARow + row,
                        static_cast<std::uint16_t>(words->at(row)));
                }
            }
#else
            (void)kFirstPlaneAColumn;
            (void)kFirstPlaneARow;
#endif
            ++tarislar_tomb_ending_fff1e6_;
            tarislar_tomb_ending_fff1e2_ += 2U;
            source_boundary("ending.presentation.name_map_column:" +
                            std::to_string(value) + ":0x5C8C6");
            break;
        }
        default:
            source_boundary("ending.presentation.unknown_event_kind:0x5BC70");
            tarislar_tomb_ending_presentation_active_ = false;
            return;
        }
    }

    if (extra_wait_count > 0U) {
        if (extra_wait_count > 0xFFFFU - tarislar_tomb_ending_t_) {
            failure_ = "Tarislar Tomb ending extra waits overflowed t";
            tarislar_tomb_ending_presentation_active_ = false;
            source_boundary("ending.presentation.extra_wait_overflow:0x5C744");
            return;
        }
        tarislar_tomb_ending_t_ = static_cast<std::uint16_t>(
            tarislar_tomb_ending_t_ + extra_wait_count);
        tarislar_tomb_ending_now_ = static_cast<std::uint16_t>(
            tarislar_tomb_ending_now_ + extra_wait_count);
    }
    if (!emit_tarislar_tomb_ending_sprite_frame(
            tarislar_tomb_ending_t_, tarislar_tomb_ending_now_)) {
        tarislar_tomb_ending_presentation_active_ = false;
        return;
    }

    ++field_world_frame_counter_;
    if (tarislar_tomb_ending_loop_iteration_ <
        tarislar_tomb_ending_iteration_count_) {
        return;
    }

    tarislar_tomb_ending_presentation_active_ = false;
    source_boundary("ending.presentation.loop_complete:0x5C3B4");
    if (!field_dialogue_.open_gameplay_task(
            scripting::ScriptDialogueSource::cinematic,
            "tarislar_tomb_credits", "present")) {
        failure_ = game_scripts_.failure().empty()
            ? "Lua Tarislar Tomb credits graph could not open after the ending"
            : game_scripts_.failure();
        source_boundary("ending.presentation.credits_open_failed:0x5C3B4");
    }
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

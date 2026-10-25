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

QJsonObject FieldSceneRuntime::debug_inspection_fields() const {
    QJsonObject fields;
    const auto scene = state_.field_scene_buffers.scene_reference;
    fields.insert(QStringLiteral("field.scene"), scene);
    fields.insert(QStringLiteral("field.mode"),
                  field_movement_transition_state_.world_mode);
    fields.insert(QStringLiteral("field.player.x"),
                  field_entity_.world_position.x);
    fields.insert(QStringLiteral("field.player.y"),
                  field_entity_.world_position.y);
    fields.insert(QStringLiteral("field.player.heading"),
                  field_movement_state_.heading);
    fields.insert(QStringLiteral("field.player.direction"),
                  field_entity_.direction);
    fields.insert(QStringLiteral("field.player.target_heading"),
                  field_target_heading_);
    fields.insert(QStringLiteral("field.route.active"),
                  active_field_script_route_.has_value());
    if (active_field_script_route_.has_value()) {
        const auto& route = *active_field_script_route_;
        const auto route_kind_name = [](const FieldScriptRouteKind kind) {
            switch (kind) {
            case FieldScriptRouteKind::dialogue:
                return QStringLiteral("dialogue");
            case FieldScriptRouteKind::field_service:
                return QStringLiteral("field_service");
            case FieldScriptRouteKind::area_event:
                return QStringLiteral("area_event");
            case FieldScriptRouteKind::source_activity_return:
                return QStringLiteral("source_activity_return");
            case FieldScriptRouteKind::source_boundary:
                return QStringLiteral("source_boundary");
            case FieldScriptRouteKind::no_route:
                return QStringLiteral("no_route");
            }
            return QStringLiteral("unknown");
        };
        const auto script_name = [](const FieldScriptId script) {
            switch (script) {
            case FieldScriptId::lua_dialogue:
                return QStringLiteral("lua_dialogue");
            case FieldScriptId::lua_field_service:
                return QStringLiteral("lua_field_service");
            case FieldScriptId::host_field_service:
                return QStringLiteral("host_field_service");
            case FieldScriptId::area_event_table:
                return QStringLiteral("area_event_table");
            case FieldScriptId::source_activity_return:
                return QStringLiteral("source_activity_return");
            case FieldScriptId::source_boundary:
                return QStringLiteral("source_boundary");
            case FieldScriptId::none:
                return QStringLiteral("none");
            }
            return QStringLiteral("unknown");
        };
        const auto destination_mode_name = [](
            const std::optional<WorldActivityMode> mode) {
            if (!mode.has_value()) {
                return QStringLiteral("none");
            }
            switch (*mode) {
            case WorldActivityMode::field:
                return QStringLiteral("field");
            case WorldActivityMode::resource_activity:
                return QStringLiteral("resource_activity");
            case WorldActivityMode::scripted_activity:
                return QStringLiteral("scripted_activity");
            case WorldActivityMode::halted:
                return QStringLiteral("halted");
            }
            return QStringLiteral("unknown");
        };
        fields.insert(QStringLiteral("field.route.kind"),
                      route_kind_name(route.kind));
        fields.insert(QStringLiteral("field.route.script"),
                      script_name(route.script));
        fields.insert(QStringLiteral("field.route.event_code"),
                      route.event_code);
        fields.insert(QStringLiteral("field.route.target_scene"),
                      route.area_event_target.has_value()
                          ? QJsonValue(*route.area_event_target)
                          : QJsonValue(QJsonValue::Null));
        fields.insert(QStringLiteral("field.route.destination_mode"),
                      destination_mode_name(
                          route.area_event_destination_mode));
        fields.insert(QStringLiteral("field.route.return_anchor"),
                      route.return_anchor ==
                              FieldLocationReturnAnchor::collision_probe_lead
                          ? QStringLiteral("collision_probe_lead")
                          : QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_facing"),
                      route.return_facing ==
                              FieldLocationReturnFacing::face_away_from_entry
                          ? QStringLiteral("face_away_from_entry")
                          : QStringLiteral("preserve_entry_heading"));
        fields.insert(QStringLiteral("field.route.entry_dialogue"),
                      !route.entry_dialogue_module.empty());
        fields.insert(QStringLiteral("field.route.entry_module"),
                      QString::fromStdString(route.entry_dialogue_module));
        fields.insert(QStringLiteral("field.route.entry_function"),
                      QString::fromStdString(route.entry_dialogue_function));
        fields.insert(QStringLiteral("field.route.preparation"),
                      route.route_preparation.has_value());
        fields.insert(QStringLiteral("field.route.preparation_module"),
                      route.route_preparation.has_value()
                          ? QString::fromStdString(
                                route.route_preparation->module)
                          : QString());
        fields.insert(QStringLiteral("field.route.preparation_function"),
                      route.route_preparation.has_value()
                          ? QString::fromStdString(
                                route.route_preparation->function)
                          : QString());
    } else {
        fields.insert(QStringLiteral("field.route.kind"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.script"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.event_code"), 255);
        fields.insert(QStringLiteral("field.route.target_scene"),
                      QJsonValue(QJsonValue::Null));
        fields.insert(QStringLiteral("field.route.destination_mode"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_anchor"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_facing"),
                      QStringLiteral("preserve_entry_heading"));
        fields.insert(QStringLiteral("field.route.entry_dialogue"), false);
        fields.insert(QStringLiteral("field.route.entry_module"),
                      QString());
        fields.insert(QStringLiteral("field.route.entry_function"),
                      QString());
    }
    fields.insert(QStringLiteral("field.camera.x"),
                  state_.field_scene_buffers.camera_x);
    fields.insert(QStringLiteral("field.camera.y"),
                  state_.field_scene_buffers.camera_y);
    fields.insert(QStringLiteral("field.world.frame"),
                  field_world_frame_counter_);
    fields.insert(QStringLiteral("field.world.pending_effect"),
                  field_pending_world_effect_);
    fields.insert(QStringLiteral("field.world.active_effect"),
                  field_world_effect_active_);
    fields.insert(QStringLiteral("field.world.encounter_sequence"),
                  static_cast<double>(field_world_encounter_sequence_));
    fields.insert(QStringLiteral("field.world.random_seed"),
                  field_random_seed_);
    // The frame debugger exposes these typed host gates so a replay can show
    // why a field frame is waiting. They are diagnostics, not a second game
    // state representation and not ROM RAM reads.
    fields.insert(QStringLiteral("field.debug.runtime_ready"), ready_);
    fields.insert(QStringLiteral("field.debug.field_activity_lock"),
                  field_runtime_state_.field_activity_lock);
    fields.insert(QStringLiteral("field.debug.world_activity_countdown"),
                  field_runtime_state_.world_activity_countdown);
    fields.insert(
        QStringLiteral("field.debug.mako_actor_transitions_remaining"),
        field_route_actor_transitions_remaining_.has_value()
            ? QJsonValue(static_cast<int>(
                  *field_route_actor_transitions_remaining_))
            : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.actor_list_callback_pending"),
                  field_pending_actor_list_callback_.has_value());
    fields.insert(QStringLiteral("field.debug.heat_level"),
                  field_heat_level_.has_value()
                      ? QJsonValue(*field_heat_level_)
                      : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.notebook_tip_flags_resolved"),
                  notebook_tip_flags_resolved_);
    fields.insert(QStringLiteral("field.debug.mode_flags_resolved"),
                  field_mode_flags_resolved_);
    fields.insert(QStringLiteral("field.debug.party_roster_available"),
                  field_party_roster_.available());
    fields.insert(QStringLiteral("field.debug.mako_result_state_available"),
                  field_heat_level_.has_value() &&
                      field_strike_team_counter_.has_value() &&
                      notebook_tip_flags_resolved_ &&
                      field_mode_flags_resolved_ &&
                      field_party_roster_.available());
    fields.insert(QStringLiteral("field.debug.strike_team_counter"),
                  field_strike_team_counter_.has_value()
                      ? QJsonValue(*field_strike_team_counter_)
                      : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.display_service_word"),
                  field_runtime_state_.display_service_word);
    fields.insert(QStringLiteral("field.debug.pending_scene_handler"),
                  field_runtime_state_.pending_field_scene_handler);
    fields.insert(QStringLiteral("field.debug.scene_transition_requested"),
                  field_runtime_state_.field_transition_requested);
    fields.insert(QStringLiteral("field.debug.opening_auto_walk"),
                  field_opening_auto_walk_.active);
    fields.insert(QStringLiteral("field.debug.interaction_transition"),
                  field_movement_transition_state_.interaction_transition_pending);
    fields.insert(QStringLiteral("field.debug.interaction_transition_timer"),
                  field_movement_transition_state_.interaction_transition_timer);
    fields.insert(QStringLiteral("field.debug.controller_port"),
                  field_controller_port_byte_);
    fields.insert(QStringLiteral("field.debug.controller_held"),
                  field_controller_sample_state_.held);
    fields.insert(QStringLiteral("field.debug.controller_events"),
                  field_controller_sample_state_.events);
    fields.insert(QStringLiteral("field.ui.command_overview"),
                  command_overview_.open());
    fields.insert(QStringLiteral("field.ui.dialogue"),
                  field_dialogue_.open());
    fields.insert(QStringLiteral("field.ui.interaction"),
                  field_interaction_.open());
    fields.insert(QStringLiteral("field.ui.hotel"), hotel_menu_.open());
    fields.insert(QStringLiteral("field.ui.hotel_karma"),
                  hotel_karma_skills_.is_open());
    fields.insert(QStringLiteral("field.ui.debug_menu_unlocked"),
                  debug_menu_unlocked_);

    std::size_t actor_count = 0U;
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != scene) {
            continue;
        }
        ++actor_count;
        const auto actor_id = actor.actor_instance_id.empty()
                                  ? std::to_string(actor.source_order)
                                  : actor.actor_instance_id;
        const auto prefix =
            QStringLiteral("actor.%1.").arg(QString::fromStdString(actor_id));
        fields.insert(prefix + QStringLiteral("profile"),
                      QString::fromStdString(actor.script_profile_id));
        fields.insert(prefix + QStringLiteral("kind"), actor.entity_kind);
        fields.insert(prefix + QStringLiteral("source_order"),
                      actor.source_order);
        fields.insert(prefix + QStringLiteral("record_slot"),
                      actor.source_record_slot.has_value()
                          ? QJsonValue(static_cast<int>(
                                *actor.source_record_slot))
                          : QJsonValue(QJsonValue::Null));
        fields.insert(prefix + QStringLiteral("entity_flags"),
                      actor.source_record_flags); // source +0x10
        fields.insert(prefix + QStringLiteral("collision_flags"),
                      actor.source_interaction_flags); // source +0x11
        fields.insert(prefix + QStringLiteral("special_flags"),
                      actor.source_spawn_state_flags); // source +0x9E
        fields.insert(prefix + QStringLiteral("activity_flags"),
                      actor.source_behavior_flags); // source +0xB0
        fields.insert(prefix + QStringLiteral("world_activity_script_actor"),
                      actor.world_activity_script_actor);
        fields.insert(prefix + QStringLiteral("x"), actor.world_position.x);
        fields.insert(prefix + QStringLiteral("y"), actor.world_position.y);
        fields.insert(prefix + QStringLiteral("action"),
                      static_cast<int>(actor.action_kind));
        fields.insert(prefix + QStringLiteral("animation"),
                      QString::fromStdString(actor.animation_id));
        fields.insert(prefix + QStringLiteral("animation_direction"),
                      actor.animation_direction);
        fields.insert(prefix + QStringLiteral("animation_frame"),
                      actor.animation_frame);
        fields.insert(prefix + QStringLiteral("physical_meter"),
                      actor.physical_meter);
        fields.insert(prefix + QStringLiteral("target_valid"),
                      actor.ai_target_valid);
        fields.insert(prefix + QStringLiteral("target_id"),
                      QString::fromStdString(actor.ai_target_id));
        fields.insert(prefix + QStringLiteral("target_heading"),
                      actor.ai_target_heading);
        fields.insert(prefix + QStringLiteral("target_distance"),
                      actor.ai_target_distance);
        fields.insert(prefix + QStringLiteral("ai_package"),
                      QString::fromStdString(actor.ai_package_id));
        fields.insert(prefix + QStringLiteral("transition_complete"),
                      actor.transition_complete);
        if (actor.ai_weapon_item_id.has_value()) {
            fields.insert(prefix + QStringLiteral("weapon_item"),
                          *actor.ai_weapon_item_id);
        }
    }
    fields.insert(QStringLiteral("field.actor_count"),
                  static_cast<double>(actor_count));

    fields.insert(QStringLiteral("party.active_slot"),
                  static_cast<int>(field_party_roster_.active_slot()));
    for (std::size_t slot = 0; slot < FieldPartyRosterRuntime::kSlotCount;
         ++slot) {
        const auto prefix =
            QStringLiteral("party.runner.%1.").arg(static_cast<qulonglong>(slot));
        fields.insert(prefix + QStringLiteral("active"),
                      field_party_roster_.slot_active(slot));
        const auto& name = field_party_roster_.name(slot);
        fields.insert(prefix + QStringLiteral("name"),
                      QString::fromUtf8(name.data(),
                                        static_cast<qsizetype>(name.size())));
    }

    const auto route = game_scripts_.menu_route_snapshot();
    fields.insert(QStringLiteral("menu.route.open"), route.open);
    fields.insert(QStringLiteral("menu.route.screen"), route.top_screen);
    fields.insert(QStringLiteral("menu.route.title"),
                  QString::fromStdString(route.top_title));
    fields.insert(QStringLiteral("menu.route.text"),
                  QString::fromStdString(route.top_text));
    fields.insert(QStringLiteral("menu.route.matrix.phase"),
                  QString::fromStdString(route.top_matrix_phase));
    fields.insert(QStringLiteral("menu.route.matrix.pending_action"),
                  QString::fromStdString(route.top_matrix_pending_action));
    if (route.top_matrix_world_effect_gate_negative.has_value()) {
        fields.insert(
            QStringLiteral("menu.route.matrix.world_effect_gate_negative"),
            *route.top_matrix_world_effect_gate_negative);
    }
    fields.insert(QStringLiteral("menu.route.matrix.traversal.present"),
                  route.top_matrix_traversal_view.has_value());
    if (route.top_matrix_traversal_view.has_value()) {
        const auto& traversal = *route.top_matrix_traversal_view;
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.world_effect_mode"),
                      traversal.world_effect_mode);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.world_effect_phase"),
                      traversal.world_effect_phase);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.entry_phase"),
                      traversal.world_entry_phase);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.setup_vblanks"),
                      traversal.world_entry_setup_vblanks);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.sequence_vblanks"),
                      traversal.world_entry_sequence_vblanks);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.sequence_duration"),
                      traversal.world_entry_sequence_duration);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.published_frame"),
                      traversal.world_entry_published_frame);
    }
    const auto insert_route_optional = [&fields](
        const QString& key, const std::optional<int> value) {
        if (value.has_value()) {
            fields.insert(key, *value);
        }
    };
    insert_route_optional(QStringLiteral("menu.debug.route.program_index"),
                          route.top_program_index);
    insert_route_optional(QStringLiteral("menu.debug.route.program_slot"),
                          route.top_program_slot);
    insert_route_optional(QStringLiteral("menu.debug.route.program_choice"),
                          route.top_program_choice);
    if (route.top_enter_callback_present.has_value()) {
        fields.insert(QStringLiteral("menu.debug.route.enter_callback_present"),
                      *route.top_enter_callback_present);
    }
    if (route.top_enter_called.has_value()) {
        fields.insert(QStringLiteral("menu.debug.route.enter_called"),
                      *route.top_enter_called);
    }
    insert_route_optional(QStringLiteral("menu.debug.route.enter_transition"),
                          route.top_enter_transition);
    fields.insert(QStringLiteral("menu.debug.route.lua_failure"),
                  QString::fromStdString(game_scripts_.failure()));
    const auto insert_optional = [&fields](const QString& key,
                                           const std::optional<int> value) {
        if (value.has_value()) {
            fields.insert(key, *value);
        }
    };
    insert_optional(QStringLiteral("menu.route.page"), route.page);
    insert_optional(QStringLiteral("menu.route.row"), route.row);
    insert_optional(QStringLiteral("menu.route.selection"),
                    route.top_selection);
    for (std::size_t index = 0; index < route.top_option_labels.size();
         ++index) {
        fields.insert(
            QStringLiteral("menu.route.option.%1")
                .arg(static_cast<qulonglong>(index)),
            QString::fromStdString(route.top_option_labels[index]));
    }
    for (const auto& [name, value] : game_scripts_.story_state()) {
        fields.insert(QStringLiteral("flag.%1").arg(QString::fromStdString(name)),
                      value);
    }
    return fields;
}


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_projectile_selector_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return static_cast<std::uint16_t>(
                spawn.source_record.animation_resource & 0x7FFFU);
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::field_weapon_projectile_list_finished_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return (spawn.source_record.animation_resource & 0x8000U) != 0U;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_projectile_frame_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return spawn.animation.frame;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::field_weapon_primary_count_for_dump() const
    noexcept {
    std::size_t count = 0U;
    for (const auto& spawn : field_weapon_projectiles_) {
        count += spawn.primary_active ? 1U : 0U;
    }
    return count;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::field_weapon_primary_position_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return SceneRunnerPosition{spawn.primary_record.position_x,
                                       spawn.primary_record.position_y};
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::field_weapon_primary_spawn_position_for_dump() const
    noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return spawn.primary_spawn_position;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_primary_heading_for_dump() const noexcept {
    for (auto spawn = field_weapon_projectiles_.rbegin();
         spawn != field_weapon_projectiles_.rend(); ++spawn) {
        if (spawn->primary_active) {
            return spawn->primary_record.heading;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<EntityHeadingOffsets>
FieldSceneRuntime::field_weapon_primary_heading_offsets_for_dump() const
    noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return EntityHeadingOffsets{
                spawn.primary_record.heading_offset_x,
                spawn.primary_record.heading_offset_y};
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::field_weapon_primary_retry_count_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return spawn.primary_record.field_0x89;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::bootstrap_for_scene_dump(
    const integration::StartupIntegrationState& startup_state,
    const std::uint16_t scene_reference) {
    if (scene_reference >= kFieldSceneProfileCount) {
        failure_ = "scene dump requested an out-of-range field scene";
        return false;
    }
    field_scene_reference_override_for_dump_ = scene_reference;
    return bootstrap(startup_state);
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::set_field_vdp_mirror_enabled_for_qa(
    const bool enabled) {
    field_vdp_mirror_qa_override_ = true;
    field_vdp_mirror_enabled_ = enabled;
    if (!enabled) {
        field_vdp_mirror_.reset();
    } else if (ready_) {
        rebuild_field_vdp_mirror();
    }
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::begin_projected_field_transition_for_dump() noexcept {
    if (!ready_) {
        return;
    }
    // This is the typed output state published by the successful 0x1651E
    // projection in 0xB7B8. The QA route starts here so the recurring frame
    // loop, rather than a test-only timer, owns the following 17 VBlank steps.
    field_movement_transition_state_.interaction_transition_pending = true;
    field_movement_transition_state_.interaction_transition_timer = 0x10U;
    field_movement_transition_state_.world_interaction_lock = true;
    field_runtime_state_.display_service_word = 0xFFFFU;
    state_.display_service_word = 0xFFFFU;
    field_palette_fade_steps_ = 0U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::dispatch_field_interaction_through_coordinator_for_dump(
    const FieldInteractionPrompt prompt,
    const std::uint8_t interaction_selection,
    const bool interaction_suppressed,
    const std::optional<std::uint8_t> security_floor_index) {
    if (!ready_ || field_interaction_.open()) {
        return false;
    }
    MovementModeTransitionContinuation continuation =
        MovementModeTransitionContinuation::none;
    switch (prompt) {
    case FieldInteractionPrompt::electromagnetic_lock_door:
        continuation =
            MovementModeTransitionContinuation::maglock_door_interaction;
        break;
    case FieldInteractionPrompt::building_security_terminal:
        continuation =
            MovementModeTransitionContinuation::building_security_terminal;
        break;
    case FieldInteractionPrompt::electromagnetic_lock_resolution:
        return false;
    }
    field_movement_transition_state_.activity_selection =
        interaction_selection;
    field_movement_transition_state_.terrain_event_suppressed =
        interaction_suppressed;
    field_security_floor_index_ = security_floor_index;

    // Inject the typed continuation at the point after collision/event
    // resolution. Use the production activity scheduler, pending-handler bit,
    // and field coordinator to enter the same modal route as gameplay.
    if (!begin_world_activity_if_idle(
            field_world_activity_state_,
            static_cast<std::uint16_t>(continuation))) {
        return false;
    }
    world_frame_logic_state_.resource_countdown =
        field_world_activity_state_.timeout;
    record_field_activity_start(continuation);
    advance_frame();
    return field_interaction_.open();
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::initialize_world_activity_for_dump(
    const std::uint8_t mode_event_code,
    const std::uint16_t random_seed) {
    if (!ready_ || !field_session_source_assets_ || !field_asset_catalog_) {
        return false;
    }

    // This route injects the already-resolved FFFBF9 byte at the 0x167AE
    // dispatcher. The normal movement/collision input that publishes it is
    // deliberately outside this deterministic scene-selection check.
    field_movement_transition_state_.mode_event_code = mode_event_code;
    field_random_seed_ = random_seed;
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.scene_runtime_state = 0U;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    field_runtime_state_.scene_refresh_marker = 0;

    // 0x167AE consumes FFFBF9 as an active-map table index. This focused QA
    // injects that byte directly, outside the collision-driven building
    // interaction branch, so resolve the same active-map table through Lua.
    const auto& event_metadata = state_.field_scene_buffers.source_metadata;
    const auto event_route = game_scripts_.resolve_field_location_route(
        FieldScriptRouteContext{
            static_cast<std::uint8_t>(field_runtime_state_.scene_id),
            field_exit_mode_value(FieldExitMode::world_activity_table),
            field_collision_event_code_from_id(mode_event_code),
            event_metadata.event_table_available,
            event_metadata.event_mode_table});
    if (!event_route.has_value() ||
        event_route->kind != FieldScriptRouteKind::area_event ||
        event_route->area_event_destination_mode !=
            WorldActivityMode::resource_activity ||
        !event_route->area_event_target.has_value()) {
        if (event_route.has_value()) {
            failure_ =
                "injected world-activity event did not resolve to a "
                "resource area (scene=" +
                std::to_string(field_runtime_state_.scene_id) + ", mode=" +
                std::to_string(field_runtime_state_.field_mode) +
                ", event=" + std::to_string(mode_event_code) +
                ", route-kind=" +
                std::to_string(static_cast<int>(event_route->kind)) +
                ", target=" +
                (event_route->area_event_target.has_value()
                     ? std::to_string(*event_route->area_event_target)
                     : "none") +
                ")";
        } else {
            failure_ = game_scripts_.failure();
        }
        source_boundary("field.world_activity.qa_route_unresolved");
        return false;
    }
    active_field_script_route_ = *event_route;

    field_runtime_callbacks_.initialize_world_activity_mode(
        field_runtime_state_);
    if (field_runtime_state_.field_transition_requested ||
        world_activity_routine_state_.mode !=
            WorldActivityMode::resource_activity ||
        !field_security_floor_index_.has_value()) {
        return false;
    }

    // 0x0A54 supplies the nonzero marker consumed by 0x2BD2's scene/profile
    // re-entry branch. Exercise that normal preparation path after the
    // dispatcher has selected and prepared the typed resource scene.
    field_runtime_state_.scene_refresh_marker = -1;
    advance_field_scene_transition_after_modal();
    return state_.field_scene_buffers.scene_reference ==
               world_activity_routine_state_.scene_id &&
           field_security_floor_index_.value() <
               world_activity_routine_state_.resource_random_index_bound;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::return_from_world_activity_for_dump() {
    if (!ready_ || !world_activity_return_snapshot_.has_value()) {
        return false;
    }
    const auto source_scene =
        world_activity_return_snapshot_->scene_buffers.scene_reference;
    field_runtime_state_.field_transition_requested = true;
    advance_frame();
    return !world_activity_return_snapshot_.has_value() &&
           !field_runtime_state_.field_transition_requested &&
           state_.field_scene_buffers.initialized &&
           state_.field_scene_buffers.scene_reference == source_scene;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::dispatch_selected_security_terminal_for_dump() {
    if (!ready_ || !field_security_floor_index_.has_value() ||
        world_activity_routine_state_.mode !=
            WorldActivityMode::resource_activity ||
        field_interaction_.open()) {
        return false;
    }
    field_activity_continuation_ =
        MovementModeTransitionContinuation::building_security_terminal;
    field_runtime_state_.pending_field_scene_handler = true;
    field_runtime_state_.field_activity_lock = false;
    field_runtime_state_.world_activity_countdown = 0U;
    field_runtime_state_.display_service_word = 0U;
    advance_frame();
    return field_interaction_.open() &&
           field_interaction_.panel().option_count == 2U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::paint_field_scene_for_dump(
    QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!ready_ || field_map_.isNull()) {
        return;
    }
    paint_field_scene(painter);
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::default_actor_count_for_dump() const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.scene_reference == scene_reference;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::world_activity_script_actor_count_for_dump()
    const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.world_activity_script_actor &&
                   actor.scene_reference == scene_reference;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::world_activity_script_actor_position_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<SceneRunnerPosition>{actor->world_position};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::world_activity_script_actor_profile_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<std::string>{actor->script_profile_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldWorldActorSlot>
FieldSceneRuntime::world_activity_script_actor_record_slot_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : actor->source_record_slot;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::world_activity_script_actor_behavior_flags_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<std::uint8_t>{actor->source_behavior_flags};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::vector<SceneRunnerPosition>
FieldSceneRuntime::active_default_actor_positions_for_dump() const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    std::vector<SceneRunnerPosition> positions;
    for (const auto& actor : field_default_actors_) {
        if (actor.active && actor.scene_reference == scene_reference) {
            positions.push_back(actor.world_position);
        }
    }
    return positions;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::selected_default_actor_id_for_dump() const {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return std::nullopt;
    }
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, selected_slot](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_record_slot.has_value() &&
                   static_cast<std::size_t>(
                       *candidate.source_record_slot) == selected_slot;
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    return actor->actor_instance_id;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldTargetMarkerDebugState>
FieldSceneRuntime::selected_default_actor_target_marker_for_dump() const noexcept {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return std::nullopt;
    }
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, selected_slot](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_record_slot.has_value() &&
                   static_cast<std::size_t>(
                       *candidate.source_record_slot) == selected_slot;
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    EntityProjectionState projection{};
    projection.world_x = actor->world_position.x;
    projection.world_y = actor->world_position.y;
    project_entity_screen_position_standard(
        projection,
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y));
    return FieldTargetMarkerDebugState{
        actor->target_marker_visible,
        actor->target_marker_animating,
        actor->target_marker_style,
        actor->target_marker_radius,
        actor->entity_kind,
        projection.screen_x,
        projection.screen_y};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldCreatureAttackEffectDebugState>
FieldSceneRuntime::default_actor_creature_attack_effect_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order &&
                   candidate.script_profile_id ==
                       "hell_hound_k0c_none00";
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    return FieldCreatureAttackEffectDebugState{
        actor->target_motion_effect_visible,
        actor->target_motion_effect_direction,
        actor->target_motion_effect_phase,
        actor->target_motion_effect_slot,
        actor->action_delay};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_default_actor_world_position_for_dump(
    const std::size_t active_actor_index, const std::uint16_t x,
    const std::uint16_t y) noexcept {
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    std::size_t active_index = 0U;
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene) {
            continue;
        }
        if (active_index == active_actor_index) {
            actor.world_position = SceneRunnerPosition{x, y};
            actor.movement_state.position_x = x;
            actor.movement_state.position_y = y;
            actor.movement_state_initialized = true;
            return true;
        }
        ++active_index;
    }
    return false;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_field_spell_effect_for_dump(
    const std::uint8_t action_code, const std::uint8_t phase,
    const bool visible) noexcept {
    if (!visible) {
        field_entity_.spell_effect_visible = false;
        field_entity_.spell_effect_action_code = 0U;
        field_entity_.spell_effect_phase = 0U;
        return true;
    }

    const auto profile = std::find_if(
        kFieldSpellEffectProfiles.cbegin(), kFieldSpellEffectProfiles.cend(),
        [action_code](const auto& candidate) {
            return candidate.action_code == action_code;
        });
    if (profile == kFieldSpellEffectProfiles.cend() ||
        phase >= profile->phase_count) {
        return false;
    }

    field_entity_.spell_effect_visible = true;
    field_entity_.spell_effect_action_code = action_code;
    field_entity_.spell_effect_phase = phase;
    return true;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::int16_t FieldSceneRuntime::probe_field_collision_for_dump(
    const std::uint16_t x, const std::uint16_t y) {
    EntityCollisionRecord collision_record{};
    std::array<std::uint8_t, 4> sample_state{};
    return evaluate_field_collision(
        x, y, collision_record, sample_state,
        field_entity_behavior_flags_, false);
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_selected_default_actor_world_position_for_dump(
    const std::uint16_t x, const std::uint16_t y) noexcept {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return false;
    }
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene ||
            !actor.source_record_slot.has_value() ||
            static_cast<std::size_t>(*actor.source_record_slot) !=
                selected_slot) {
            continue;
        }
        actor.world_position = SceneRunnerPosition{x, y};
        actor.movement_state.position_x = x;
        actor.movement_state.position_y = y;
        actor.movement_state_initialized = true;
        return true;
    }
    return false;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::resolve_and_spawn_field_encounter_for_dump(
    const std::uint8_t first_roll, const std::uint8_t second_roll,
    const std::uint16_t random_seed) {
    if (!ready_) {
        failure_ = "field encounter QA requested before field bootstrap";
        return 0U;
    }
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    field_random_seed_ = random_seed;
    auto outcome = game_scripts_.resolve_field_encounter(
        scene_reference, first_roll, second_roll);
    if (!outcome.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.qa_selector_unresolved");
        return 0U;
    }
    if (outcome->kind != scripting::ScriptFieldEncounterOutcomeKind::
                             dynamic_world_effect_sequence) {
        failure_ = "field encounter QA selector did not resolve an actor sequence";
        source_boundary("field.random_encounter.qa_outcome_not_actor_sequence");
        return 0U;
    }

    const auto count_before = default_actor_count_for_dump();
    pending_field_encounter_outcome_ = std::move(*outcome);
    // This diagnostic entry point injects the already-resolved Lua selector
    // rolls. The 0x1E166 chance gate and 0x59986 frame eligibility remain
    // exercised by the live field loop, not by this focused spawn QA.
    spawn_field_world_effect_sequence();
    const auto count_after = default_actor_count_for_dump();
    return count_after >= count_before ? count_after - count_before : 0U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t
FieldSceneRuntime::default_actor_transition_started_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.active &&
                   actor.transition_started;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t
FieldSceneRuntime::default_actor_transition_complete_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.active &&
                   actor.transition_complete;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_direction_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->animation_direction};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_animation_frame_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{
                     actor->animation_frame};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_animation_id_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::string>{actor->animation_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldWorldActorSlot>
FieldSceneRuntime::default_actor_record_slot_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : actor->source_record_slot;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_script_profile_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() ||
                   actor->script_profile_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->script_profile_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_ai_package_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() ||
                   actor->ai_package_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->ai_package_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_ai_target_id_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() || actor->ai_target_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->ai_target_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::default_actor_transition_complete_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<bool>{actor->transition_complete};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::default_actor_has_ai_target_pool_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<bool>{actor->ai_target_valid};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::default_actor_ai_target_distance_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() || actor->ai_target_id.empty()
               ? std::nullopt
               : std::optional<std::uint16_t>{actor->ai_target_distance};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_weapon_item_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : actor->ai_weapon_item_id;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_source_action_ammo_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active && candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->source_action_ammo};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::default_actor_source_animation_selector_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active && candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint16_t>{
                     actor->source_animation_selector};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_physical_meter_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->physical_meter};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::visible_default_actor_count_for_dump() const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    std::size_t visible = 0U;
    for (const auto& actor : field_default_actors_) {
        if (actor.scene_reference != scene_reference) {
            continue;
        }
        const auto sprite = default_actor_sprite_view(actor);
        if (sprite.image == nullptr || sprite.image->isNull()) {
            continue;
        }
        EntityProjectionState projection{};
        projection.world_x = actor.world_position.x;
        projection.world_y = actor.world_position.y;
        if (!project_entity_screen_position_standard(
                 projection, camera_x, camera_y)
                 .visible) {
            continue;
        }
        const auto left = static_cast<int>(projection.screen_x) -
                          kGenesisSpriteOrigin + sprite.origin_x;
        const auto top = static_cast<int>(projection.screen_y) -
                         kGenesisSpriteOrigin + sprite.origin_y;
        if (left < 272 && left + sprite.image->width() > 0 && top < 224 &&
            top + sprite.image->height() > 0) {
            ++visible;
        }
    }
    return visible;
}
#endif // SHADOWRUN_QT_QA_DRIVER

} // namespace shadowrun::ghidra_conversion::qt_runtime

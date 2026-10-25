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

bool FieldSceneRuntime::load_field_weapon_spawn_assets(
    const ConversionAssetCatalog& catalog) {
    for (std::size_t animation = 0;
         animation < field_weapon_projectile_images_.size(); ++animation) {
        auto& images = field_weapon_projectile_images_[animation];
        auto& origin_x = field_weapon_projectile_origin_x_[animation];
        auto& origin_y = field_weapon_projectile_origin_y_[animation];
        images.clear();
        origin_x.clear();
        origin_y.clear();

        const auto selector = animation * 4U;
        const auto expected_frames = animation < 8U ? 3U : 11U;
        for (std::size_t frame = 0; frame < expected_frames; ++frame) {
            const auto alias =
                std::string(kFieldWeaponProjectileAliasPrefix) +
                std::to_string(selector) + "." + std::to_string(frame);
            QImage image;
            int source_origin_x = 0;
            int source_origin_y = 0;
            if (!load_sprite_image(catalog, alias, image,
                                   source_origin_x, source_origin_y)) {
                return false;
            }
            images.push_back(std::move(image));
            origin_x.push_back(source_origin_x);
            origin_y.push_back(source_origin_y);
        }
    }
    return true;
}


bool FieldSceneRuntime::load_field_hit_spark_assets(
    const ConversionAssetCatalog& catalog) {
    for (std::size_t frame = 0U; frame < field_hit_spark_images_.size();
         ++frame) {
        QImage image;
        int origin_x = 0;
        int origin_y = 0;
        const auto alias = std::string(kFieldHitSparkAliasPrefix) +
                           std::to_string(frame);
        if (!load_sprite_image(catalog, alias, image, origin_x, origin_y)) {
            return false;
        }
        field_hit_spark_images_[frame] = std::move(image);
        field_hit_spark_origin_x_[frame] = origin_x;
        field_hit_spark_origin_y_[frame] = origin_y;
    }
    return true;
}


bool FieldSceneRuntime::load_field_hud_overlays(
    const ConversionAssetCatalog& catalog) {
    int origin_x = 0;
    int origin_y = 0;
    const auto load_slot = [this, &catalog, &origin_x, &origin_y](
                               const std::string_view prefix,
                               std::array<QImage, kFieldHudSlotCount>&
                                   images) {
        for (std::size_t slot = 0; slot < images.size(); ++slot) {
            const auto alias = std::string(prefix) + std::to_string(slot);
            if (!load_sprite_image(catalog, alias, images[slot], origin_x,
                                   origin_y)) {
                return false;
            }
        }
        return true;
    };
    if (!load_slot(kFieldHudFaceAliasPrefix, field_hud_faces_) ||
        !load_slot(kFieldHudItemAliasPrefix, field_hud_items_) ||
        !load_slot(kFieldHudUnarmedItemAliasPrefix,
                   field_hud_unarmed_items_)) {
        return false;
    }
    for (std::size_t family = 0; family < field_hud_party_faces_.size();
         ++family) {
        for (std::size_t palette = 0;
             palette < field_hud_party_faces_[family].size(); ++palette) {
            const auto alias = std::string(kFieldHudPartyFaceAliasPrefix) +
                               std::to_string(family) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_party_faces_[family][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t item = 0; item < field_hud_party_items_.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party_items_[item].size(); ++palette) {
            const auto alias = std::string(kFieldHudPartyItemAliasPrefix) +
                               std::to_string(item + 1U) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_party_items_[item][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t item = 0;
         item < field_hud_party_unarmed_items_.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party_unarmed_items_[item].size(); ++palette) {
            const auto alias =
                std::string(kFieldHudPartyUnarmedItemAliasPrefix) +
                std::to_string(item) + ".palette." +
                std::to_string(palette);
            if (!load_sprite_image(
                    catalog, alias,
                    field_hud_party_unarmed_items_[item][palette],
                    origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t spell = 0; spell < field_hud_magic_items_.size();
         ++spell) {
        for (std::size_t palette = 0;
             palette < field_hud_magic_items_[spell].size(); ++palette) {
            const auto alias = std::string(kFieldHudMagicItemAliasPrefix) +
                               std::to_string(spell) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_magic_items_[spell][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t meter = 0; meter < kFieldHudMeterLevelCount; ++meter) {
        const auto delay_alias =
            std::string(kFieldHudDelayMeterAliasPrefix) +
            std::to_string(meter);
        const auto primary_alias =
            std::string(kFieldHudBehaviorMeterPrimaryAliasPrefix) +
            std::to_string(meter);
        const auto companion_alias =
            std::string(kFieldHudBehaviorMeterCompanionAliasPrefix) +
            std::to_string(meter);
        if (!load_sprite_image(catalog, delay_alias,
                               field_hud_delay_meter_counters_[meter],
                               origin_x, origin_y) ||
            !load_sprite_image(
                catalog, primary_alias,
                field_hud_behavior_meter_counters_[meter], origin_x,
                origin_y) ||
            !load_sprite_image(
                catalog, companion_alias,
                field_hud_behavior_meter_counters_[
                    kFieldHudMeterLevelCount + meter],
                origin_x, origin_y)) {
            return false;
        }
    }
    for (std::size_t ammo = 0; ammo < field_hud_ammo_counters_.size();
         ++ammo) {
        const auto alias = std::string(kFieldHudAmmoCounterAliasPrefix) +
                           std::to_string(ammo);
        if (!load_sprite_image(catalog, alias, field_hud_ammo_counters_[ammo],
                               origin_x, origin_y)) {
            return false;
        }
    }
    return true;
}


void FieldSceneRuntime::resolve_field_party_wipe() {
    // ROM 0xC64C clears the scene/effect handoff state before dispatching
    // FFFBED. Keep these writes on the host's typed fields [C].
    field_mode_exit_transition_state_.effect_transition_marker = 0U;
    field_mode_exit_transition_state_.effect_animation_marker = 0U;
    field_mode_exit_transition_state_.mode_prompt_pending = 0U;
    field_runtime_state_.scene_runtime_state = 0U;
    world_frame_logic_state_.scene_state = 0U;
    field_pending_world_effect_ = false;
    world_activity_request_state_.clear_82 = 0U; // FFE182 [C]

    // 0x0A72 carries the typed FFFC1C bit-7 scene-reset state into Lua [C].
    const auto run_failure = game_scripts_.call_integers(
        "party_down", "prepare_run_failure_flat",
        {field_runtime_state_.world_activity_scene_reset_bit ? 1 : 0,
         field_world_effect_player_spawn_pending_ ? 1 : 0});
    if (!run_failure.has_value() || run_failure->size() != 6U) {
        failure_ = game_scripts_.failure();
        source_boundary("field.party_wipe.run_failure_script_failed");
        return;
    }
    if ((*run_failure)[2] != 0) {
        // Only type 5 reaches 0xC684's FFE183 clear. That mode-five marker is
        // not used by player-side gameplay [C].
        world_activity_request_state_.request_flag = 0U;
        field_mode_exit_transition_state_.mode_five_effect_marker = 0U;
        source_boundary("field.party_wipe.world_activity_request_cleared");
    }
    if ((*run_failure)[4] != 0) {
        field_world_effect_player_spawn_pending_ = false; // FFFC3C [C]
        source_boundary("field.party_wipe.player_spawn_request_cleared");
    }
    if ((*run_failure)[1] != 0) {
        // 0x1DED8 clears FFFC3C before its seven FF0400..FF0A00 world-actor
        // records. Apply the typed player/field write; leave that record sweep
        // to the combat branch [C].
        source_boundary("field.party_wipe.world_actor_sweep_boundary");
    }
    if ((*run_failure)[3] == 2) {
        source_boundary("field.party_wipe.group_item_flags_unresolved");
    }
    if ((*run_failure)[0] !=
        static_cast<std::int64_t>(
            std::numeric_limits<std::uint8_t>::max())) {
        // 0x1E68C clears FFFC1C after the Lua registry drops its active offer.
        field_runtime_state_.world_activity_scene_reset_bit = false;
        game_scripts_.set_location_context("field_map", "party_wipe",
                                           "field.world_event",
                                           "field_narrator");
        field_party_wipe_dialogue_stage_ =
            FieldPartyWipeDialogueStage::run_failure;
        if (!field_dialogue_.open_gameplay_task(
                "party_down", "present_run_failure")) {
            field_party_wipe_dialogue_stage_ =
                FieldPartyWipeDialogueStage::inactive;
            source_boundary("field.party_wipe.run_failure_text_open_failed");
            complete_field_party_wipe();
        }
        return;
    }
    complete_field_party_wipe();
}


void FieldSceneRuntime::complete_field_party_wipe() {
    // 0xC716 performs the common recovery after the optional run-failure page [C].
    auto& records = field_party_roster_.records();
    const auto previous_slot = field_party_roster_.active_slot();
    const auto status = [this, &records](const std::size_t index) {
        const auto flags =
            static_cast<std::int64_t>(records[index].flags >> 8U);
        return field_party_roster_.slot_active(index)
            ? flags : (flags & ~std::int64_t{0x01});
    };
    const auto story = game_scripts_.story_state();
    const auto bit0 = story.find("story_flag_bit_0");
    const bool docwagon = bit0 != story.end() && bit0->second;
    const auto wipe = game_scripts_.call_integers(
        "party_down", "resolve_wipe_flat",
        {static_cast<std::int64_t>(field_funds_.value_or(0U)),
         docwagon ? 1 : 0,
         status(0U), records[0].delay_meter, records[0].behavior_meter,
         status(1U), records[1].delay_meter, records[1].behavior_meter,
         status(2U), records[2].delay_meter, records[2].behavior_meter});
    if (!wipe.has_value() || wipe->size() != 9U) {
        failure_ = game_scripts_.failure();
        source_boundary("field.party_wipe.lua_failed");
        return;
    }
    // ROM 0xC716 clears FF024C and FF034C before 0xC802 dismisses the hired
    // records. Those +0x4C values are each typed delay meters [C].
    records[1U].delay_meter = static_cast<std::uint8_t>((*wipe)[7U]);
    records[2U].delay_meter = static_cast<std::uint8_t>((*wipe)[8U]);
    for (std::size_t index = 2U; index >= 1U; --index) {
        if ((*wipe)[2U + index] != 0 &&
            field_party_roster_.deactivate_party_member(index)) {
            source_boundary("field.party_wipe.hired_runner_dismissed");
        }
    }
    records[0].flags = static_cast<std::uint16_t>(
        (records[0].flags & 0x00ffU) |
        (static_cast<std::uint16_t>((*wipe)[0] & 0xff) << 8U));
    records[0].delay_meter = static_cast<std::uint8_t>((*wipe)[1]);
    records[0].behavior_meter = static_cast<std::uint8_t>((*wipe)[2]);
    if (field_funds_.has_value()) {
        field_funds_ = static_cast<std::uint32_t>((*wipe)[5]);
    }

    // 0x391A does a normal roster walk after 0xC716 revives the leader. Lua
    // receives typed slot/type/flag facts and returns the selected runner and
    // transient +0x10 bit-1 markers; the host applies those writes [C].
    const auto selection_flags = [this, &records](const std::size_t slot) {
        auto flags = static_cast<std::uint8_t>(records[slot].flags >> 8U);
        if (!field_party_roster_.slot_active(slot)) {
            flags = static_cast<std::uint8_t>(
                flags & ~static_cast<std::uint8_t>(0x01U));
        }
        return flags;
    };
    const auto selection = game_scripts_.call_integers(
        "party_down", "refresh_runner_after_wipe_flat",
        {static_cast<std::int64_t>(previous_slot),
         field_entity_selection_state_.suppression_override ? 1 : 0,
         field_entity_selection_state_.records[0].record_type,
         selection_flags(0U),
         field_entity_selection_state_.records[1].record_type,
         selection_flags(1U),
         field_entity_selection_state_.records[2].record_type,
         selection_flags(2U)});
    if (!selection.has_value() || selection->size() != 6U ||
        (*selection)[0] < 0 ||
        (*selection)[0] >= static_cast<std::int64_t>(records.size()) ||
        !field_party_roster_.slot_active(
            static_cast<std::size_t>((*selection)[0])) ||
        (*selection)[5] < 0 || (*selection)[5] > 0xFFFF) {
        failure_ = game_scripts_.failure();
        source_boundary("field.party_wipe.runner_refresh_script_failed");
        return;
    }
    if (source_audio_event_callback_) {
        // 0x391A begins by requesting event 0x25 through 0x530DE [C].
        source_audio_event_callback_(static_cast<std::uint16_t>((*selection)[5]));
    }
    if ((*selection)[4] != 0) {
        // 0x39E0 calls 0x1E540 when the selected record changes [C].
        initialize_effect_fade_marker(field_effect_fade_marker_);
        source_boundary("field.party_wipe.runner_change_fade_marker_reset");
    }
    const auto selected_slot = static_cast<std::size_t>((*selection)[0]);
    const auto selected_marker = static_cast<std::uint8_t>(
        srx::RunnerStatus::SELECTION_MARKER);
    for (std::size_t slot = 0; slot < records.size(); ++slot) {
        auto& record = records[slot];
        const auto returned_flags = static_cast<std::uint8_t>(
            (*selection)[slot + 1U] & 0xFF);
        auto record_flags = static_cast<std::uint8_t>(record.flags >> 8U);
        if (field_party_roster_.slot_active(slot)) {
            record_flags = returned_flags;
        } else {
            // Membership is a separate typed host fact. Preserve its live
            // record status while applying the source selection-marker clear.
            record_flags = static_cast<std::uint8_t>(
                (record_flags & ~selected_marker) |
                (returned_flags & selected_marker));
        }
        record.flags = static_cast<std::uint16_t>(
            (record.flags & 0x00FFU) |
            (static_cast<std::uint16_t>(record_flags) << 8U));
        if (slot < field_entity_selection_state_.records.size()) {
            field_entity_selection_state_.records[slot].flags =
                selection_flags(slot);
        }
    }
    field_party_roster_.set_active_slot(selected_slot);
    field_entity_selection_state_.active_index = selected_slot;
    field_entity_behavior_flags_ = static_cast<std::uint8_t>(
        active_runner_record().flags >> 8U);
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    // The source calls 0x33AA twice and 0x3548 for the three HUD panels [C].
    // This host redraws the typed player projection and uses named assets in
    // place of the source uploads [H].
    field_entity_.projection.sprite_offset_y =
        active_runner_record().projection_word_08;
    field_entity_animation_.redraw_requested = true;
    refresh_field_hud_runner_slot();
    source_boundary((*selection)[4] != 0
                        ? "field.party_wipe.active_runner_changed"
                        : "field.party_wipe.active_runner_retained");
    // The following source call, 0x1DC4C, scans and edits world-actor records;
    // it is deliberately left for the combat branch [C].
    source_boundary("field.party_wipe.world_actor_refresh_boundary");
    source_boundary("field.party_wipe.player_hud_and_projection_refreshed");
    // The source calls 0x38FA for the three player records [C]. The existing
    // host animation heartbeat owns that work [H].
    source_boundary((*wipe)[6] != 0 ? "field.party_wipe.docwagon"
                                    : "field.party_wipe.little_chiba");
    game_scripts_.set_location_context("field_map", "party_wipe",
                                       "field.world_event", "field_narrator");
    field_party_wipe_dialogue_stage_ = FieldPartyWipeDialogueStage::wake;
    if (!field_dialogue_.open_gameplay_task("party_down", "present_wake")) {
        field_party_wipe_dialogue_stage_ =
            FieldPartyWipeDialogueStage::inactive;
        source_boundary("field.party_wipe.wake_text_open_failed");
    }
}


void FieldSceneRuntime::start_field_party_wake_transition() {
    const auto destination = game_scripts_.call_integers(
        "party_down", "take_pending_wake_destination_flat", {});
    if (!destination.has_value() || destination->size() != 3U ||
        (*destination)[0] < 0 || (*destination)[0] > 0xFFFF ||
        (*destination)[1] < 0 || (*destination)[1] > 0xFFFF ||
        (*destination)[2] < 0 || (*destination)[2] > 0xFFFF) {
        failure_ = game_scripts_.failure();
        source_boundary("field.party_wipe.destination_script_failed");
        return;
    }
    const auto scene = static_cast<std::uint16_t>((*destination)[0]);
    // 0x2A32/0x2A44 clear these transition/effect markers after starting the
    // fade and before 0x136DE installs the selected FFFBF8 scene [C].
    field_runtime_state_.pending_field_scene_handler = false; // FFE12E
    field_runtime_state_.world_activity_transition_marker = 0U; // FFE18F
    field_world_effect_active_ = false; // FFE180
    field_pending_world_effect_ = false; // FFE182
    world_activity_request_state_ = WorldActivityRoutineRequestState{};
    field_mode_exit_transition_state_.effect_animation_marker = 0U; // FFE182
    field_mode_exit_transition_state_.mode_five_effect_marker = 0U; // FFE183
    world_frame_logic_state_.world_active = 0U; // FFDF84
    field_dialogue_scene_pending_destination_ = scene;
    field_dialogue_scene_pending_spawn_ = SceneRunnerPosition{
        static_cast<std::uint16_t>((*destination)[1]),
        static_cast<std::uint16_t>((*destination)[2])};
    if (field_elevator_scene_transition_.begin_fade_out()) {
        field_dialogue_scene_transition_active_ = true;
        source_boundary("field.party_wipe.destination_fade_out_started");
    } else {
        field_dialogue_scene_pending_destination_.reset();
        field_dialogue_scene_pending_spawn_.reset();
        source_boundary("field.party_wipe.destination_transition_busy");
    }
}


void FieldSceneRuntime::advance_field_movement() {
    if (!field_entity_.active) {
        return;
    }
    if (field_party_wipe_countdown_ != 0U) {
        // ROM 0xAF4 blocks the player service while FFE166 is nonzero;
        // 0x1DDC6 decrements it and enters 0xC64C at zero.
        if (--field_party_wipe_countdown_ == 0U) {
            resolve_field_party_wipe();
        }
        return;
    }
    if ((active_runner_record().flags & kRunnerFlagsDown) != 0U) {
        // ROM 0xB330 installs 0xB38A at +0x3A: no input or movement.
        return;
    }
    if (field_terrain_probe_trace_enabled_) {
        field_terrain_probe_trace_.clear();
    }

    const bool movement_transition_paused =
        field_opening_auto_walk_.active &&
        field_movement_transition_state_.interaction_transition_pending;
    const auto actor_decision = heartbeat_field_player(
        scripting::ScriptActorHeartbeatPhase::behavior);
    if (!actor_decision.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.player.lua_heartbeat_failed");
        return;
    }

    service_player_spell_cast(*actor_decision);
    service_player_melee(*actor_decision);
    service_player_weapon_action(
        actor_decision->combat_action_requested);

    if (!ready_) {
        // The source keeps the walking pose only while a direction is held.
        // When the controller goes idle, the heading animation returns to
        // the directional idle list instead of looping the walk cycle while
        // the runner stands still.
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (movement_transition_paused) {
        // Keep the startup marker alive through the fade, as the ROM does,
        // while the projected doorway event owns the field transition.
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (!field_opening_auto_walk_.active && field_controller_nibble_ == 0U) {
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (!actor_decision->movement_heading.has_value()) {
        return;
    }

    if (!actor_decision->movement_current_heading.has_value()) {
        failure_ = "Lua player heartbeat omitted its current heading";
        source_boundary("field.player.lua_heading_missing");
        return;
    }
    if (!actor_decision->movement_step.has_value()) {
        failure_ = "Lua player heartbeat omitted its collision-resolved step";
        source_boundary("field.player.lua_movement_step_missing");
        return;
    }

    field_target_heading_ = *actor_decision->movement_heading;
    const auto previous_x = field_movement_state_.position_x;
    const auto previous_y = field_movement_state_.position_y;
    const auto& movement_step = *actor_decision->movement_step;
    const auto& script_state = movement_step.state;
    field_movement_state_.position_x = script_state.position_x;
    field_movement_state_.position_y = script_state.position_y;
    field_movement_state_.heading = script_state.heading;
    field_movement_state_.animation_heading = script_state.animation_heading;
    field_movement_state_.collision_context = script_state.collision_context;
    field_movement_state_.movement_substate = script_state.movement_substate;
    field_movement_state_.route_status = script_state.route_status;
    field_movement_state_.movement_flags = script_state.movement_flags;
    field_movement_state_.collision_flags = script_state.collision_flags;
    field_movement_state_.event_flags = script_state.event_flags;
    field_movement_state_.collision_extent = script_state.collision_extent;
    field_movement_transition_state_.terrain_collision_code =
        script_state.terrain_collision_code;
    field_movement_behavior_record_.route_flags = script_state.route_flags;

    if (movement_step.direction_animation !=
        field_entity_animation_selector_) {
        field_entity_animation_.redraw_requested = true;
    }
    field_entity_animation_selector_ = movement_step.direction_animation;

    if (movement_step.outcome ==
        scripting::ScriptActorMovementStep::Outcome::reset) {
        field_player_animation_reset_pending_ = true;
        field_entity_animation_.redraw_requested = true;
        field_entity_attack_pose_active_ = false;
        field_entity_walking_ = false;
        source_boundary("field.movement.reset_blocked_step");
    }

    field_entity_.world_position.x = field_movement_state_.position_x;
    field_entity_.world_position.y = field_movement_state_.position_y;
    field_entity_.direction = actor_decision->direction;
    if (previous_x != field_movement_state_.position_x ||
        previous_y != field_movement_state_.position_y) {
        source_boundary("field.movement.accepted_step");
        field_entity_attack_pose_active_ = false;
        field_entity_walking_ = true;
    } else {
        field_entity_walking_ = false;
    }
}


void FieldSceneRuntime::advance_player_weapon_action_gate() {
    if (!ready_ || !field_entity_.active) {
        return;
    }

    auto& runner = active_runner_record();
    advance_active_entity_animation_action_gate(runner);
    if (runner.animation_action_gate >= 0 ||
        !field_entity_attack_pose_active_) {
        return;
    }

    field_entity_attack_pose_active_ = false;
    if (!field_entity_walking_) {
        field_entity_animation_selector_ = static_cast<std::uint16_t>(
            field_entity_attack_direction_ * 4U);
        field_entity_animation_.frame = 0U;
        field_entity_animation_.accumulator = 0;
        field_entity_animation_.redraw_requested = true;
    }
    source_boundary("field.combat.player_attack_pose_expired");
}


void FieldSceneRuntime::service_player_spell_cast(
    const scripting::ScriptActorBehaviorDecision& decision) {
    const auto dispatch = decision.source_action_dispatch.value_or("");
    // 0xAD94: no cast (blocked line, FFE186 == 3, no +0xA2) still runs 0xAF7C.
    const bool blocked = dispatch == "player_spell_blocked" ||
        dispatch == "player_spell_suppressed" ||
        dispatch == "player_spell_no_target";
    if (!decision.spell_action_resolution.has_value() && !blocked) {
        return;
    }
    auto& runner = active_runner_record();
    if (decision.spell_action_resolution.has_value()) {
        const auto& resolution = *decision.spell_action_resolution;
        const bool field_barrier = resolution.field_effect_kind ==
            scripting::ScriptActorSpellFieldEffectKind::field_barrier;
        // ROM 0x49A2 resets an existing barrier before attempting a new
        // shared effect record. Clear the host-owned record even if the new
        // 0x1582A allocation later finds the pool full [C].
        if (field_barrier) {
            clear_field_barrier_collision_overlay();
            for (std::size_t slot = 0U;
                 slot < field_spell_effect_slot_target_ids_.size(); ++slot) {
                if (field_spell_effect_slot_target_ids_[slot] ==
                    kFieldPlayerBarrierEffectOwner) {
                    field_world_effect_slots_occupied_[slot] = false;
                    field_spell_effect_slot_target_ids_[slot].clear();
                }
            }
        }
        if (resolution.effect_started &&
            resolution.spell_effect_slot.has_value()) {
            const auto slot = *resolution.spell_effect_slot;
            const auto owner_id = field_barrier
                ? kFieldPlayerBarrierEffectOwner
                : std::string_view{resolution.target_id};
            if (slot >= field_world_effect_slots_occupied_.size() ||
                field_world_effect_slots_occupied_[slot] ||
                owner_id.empty() ||
                (resolution.untargeted && !field_barrier) ||
                !field_spell_effect_slot_target_ids_[slot].empty()) {
                failure_ = "Lua selected an occupied spell effect record";
                source_boundary(
                    "field.combat.player_spell.effect_slot_invalid");
                return;
            }
            field_world_effect_slots_occupied_[slot] = true;
            field_spell_effect_slot_target_ids_[slot] =
                std::string(owner_id);
            source_boundary("field.combat.player_spell.effect_slot_reserved");

            if (field_barrier) {
                // ROM 0x49FA reads caster +0x2E, 0x4A14/0x4A24 read its
                // world Y/X, and 0x4A74 selects the cell byte from +0x57 [C].
                // These are the typed field position, movement heading, and
                // active runner's equipped action code.
                const auto placement = game_scripts_.call_integers(
                    "field_barrier", "place_flat",
                    {field_entity_.world_position.x,
                     field_entity_.world_position.y,
                     field_movement_state_.heading,
                     runner.equipped_item_code});
                const auto expected_code = runner.equipped_item_code <=
                        static_cast<std::uint8_t>(srx::v(
                            srx::FieldBarrier::SUPER_MAX_ACTION_CODE))
                    ? srx::v(srx::FieldBarrier::CELL_CODE_SUPER_BARRIER)
                    : srx::v(srx::FieldBarrier::CELL_CODE_BARRIER);
                const auto placement_size =
                    kFieldBarrierCollisionCellCount + 4U;
                const bool placement_valid = placement.has_value() &&
                    placement->size() == placement_size &&
                    placement->at(0U) >= 0 && placement->at(0U) <= 7 &&
                    placement->at(1U) >= 0 && placement->at(1U) <= 0xFFFF &&
                    placement->at(2U) >= 0 && placement->at(2U) <= 0xFFFF &&
                    placement->back() == expected_code;
                bool cells_valid = placement_valid;
                std::array<FieldBarrierCollisionCell,
                           kFieldBarrierCollisionCellCount> cells{};
                for (std::size_t index = 0U;
                     cells_valid && index < cells.size(); ++index) {
                    const auto value = placement->at(index + 3U);
                    cells_valid = value >= 0 && value <= 0xFFFF;
                    if (cells_valid) {
                        cells[index] = FieldBarrierCollisionCell{
                            static_cast<std::uint16_t>(value),
                            static_cast<std::uint8_t>(placement->back())};
                    }
                }
                if (!cells_valid) {
                    failure_ = game_scripts_.failure().empty()
                        ? "Lua returned an invalid field barrier placement"
                        : game_scripts_.failure();
                    source_boundary(
                        "field.combat.player_spell.barrier_placement_unresolved");
                } else {
                    set_field_barrier_collision_overlay(cells);
                    source_boundary(
                        "field.combat.player_spell.barrier_collision_started");
                }
            }
        }
        if (resolution.roll_attempted) {
            // 0x103B2 publishes the caster's threshold result at +0x9F [C].
            runner.action_result = static_cast<std::int8_t>(
                resolution.action_result);
        }
        // 0x1033E writes the caster's +0x9C meter and 0x10660 fetish cells.
        runner.behavior_meter = resolution.mental_meter;
        runner.inventory_item_codes = resolution.item_codes;
        runner.inventory_phase_bytes = resolution.item_phases;
        if (decision.audio_event.has_value() && source_audio_event_callback_) {
            source_audio_event_callback_(*decision.audio_event);
        }
    }
    // 0xAFAE (cast) and 0xAF7C (blocked) both call 0xABFC with mode 0.
    EntityDelayState delay{runner.animation_action_gate};
    update_entity_animation_delay(
        delay,
        EntityDelayAttributes{runner.attributes[1U], runner.attributes[4U],
                              runner.cyberware_flags},
        decode_entity_delay_scale(0U));
    runner.animation_action_gate = delay.animation_delay;
    // 0xAF7C/0xAFAE: +0x16 = C082[dir(+0x2E)], then 0xAF6C clears the clock.
    const auto cast_direction = field_movement_direction_from_heading(
        field_movement_state_.heading);
    field_entity_animation_selector_ =
        static_cast<std::uint16_t>(0x0080U + cast_direction * 4U);
    field_entity_attack_direction_ = cast_direction;
    field_entity_.direction = cast_direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    field_entity_attack_pose_kind_ = kFieldPlayerPoseCast;
    field_entity_animation_.accumulator = 0;
    field_entity_animation_.frame = 0U;
    field_entity_animation_.redraw_requested = true;
    refresh_field_hud_runner_slot();
    if (blocked) {
        source_boundary(dispatch == "player_spell_blocked"
            ? "field.combat.player_spell.blocked"
            : dispatch == "player_spell_suppressed"
            ? "field.combat.player_spell.scene_suppressed"
            : "field.combat.player_spell.no_target");
    } else {
        source_boundary(decision.spell_action_resolution->effect_requested
            ? "field.combat.player_spell.effect_requested"
            : "field.combat.player_spell.action_resolved");
    }
}


void FieldSceneRuntime::service_player_melee(
    const scripting::ScriptActorBehaviorDecision& decision) {
    if (!decision.source_action_dispatch.has_value() ||
        (*decision.source_action_dispatch != "player_melee" &&
         *decision.source_action_dispatch != "player_melee_hit")) {
        return;
    }
    auto& runner = active_runner_record();
    // ROM 0xB1DC: 0xABFC mode 1 first, then the 0x4C/0x55 swing request.
    EntityDelayState delay{runner.animation_action_gate};
    update_entity_animation_delay(
        delay,
        EntityDelayAttributes{runner.attributes[1U], runner.attributes[4U],
                              runner.cyberware_flags},
        decode_entity_delay_scale(1U));
    runner.animation_action_gate = delay.animation_delay;
    // ROM 0xB212..0xB23A: a linked strike turns +0x34/+0x2E to the Lua
    // bearing; +0x16 = C0A2[dir(+0x2E)], then 0xAF6C clears the frame clock.
    if (decision.strike_heading.has_value()) {
        field_movement_state_.heading = *decision.strike_heading;
        field_target_heading_ = *decision.strike_heading;
    }
    const auto strike_direction = field_movement_direction_from_heading(
        field_movement_state_.heading);
    field_entity_animation_selector_ =
        static_cast<std::uint16_t>(0x00C0U + strike_direction * 4U);
    field_entity_attack_direction_ = strike_direction;
    field_entity_.direction = strike_direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    field_entity_attack_pose_kind_ = kFieldPlayerPoseStrike;
    field_entity_animation_.accumulator = 0;
    field_entity_animation_.frame = 0U;
    field_entity_animation_.redraw_requested = true;
    if (source_audio_event_callback_) {
        if (decision.audio_event.has_value()) {
            source_audio_event_callback_(*decision.audio_event);
        }
        // 0xB25E: in-range strikes add the 0x4D/0x4E target cue.
        if (decision.target_heading_audio_event.has_value()) {
            source_audio_event_callback_(*decision.target_heading_audio_event);
        }
    }
    if (decision.combat_meter_update.has_value() &&
        !apply_script_combat_meter_update(*decision.combat_meter_update,
                                          "field.combat.player_melee")) {
        return;
    }
    if (decision.hit_spark_spawn_request.has_value()) {
        spawn_field_hit_spark(*decision.hit_spark_spawn_request);
    }
    refresh_field_hud_runner_slot();
    source_boundary(*decision.source_action_dispatch == "player_melee_hit"
        ? "field.combat.player_melee.hit"
        : "field.combat.player_melee.swing");
}


void FieldSceneRuntime::spawn_field_hit_spark(
    const scripting::ScriptActorHitSparkSpawnRequest& request) {
    // ROM 0x15A8C scans the same six FF0B00 child slots used by weapon
    // records. Mirror those occupancy facts into the typed source helper;
    // 0xB2B2..0xB2EA then install kind 1, selector 4, palette 1 and AC4E [C].
    std::array<AuxiliaryEntitySlot, 6> source_slots{};
    for (std::size_t index = 0U; index < source_slots.size(); ++index) {
        source_slots[index].active = field_weapon_slots_occupied_[index];
    }
    auto child = spawn_reaction_auxiliary_child(
        source_slots, request.world_x, request.world_y);
    if (!child.has_value()) {
        source_boundary("field.combat.player_hit_spark.pool_full");
        return;
    }
    if (child->slot < 9U || child->slot > 14U) {
        source_boundary("field.combat.player_hit_spark.slot_invalid");
        return;
    }
    const auto slot_index = static_cast<std::size_t>(child->slot - 9U);
    field_weapon_slots_occupied_[slot_index] = true;
    child->entity_kind = kFieldHitSparkKind;
    child->animation_selector =
        static_cast<std::int16_t>(kFieldHitSparkSelector);
    child->palette_attributes = kFieldHitSparkPalette;
    child->heading = kFieldHitSparkPalette;
    const auto expected_y = static_cast<std::uint16_t>(
        request.world_y + kFieldHitSparkTargetYOffset);
    if (child->world_y != expected_y) {
        field_weapon_slots_occupied_[slot_index] = false;
        source_boundary("field.combat.player_hit_spark.position_invalid");
        return;
    }

    FieldHitSparkPresentation spark{};
    spark.child = *child;
    spark.actor_instance_id =
        "field_hit_spark_slot_" + std::to_string(child->slot);
    spark.animation.increment = kFieldHitSparkFrameIncrement;
    spark.animation.period = kFieldHitSparkFramePeriod;
    field_hit_sparks_.push_back(std::move(spark));
    source_boundary("field.combat.player_hit_spark.spawned");
}


bool FieldSceneRuntime::probe_action_line_clear(
    const SpawnEntityParentState& parent,
    const std::optional<std::size_t> protected_source_slot,
    const std::uint16_t target_x,
    const std::uint16_t target_y) {
    // ROM 0x14D8E line probe, reached from the NPC 0x13FFE spell/item branch
    // and the field player's 0xAE6E cast. Lua owns the target policy.
    EntityAnimationSpawnProbeState probe_state{};
    std::optional<SpawnEntityLinkedState> linked_target{
        SpawnEntityLinkedState{target_x, target_y, 0x01U, false}};
    EntityAnimationSpawnProbeRequest request{};
    request.parent = parent;
    request.linked_target = linked_target;
    request.protected_source_candidate = protected_source_slot;

    EntityCollisionRecord probe_collision_record{};
    std::array<std::uint8_t, 4> probe_sample_state{};
    std::vector<CollisionCandidate> collision_candidates(10U);
    for (std::size_t slot = 0U;
         slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        auto position = state_.field_scene_buffers
                            .source_metadata
                            .initial_field_records[slot];
        if (slot == field_party_roster_.active_slot() &&
            field_entity_.active) {
            position = field_entity_.world_position;
        }
        const auto& runner = field_party_roster_.records()[slot];
        auto& candidate = collision_candidates[slot];
        candidate.world_x = position.x;
        candidate.world_y = position.y;
        candidate.flags = static_cast<std::uint8_t>(
            runner.flags >> 8U);
        candidate.interaction_flags =
            static_cast<std::uint8_t>(runner.flags & 0x00FFU);
        candidate.collision_enabled = runner.delay_meter != 0U;
    }
    for (const auto& target : field_default_actors_) {
        if (!target.active ||
            target.scene_reference != state_.field_scene_buffers.scene_reference ||
            !target.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::size_t>(
            *target.source_record_slot);
        if (slot >= collision_candidates.size()) {
            continue;
        }
        auto& candidate = collision_candidates[slot];
        candidate.world_x = target.world_position.x;
        candidate.world_y = target.world_position.y;
        candidate.flags = target.source_record_flags;
        candidate.interaction_flags =
            target.source_interaction_flags;
        candidate.behavior_flags = target.source_behavior_flags;
        candidate.collision_enabled =
            target.physical_meter != 0U;
    }

    const auto probe_result = probe_entity_spawn_collision_window(
        probe_state,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [this, &probe_state,
             &probe_collision_record, &probe_sample_state](
                const std::int32_t fixed_x,
                const std::int32_t fixed_y) {
                // 0x14F26 supplies 16.16 values. The field
                // collision service takes the source 16.4 word;
                // retaining the high word preserves world units.
                const auto collision_coordinate = [](
                    const std::int32_t value) {
                    return static_cast<std::uint16_t>(
                        static_cast<std::uint32_t>(value) >> 16U);
                };
                bool terrain_boundary_active = false;
                bool outside_bounds = false;
                const auto collision_code =
                    evaluate_field_collision(
                        collision_coordinate(fixed_x),
                        collision_coordinate(fixed_y),
                        probe_collision_record,
                        probe_sample_state,
                        probe_state.scratch_record
                            .behavior_defaults.flags,
                        false,
                        nullptr,
                        &terrain_boundary_active,
                        &outside_bounds);
                return EntityTerrainProbeSample{
                    static_cast<std::uint16_t>(collision_code),
                    terrain_boundary_active, outside_bounds};
            },
            [&collision_candidates](
                const EntityCollisionSearchContext& collision) {
                return find_colliding_entity(
                    collision, collision_candidates);
            }});
    return !probe_result.boundary_unresolved &&
        !probe_result.source_carry;
}


bool FieldSceneRuntime::apply_script_combat_meter_update(
    const scripting::ScriptCombatMeterUpdate& update,
    const std::string_view boundary_prefix) {
    const auto report = [this, boundary_prefix](const std::string_view suffix) {
        std::string boundary(boundary_prefix);
        boundary.push_back('.');
        boundary.append(suffix);
        source_boundary(boundary);
    };
    if (!update.changed) {
        return true;
    }
    if (update.record_type < FieldPartyRosterRuntime::kSlotCount) {
        const auto slot = static_cast<std::size_t>(
            update.source_record_slot);
        if (slot >= FieldPartyRosterRuntime::kSlotCount ||
            !field_party_roster_.slot_active(slot)) {
            report("party_target_invalid");
            return false;
        }
        auto& target = field_party_roster_.records()[slot];
        if (target.record_type != update.record_type) {
            report("party_target_type_mismatch");
            return false;
        }
        target.delay_meter = update.physical_meter;
        target.behavior_meter = update.mental_meter;
        if (update.source_record_flags_updated) {
            target.flags = static_cast<std::uint16_t>(
                (target.flags & 0x00ffU) |
                (static_cast<std::uint16_t>(
                     update.source_record_flags) << 8U));
        }
        if (update.physical && target.delay_meter == 0U &&
            (target.flags & kRunnerFlagsDown) == 0U) {
            // ROM 0xAFF2 -> 0xB330: a zero +0x4C downs the runner. Lua owns
            // the flag, sound, C0B2 selector and the 0xABB6 wipe check.
            const auto& records = field_party_roster_.records();
            const auto status = [this, &records](const std::size_t index) {
                const auto flags =
                    static_cast<std::int64_t>(records[index].flags >> 8U);
                return field_party_roster_.slot_active(index)
                    ? flags : (flags & ~std::int64_t{0x01});
            };
            const auto heading = slot == field_party_roster_.active_slot()
                ? field_movement_state_.heading
                : std::uint16_t{0U};
            const auto down = game_scripts_.call_integers(
                "party_down", "runner_down_flat",
                {static_cast<std::int64_t>(slot), status(0U), status(1U),
                 status(2U), static_cast<std::int64_t>(target.kind),
                 static_cast<std::int64_t>(heading)});
            if (!down.has_value() || down->size() != 4U) {
                failure_ = game_scripts_.failure();
                report("party_runner_down_failed");
                return false;
            }
            target.flags = static_cast<std::uint16_t>(
                (target.flags & 0x00ffU) |
                (static_cast<std::uint16_t>((*down)[0] & 0xff) << 8U));
            if ((*down)[1] != 0 && source_audio_event_callback_) {
                source_audio_event_callback_(
                    static_cast<std::uint16_t>((*down)[1]));
            }
            if ((*down)[3] != 0) {
                field_party_wipe_countdown_ =
                    static_cast<std::uint8_t>((*down)[3]);
                report("party_wipe_countdown_started");
            }
            report("party_runner_down");
        }
        if (slot == field_party_roster_.active_slot()) {
            refresh_field_hud_runner_slot();
        }
        report(update.physical ? "party_physical_meter_changed"
                               : "party_mental_meter_changed");
        return true;
    }

    if (update.source_record_slot <
            static_cast<std::uint8_t>(FieldWorldActorSlot::slot_3) ||
        update.source_record_slot >
            static_cast<std::uint8_t>(FieldWorldActorSlot::slot_9)) {
        report("world_target_slot_invalid");
        return false;
    }
    const auto target = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [this, &update](const auto& actor) {
            return actor.active && actor.scene_reference ==
                       state_.field_scene_buffers.scene_reference &&
                   actor.source_record_slot.has_value() &&
                   static_cast<std::uint8_t>(*actor.source_record_slot) ==
                       update.source_record_slot &&
                   actor.actor_instance_id == update.target_id &&
                   actor.combatant.record_type == update.record_type;
        });
    if (target == field_default_actors_.end()) {
        report("world_target_missing");
        return false;
    }

    const bool physically_defeated = update.physical &&
        target->combatant.physical_meter != 0U &&
        update.physical_meter == 0U;
    const scripting::ScriptActorDefeatFacts defeat_facts{
        target->combatant.race_or_state};

    target->combatant.physical_meter = update.physical_meter;
    target->combatant.mental_meter = update.mental_meter;
    if (update.source_record_flags_updated) {
        target->source_record_flags = update.source_record_flags;
        target->combatant.source_flags = update.source_record_flags;
    }
    // This meter gates the live actor collision record. Keep the update on
    // that record as well as in the Lua-owned combat facts so later movement,
    // collision, and drawing passes see one current actor state.
    target->physical_meter = update.physical_meter;
    if (physically_defeated) {
        const auto decision =
            game_scripts_.resolve_active_run_actor_defeat(defeat_facts);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            report("run_actor_defeat_failed");
            return false;
        }
        if (decision->action ==
            scripting::ScriptRunActorDefeatAction::objective_advanced) {
            report("run_actor_objective_advanced");
        }
    }
    report(update.physical ? "world_physical_meter_changed"
                           : "world_mental_meter_changed");
    return true;
}


void FieldSceneRuntime::advance_field_actor_animation() {
    if (field_entity_.active) {
        const auto previous_frame = field_entity_animation_.frame;
        const auto previous_animation = field_entity_animation_id_;
        const auto decision = heartbeat_field_player(
            scripting::ScriptActorHeartbeatPhase::render);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.player.lua_animation_failed");
        } else {
            field_entity_.direction = decision->direction;
            field_entity_animation_.frame = decision->animation_frame;
            field_entity_animation_.accumulator =
                decision->animation_accumulator;
            field_entity_animation_id_ = decision->animation_id;
            field_entity_.spell_effect_visible =
                decision->spell_effect_visible;
            field_entity_.spell_effect_action_code =
                decision->spell_effect_action_code;
            field_entity_.spell_effect_phase = decision->spell_effect_phase;
            if (decision->spell_effect_resolution.has_value()) {
                if (decision->combat_meter_update.has_value() &&
                    !apply_script_combat_meter_update(
                        *decision->combat_meter_update,
                        "field.player.spell.effect")) {
                    failure_ =
                        "Lua spell effect targeted an unavailable runner record";
                    source_boundary(
                        "field.player.spell.effect_apply_failed");
                } else {
                    source_boundary("field.player.spell.effect_resolved");
                }
            }
            if (decision->spell_effect_slot_released.has_value()) {
                const auto slot = *decision->spell_effect_slot_released;
                const auto active_party_target_id =
                    "party.runner." + std::to_string(
                        field_party_roster_.active_slot());
                if (slot >= field_world_effect_slots_occupied_.size() ||
                    !field_world_effect_slots_occupied_[slot] ||
                    (field_spell_effect_slot_target_ids_[slot] !=
                         "field_player_runner" &&
                     field_spell_effect_slot_target_ids_[slot] !=
                         active_party_target_id &&
                     field_spell_effect_slot_target_ids_[slot] !=
                         kFieldPlayerBarrierEffectOwner)) {
                    failure_ = "Lua released an unowned runner spell effect record";
                    source_boundary(
                        "field.player.spell.effect_slot_release_invalid");
                } else {
                    if (field_spell_effect_slot_target_ids_[slot] ==
                        kFieldPlayerBarrierEffectOwner) {
                        // ROM 0x4ADC expires the record through 0x4B6C [C].
                        clear_field_barrier_collision_overlay();
                    }
                    field_world_effect_slots_occupied_[slot] = false;
                    field_spell_effect_slot_target_ids_[slot].clear();
                    source_boundary(
                        "field.player.spell.effect_slot_released");
                }
            }
            const auto selector_changed =
                decision->animation_selector.has_value() &&
                *decision->animation_selector !=
                    field_entity_animation_selector_;
            if (decision->animation_selector.has_value()) {
                field_entity_animation_selector_ =
                    *decision->animation_selector;
            }
            field_entity_animation_.redraw_requested =
                selector_changed ||
                previous_frame != field_entity_animation_.frame ||
                previous_animation != field_entity_animation_id_;
        }
    }
}


void FieldSceneRuntime::advance_field_hit_spark_behaviors() {
    const auto release_slot = [this](FieldHitSparkPresentation& spark) {
        if (spark.child.slot >= 9U && spark.child.slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(spark.child.slot - 9U)] = false;
        }
        spark.child.active = false;
    };
    for (auto& spark : field_hit_sparks_) {
        if (!spark.child.active) {
            continue;
        }
        scripting::ScriptActorBehaviorContext context{};
        context.profile_id = "field_hit_spark";
        context.entity_kind = spark.child.entity_kind;
        context.action_kind = scripting::ScriptActorActionKind::none;
        context.action_code = 0U;
        context.instance_id = spark.actor_instance_id;
        context.initial_heading = spark.child.heading;
        context.spawn = !spark.script_initialized;
        context.phase = scripting::ScriptActorHeartbeatPhase::behavior;
        context.hit_spark_animation = scripting::ScriptActorHitSparkAnimation{
            static_cast<std::uint16_t>(spark.child.animation_selector),
            static_cast<std::uint8_t>(kFieldHitSparkFrameCount),
            static_cast<std::uint8_t>(spark.animation.frame),
            spark.animation.accumulator,
            spark.animation.increment,
            spark.animation.period};
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value() ||
            decision->animation_id != "hit_spark" ||
            !decision->animation_selector.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary(
                "field.combat.player_hit_spark.lua_behavior_failed");
            release_slot(spark);
            continue;
        }
        spark.script_initialized = true;
        spark.child.animation_selector = static_cast<std::int16_t>(
            *decision->animation_selector);
        if (!decision->hit_spark_retired) {
            continue;
        }

        AuxiliaryEntitySlot slot{};
        slot.active = true;
        if (retire_auxiliary_child(slot, spark.child)) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(spark.child.slot - 9U)] = false;
            source_boundary("field.combat.player_hit_spark.retired");
        } else {
            source_boundary(
                "field.combat.player_hit_spark.retirement_selector_invalid");
        }
    }
}


void FieldSceneRuntime::advance_field_hit_spark_animations() {
    const auto release_slot = [this](FieldHitSparkPresentation& spark) {
        if (spark.child.slot >= 9U && spark.child.slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(spark.child.slot - 9U)] = false;
        }
        spark.child.active = false;
    };
    for (auto& spark : field_hit_sparks_) {
        if (!spark.child.active) {
            continue;
        }
        scripting::ScriptActorBehaviorContext context{};
        context.profile_id = "field_hit_spark";
        context.entity_kind = spark.child.entity_kind;
        context.action_kind = scripting::ScriptActorActionKind::none;
        context.action_code = 0U;
        context.instance_id = spark.actor_instance_id;
        context.initial_heading = spark.child.heading;
        context.spawn = !spark.script_initialized;
        context.phase = scripting::ScriptActorHeartbeatPhase::render;
        context.hit_spark_animation = scripting::ScriptActorHitSparkAnimation{
            static_cast<std::uint16_t>(spark.child.animation_selector),
            static_cast<std::uint8_t>(kFieldHitSparkFrameCount),
            static_cast<std::uint8_t>(spark.animation.frame),
            spark.animation.accumulator,
            spark.animation.increment,
            spark.animation.period};
        const auto previous_frame = spark.animation.frame;
        const auto previous_selector = spark.child.animation_selector;
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value() ||
            decision->animation_id != "hit_spark" ||
            !decision->animation_selector.has_value() ||
            !decision->animation_increment.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary(
                "field.combat.player_hit_spark.lua_animation_failed");
            release_slot(spark);
            continue;
        }
        spark.script_initialized = true;
        spark.child.animation_selector = static_cast<std::int16_t>(
            *decision->animation_selector);
        spark.animation.frame = decision->animation_frame;
        spark.animation.accumulator = decision->animation_accumulator;
        spark.animation.increment = *decision->animation_increment;
        spark.animation.redraw_requested =
            previous_frame != spark.animation.frame ||
            previous_selector != spark.child.animation_selector;
    }
}


void FieldSceneRuntime::advance_field_weapon_projectile_animations() {
    const auto release_slot = [this](
                                   std::optional<std::uint16_t>& slot) {
        if (slot.has_value() && *slot >= 9U && *slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(*slot - 9U)] = false;
        }
        slot.reset();
    };
    const auto advance_record = [this, &release_slot](
                                    SpawnEntityRecordState& record,
                                    EntityAnimationState& animation,
                                    bool& active,
                                    bool& script_initialized,
                                    std::optional<std::uint16_t>& slot) {
        if (!active) {
            return;
        }
        const auto disable_record = [&] {
            record.flags = static_cast<std::uint8_t>(
                record.flags & static_cast<std::uint8_t>(~0x01U));
            active = false;
            release_slot(slot);
        };
        if (!slot.has_value()) {
            source_boundary("field.combat.weapon_projectile_slot_missing");
            disable_record();
            return;
        }

        const auto selector = static_cast<std::uint16_t>(
            record.animation_resource & 0x7FFFU);
        const auto animation_index =
            static_cast<std::size_t>(selector / 4U);
        if ((selector & 0x0003U) != 0U ||
            animation_index >= field_weapon_projectile_images_.size()) {
            source_boundary(
                "field.combat.weapon_projectile_selector_invalid");
            disable_record();
            return;
        }
        const auto frame_count =
            field_weapon_projectile_images_[animation_index].size();
        if (frame_count == 0U || frame_count > 16U) {
            source_boundary("field.combat.weapon_projectile_frames_invalid");
            disable_record();
            return;
        }

        const auto instance_id = "field_weapon_slot_" +
                                 std::to_string(*slot);
        scripting::ScriptActorBehaviorContext context{};
        context.profile_id = "weapon_projectile";
        context.entity_kind = record.behavior_defaults.animation_state;
        context.action_kind = scripting::ScriptActorActionKind::none;
        context.action_code = 0U;
        context.instance_id = instance_id;
        context.initial_heading = record.heading;
        context.spawn = !script_initialized;
        context.phase = scripting::ScriptActorHeartbeatPhase::render;
        context.projectile_animation =
            scripting::ScriptActorProjectileAnimation{
                record.animation_resource,
                static_cast<std::uint16_t>(frame_count),
                animation.frame,
                animation.accumulator,
                animation.increment,
                animation.period,
                true};
        const auto previous_frame = animation.frame;
        const auto previous_selector = record.animation_resource;
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value() ||
            !decision->animation_selector.has_value() ||
            !decision->animation_increment.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.combat.weapon_projectile_lua_animation_failed");
            disable_record();
            return;
        }

        script_initialized = true;
        record.animation_resource = *decision->animation_selector;
        animation.frame = decision->animation_frame;
        animation.accumulator = decision->animation_accumulator;
        animation.increment = *decision->animation_increment;
        animation.redraw_requested =
            previous_frame != animation.frame ||
            previous_selector != record.animation_resource;
    };

    for (auto& spawn : field_weapon_projectiles_) {
        advance_record(spawn.primary_record, spawn.primary_animation,
                       spawn.primary_active,
                       spawn.primary_animation_script_initialized,
                       spawn.primary_slot);
        advance_record(spawn.source_record, spawn.animation,
                       spawn.secondary_active,
                       spawn.secondary_animation_script_initialized,
                       spawn.secondary_slot);
    }
}


const MagicSpellTable* FieldSceneRuntime::field_magic_spell_table(
    const std::uint16_t table_index) const noexcept {
    if (table_index >= field_magic_spell_table_overrides_.size()) {
        return nullptr;
    }
    const auto& override_table =
        field_magic_spell_table_overrides_[table_index];
    return override_table.has_value()
               ? &*override_table
               : find_us_magic_spell_table(table_index);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

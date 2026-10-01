#include "field_scene_runtime.hpp"

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "genesis_image_compositor.hpp"

#include <QPainter>
#include <QColor>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::string_view kInitialFieldMapAlias = "field.maps.mode.0";
constexpr std::string_view kFieldHudAlias = "field.hud.background";
constexpr std::string_view kFieldHudFaceAliasPrefix = "field.hud.face.";
constexpr std::string_view kFieldHudItemAliasPrefix = "field.hud.item.";
constexpr std::string_view kFieldHudUnarmedItemAliasPrefix =
    "field.hud.unarmed.item.";
constexpr std::string_view kFieldHudVerticalCounterAliasPrefix =
    "field.hud.vertical.counter.";
constexpr std::string_view kFieldHudHorizontalCounterAliasPrefix =
    "field.hud.horizontal.counter.";
constexpr std::string_view kFieldHudAmmoCounterAliasPrefix =
    "field.hud.ammo.counter.";
constexpr std::string_view kFieldEntityIdleAliasPrefix = "field.entities.idle.";
constexpr std::string_view kFieldEntityWalkAliasPrefix = "field.entities.walk.";
constexpr int kGenesisSpriteOrigin = 0x80;
constexpr std::string_view kCommandOverviewBackgroundAlias =
    "menu.field.background";
constexpr std::string_view kCommandOverviewInventoryBoxesAlias =
    "menu.field.inventory.boxes";
constexpr std::string_view kCommandOverviewCursorAlias = "menu.field.cursor";
constexpr std::string_view kCommandOverviewPortraitAliasPrefix =
    "menu.field.runner.portrait.";
constexpr std::string_view kCommandOverviewFocusAliasPrefix =
    "menu.field.command.focus.";
constexpr std::string_view kCommandOverviewStatusAliasPrefix =
    "menu.field.status.bar.";
constexpr std::string_view kCommandOverviewSelectedCellAliasPrefix =
    "menu.field.selected.cell.";
constexpr std::string_view kCommandOverviewDamageBarAliasPrefix =
    "menu.field.damage.bar.";
constexpr std::string_view kCommandOverviewDrainBarAliasPrefix =
    "menu.field.drain.bar.";
constexpr std::string_view kCommandOverviewDetailValueAliasPrefix =
    "menu.field.detail.value.";
constexpr std::string_view kCommandOverviewAttackBarAliasPrefix =
    "menu.field.attack.bar.";
constexpr std::string_view kCommandOverviewDefenseBarAliasPrefix =
    "menu.field.defense.bar.";
// FFDE08 is profile word 33. The 48-word block starts at FFDDC6.
constexpr std::size_t kSceneProfileEntityPositionWord = 33U;

std::string command_overview_item_alias(const std::uint8_t item_code,
                                        const std::uint8_t palette_variant) {
    if (item_code == 0U || item_code > 50U) {
        return {};
    }
    std::string category;
    if (item_code <= 15U) {
        category = "weapons";
    } else if (item_code <= 18U) {
        category = "first.aid";
    } else if (item_code <= 22U) {
        category = "miscellaneous";
    } else if (item_code <= 43U) {
        category = "magic";
    } else {
        category = "armor";
    }
    const auto item_number = item_code < 10U
                                 ? "0" + std::to_string(item_code)
                                 : std::to_string(item_code);
    return "items." + category + ".icons.item." + item_number + "." +
           std::to_string(palette_variant);
}

std::string command_overview_item_detail_alias(
    const std::uint8_t item_code) {
    auto alias = command_overview_item_alias(item_code, 0U);
    const auto marker = alias.find(".icons.item.");
    if (marker == std::string::npos) {
        return {};
    }
    alias.replace(marker, std::string(".icons.item.").size(), ".detail.item.");
    // The detail catalog has one source image per item.  Only the inventory
    // icon table is palette-variant; do not carry the icon builder's `.0`
    // suffix into the direct detail alias.
    const auto palette_separator = alias.rfind('.');
    if (palette_separator != std::string::npos) {
        alias.erase(palette_separator);
    }
    return alias;
}

bool command_overview_uses_alternate_icon_palette(
    const RunnerRecordFields& record) {
    return use_command_overview_alternate_icon_palette(
        HudGlyphAttributeInput{
            static_cast<std::int8_t>(record.equipped_item_code),
            static_cast<std::int8_t>(record.archetype_index),
            record.equipped_inventory_slot});
}

std::array<std::uint8_t, 4> field_sample_state_from_result(
    const std::uint32_t packed_state) {
    return {
        static_cast<std::uint8_t>(packed_state >> 24U),
        static_cast<std::uint8_t>(packed_state >> 16U),
        static_cast<std::uint8_t>(packed_state >> 8U),
        static_cast<std::uint8_t>(packed_state)};
}

} // namespace

FieldSceneRuntime::FieldSceneRuntime(
    std::function<void(std::string_view)> source_boundary)
    : source_boundary_callback_(std::move(source_boundary)) {
    configure_callbacks();
}

void FieldSceneRuntime::source_boundary(
    const std::string_view boundary) const {
    if (source_boundary_callback_) {
        source_boundary_callback_(boundary);
    }
}

void FieldSceneRuntime::initialize_field_entity_selection_state() {
    // ROM 0x42B0 publishes the three field-record types in slot order. The
    // current new-game composition has Joshua in slot zero; the recruited
    // primary and secondary slots remain inactive until their own source
    // selection path is connected.
    field_entity_selection_state_ = RunnerSwitchState{
        {{0U, field_entity_behavior_flags_}, {1U, 0U}, {2U, 0U}}, 0U,
        false};
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
    const auto decision = advance_runner_switch(field_entity_selection_state_);
    if (field_entity_selection_state_.active_index <
        field_entity_selection_state_.records.size()) {
        field_entity_behavior_flags_ = static_cast<std::uint8_t>(
            field_entity_selection_state_
                .records[field_entity_selection_state_.active_index]
                .flags);
    }
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
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
    }
    if (effect.world_effect_y_dirty) {
        field_camera_follow_.layer_y_dirty = true;
    }
    if (effect.world_effect_x_dirty || effect.world_effect_y_dirty) {
        auto& layer = effect.world_effect_y_dirty
                          ? state_.field_scene_buffers.layer_b
                          : state_.field_scene_buffers.layer_a;
        for (const auto& write : effect.map_writes) {
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
        }
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
    make_genesis_sprite_color_zero_transparent(destination);
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
                   field_hud_unarmed_items_) ||
        !load_slot(kFieldHudVerticalCounterAliasPrefix,
                   field_hud_vertical_counters_) ||
        !load_slot(kFieldHudHorizontalCounterAliasPrefix,
                   field_hud_horizontal_counters_)) {
        return false;
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

bool FieldSceneRuntime::load_command_overview_assets(
    const ConversionAssetCatalog& catalog) {
    if (!load_image(catalog, kCommandOverviewBackgroundAlias,
                    command_overview_background_) ||
        !load_image(catalog, kCommandOverviewInventoryBoxesAlias,
                    command_overview_inventory_boxes_) ||
        !load_image(catalog, "menu.field.font", command_overview_font_)) {
        return false;
    }

    const auto load_sprite = [this, &catalog](const std::string& alias,
                                               QImage& image) {
        if (!load_image(catalog, alias, image)) {
            return false;
        }
        make_genesis_sprite_color_zero_transparent(image);
        return true;
    };
    for (std::size_t index = 0; index < command_overview_portraits_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewPortraitAliasPrefix) +
                             std::to_string(index),
                         command_overview_portraits_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_focus_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewFocusAliasPrefix) +
                             std::to_string(index),
                         command_overview_focus_[index])) {
            return false;
        }
        if (!load_sprite(std::string(kCommandOverviewStatusAliasPrefix) +
                             std::to_string(index),
                         command_overview_status_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_selected_cells_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewSelectedCellAliasPrefix) +
                             std::to_string(index),
                         command_overview_selected_cells_[index])) {
            return false;
        }
    }
    if (!load_sprite(std::string(kCommandOverviewCursorAlias),
                     command_overview_cursor_)) {
        return false;
    }

    for (std::size_t item = 1U; item <= 50U; ++item) {
        for (std::size_t palette = 0U; palette < 2U; ++palette) {
            const auto alias = command_overview_item_alias(
                static_cast<std::uint8_t>(item),
                static_cast<std::uint8_t>(palette));
            if (!load_sprite(alias, command_overview_inventory_icons_[item - 1U]
                                             [palette])) {
                return false;
            }
        }
        if (!load_sprite(command_overview_item_detail_alias(
                             static_cast<std::uint8_t>(item)),
                         command_overview_item_details_[item - 1U])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_damage_bars_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewDamageBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_damage_bars_[index]) ||
            !load_sprite(std::string(kCommandOverviewDrainBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_drain_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_detail_values_.size(); ++index) {
        if (!load_sprite(std::string(kCommandOverviewDetailValueAliasPrefix) +
                             std::to_string(index),
                         command_overview_detail_values_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_attack_bars_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewAttackBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_attack_bars_[index]) ||
            !load_sprite(std::string(kCommandOverviewDefenseBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_defense_bars_[index])) {
            return false;
        }
    }
    return true;
}

void FieldSceneRuntime::configure_callbacks() {
    callbacks_.mode_entry_audio.run_fixed_audio_transaction =
        [this](const std::uint32_t command) {
            source_boundary("field.audio.mode_entry.command");
            (void)command;
        };
    callbacks_.mode_entry_audio.wait_for_vblank_frames =
        [this](const std::uint16_t counter) {
            source_boundary("field.audio.mode_entry.wait");
            (void)counter;
        };
    callbacks_.mode_entry_audio.flush_audio_bus = [this] {
        source_boundary("field.audio.mode_entry.flush");
    };
    callbacks_.mode_entry_audio.service_audio_tick = [this] {
        source_boundary("field.audio.mode_entry.service");
    };

    callbacks_.initialize_entity_order = [this] {
        source_boundary("field.entities.initialize_order");
        state_.render_entities[0].active = field_entity_.active;
        state_.render_entities[0].id = 0U;
    };
    callbacks_.prepare_field_scene_runtime = [this] {
        const auto decision = prepare_field_scene_runtime(
            field_action_counter_,
            {[this] { refresh_field_runner_state_and_assets(); },
             [this] { run_field_mode_exit_transition(); }});
        source_boundary(
            decision.path ==
                    FieldSceneRuntimePreparationPath::
                        refresh_runner_state_and_assets
                ? "field.scene.runtime_path.refresh"
                : "field.scene.runtime_path.finalize");
    };
    callbacks_.field_resources.prepare_asset_setup = [this] {
        source_boundary("field.resources.prepare_asset_setup");
    };
    callbacks_.field_resources.upload_asset_sequence = [this] {
        source_boundary("field.resources.upload_asset_sequence");
    };
    callbacks_.field_resources.refresh_animation_assets = [this] {
        source_boundary("field.resources.refresh_animation_assets");
    };
    callbacks_.field_resources.load_font = [this] {
        source_boundary("field.resources.load_font");
    };

    world_frame_callbacks_.service_audio = [this] {
        source_boundary("field.world.service_audio");
    };
    world_frame_callbacks_.reset_effect_record = [this] {
        source_boundary("field.world.reset_effect_record");
    };
    world_frame_callbacks_.begin_effect_transition = [this] {
        source_boundary("field.world.begin_effect_transition");
    };
    world_frame_callbacks_.clear_effect_slots = [this] {
        source_boundary("field.world.clear_effect_slots");
    };
    world_frame_callbacks_.clear_entity_effects = [this] {
        source_boundary("field.world.clear_entity_effects");
    };
    world_frame_callbacks_.clear_scene_pending = [this] {
        source_boundary("field.world.clear_scene_pending");
    };
    world_frame_callbacks_.finalize_entity_templates = [this] {
        source_boundary("field.world.finalize_entity_templates");
    };
    world_frame_callbacks_.service_scene_effect = [this] {
        source_boundary("field.world.service_scene_effect");
    };
    world_frame_callbacks_.copy_terrain_layers = [this] {
        source_boundary("field.world.copy_terrain_layers");
    };
    world_frame_callbacks_.process_terrain_entries = [this] {
        source_boundary("field.world.process_terrain_entries");
    };
    world_frame_callbacks_.rebuild_collision_map = [this] {
        source_boundary("field.world.rebuild_collision_map");
    };
    world_frame_callbacks_.load_scene_resources = [this] {
        source_boundary("field.world.load_scene_resources");
    };
    world_frame_callbacks_.clear_scene_transitions = [this] {
        source_boundary("field.world.clear_scene_transitions");
    };
    world_frame_callbacks_.populate_entities = [this] {
        source_boundary("field.world.populate_entities");
    };
    world_frame_callbacks_.dispatch_entity_actions = [this] {
        source_boundary("field.world.dispatch_entity_actions");
    };
    world_frame_callbacks_.decode_entity_data = [this] {
        source_boundary("field.world.decode_entity_data");
    };
    world_frame_callbacks_.mark_entities_for_render = [this] {
        source_boundary("field.world.mark_entities_for_render");
    };
    callbacks_.run_world_frame_services = [this] {
        run_world_frame_services(world_frame_state_, world_frame_callbacks_);
    };

    const auto publish_runner_positions =
        [this](const std::array<SceneRunnerPosition, 3>& positions) {
            auto& records = state_.field_scene_buffers.source_metadata
                                .initial_field_records;
            for (std::size_t index = 0; index < positions.size() &&
                                        index < records.size();
                 ++index) {
                records[index] = positions[index];
            }
            field_entity_.world_position.x = positions[0].x;
            field_entity_.world_position.y = positions[0].y;
            field_movement_state_.position_x = positions[0].x;
            field_movement_state_.position_y = positions[0].y;
        };
    callbacks_.apply_scene_profile_selection = [this, publish_runner_positions] {
        if (field_pending_scene_profile_.has_value()) {
            SceneProfileState profile{};
            SceneProfileDefaults defaults{};
            apply_scene_profile_selection(
                profile, field_pending_scene_profile_, defaults);
            state_.field_scene_buffers.camera_x = profile.camera_x;
            state_.field_scene_buffers.camera_y = profile.camera_y;
            publish_runner_positions(profile.runner_positions);
            return;
        }

        // ROM 0x2B3E copies the six FFDE08 words into the three entity slots.
        // It does not move the camera.
        const auto& scene_profile = state_.field_scene_buffers.profile;
        if (scene_profile.size() < kSceneProfileEntityPositionWord + 6U) {
            source_boundary("field.scene.profile_positions_missing");
            return;
        }
        publish_runner_positions({
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord],
                scene_profile[kSceneProfileEntityPositionWord + 1U]},
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord + 2U],
                scene_profile[kSceneProfileEntityPositionWord + 3U]},
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord + 4U],
                scene_profile[kSceneProfileEntityPositionWord + 5U]}});
    };
    callbacks_.field_viewports.prepare_title_resources = [this] {
        source_boundary("field.viewport.prepare_title_resources");
    };
    callbacks_.field_viewports.copy_mode_selection_state_table = [this] {
        source_boundary("field.viewport.copy_mode_selection_state_table");
    };
    callbacks_.field_viewports.upload_mode_word_block = [this] {
        source_boundary("field.viewport.upload_mode_word_block");
    };
    callbacks_.field_viewports.render_normal_viewport = [this] {
        // ROM 0x2C92 clears FFE100 before drawing the normal viewport.
        field_camera_follow_.layer_x_dirty = false;
        render_normal_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_normal"); });
    };
    callbacks_.field_viewports.render_alternate_viewport = [this] {
        // ROM 0x2CB2 clears FFE102 before drawing the alternate viewport.
        field_camera_follow_.layer_y_dirty = false;
        render_alternate_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_alternate"); });
    };
    callbacks_.upload_scene_profile_block = [this] {
        source_boundary("field.scene.upload_profile_block");
    };
    callbacks_.guard_world_effect_player_spawn = [this] {
        source_boundary("field.world.guard_player_spawn");
    };

    callbacks_.entity_behavior_frames.dispatch_entity_action_candidates =
        [this] { source_boundary("field.entities.dispatch_action_candidates"); };
    callbacks_.entity_behavior_frames.noop_entity_action_hook =
        [](EntityBehaviorFrameRecord& record) { return record.action_result; };
    const auto observe_entity_boundary =
        [this](const std::string_view boundary,
               EntityBehaviorFrameRecord& record) {
            source_boundary(boundary);
            (void)record;
        };
    callbacks_.entity_behavior_frames.complete_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.complete_transition", record);
        };
    callbacks_.entity_behavior_frames.clear_effect_slot_state =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_effect_slot", record);
        };
    callbacks_.entity_behavior_frames.handle_mode_action_event =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.handle_action_event", record);
        };
    callbacks_.entity_behavior_frames.advance_entity_behavior_action =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.advance_action", record);
        };
    callbacks_.entity_behavior_frames.enter_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.enter_transition", record);
        };
    callbacks_.entity_behavior_frames.dispatch_entity_collision_behavior =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.dispatch_collision", record);
        };
    callbacks_.entity_behavior_frames.apply_mode_five_boundary_motion =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.apply_boundary_motion", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_proximity_interactions =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_proximity", record);
        };
    callbacks_.entity_behavior_frames.clear_entity_effect_if_forced =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_forced_effect", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_fallback_movement =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_fallback", record);
        };
    callbacks_.entity_behavior_frames.run_entity_continuation =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.run_continuation", record);
        };

    callbacks_.update_field_camera = [this] { update_source_field_camera(); };
    callbacks_.entity_render_pass.project_screen_position =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.project_screen_position");
            if (!field_entity_.active || entity.id != 0U) {
                field_entity_.visible = false;
                return;
            }
            field_entity_.projection.world_x =
                field_entity_.world_position.x;
            field_entity_.projection.world_y =
                field_entity_.world_position.y;
            const auto projection = project_entity_screen_position_standard(
                field_entity_.projection,
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_x),
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_y));
            field_entity_.visible = projection.visible;
        };
    callbacks_.entity_render_pass.advance_animation =
        [this](RenderEntityState& entity) {
            (void)entity;
            if (!field_entity_walking_) {
                return;
            }
            advance_entity_animation_accumulator(field_entity_animation_);
            const EntitySpriteDescriptorList walk_list{
                static_cast<std::int16_t>(0x8002), 0x0300U, {0U, 1U}};
            EntitySpriteDescriptorState descriptor{
                kFieldEntityKind,
                field_entity_animation_selector_,
                0U,
                field_entity_animation_.frame,
                static_cast<std::uint16_t>(
                    field_entity_animation_.increment),
                0U,
                0U};
            advance_entity_sprite_descriptor(descriptor, walk_list, 0U);
            field_entity_animation_.frame = descriptor.frame;
            field_entity_animation_.increment =
                static_cast<std::int16_t>(descriptor.increment);
            field_entity_animation_selector_ = descriptor.selector;
        };
    callbacks_.entity_render_pass.finish_entity =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.finish_entity");
            (void)entity;
        };
    callbacks_.entity_render_pass.finish_pass = [this] {
        source_boundary("field.entities.finish_render_pass");
    };
    callbacks_.wait_for_vblank = [this] {
        source_boundary("field.timing.wait_for_vblank");
    };
    callbacks_.advance_title_palette_fade = [this] {
        source_boundary("field.palette.advance_transition");
    };
    callbacks_.sample_controller_input = [this] {
        source_boundary("field.controller.sample");
    };
    configure_runtime_coordinator_callbacks();
    callbacks_.phase_changed = [this](
                                  const integration::FieldSessionBootstrapPhase phase) {
        source_boundary(std::string("field.bootstrap.phase.") +
                        integration::field_session_bootstrap_phase_name(phase));
    };
}

void FieldSceneRuntime::configure_runtime_coordinator_callbacks() {
    field_runtime_callbacks_.wait_for_vblank = [this] {
        source_boundary("field.runtime.wait_for_vblank");
    };
    field_runtime_callbacks_.prepare_mode_entry_service = [this] {
        // ROM 0x0976 enters the same six-command mode-entry audio service
        // used by field bootstrap. This is a converted leaf, not a marker for
        // an unresolved transition callback.
        prepare_audio_mode_entry(callbacks_.mode_entry_audio);
    };
    field_runtime_callbacks_.toggle_field_scene_variant = [this](
                                                                  auto& runtime) {
        FieldSceneVariantState variant{};
        variant.field_mode = runtime.field_mode;
        variant.scene_id = runtime.scene_id;
        variant.camera_x = state_.field_scene_buffers.camera_x;
        variant.pending_profile = field_pending_scene_profile_;
        const auto carry = toggle_field_scene_variant(variant);
        runtime.scene_id = variant.scene_id;
        state_.field_scene_buffers.scene_reference = variant.scene_id;
        field_pending_scene_profile_ = variant.pending_profile;
        if (carry) {
            source_boundary("field.runtime.scene_variant_selected");
        }
        return carry;
    };
    field_runtime_callbacks_.latch_controller_events = [this] {
        source_boundary("field.runtime.latch_controller_events");
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.confirm_latch =
            field_runtime_state_.confirm_latch;
        field_controller_latch_state_.start_latch =
            field_runtime_state_.start_latch;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        field_runtime_state_.confirm_latch =
            field_controller_latch_state_.confirm_latch;
        field_runtime_state_.start_latch =
            field_controller_latch_state_.start_latch;
    };
    field_runtime_callbacks_.sample_controller_input = [this] {
        source_boundary("field.runtime.sample_controller_input");
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        field_runtime_state_.controller_held =
            field_controller_sample_state_.held;
        field_runtime_state_.controller_heading =
            field_controller_sample_state_.heading;
        return field_controller_sample_state_.events;
    };
    field_runtime_callbacks_.service_entity_behavior_frames = [this] {
        // The converted movement/behavior path is the first active entity
        // consumer in this host. The remaining entity slots stay owned by the
        // conversion callback boundary until their source records are wired.
        advance_field_movement();
    };
    field_runtime_callbacks_.update_field_camera = [this] {
        update_source_field_camera();
    };
    field_runtime_callbacks_.update_entity_render_pass = [this] {
        update_entity_render_pass(state_.render_entities,
                                  callbacks_.entity_render_pass);
    };
    field_runtime_callbacks_.advance_palette_fade = [this] {
        source_boundary("field.runtime.advance_palette_fade");
    };
    field_runtime_callbacks_.advance_world_frame_logic = [this] {
        advance_world_frame_logic(world_frame_logic_state_,
                                  world_frame_logic_callbacks_);
    };
    field_runtime_callbacks_.handle_mode_action_event = [this] {
        handle_field_mode_action_event();
    };
    field_runtime_callbacks_.run_fixed_audio_transaction =
        [this](const std::uint16_t command) {
            source_boundary("field.runtime.command_audio_transaction");
            (void)command;
        };
    field_runtime_callbacks_.run_command_overview = [this] {
        open_command_overview();
    };
    field_runtime_callbacks_.service_audio_tick = [this] {
        source_boundary("field.runtime.command_audio_tick");
    };
    field_runtime_callbacks_.select_next_entity = [this] {
        source_boundary("field.runtime.select_next_entity");
    };
    field_runtime_callbacks_.restart_field_session = [this] {
        source_boundary("field.runtime.restart_field_session");
    };
    field_runtime_callbacks_.advance_field_scene_transition = [this] {
        if (command_overview_.open()) {
            // The source routine returns from the blocking command loop before
            // the field transition leaf can run. The Qt bridge keeps that
            // ordering explicit while the menu owns the next host frames.
            source_boundary(
                "field.runtime.scene_transition_suspended_for_command_overview");
            return;
        }
        source_boundary("field.runtime.advance_scene_transition");
    };

    world_frame_logic_callbacks_.prepare_mode_entry = [this] {
        source_boundary("field.world_logic.prepare_mode_entry");
    };
    world_frame_logic_callbacks_.begin_world_activity = [this] {
        source_boundary("field.world_logic.begin_world_activity");
    };
    world_frame_logic_callbacks_.select_entity_service_target = [this] {
        source_boundary("field.world_logic.select_entity_service_target");
    };
    world_frame_logic_callbacks_.reset_entity_service_records = [this] {
        source_boundary("field.world_logic.reset_entity_service_records");
    };
    world_frame_logic_callbacks_.clear_inactive_entity_effects = [this] {
        source_boundary("field.world_logic.clear_inactive_entity_effects");
    };
    world_frame_logic_callbacks_.process_terrain_work_batch = [this] {
        source_boundary("field.world_logic.process_terrain_work_batch");
    };
    world_frame_logic_callbacks_.advance_world_effect_trigger = [this] {
        source_boundary("field.world_logic.advance_world_effect_trigger");
    };
    world_frame_logic_callbacks_.trigger_world_effect_sequence = [this] {
        source_boundary("field.world_logic.trigger_world_effect_sequence");
    };
    world_frame_logic_callbacks_.resolve_nearby_effects = [this] {
        source_boundary("field.world_logic.resolve_nearby_effects");
    };
}

void FieldSceneRuntime::open_command_overview() {
    if (!ready_ || command_overview_.open()) {
        return;
    }

    const auto command_asset_available = [this](const std::string_view alias) {
        if (alias == "menu.field.command_overview.background") {
            // The converted source preparation names the tile block; the
            // native host presents its already extracted full command shell.
            return !command_overview_background_.isNull();
        }
        if (alias == "menu.field.cursor") {
            return !command_overview_cursor_.isNull();
        }
        return false;
    };
    const CommandOverviewInput input{
        static_cast<std::uint8_t>(field_active_runner_record_.flags & 0xFFU),
        static_cast<std::int8_t>(field_active_runner_record_.archetype_index)};
    const CommandOverviewPreparationInput preparation_input{};
    const CommandOverviewHudPanelInput hud_panel_input{
        field_active_runner_record_.record_type,
        field_active_runner_record_.command_overview_panel_parameter};
    command_overview_.begin(
        input, preparation_input, hud_panel_input,
        {[this](const std::string_view boundary) { source_boundary(boundary); },
         command_asset_available,
         [this] { return field_active_runner_record_.equipped_item_code; },
         [](const CommandOverviewPreparationResult&) {},
         [](const CommandOverviewHudPanelPlan&) {},
         [](const CommandOverviewActionRefreshResult&) {}});
    refresh_command_overview_combat_bars();
    refresh_command_overview_item_detail();
}

bool FieldSceneRuntime::bootstrap(
    const integration::StartupIntegrationState& startup_state) {
    ready_ = false;
    failure_.clear();
    state_ = integration::FieldSessionBootstrapState{};
    world_frame_state_ = WorldFrameState{};
    viewport_state_ = FieldViewportState{};

    if (startup_state.restored_field_state.has_value()) {
        failure_ =
            "loaded-save field scene composition remains a source boundary";
        source_boundary("field.bootstrap.loaded_save_unresolved");
        return false;
    }
    if (!startup_state.field_session_source_assets.has_value()) {
        failure_ = "field-session source records are unavailable";
        source_boundary("field.bootstrap.source_records_missing");
        return false;
    }

    const auto& assets = *startup_state.field_session_source_assets;
    field_movement_direction_offsets_ = assets.field_movement_direction_offsets;
    field_collision_probe_offset_banks_ =
        assets.field_collision_probe_offset_banks;
    field_controller_heading_table_ = assets.field_controller_heading_table;
    field_movement_route_sources_ = assets.field_movement_routes;
    for (std::size_t index = 0; index < field_movement_routes_.size();
         ++index) {
        const auto& source_route = assets.field_movement_routes[index];
        field_movement_routes_[index] = DirectionalMovementRoute{
            0,
            0,
            source_route.primary_fallback_slot,
            source_route.secondary_fallback_slot};
    }
    field_controller_nibble_ = 0U;
    field_controller_port_byte_ = 0U;
    field_controller_sample_state_ = ControllerSampleState{};
    field_controller_latch_state_ = ControllerLatchState{};
    field_target_heading_ = 0U;
    field_movement_state_ = EntityMovementStepState{};
    field_collision_record_ = EntityCollisionRecord{};
    field_collision_probe_state_ = CollisionProbeState{};
    field_movement_behavior_record_ = EntityBehaviorFrameRecord{};
    field_movement_transition_state_ = EntityMovementModeTransitionState{};
    field_world_activity_state_ = WorldActivityState{};
    field_activity_continuation_ = MovementModeTransitionContinuation::none;
    field_projected_terrain_mark_count_ = 0U;
    field_terrain_render_dirty_ = false;
    field_terrain_sample_state_ = {};
    field_action_counter_ = 0U;
    field_entity_behavior_flags_ = 0U;
    field_entity_selection_state_ = RunnerSwitchState{};
    field_runner_refresh_state_ = FieldRunnerRefreshState{};
    field_mode_exit_transition_state_ = FieldModeExitTransitionState{};
    field_entity_table_occupancy_ = {};
    field_entity_table_ = {};
    field_collision_marker_index_ = 0U;
    field_pending_scene_profile_.reset();
    field_camera_follow_ = {};
    field_camera_alignment_ready_ = false;
    command_overview_.close();
    field_active_runner_record_ = RunnerRecordFields{};
    field_menu_text_ = FieldMenuTextCatalog{};
    command_overview_resource_variant_ = ResourceVariantState{};
    command_overview_combat_bars_ = EquipmentCombatBarResult{};
    command_overview_item_detail_ = ItemActionDetailResult{};
    field_joshua_archetype_selection_ = 0U;
    field_hud_slots_ = {};
    field_entity_animation_ = {};
    field_entity_animation_.increment = kFieldEntityAnimationPeriod;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_selector_ =
        select_field_entity_animation_selector(0, false);
    field_entity_walking_ = false;
    input_ = integration::FieldSessionBootstrapInput{};
    input_.action_selection_profile_source =
        assets.action_selection_profile_source;
    input_.field_mode_selection_state_source =
        assets.field_mode_selection_state_source;
    input_.field_scene =
        integration::make_initial_mode2_field_scene_input(assets);

    field_entity_ = FieldEntityPresentation{};
    field_entity_.active = startup_state.joshua_selection.has_value();
    field_entity_.world_position =
        input_.field_scene.source_metadata.initial_field_records[0];
    if (field_entity_.active) {
        field_active_runner_record_ =
            startup_state.joshua_selection->typed_live_joshua_record;
        field_joshua_archetype_selection_ = static_cast<std::size_t>(
            startup_state.joshua_archetype.selected);
        field_entity_.projection.sprite_offset_y =
            startup_state.joshua_selection->typed_live_joshua_record
                .projection_word_08;
    }
    // ROM 0x44BC installs behavior byte 0x05 in a field entity before the
    // source 0xC64C/0x391A handoff. Keep the typed state initialized before
    // field-buffer preparation can invoke 0xC7F8.
    field_entity_behavior_flags_ = field_entity_.active ? 0x05U : 0U;
    initialize_field_entity_selection_state();
    if (startup_state.joshua_selection.has_value()) {
        const auto& record =
            startup_state.joshua_selection->typed_live_joshua_record;
        field_hud_slots_[0].flags_byte =
            static_cast<std::uint8_t>(record.flags >> 8U);
        field_hud_slots_[0].equipped_inventory_slot =
            record.equipped_inventory_slot;
        field_hud_slots_[0].equipped_item_code = record.equipped_item_code;
        field_hud_slots_[0].ammo_rounds = record.ammo_rounds;
    }

    const auto menu_text = load_field_menu_text_catalog(
        startup_state.asset_catalog.catalog);
    if (!menu_text.complete()) {
        failure_ = menu_text.errors.empty()
                       ? "field menu text catalog is incomplete"
                       : menu_text.errors.front();
        source_boundary("field.bootstrap.menu_text_missing");
        return false;
    }
    field_menu_text_ = menu_text.catalog;

    if (!load_image(startup_state.asset_catalog.catalog,
                    kInitialFieldMapAlias, field_map_) ||
        !load_image(startup_state.asset_catalog.catalog, kFieldHudAlias,
                    field_hud_) ||
        !load_field_hud_overlays(startup_state.asset_catalog.catalog) ||
        !load_command_overview_assets(startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.direct_assets_missing");
        return false;
    }
    // The extracted HUD record is a plane-sized image: color zero is a hole
    // over the 272px playfield, but the source six-tile chrome column at
    // x=272..319 is an opaque panel.  Applying sprite transparency to the
    // whole bitmap makes that sidebar transparent over the map.
    make_field_hud_layer_transparent_outside_chrome(field_hud_);
    for (std::size_t direction = 0;
         direction < field_entity_idle_images_.size(); ++direction) {
        const auto alias = std::string(kFieldEntityIdleAliasPrefix) +
                           std::to_string(direction);
        if (!load_sprite_image(startup_state.asset_catalog.catalog, alias,
                               field_entity_idle_images_[direction],
                               field_entity_idle_origin_x_[direction],
                               field_entity_idle_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
        for (std::size_t frame = 0; frame < kFieldWalkAnimationFrameCount;
             ++frame) {
            const auto walk_alias =
                std::string(kFieldEntityWalkAliasPrefix) +
                std::to_string(direction) + "." + std::to_string(frame);
            if (!load_sprite_image(
                    startup_state.asset_catalog.catalog, walk_alias,
                    field_entity_walk_images_[direction][frame],
                    field_entity_walk_origin_x_[direction][frame],
                    field_entity_walk_origin_y_[direction][frame])) {
                source_boundary("field.bootstrap.entity_assets_missing");
                return false;
            }
        }
    }

    const auto result = integration::bootstrap_field_session(
        input_, state_, callbacks_);
    if (result != integration::FieldSessionBootstrapResult::field_ready) {
        failure_ = std::string("field bootstrap did not reach field_ready: ") +
                   integration::field_session_bootstrap_phase_name(
                       state_.phase);
        source_boundary("field.bootstrap.incomplete");
        return false;
    }

    field_movement_state_.position_x = field_entity_.world_position.x;
    field_movement_state_.position_y = field_entity_.world_position.y;
    field_movement_state_.heading = 0U;
    // This is distinct from the live runner record's +0x10 flags word used by
    // menu/status code; movement and terrain probes consume the entity slot.
    field_movement_behavior_record_.active = field_entity_.active;
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_movement_behavior_record_.current_heading = 0U;
    field_movement_behavior_record_.target_heading = 0U;
    const auto interaction_mode = select_scene_interaction_mode(
        state_.field_scene_buffers.scene_reference);
    if (!interaction_mode.has_value()) {
        // ROM 0x167AE stop. Scene >= 0x38 is not mode 0.
        failure_ = "ROM 0x167AE stop: scene id >= 0x38";
        source_boundary("field.world.interaction_mode_stop");
        return false;
    }
    field_movement_transition_state_.world_mode = *interaction_mode;
    field_movement_transition_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    field_movement_transition_state_.world_activity_source_word = 0U;
    field_runtime_state_ = integration::FieldRuntimeCoordinatorState{};
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.display_dirty_flags = state_.display_dirty_flags;
    world_frame_logic_state_ = WorldFrameLogicState{};
    world_frame_logic_state_.world_mode =
        state_.field_scene_buffers.scene_reference;
    field_entity_.direction =
        field_movement_direction_from_heading(field_movement_state_.heading);

    ready_ = true;
    source_boundary("field.bootstrap.ready");
    return true;
}

void FieldSceneRuntime::set_controller_nibble(
    const std::uint8_t dpad_nibble) noexcept {
    set_controller_port_byte(static_cast<std::uint8_t>(
        (field_controller_port_byte_ & 0xF0U) | (dpad_nibble & 0x0FU)));
}

void FieldSceneRuntime::set_controller_port_byte(
    const std::uint8_t port_byte) noexcept {
    field_controller_port_byte_ = port_byte;
    field_controller_nibble_ = static_cast<std::uint8_t>(port_byte & 0x0FU);
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

void FieldSceneRuntime::record_field_activity_start(
    const MovementModeTransitionContinuation continuation) {
    field_activity_continuation_ = continuation;
    // ROM 0x1E2E0/0x1E2EC stores the typed continuation in the activity
    // timer. Scene-table selection belongs to the later 0x167AE dispatcher;
    // it is not a return value of the timer helper.
    source_boundary("field.world.activity.started");
}

bool FieldSceneRuntime::recover_blocked_field_movement(
    const MovementProbeResult& initial_probe) {
    field_movement_behavior_record_.current_heading =
        field_movement_state_.heading;
    field_movement_behavior_record_.target_heading = field_target_heading_;
    for (std::size_t index = 0;
         index < field_movement_transition_state_.directional_probe_signs.size();
         ++index) {
        field_movement_transition_state_.directional_probe_signs[index] =
            static_cast<std::int8_t>(field_terrain_sample_state_[index]);
    }
    field_movement_transition_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);

    EntityMovementModeTransitionCallbacks transition_callbacks{};
    transition_callbacks.build_collision_probe_points = [this](
                                                               EntityBehaviorFrameRecord&) {
        const std::array<std::uint8_t, 3> probe_bytes{
            field_terrain_sample_state_[0], field_terrain_sample_state_[1],
            field_terrain_sample_state_[2]};
        build_collision_probe_points(
            field_collision_probe_state_,
            field_movement_transition_state_.world_mode != 0U,
            field_entity_.world_position.x, field_entity_.world_position.y,
            probe_bytes, field_collision_probe_offset_banks_,
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y));
        return MovementProjectionPreparation{
            field_collision_probe_state_.selected_offset_bank};
    };
    transition_callbacks.quantize_field_delta =
        [](const std::int16_t target, const std::uint16_t origin) {
            return quantize_field_delta(target, origin);
        };
    transition_callbacks.begin_world_activity_if_idle =
        [this](const MovementModeTransitionContinuation continuation) {
            const auto started = begin_world_activity_if_idle(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            if (started) {
                record_field_activity_start(continuation);
            }
            return started;
        };
    transition_callbacks.begin_world_activity =
        [this](const MovementModeTransitionContinuation continuation) {
            begin_world_activity(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            record_field_activity_start(continuation);
        };
    transition_callbacks.mark_projected_terrain_cell = [this] {
        // ROM 0x16760 marks the projected field work cell in the active
        // terrain layer. Keep this bridge local: the older menu conversion
        // header owns a legacy TerrainWorkEntry name that is unrelated to
        // this runtime's source-backed field scene state.
        const auto cell = projected_terrain_cell();
        if (!cell.has_value() ||
            *cell >= state_.field_scene_buffers.layer_a.size() ||
            state_.field_scene_buffers.layer_a[*cell] == 0x000BU) {
            return false;
        }
        ++field_projected_terrain_mark_count_;
        state_.field_scene_buffers.layer_a[*cell] = 0x000BU;
        field_terrain_render_dirty_ = true;
        return true;
    };
    transition_callbacks.project_entity_world_interaction = [this] {
        return project_field_world_interaction();
    };
    transition_callbacks.prepare_mode_entry_service = [this] {
        prepare_audio_mode_entry(callbacks_.mode_entry_audio);
    };

    const auto result = advance_entity_movement_and_mode_transition(
        field_movement_behavior_record_, initial_probe.candidate_x,
        initial_probe.candidate_y, field_movement_transition_state_,
        transition_callbacks);
    if (result.mode_event_code_published) {
        source_boundary("field.world.activity.mode_event_published");
    }
    source_boundary("field.movement.blocked_transition");
    return (field_movement_behavior_record_.route_flags & 0x01U) != 0U;
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y) {
    const auto collision_evaluator =
        [this](const std::uint16_t evaluated_x,
               const std::uint16_t evaluated_y) {
            const auto& metadata = state_.field_scene_buffers.source_metadata;
            const TerrainCollisionBounds bounds{
                metadata.terrain_x_inclusive_lower,
                metadata.terrain_x_exclusive_upper,
                metadata.terrain_y_inclusive_lower,
                metadata.terrain_y_exclusive_upper};
            const auto result = evaluate_terrain_collision(
                TerrainCollisionInput{evaluated_x,
                                      evaluated_y,
                                      field_entity_behavior_flags_,
                                      bounds,
                                      field_terrain_sample_state_},
                TerrainCollisionCallbacks{
                    [this](const std::uint16_t normalized_x,
                           const std::uint16_t normalized_y,
                           const bool expanded_probe) {
                        return sample_terrain_collision_tiles(
                            normalized_x, normalized_y, expanded_probe,
                            [this](const std::uint16_t tile_index) {
                                const auto& collision_map =
                                    state_.field_scene_buffers.source_metadata
                                        .terrain_collision_map;
                                if (tile_index >= collision_map.size()) {
                                    return static_cast<std::uint8_t>(0x7FU);
                                }
                                return collision_map[tile_index];
                            });
                    },
                    [this](const std::uint8_t object_id,
                           const std::int16_t object_x,
                           const std::int16_t object_y) {
                        const auto& rectangles =
                            state_.field_scene_buffers.source_metadata
                                .terrain_collision_rectangles;
                        if (object_id >= rectangles.size()) {
                            source_boundary(
                                "field.movement.terrain_object_record_missing");
                            return true;
                        }
                        const auto object_lookup =
                            [&rectangles](const std::uint16_t lookup_id) {
                                if (lookup_id >= rectangles.size()) {
                                    return TerrainObjectBounds{};
                                }
                                const auto& record = rectangles[lookup_id];
                                return TerrainObjectBounds{
                                    static_cast<std::int16_t>(record.left),
                                    static_cast<std::int16_t>(record.right),
                                    static_cast<std::int16_t>(record.top),
                                    static_cast<std::int16_t>(record.bottom)};
                            };
                        return contains_terrain_object_bounds(
                            object_id, object_x, object_y, object_lookup);
                    },
                    {}});
            field_terrain_sample_state_ = field_sample_state_from_result(
                result.packed_sample_state);
            // Outside the map, 0xE7C8 returns before 0xE8A0, so FFDFE4 stays
            // at the previous sample.
            if (!result.state.outside_bounds_marker) {
                field_collision_marker_index_ =
                    result.state.sample.selected_tile_index;
            }
            return CollisionEvaluation{
                result.code, result.packed_sample_state};
        };
    return evaluate_entity_collision(field_collision_record_, candidate_x,
                                     candidate_y, collision_evaluator);
}

void FieldSceneRuntime::advance_field_movement() {
    if (!ready_ || !field_entity_.active || field_controller_nibble_ == 0U) {
        return;
    }

    const auto input = decode_field_movement_input(
        field_controller_nibble_, field_controller_heading_table_);
    if (!input.valid) {
        return;
    }

    field_target_heading_ = input.target_heading;
    EntityCoordinateTarget heading{
        field_movement_state_.heading, field_target_heading_};
    approach_entity_fixed_point_target(heading);
    field_movement_state_.heading = heading.current;

    MovementProbeResult last_probe{};
    const auto probe = [this, &last_probe](const std::uint16_t direction_slot) {
        const auto direction = static_cast<std::uint8_t>(
            (direction_slot >> 1U) & 0x07U);
        last_probe = probe_entity_directional_collision(
            MovementProbeInput{
                field_movement_state_.position_x,
                field_movement_state_.position_y,
                direction,
                field_entity_behavior_flags_},
            field_movement_direction_offsets_,
            [this](const std::uint16_t candidate_x,
                   const std::uint16_t candidate_y) {
                return evaluate_field_collision(candidate_x, candidate_y);
            });
        return last_probe;
    };
    const auto policy_probe = [probe](const std::uint16_t direction_slot) {
        return probe(direction_slot).collision_code;
    };

    const auto previous_x = field_movement_state_.position_x;
    const auto previous_y = field_movement_state_.position_y;
    const MovementResultPolicy result_policy{
        false,
        state_.field_scene_buffers.scene_reference,
        field_movement_transition_state_.world_mode};
    begin_entity_movement_pass(
        field_movement_state_,
        result_policy,
        probe,
        [this, &last_probe] { return recover_blocked_field_movement(last_probe); },
        [this, &result_policy, &policy_probe] {
            const auto route_lookup = [this](const std::uint16_t route_key) {
                if ((route_key & 0x03U) != 0U ||
                    route_key / 4U >= field_movement_routes_.size()) {
                    source_boundary("field.movement.route_key_out_of_range");
                    return DirectionalMovementRoute{};
                }
                auto route = field_movement_routes_[route_key / 4U];
                const auto& source_route =
                    field_movement_route_sources_[route_key / 4U];
                const auto sample_value = [this](
                                              const std::uint8_t sample_index) {
                    if (sample_index >= field_terrain_sample_state_.size()) {
                        source_boundary(
                            "field.movement.route_sample_out_of_range");
                        return static_cast<std::int8_t>(-1);
                    }
                    return static_cast<std::int8_t>(
                        field_terrain_sample_state_[sample_index]);
                };
                route.primary_marker =
                    sample_value(source_route.primary_sample_index);
                route.secondary_marker =
                    sample_value(source_route.secondary_sample_index);
                return route;
            };
            return evaluate_entity_movement_route(
                field_movement_state_.heading, result_policy, route_lookup,
                policy_probe);
        },
        policy_probe,
        [this] { source_boundary("field.movement.reset_blocked_step"); });

    field_entity_.world_position.x = field_movement_state_.position_x;
    field_entity_.world_position.y = field_movement_state_.position_y;
    field_entity_.direction =
        field_movement_direction_from_heading(field_movement_state_.heading);
    if (previous_x != field_movement_state_.position_x ||
        previous_y != field_movement_state_.position_y) {
        source_boundary("field.movement.accepted_step");
        apply_field_entity_animation(true);
    } else {
        apply_field_entity_animation(false);
    }
}

void FieldSceneRuntime::apply_field_entity_animation(const bool walking) {
    const auto selector = select_field_entity_animation_selector(
        field_entity_.direction, walking);
    if (selector != field_entity_animation_selector_) {
        field_entity_animation_selector_ = selector;
        field_entity_animation_.frame = 0;
        field_entity_animation_.accumulator = 0;
        field_entity_animation_.redraw_requested = true;
        field_entity_animation_.increment =
            walking ? kFieldWalkAnimationIncrement
                    : kFieldEntityAnimationPeriod;
        field_entity_animation_.period = kFieldEntityAnimationPeriod;
    }
    field_entity_walking_ = walking;
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

void FieldSceneRuntime::advance_frame() {
    if (!ready_) {
        return;
    }
    if (command_overview_.open()) {
        // This is the non-blocking host form of the source command loop's
        // wait/latch/sample sequence. Field movement and world services stay
        // suspended until Start returns from the overview.
        source_boundary("field.command_overview.wait_for_vblank");
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        command_overview_.advance(field_controller_sample_state_.events);
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
        return;
    }
    field_runtime_state_.field_activity_lock = state_.field_lock != 0U;
    field_runtime_state_.world_activity_countdown =
        world_frame_logic_state_.activity_countdown;
    field_runtime_state_.display_service_word = state_.display_service_word;
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;

    const auto result = integration::advance_field_runtime_frame(
        field_runtime_state_, field_runtime_callbacks_);
    state_.display_dirty_flags = field_runtime_state_.display_dirty_flags;
    state_.display_service_word = field_runtime_state_.display_service_word;
    world_frame_logic_state_.activity_countdown =
        field_runtime_state_.world_activity_countdown;
    if (result == integration::FieldRuntimeFrameResult::callback_unresolved) {
        source_boundary("field.runtime.callback_unresolved");
    }
}

bool FieldSceneRuntime::open_command_overview_for_dump() {
    if (!ready_) {
        return false;
    }
    // Exercise the same converted field-coordinator Start path used by the
    // window rather than directly toggling the presentation state.
    set_controller_port_byte(static_cast<std::uint8_t>(
        field_controller_port_byte_ | 0x80U));
    advance_frame();
    set_controller_port_byte(static_cast<std::uint8_t>(
        field_controller_port_byte_ & static_cast<std::uint8_t>(~0x80U)));
    return command_overview_.open();
}

void FieldSceneRuntime::paint_field_entity(QPainter& painter) const {
    if (!field_entity_.active || !field_entity_.visible) {
        return;
    }
    const auto direction = static_cast<std::size_t>(
        field_entity_.direction % field_entity_idle_images_.size());
    const auto walking = field_entity_walking_;
    const auto frame = bound_field_walk_animation_frame(
        field_entity_animation_.frame);
    const auto& image =
        walking ? field_entity_walk_images_[direction][frame]
                : field_entity_idle_images_[direction];
    const auto origin_x =
        walking ? field_entity_walk_origin_x_[direction][frame]
                : field_entity_idle_origin_x_[direction];
    const auto origin_y =
        walking ? field_entity_walk_origin_y_[direction][frame]
                : field_entity_idle_origin_y_[direction];
    const auto x = static_cast<int>(field_entity_.projection.screen_x) -
                   kGenesisSpriteOrigin + origin_x;
    const auto y = static_cast<int>(field_entity_.projection.screen_y) -
                   kGenesisSpriteOrigin + origin_y;
    painter.drawImage(x, y, image);
}

void FieldSceneRuntime::paint_field_hud_overlays(QPainter& painter) const {
    const auto image_for = [this](const FieldHudOverlay& overlay)
        -> const QImage* {
        switch (overlay.kind) {
        case FieldHudOverlayKind::portrait:
            return &field_hud_faces_[overlay.slot];
        case FieldHudOverlayKind::physical_bar:
            return &field_hud_vertical_counters_[overlay.slot];
        case FieldHudOverlayKind::mental_bar:
            return &field_hud_horizontal_counters_[overlay.slot];
        case FieldHudOverlayKind::item:
            return &field_hud_items_[overlay.slot];
        case FieldHudOverlayKind::unarmed_item:
            return &field_hud_unarmed_items_[overlay.slot];
        case FieldHudOverlayKind::ammo:
            return &field_hud_ammo_counters_[overlay.asset_index];
        }
        return nullptr;
    };
    for (const auto& overlay : compose_field_hud_overlays(field_hud_slots_)) {
        const auto* image = image_for(overlay);
        if (image == nullptr || image->isNull()) {
            continue;
        }
        painter.drawImage(overlay.x, overlay.y, *image);
    }
}

void FieldSceneRuntime::draw_command_overview_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text) const {
    if (command_overview_font_.isNull() || command_overview_font_.width() < 8 ||
        command_overview_font_.height() < 8) {
        return;
    }
    auto cursor = x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > command_overview_font_.width() ||
            source_y + 8 > command_overview_font_.height()) {
            cursor += 8;
            continue;
        }
        QImage tinted_glyph(8, 8, QImage::Format_ARGB32_Premultiplied);
        tinted_glyph.fill(Qt::transparent);
        for (int glyph_y = 0; glyph_y < 8; ++glyph_y) {
            for (int glyph_x = 0; glyph_x < 8; ++glyph_x) {
                const auto source_pixel = command_overview_font_.pixelColor(
                    source_x + glyph_x, source_y + glyph_y);
                const auto coverage = std::max(
                    {source_pixel.red(), source_pixel.green(),
                     source_pixel.blue()});
                if (source_pixel.alpha() == 0 || coverage == 0) {
                    continue;
                }
                const auto alpha = static_cast<int>(
                    (static_cast<std::uint32_t>(source_pixel.alpha()) *
                     static_cast<std::uint32_t>(coverage)) /
                    255U);
                tinted_glyph.setPixelColor(
                    glyph_x, glyph_y, QColor(0, 255, 0, alpha));
            }
        }
        painter.drawImage(cursor, y, tinted_glyph);
        cursor += 8;
    }
}

void FieldSceneRuntime::refresh_command_overview_combat_bars() {
    if (!command_overview_.open()) {
        command_overview_combat_bars_ = EquipmentCombatBarResult{};
        return;
    }

    const auto& record = field_active_runner_record_;
    EquipmentCombatInput input{};
    input.attributes = record.attributes;
    // The source combat adapter names the sparse +0x81..+0x8C window. The
    // live-record projection stores +0x80..+0x85 and +0x87..+0x8C as the
    // typed skill array, with +0x86 retained in auxiliary[1].
    input.skills = {record.skills[1],  record.skills[2], record.skills[3],
                    record.skills[4],  record.skills[5], record.auxiliary[1],
                    record.skills[6],  record.skills[7], record.skills[8],
                    record.skills[9],  record.skills[10], record.skills[11]};
    input.inventory_item_codes = record.inventory_item_codes;
    input.inventory_phase_bytes = record.inventory_phase_bytes;
    input.display_state = record.equipped_inventory_slot;
    input.active_item_code = record.equipped_item_code;
    input.posture = record.posture;
    // The command overview's source selector is outside the eight-cell item
    // grid. This keeps the normal higher-mode defense branch from treating a
    // highlighted possession cell as an action lookup.
    input.selected_inventory_slot =
        static_cast<std::uint8_t>(record.inventory_item_codes.size());
    input.cyberware_flags = record.cyberware_flags;
    input.action_flags = static_cast<std::uint8_t>(record.flags & 0xFFU);
    // ROM 0x12CEE's ordinary command-overview caller is in the higher-mode
    // branch. It uses +0x56/+0x57 directly for the attack strip and still
    // performs the typed defense calculation below it.
    input.source_mode = 2U;
    command_overview_combat_bars_ = compute_equipment_combat_bars(input);
}

void FieldSceneRuntime::refresh_command_overview_item_detail() {
    command_overview_item_detail_ = ItemActionDetailResult{};
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page >= 2U) {
        return;
    }

    const auto slot =
        command_overview_.view().source_state.secondary_page * 2U +
        command_overview_.view().source_state.primary_page;
    if (slot >= field_active_runner_record_.inventory_item_codes.size()) {
        return;
    }
    ItemActionDetailInput input{};
    input.item_code = field_active_runner_record_.inventory_item_codes[slot];
    for (std::size_t index = 0;
         index < field_active_runner_record_.inventory_phase_bytes.size();
         ++index) {
        input.phase_bytes[index] =
            field_active_runner_record_.inventory_phase_bytes[index];
    }
    // The ninth byte in the source phase span is active record +0x76, the
    // one-based armor ordinal. Preserve it as typed data for 0x12A8C's
    // item-detail value selection.
    input.phase_bytes[8] = field_active_runner_record_.equipped_armor_index;
    input.phase_slot = 8U;
    command_overview_item_detail_ = render_item_action_details(
        input, us_item_catalog(), command_overview_resource_variant_);
}

void FieldSceneRuntime::paint_command_overview(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!command_overview_.open()) {
        return;
    }

    painter.drawImage(0, 0, command_overview_background_);
    painter.drawImage(0, 0, command_overview_inventory_boxes_);

    // ROM 0x110BC writes the active runner's 8x10 portrait at E104, which is
    // the fixed 16,16 host location. The source template selection, rather
    // than the field slot number, chooses Joshua's portrait.
    if (field_joshua_archetype_selection_ <
            command_overview_portraits_.size() &&
        !command_overview_portraits_[field_joshua_archetype_selection_]
             .isNull()) {
        painter.drawImage(
            16, 16,
            command_overview_portraits_[field_joshua_archetype_selection_]);
    }

    draw_command_overview_text(painter, 128, 8,
                               field_menu_text_.runner_names[0]);

    // 0x11928/0x11AE4 places the five-state posture bar at CAAC (176,168).
    const auto posture = std::min<std::size_t>(
        field_active_runner_record_.posture,
        command_overview_status_bars_.size() - 1U);
    painter.drawImage(176, 168, command_overview_status_bars_[posture]);
    draw_command_overview_text(painter, 136, 176,
                               field_menu_text_.status_names[posture]);

    const auto draw_item_detail_rows =
        [this, &painter](const std::uint8_t item_code,
                         const std::optional<std::size_t> selected_slot,
                         const bool include_art) {
            if (item_code == 0U || item_code > us_item_catalog().size()) {
                return;
            }
            const auto* item = find_us_item_definition(item_code);
            if (item == nullptr) {
                return;
            }
            if (include_art && selected_slot.has_value() &&
                command_overview_item_detail_.returned_item_code == item_code &&
                command_overview_item_detail_.item_art_written) {
                painter.drawImage(144, 24,
                                  command_overview_item_details_[item_code - 1U]);
            }

            // ROM 0x12E9E uses the item record's +0x25/+0x26 only for the
            // weapon detail rows. The typed catalog retains those bytes under
            // their action/detail names; no host damage statistic is derived.
            if (item_code < 0x10U) {
                const auto damage_index = static_cast<std::size_t>(
                    static_cast<std::uint8_t>(item->signed_detail_asset_index));
                if (damage_index < command_overview_damage_bars_.size()) {
                    draw_command_overview_text(painter, 112, 80,
                                                field_menu_text_.damage_label);
                    painter.drawImage(168, 80,
                                      command_overview_damage_bars_[damage_index]);
                }
                if (item->detail_selector != 0U &&
                    item->detail_selector <= field_menu_text_.power_labels.size()) {
                    draw_command_overview_text(painter, 112, 88,
                                                field_menu_text_.power_label);
                    draw_command_overview_text(
                        painter, 168, 88,
                        field_menu_text_.power_labels[item->detail_selector - 1U]);
                }
            }

            // ROM 0x115F2 routes armor through the typed D4D50 row instead of
            // the weapon Damage/Power path. Its +0x26/+0x25 values select the
            // Wpns/Guns detail bars.
            if (item_code >= 0x2CU && item_code <= 0x32U) {
                const auto* armor = find_us_armor_definition(
                    field_active_runner_record_.equipped_armor_index);
                if (armor != nullptr) {
                    draw_command_overview_text(painter, 112, 80,
                                                field_menu_text_.weapons_label);
                    if (armor->secondary_modifier <
                        command_overview_damage_bars_.size()) {
                        painter.drawImage(
                            168, 80,
                            command_overview_damage_bars_[armor->secondary_modifier]);
                    }
                    draw_command_overview_text(painter, 112, 88,
                                                field_menu_text_.guns_label);
                    if (armor->primary_modifier <
                        command_overview_drain_bars_.size()) {
                        painter.drawImage(
                            168, 88,
                            command_overview_drain_bars_[armor->primary_modifier]);
                    }
                }
            }

            if (!include_art || !selected_slot.has_value()) {
                return;
            }
            std::optional<std::size_t> label_index;
            if (item_code < 0x0DU) {
                label_index = 0U;
            } else if (item_code < 0x14U) {
                label_index = 1U;
            } else if (item_code == 0x14U ||
                       (item_code >= 0x18U && item_code < 0x29U)) {
                label_index = 2U;
            }
            if (!label_index.has_value()) {
                return;
            }
            draw_command_overview_text(
                painter, 112, 48,
                field_menu_text_.item_detail_labels[*label_index]);
            const auto phase = field_active_runner_record_
                                   .inventory_phase_bytes[*selected_slot];
            if (phase < command_overview_detail_values_.size()) {
                painter.drawImage(120, 56,
                                  command_overview_detail_values_[phase]);
            }
        };
    const auto draw_armor_summary = [this, &painter] {
        const auto armor_index = field_active_runner_record_.equipped_armor_index;
        if (armor_index != 0U && armor_index <= field_menu_text_.armor_labels.size()) {
            const auto& armor = field_menu_text_.armor_labels[armor_index - 1U];
            for (std::size_t row = 0; row < armor.size(); ++row) {
                if (!armor[row].empty()) {
                    draw_command_overview_text(
                        painter, 128, 128 + static_cast<int>(row * 8U),
                        armor[row]);
                }
            }
        } else {
            draw_command_overview_text(painter, 152, 136,
                                       field_menu_text_.no_armor_label);
        }
        draw_command_overview_text(painter, 136, 144,
                                   field_menu_text_.weapons_label);
        draw_command_overview_text(painter, 136, 152,
                                   field_menu_text_.guns_label);
        const auto* armor = find_us_armor_definition(armor_index);
        if (armor == nullptr) {
            return;
        }
        if (armor->secondary_modifier < command_overview_damage_bars_.size()) {
            painter.drawImage(176, 144,
                              command_overview_damage_bars_[armor->secondary_modifier]);
        }
        if (armor->primary_modifier < command_overview_drain_bars_.size()) {
            painter.drawImage(176, 152,
                              command_overview_drain_bars_[armor->primary_modifier]);
        }
    };

    const auto& view = command_overview_.view();
    if (view.source_state.primary_page >= 2U) {
        std::string archetype;
        switch (field_active_runner_record_.archetype_index) {
        case 1U:
            archetype = field_menu_text_.archetype_names[2] + " " +
                        field_menu_text_.archetype_names[3];
            break;
        case 2U:
            archetype = field_menu_text_.archetype_names[0];
            break;
        case 3U:
            archetype = field_menu_text_.archetype_names[1];
            break;
        default:
            break;
        }
        if (!archetype.empty()) {
            draw_command_overview_text(painter, 128, 24, archetype);
        }
        draw_command_overview_text(painter, 128, 40,
                                   field_menu_text_.physical_label);
        draw_command_overview_text(painter, 128, 48,
                                   field_menu_text_.mental_label);
        draw_command_overview_text(
            painter, 208, 40,
            std::to_string(std::min<unsigned>(
                100U, static_cast<unsigned>(
                          field_active_runner_record_.delay_meter) * 10U)) +
                "%");
        draw_command_overview_text(
            painter, 208, 48,
            std::to_string(std::min<unsigned>(
                100U, static_cast<unsigned>(
                          field_active_runner_record_.behavior_meter) * 10U)) +
                "%");
        draw_item_detail_rows(field_active_runner_record_.equipped_item_code,
                              std::nullopt, false);
        draw_command_overview_text(painter, 112, 104,
                                   field_menu_text_.attack_label);
        draw_command_overview_text(painter, 112, 112,
                                   field_menu_text_.defense_label);
        if (command_overview_combat_bars_.attack_written &&
            command_overview_combat_bars_.attack_bar <
                command_overview_attack_bars_.size()) {
            painter.drawImage(
                144, 104,
                command_overview_attack_bars_[
                    command_overview_combat_bars_.attack_bar]);
        }
        if (command_overview_combat_bars_.defense_written &&
            command_overview_combat_bars_.defense_bar <
                command_overview_defense_bars_.size()) {
            painter.drawImage(
                144, 112,
                command_overview_defense_bars_[
                    command_overview_combat_bars_.defense_bar]);
        }
        draw_armor_summary();
    } else {
        const auto slot = view.source_state.secondary_page * 2U +
                          view.source_state.primary_page;
        if (slot < field_active_runner_record_.inventory_item_codes.size()) {
            const auto item_code =
                field_active_runner_record_.inventory_item_codes[slot];
            if (item_code >= 1U && item_code <= field_menu_text_.item_names.size()) {
                draw_command_overview_text(
                    painter, 128, 64,
                    field_menu_text_.item_names[item_code - 1U]);
                draw_command_overview_text(
                    painter, 128, 72,
                    field_menu_text_.item_categories[item_code - 1U]);
                draw_item_detail_rows(item_code, slot, true);
            }
        }
    }

    // The source inventory is an 8-cell, two-column/four-row grid. Direct
    // item aliases are already palette-decoded; no Genesis VRAM or palette
    // operation is reproduced here.
    constexpr std::array<std::array<int, 2>, 8> kInventoryIconPositions{{
        {{16, 120}}, {{56, 120}}, {{16, 144}}, {{56, 144}},
        {{16, 168}}, {{56, 168}}, {{16, 192}}, {{56, 192}},
    }};
    const auto icon_variant = command_overview_uses_alternate_icon_palette(
                                  field_active_runner_record_)
                                  ? 1U
                                  : 0U;
    for (std::size_t slot = 0;
         slot < field_active_runner_record_.inventory_item_codes.size();
         ++slot) {
        const auto item_code = field_active_runner_record_.inventory_item_codes[
            slot];
        if (item_code == 0U || item_code > 50U) {
            continue;
        }
        const auto& icon =
            command_overview_inventory_icons_[item_code - 1U][icon_variant];
        painter.drawImage(kInventoryIconPositions[slot][0],
                          kInventoryIconPositions[slot][1], icon);
    }

    const auto draw_equipment_border = [this, &painter](const std::size_t slot) {
        if (slot >= command_overview_selected_cells_.size() * 4U) {
            return;
        }
        const auto overlay = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
        painter.drawImage(
            8 + static_cast<int>((slot % 2U) * 40U),
            112 + static_cast<int>((slot / 2U) * 24U),
            command_overview_selected_cells_[overlay]);
    };
    if (field_active_runner_record_.equipped_inventory_slot >= 0) {
        draw_equipment_border(static_cast<std::size_t>(
            field_active_runner_record_.equipped_inventory_slot));
    }
    if (field_active_runner_record_.equipped_armor_index != 0U) {
        const auto armor_code = static_cast<std::uint8_t>(
            0x2BU + field_active_runner_record_.equipped_armor_index);
        for (std::size_t slot = 0;
             slot < field_active_runner_record_.inventory_item_codes.size();
             ++slot) {
            if (field_active_runner_record_.inventory_item_codes[slot] ==
                armor_code) {
                draw_equipment_border(slot);
                break;
            }
        }
    }

    if (view.source_state.primary_page < 2U) {
        const auto x = 8 + static_cast<int>(
                              view.source_state.primary_page * 40U);
        const auto y = 112 + static_cast<int>(
                              view.source_state.secondary_page * 24U);
        painter.drawImage(x, y, command_overview_cursor_);
    } else if (view.source_state.primary_page <= 6U) {
        const auto focus_index = static_cast<std::size_t>(
            view.source_state.primary_page - 2U);
        painter.drawImage(128 + static_cast<int>(focus_index * 24U), 184,
                          command_overview_focus_[focus_index]);
    }
    if (view.source_state.primary_page < field_menu_text_.command_labels.size()) {
        draw_command_overview_text(
            painter, 112, 200,
            field_menu_text_.command_labels[view.source_state.primary_page]);
    }
}

void FieldSceneRuntime::paint(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!ready_ || field_map_.isNull()) {
        return;
    }
    if (command_overview_.open()) {
        paint_command_overview(painter);
        return;
    }

    const auto& scene = state_.field_scene_buffers;
    const auto camera_x = std::max(
        0, static_cast<int>(scene.camera_x >> 4U));
    const auto camera_y = std::max(
        0, static_cast<int>(scene.camera_y >> 4U));
    const auto max_x = std::max(0, field_map_.width() - 320);
    const auto max_y = std::max(0, field_map_.height() - 224);
    const auto viewport_x = std::min(camera_x, max_x);
    const auto viewport_y = std::min(camera_y, max_y);
    painter.drawImage(0, 0, field_map_, viewport_x, viewport_y, 320, 224);
    paint_field_entity(painter);

    // 0x334C supplies the static chrome. 0x35D4/0x35F0/0x36FC/0x3674 stamp
    // the live face, item, and counter overlays at the $F000 nametable
    // cells. New-game keeps slots 1 and 2 inactive.
    painter.drawImage(0, 0, field_hud_);
    paint_field_hud_overlays(painter);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

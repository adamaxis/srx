#include "field_scene_runtime.hpp"

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
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

// The game loop seeds FFDDC4 with scene 2 before 0x03084. Loaded saves replace
// that scene reference with the saved row; both paths use the same typed scene
// resource selection below.
constexpr std::uint16_t kInitialFieldSceneReference = 2U;
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
constexpr std::string_view kCommandOverviewActionBackgroundAlias =
    "menu.field.action.background";
constexpr std::string_view kCommandOverviewActionFocusAliasPrefix =
    "menu.field.action.focus.";
constexpr std::string_view kCommandOverviewClipsFocusAliasPrefix =
    "menu.field.clips.focus.";
constexpr std::string_view kCommandOverviewClipsTradeFocusAliasPrefix =
    "menu.field.clips.trade.focus.";
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
// ROM 0x117A0 writes the 15x2 action row at the same lower command-panel
// origin used by the five command tabs.  0x117A4 overlays one 2x2 focus
// block at each of the four action-button centers.
constexpr int kCommandOverviewActionBackgroundX = 120;
constexpr int kCommandOverviewActionBackgroundY = 184;
constexpr std::array<int, 4> kCommandOverviewActionFocusX{{128, 152, 176,
                                                             200}};
constexpr int kCommandOverviewActionFocusY = 184;
// 0x11B58 and 0x11E7E use the same lower four-button row origin as the
// inventory action loop, but select from their own four-entry focus tables.
// Painting this opaque 120x16 row is intentional: it replaces the five
// command-tab blocks in the base panel, including the unused fifth position.
constexpr int kCommandOverviewClipsBackgroundX = 120;
constexpr int kCommandOverviewClipsBackgroundY = 184;
constexpr std::array<int, 4> kCommandOverviewClipsFocusX{{128, 152, 176,
                                                            200}};
constexpr int kCommandOverviewClipsFocusY = 184;
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
    : source_boundary_callback_(std::move(source_boundary)),
      field_dialogue_([this](const std::string_view boundary) {
          this->source_boundary(boundary);
      }) {
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

bool FieldSceneRuntime::run_field_exit_orchestrator(
    integration::FieldRuntimeCoordinatorState& runtime_state) {
    FieldExitOrchestratorState orchestrator_state{};
    // FF0110 is a graphics-owned gate in the ROM. The field controller has no
    // equivalent host flag yet, so leave it clear rather than treating a
    // controller button as a guessed graphics route.
    orchestrator_state.controller_flags = 0U;
    orchestrator_state.prompt_display_prepared = false;

    FieldExitOrchestratorCallbacks callbacks{};
    callbacks.select_mode_entry_resource = [this] {
        source_boundary("field.exit.select_mode_entry_resource");
    };
    callbacks.select_field_exit_prompt =
        [this, &runtime_state](FieldExitOrchestratorState& state) {
            FieldExitPromptState prompt_state{};
            prompt_state.world_flags = field_exit_world_flags_;
            const auto action = select_field_exit_prompt(
                static_cast<std::uint8_t>(runtime_state.scene_id),
                runtime_state.field_mode, field_exit_alternate_result_,
                prompt_state,
                {[this](FieldExitPromptSelection& selection,
                       FieldExitPromptState&) {
                     // 0x1F246/0x1F3C4 is blocking on the Genesis but is a
                     // normal host state transition here. The panel owns the
                     // subsequent controller edges without re-entering field
                     // movement or inventing a second dialogue machine.
                     (void)field_dialogue_.open_field_exit_prompt(selection);
                 },
                 [this] {
                     source_boundary("field.exit.hotel_options_unresolved");
                 }});
            state.caller_exit_status = prompt_state.caller_exit_status;
            if (action == FieldExitPromptAction::no_matching_scene) {
                source_boundary("field.exit.prompt_scene_unmatched");
            }
            return false;
        };

    // The remaining routes are deliberately explicit until their resource
    // selectors are connected. Returning true stops the converted
    // orchestrator at the same typed boundary instead of silently allowing a
    // scene transition with no source-backed presentation.
    callbacks.prepare_field_mode_exit = [this](FieldExitOrchestratorState& state) {
        source_boundary("field.exit.prepare_mode_exit_unresolved");
        state.caller_exit_status = -1;
        return true;
    };
    callbacks.select_field_mode_service = [this](FieldExitOrchestratorState& state) {
        source_boundary("field.exit.mode_service_unresolved");
        state.caller_exit_status = -1;
        return true;
    };
    callbacks.finalize_field_mode_exit = [this](FieldExitOrchestratorState& state) {
        source_boundary("field.exit.finalize_mode_exit_unresolved");
        state.caller_exit_status = -1;
        return true;
    };

    const auto action = ::shadowrun::ghidra_conversion::
        run_field_exit_orchestrator(orchestrator_state, callbacks);
    if (action == FieldExitOrchestratorAction::graphics_service_routed) {
        source_boundary("field.exit.graphics_service_unresolved");
    }
    return orchestrator_state.caller_exit_status != 0;
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
    // The extraction bakes color-zero as alpha 0 and loads it through the
    // V4 BMP alpha mask. Re-deriving transparency from black pixels here
    // would punch holes in the sprites' black outlines.
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
        !load_image(catalog, kCommandOverviewActionBackgroundAlias,
                    command_overview_action_background_) ||
        !load_image(catalog, "menu.field.font", command_overview_font_) ||
        !load_image(catalog, "menu.pocket.secretary.background",
                    pocket_secretary_background_) ||
        !load_image(catalog, "menu.field.attributes.background",
                    command_overview_attributes_background_) ||
        !load_image(catalog, "menu.field.posture.focus", posture_focus_)) {
        return false;
    }
    // The source inventory-box writer changes only the box outline cells;
    // its extracted plane-sized bitmap carries color-zero as alpha 0. The
    // host already has the opaque command shell underneath, so preserve that
    // shell instead of letting the overlay's black canvas erase it.
    const auto load_sprite = [this, &catalog](const std::string& alias,
                                               QImage& image) {
        if (!load_image(catalog, alias, image)) {
            return false;
        }
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
    for (std::size_t index = 0;
         index < command_overview_action_focus_.size(); ++index) {
        // This is a source name-table block, not a transparent SAT sprite.
        // The extractor preserves color-zero pixels as opaque because the
        // Genesis writes the complete 16x16 action focus block over the
        // action row.  Keep that composition when presenting the direct
        // host image.
        if (!load_image(catalog,
                        std::string(kCommandOverviewActionFocusAliasPrefix) +
                            std::to_string(index),
                        command_overview_action_focus_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_clips_focus_.size(); ++index) {
        // 0x11B58 passes CD43A to the same complete 2x2 name-table writer.
        // These are opaque source blocks, not transparent host highlights.
        if (!load_image(catalog,
                        std::string(kCommandOverviewClipsFocusAliasPrefix) +
                            std::to_string(index),
                        command_overview_clips_focus_[index]) ||
            !load_image(
                catalog,
                std::string(kCommandOverviewClipsTradeFocusAliasPrefix) +
                    std::to_string(index),
                command_overview_clips_trade_focus_[index])) {
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
            // The 0xC0B2 walking lists carry an 0x8008 marker, an 0x0200
            // auxiliary increment, and eight frame pointers. Mirrored
            // direction pairs share lists, but the typed state keeps one
            // frame sequence per heading slot.
            const EntitySpriteDescriptorList walk_list{
                static_cast<std::int16_t>(0x8008), 0x0200U,
                {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U}};
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
    field_runtime_callbacks_.run_field_exit_orchestrator =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            return run_field_exit_orchestrator(runtime);
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

    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_action_message_.clear();
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    command_overview_clips_.close();
    command_overview_cyberware_active_ = false;
    command_overview_cyberware_ = CyberwareMenuModel{};

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
        static_cast<std::uint8_t>(active_runner_record().flags & 0xFFU),
        static_cast<std::int8_t>(active_runner_record().archetype_index)};
    const CommandOverviewPreparationInput preparation_input{};
    const CommandOverviewHudPanelInput hud_panel_input{
        active_runner_record().record_type,
        active_runner_record().command_overview_panel_parameter};
    command_overview_.begin(
        input, preparation_input, hud_panel_input,
        {[this](const std::string_view boundary) { source_boundary(boundary); },
         command_asset_available,
         [this] { return active_runner_record().equipped_item_code; },
         [](const CommandOverviewPreparationResult&) {},
         [](const CommandOverviewHudPanelPlan&) {},
         [](const CommandOverviewActionRefreshResult&) {}});
    refresh_command_overview_combat_bars();
    refresh_command_overview_item_detail();
}

void FieldSceneRuntime::refresh_field_hud_runner_slot() {
    if (field_hud_slots_.empty()) {
        return;
    }
    auto& slot = field_hud_slots_[0];
    slot.flags_byte = static_cast<std::uint8_t>(
        active_runner_record().flags >> 8U);
    slot.equipped_inventory_slot =
        active_runner_record().equipped_inventory_slot;
    slot.equipped_item_code = active_runner_record().equipped_item_code;
    slot.ammo_rounds = active_runner_record().ammo_rounds;
}

void FieldSceneRuntime::initialize_field_party_roster(
    const integration::StartupIntegrationState& startup_state) {
    const auto source = field_party_roster_.initialize(startup_state);
    if (source == FieldPartyRosterRuntime::Source::unavailable) {
        source_boundary("field.bootstrap.party_records_missing");
    } else if (source == FieldPartyRosterRuntime::Source::restored_save) {
        source_boundary("field.bootstrap.party_records.restored_save");
    } else {
        source_boundary("field.bootstrap.party_records.new_game");
    }
}

void FieldSceneRuntime::begin_command_overview_inventory_action() {
    const auto& view = command_overview_.view();
    if (view.source_state.primary_page >= 2U) {
        source_boundary("field.command_overview.inventory_action.invalid_route");
        return;
    }

    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_inventory_action_.column =
        view.source_state.primary_page;
    command_overview_inventory_action_.row =
        view.source_state.secondary_page;
    const auto result = advance_inventory_action_loop(
        active_runner_record(), command_overview_inventory_action_,
        InventoryActionInput{false, false, true, false, false});
    if (result.event == InventoryActionEvent::opened && result.item.has_value()) {
        command_overview_selected_inventory_item_ = result.item;
        command_overview_inventory_action_active_ = true;
        source_boundary("field.command_overview.inventory_action.opened");
        return;
    }
    if (result.event == InventoryActionEvent::empty_cell) {
        source_boundary("field.command_overview.inventory_action.empty_cell");
    } else {
        source_boundary("field.command_overview.inventory_action.not_opened");
    }
    command_overview_inventory_action_active_ = false;
    command_overview_selected_inventory_item_.reset();
}

void FieldSceneRuntime::begin_pocket_secretary() {
    if (!command_overview_.open()) {
        return;
    }
    // ROM 0xC2A6 with FFF101 selecting the five-entry field variant. The
    // six-entry extended form is a different caller; the field command
    // route uses this one until the video identifies the extended case.
    initialize_pocket_secretary_menu(
        pocket_secretary_, false,
        {[this] { source_boundary("field.pocket_secretary.transfer_menu_assets"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_grid"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_text"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_numbers"); },
         {}, {}});
    source_boundary("field.pocket_secretary.opened");
}

void FieldSceneRuntime::begin_command_overview_clips() {
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page != 2U) {
        source_boundary("field.command_overview.clips.invalid_route");
        return;
    }
    command_overview_clips_.open();
    source_boundary("field.command_overview.clips.opened");
}

void FieldSceneRuntime::begin_command_overview_cyberware() {
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page != 4U) {
        source_boundary("field.command_overview.cyberware.invalid_route");
        return;
    }
    // ROM 0x11950 reads the active record's +0x90 word. A zero word selects
    // NONE; otherwise visible bits are emitted from 0 through 14, compacted
    // into successive rows. Bit 15 alone is nonzero but names no row.
    command_overview_cyberware_ = build_cyberware_menu(
        active_runner_record().cyberware_flags);
    command_overview_cyberware_active_ = true;
    source_boundary("field.command_overview.cyberware.opened");
}

void FieldSceneRuntime::advance_command_overview_clips(
    const std::uint16_t controller_events) {
    const auto* item = find_us_item_definition(
        active_runner_record().equipped_item_code);
    const auto capacity = item == nullptr ? std::uint8_t{0}
                                           : item->source_combat_factor;
    const auto event = command_overview_clips_.advance(
        controller_events, field_party_roster_.records(),
        field_party_roster_.active_slot(), capacity);
    switch (event) {
    case ClipsMenuEvent::selection_moved:
        source_boundary("field.command_overview.clips.selector_moved");
        break;
    case ClipsMenuEvent::confirmation_opened:
        source_boundary(
            "field.command_overview.clips.reload_confirmation_opened");
        break;
    case ClipsMenuEvent::confirmation_toggled:
        source_boundary(
            command_overview_clips_.reload_confirmation_yes()
                ? "field.command_overview.clips.reload_confirmation_yes"
                : "field.command_overview.clips.reload_confirmation_no");
        break;
    case ClipsMenuEvent::reloaded:
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.reloaded");
        break;
    case ClipsMenuEvent::trade_opened:
        source_boundary("field.command_overview.clips.trade.opened");
        if (command_overview_clips_.trade_recipient_record() >=
                FieldPartyRosterRuntime::kSlotCount ||
            field_party_roster_
                .name(command_overview_clips_.trade_recipient_record())
                .empty()) {
            source_boundary(
                "field.command_overview.clips.trade.recipient_name_missing");
        }
        break;
    case ClipsMenuEvent::trade_selection_moved:
        source_boundary(
            "field.command_overview.clips.trade.selector_moved");
        break;
    case ClipsMenuEvent::trade_quantity_changed:
        source_boundary(
            "field.command_overview.clips.trade.quantity_changed");
        break;
    case ClipsMenuEvent::trade_recipient_changed:
        source_boundary(
            "field.command_overview.clips.trade.recipient_changed");
        break;
    case ClipsMenuEvent::trade_transferred:
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary(
            "field.command_overview.clips.trade.transferred");
        break;
    case ClipsMenuEvent::trade_closed:
        source_boundary("field.command_overview.clips.trade.closed");
        break;
    case ClipsMenuEvent::distributed:
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.distributed");
        break;
    case ClipsMenuEvent::message_shown:
        switch (command_overview_clips_.message()) {
        case ClipsMenuMessage::out_of_ammo:
            source_boundary("field.command_overview.clips.out_of_ammo");
            break;
        case ClipsMenuMessage::item_does_not_use_clips:
            source_boundary(
                "field.command_overview.clips.item_does_not_use_clips");
            break;
        case ClipsMenuMessage::no_party_members:
            source_boundary("field.command_overview.clips.no_party_members");
            break;
        case ClipsMenuMessage::reload_success:
        case ClipsMenuMessage::exchange_complete:
        case ClipsMenuMessage::none:
            source_boundary("field.command_overview.clips.message_shown");
            break;
        }
        break;
    case ClipsMenuEvent::message_acknowledged:
        source_boundary("field.command_overview.clips.message_acknowledged");
        break;
    case ClipsMenuEvent::closed:
        // `closed` also describes leaving the nested reload prompt. The
        // runtime remains on the parent Clips screen unless the top-level
        // selector itself was cancelled.
        if (!command_overview_clips_.is_open()) {
            source_boundary("field.command_overview.clips.returned");
        } else {
            source_boundary(
                "field.command_overview.clips.reload_confirmation_closed");
        }
        break;
    case ClipsMenuEvent::no_change:
        break;
    }
    refresh_field_hud_runner_slot();
    refresh_command_overview_combat_bars();
}

void FieldSceneRuntime::dispatch_command_overview_inventory_action(
    const InventoryActionResult& result) {
    if (!result.item.has_value() || !result.choice.has_value()) {
        source_boundary("field.command_overview.inventory_action.missing_dispatch");
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }

    command_overview_selected_inventory_item_ = result.item;
    const auto choice = *result.choice;
    if (choice == InventoryActionChoice::equip_or_use) {
        const auto branch = result.branch.value_or(
            classify_selected_item(result.item->item_code, true));
        if (branch == SelectedItemBranch::weapon_or_gear ||
            branch == SelectedItemBranch::armor) {
            const auto item_factor = [](const std::uint8_t item_code) {
                const auto* definition = find_us_item_definition(item_code);
                return definition == nullptr ? std::uint8_t{0}
                                             : definition->source_combat_factor;
            };
            const auto mutation = toggle_selected_equipment(
                active_runner_record(), *result.item,
                item_factor(active_runner_record().equipped_item_code),
                item_factor(result.item->item_code));
            if (mutation == EquipmentMutation::invalid_selection ||
                mutation == EquipmentMutation::rejected_item_action) {
                source_boundary(
                    "field.command_overview.inventory_action.equipment_rejected");
            } else {
                source_boundary(
                    "field.command_overview.inventory_action.equipment_mutated");
                refresh_field_hud_runner_slot();
                refresh_command_overview_combat_bars();
                refresh_command_overview_item_detail();
            }
        } else if (branch == SelectedItemBranch::first_aid) {
            // 0x12298/0x1253E owns target selection and the typed First Aid
            // effect. The common item loop is connected, but this active
            // record has not yet been given the party-target list it needs.
            source_boundary(
                "field.command_overview.inventory_action.first_aid_pending");
        } else if (branch == SelectedItemBranch::automatic_use) {
            source_boundary(
                "field.command_overview.inventory_action.automatic_use_pending");
        } else {
            source_boundary(
                "field.command_overview.inventory_action.item_action_pending");
        }
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }

    if (choice == InventoryActionChoice::discard) {
        if (!can_discard_selected_item(*result.item)) {
            command_overview_action_message_ =
                field_menu_text_.action_cannot_discard;
            command_overview_inventory_action_active_ = true;
            source_boundary(
                "field.command_overview.inventory_action.discard_rejected");
            return;
        }
        // ROM 0x12634 enters the source two-choice prompt before 0x1275C
        // clears possession. The converted MenuConfirmationState is retained
        // as typed host state; this is advanced one source frame at a time in
        // the Qt loop instead of blocking the event loop.
        command_overview_discard_confirmation_ = MenuConfirmationState{};
        command_overview_discard_confirmation_active_ = true;
        command_overview_inventory_action_active_ = true;
        source_boundary(
            "field.command_overview.inventory_action.discard_confirmation_opened");
        return;
    }

    if (choice == InventoryActionChoice::trade) {
        if (!can_trade_selected_item(result.item->item_code)) {
            command_overview_action_message_ = field_menu_text_.action_cannot_trade;
            command_overview_inventory_action_active_ = true;
            source_boundary(
                "field.command_overview.inventory_action.trade_rejected");
            return;
        }
        // The source 0x12836 roster scan and 0x129FE transfer are converted,
        // but the Qt field session currently exposes Joshua alone. Do not
        // fabricate a recipient; leave this as an explicit source boundary.
        source_boundary(
            "field.command_overview.inventory_action.trade_recipient_pending");
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }

    command_overview_inventory_action_active_ = false;
    command_overview_selected_inventory_item_.reset();
}

void FieldSceneRuntime::advance_command_overview_discard_confirmation(
    const std::uint16_t controller_events) {
    if (((controller_events & 0x0400U) != 0U ||
         (controller_events & 0x0800U) != 0U) &&
        (controller_events & 0x0C00U) != 0x0C00U) {
        // ROM 0x12704 calls the shared Yes/No writer for either horizontal
        // edge; the source toggles its binary choice rather than assigning a
        // direction-specific value. The prompt starts on No.
        command_overview_discard_confirmation_.selected_option =
            command_overview_discard_confirmation_.selected_option == 0U ? 1U
                                                                          : 0U;
        source_boundary(
            command_overview_discard_confirmation_.selected_option == 0U
                ? "field.command_overview.discard_confirmation.select_yes"
                : "field.command_overview.discard_confirmation.select_no");
        return;
    }
    if ((controller_events & 0x1000U) != 0U ||
        (controller_events & 0x8000U) != 0U) {
        command_overview_discard_confirmation_active_ = false;
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        source_boundary(
            "field.command_overview.discard_confirmation.cancelled");
        return;
    }
    if ((controller_events & 0x4000U) == 0U) {
        return;
    }

    const auto selected = command_overview_selected_inventory_item_;
    command_overview_discard_confirmation_active_ = false;
    command_overview_inventory_action_active_ = false;
    command_overview_selected_inventory_item_.reset();
    if (!selected.has_value()) {
        source_boundary(
            "field.command_overview.discard_confirmation.missing_selection");
        return;
    }
    if (command_overview_discard_confirmation_.selected_option == 0U) {
        const auto mutation = discard_selected_item(
            active_runner_record(), *selected);
        if (mutation == EquipmentMutation::discarded) {
            refresh_field_hud_runner_slot();
            refresh_command_overview_combat_bars();
            refresh_command_overview_item_detail();
            source_boundary(
                "field.command_overview.inventory_action.discarded");
        } else {
            source_boundary(
                "field.command_overview.inventory_action.discard_failed");
        }
    } else {
        source_boundary(
            "field.command_overview.discard_confirmation.selected_no");
    }
}

void FieldSceneRuntime::advance_command_overview_inventory_action(
    const std::uint16_t controller_events) {
    if (command_overview_discard_confirmation_active_) {
        advance_command_overview_discard_confirmation(controller_events);
        return;
    }
    if (!command_overview_action_message_.empty()) {
        if ((controller_events & 0x1000U) != 0U ||
            (controller_events & 0x4000U) != 0U ||
            (controller_events & 0x8000U) != 0U) {
            command_overview_action_message_.clear();
            command_overview_inventory_action_active_ = false;
            command_overview_selected_inventory_item_.reset();
            source_boundary(
                "field.command_overview.inventory_action.message_acknowledged");
        }
        return;
    }
    if (!command_overview_inventory_action_active_) {
        return;
    }

    const InventoryActionInput input{
        (controller_events & 0x0400U) != 0U,
        (controller_events & 0x0800U) != 0U,
        (controller_events & 0x4000U) != 0U,
        (controller_events & 0x1000U) != 0U,
        (controller_events & 0x8000U) != 0U};
    const auto result = advance_inventory_action_loop(
        active_runner_record(), command_overview_inventory_action_, input);
    switch (result.event) {
    case InventoryActionEvent::action_moved:
        source_boundary(
            "field.command_overview.inventory_action.selector_moved");
        break;
    case InventoryActionEvent::closed:
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        source_boundary("field.command_overview.inventory_action.returned");
        break;
    case InventoryActionEvent::empty_cell:
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        source_boundary(
            "field.command_overview.inventory_action.empty_cell_after_open");
        break;
    case InventoryActionEvent::dispatched:
        dispatch_command_overview_inventory_action(result);
        break;
    case InventoryActionEvent::opened:
    case InventoryActionEvent::no_change:
        break;
    }
}

bool FieldSceneRuntime::bootstrap(
    const integration::StartupIntegrationState& startup_state) {
    ready_ = false;
    failure_.clear();
    state_ = integration::FieldSessionBootstrapState{};
    world_frame_state_ = WorldFrameState{};
    viewport_state_ = FieldViewportState{};

    if (!startup_state.field_session_source_assets.has_value()) {
        failure_ = "field-session source records are unavailable";
        source_boundary("field.bootstrap.source_records_missing");
        return false;
    }

    const auto* restored_field_state =
        startup_state.restored_field_state.has_value()
            ? &*startup_state.restored_field_state
            : nullptr;
    const auto& assets = *startup_state.field_session_source_assets;
    const auto scene_reference = restored_field_state != nullptr
                                     ? restored_field_state->world_mode
                                     : kInitialFieldSceneReference;
    const auto scene_input = integration::make_field_scene_input(
        assets, scene_reference);
    if (!scene_input.has_value()) {
        failure_ = "field scene resources are not extracted for scene " +
                   std::to_string(scene_reference);
        source_boundary("field.bootstrap.field_scene_resources_missing");
        return false;
    }
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
    field_exit_alternate_result_ = 0;
    field_exit_world_flags_ = 0U;
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
    field_dialogue_.close();
    field_party_roster_.clear();
    field_menu_text_ = FieldMenuTextCatalog{};
    command_overview_resource_variant_ = ResourceVariantState{};
    command_overview_combat_bars_ = EquipmentCombatBarResult{};
    command_overview_item_detail_ = ItemActionDetailResult{};
    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_action_message_.clear();
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    command_overview_clips_.close();
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
    input_.field_scene = *scene_input;
    if (restored_field_state != nullptr) {
        std::array<SceneRunnerPosition, 3> restored_positions{};
        for (std::size_t index = 0; index < restored_positions.size();
             ++index) {
            restored_positions[index] = SceneRunnerPosition{
                restored_field_state->runner_positions[index].x,
                restored_field_state->runner_positions[index].y};
        }
        input_.restored_field_scene = integration::RestoredFieldSceneInput{
            restored_field_state->world_mode,
            restored_field_state->camera_x,
            restored_field_state->camera_y,
            restored_positions};
    }

    field_entity_ = FieldEntityPresentation{};
    initialize_field_party_roster(startup_state);
    field_entity_.active = field_party_roster_.available();
    field_entity_.world_position =
        input_.field_scene.source_metadata.initial_field_records[0];
    if (field_entity_.active) {
        field_joshua_archetype_selection_ = static_cast<std::size_t>(
            startup_state.joshua_archetype.selected);
        field_entity_.projection.sprite_offset_y =
            active_runner_record()
                .projection_word_08;
    }
    // ROM 0x44BC installs behavior byte 0x05 in a field entity before the
    // source 0xC64C/0x391A handoff. Keep the typed state initialized before
    // field-buffer preparation can invoke 0xC7F8.
    field_entity_behavior_flags_ = field_entity_.active ? 0x05U : 0U;
    initialize_field_entity_selection_state();
    if (field_entity_.active) {
        const auto& record = active_runner_record();
        field_hud_slots_[0].flags_byte =
            static_cast<std::uint8_t>(record.flags >> 8U);
        field_hud_slots_[0].equipped_inventory_slot =
            record.equipped_inventory_slot;
        field_hud_slots_[0].equipped_item_code = record.equipped_item_code;
        field_hud_slots_[0].ammo_rounds = record.ammo_rounds;
        // The extracted face/bar/item families follow the archetype record
        // that produced them, not the field slot number.
        field_hud_slots_[0].asset_family_index = static_cast<std::uint8_t>(
            field_joshua_archetype_selection_);
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

    // Dialogue is optional during initial field bootstrap because the player
    // does not start inside an NPC prompt. Load it once here so the later
    // 0x1F65A callback can bind direct assets without rebuilding a catalog on
    // every prompt or reaching back into the ROM.
    (void)field_dialogue_.load_assets(startup_state.asset_catalog.catalog);

    const auto field_map_alias =
        std::string("field.maps.mode.") + std::to_string(scene_reference);
    if (!load_image(startup_state.asset_catalog.catalog, field_map_alias,
                    field_map_) ||
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
        // Load consecutive extracted frames until the catalog stops naming
        // them; every direction must carry at least one walking frame.
        std::size_t loaded_frames = 0;
        for (;;) {
            const auto walk_alias =
                std::string(kFieldEntityWalkAliasPrefix) +
                std::to_string(direction) + "." +
                std::to_string(loaded_frames);
            if (startup_state.asset_catalog.catalog.find(walk_alias) ==
                nullptr) {
                break;
            }
            QImage walk_image;
            int walk_origin_x = 0;
            int walk_origin_y = 0;
            if (!load_sprite_image(
                    startup_state.asset_catalog.catalog, walk_alias,
                    walk_image, walk_origin_x, walk_origin_y)) {
                source_boundary("field.bootstrap.entity_assets_missing");
                return false;
            }
            field_entity_walk_images_[direction].push_back(
                std::move(walk_image));
            field_entity_walk_origin_x_[direction].push_back(walk_origin_x);
            field_entity_walk_origin_y_[direction].push_back(walk_origin_y);
            ++loaded_frames;
        }
        if (loaded_frames == 0) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
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

    if (restored_field_state != nullptr) {
        // The restore path's live runner positions are separate from the
        // startup profile defaults. The conversion bootstrap publishes them
        // in typed scene metadata; the presenter consumes that state here.
        field_entity_.world_position =
            state_.field_scene_buffers.source_metadata.initial_field_records[0];
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
        // The source keeps the walking pose only while a direction is held.
        // When the controller goes idle, the heading animation returns to
        // the directional idle list instead of looping the walk cycle while
        // the runner stands still.
        if (field_entity_walking_) {
            apply_field_entity_animation(false);
        }
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
    if (field_dialogue_.open()) {
        // The source prompt loop waits for a high-byte controller edge. The
        // host keeps that loop non-blocking and suspends field movement while
        // the common dialogue panel owns the current prompt.
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        const auto result = field_dialogue_.advance_controller_events(
            field_controller_sample_state_.events);
        if (result == FieldDialogueAdvanceResult::returned_to_field) {
            source_boundary("field.dialogue.returned_to_field");
        }
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
        if (command_overview_clips_.is_open()) {
            advance_command_overview_clips(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_cyberware_active_) {
            // ROM 0x11950 waits for one controller event after drawing the
            // +0x90 bit rows, refreshes the shared menu row, and returns.
            // Keep the wait non-blocking in the Qt event loop.
            if ((field_controller_sample_state_.events & 0xFF00U) != 0U) {
                command_overview_cyberware_active_ = false;
                source_boundary("field.command_overview.cyberware.returned");
            }
            return;
        }
        if (command_overview_.take_action_dialog()) {
            // 0x11506 armed the 0x119CE posture adjustment dialog. Its
            // non-blocking host form mirrors the source loop: Down raises,
            // Left lowers, Start exits, and 0x11AE4 clamps to 0..4.
            posture_dialog_active_ = true;
            source_boundary("field.posture.dialog_opened");
        }
        if (posture_dialog_active_) {
            const auto events = field_controller_sample_state_.events;
            if ((events & 0x0800U) != 0U) {
                adjust_runner_posture(active_runner_record(), 1);
                refresh_command_overview_combat_bars();
                source_boundary("field.posture.dialog.raised");
            } else if ((events & 0x0400U) != 0U) {
                adjust_runner_posture(active_runner_record(), -1);
                refresh_command_overview_combat_bars();
                source_boundary("field.posture.dialog.lowered");
            } else if ((events & 0x8000U) != 0U) {
                posture_dialog_active_ = false;
                refresh_command_overview_combat_bars();
                source_boundary("field.posture.dialog.closed");
            }
            return;
        }
        if (pocket_secretary_.open) {
            const auto event = apply_pocket_secretary_input(
                pocket_secretary_, field_controller_sample_state_.events,
                {[this] {
                     source_boundary(
                         "field.pocket_secretary.transfer_menu_assets");
                 },
                 [this] {
                     source_boundary("field.pocket_secretary.render_menu_grid");
                 },
                 [this] {
                     source_boundary("field.pocket_secretary.render_menu_text");
                 },
                 [this] {
                     source_boundary(
                         "field.pocket_secretary.render_menu_numbers");
                 },
                 [this](const std::uint8_t selection) {
                     source_boundary("field.pocket_secretary.handler");
                     (void)selection;
                     return true;
                 },
                 [this] {
                     source_boundary("field.pocket_secretary.mode_action");
                 }});
            if (event == PocketSecretaryMenuEvent::exited) {
                pocket_secretary_.open = false;
                source_boundary("field.pocket_secretary.exited");
            }
            return;
        }
        if (command_overview_inventory_action_active_) {
            advance_command_overview_inventory_action(
                field_controller_sample_state_.events);
            refresh_command_overview_combat_bars();
            refresh_command_overview_item_detail();
            return;
        }
        const auto was_open = command_overview_.open();
        command_overview_.advance(field_controller_sample_state_.events);
        if (was_open && !command_overview_.open()) {
            // The source command loop returns through the field coordinator's
            // display-service tail before the next field pass. The converted
            // coordinator publishes FFDFE8=FFFF while dispatching Start, so
            // the host must publish the corresponding completion here; if it
            // remains set, the coordinator's 0x0AEA guard permanently blocks
            // all later Start edges and the menu can only open once.
            state_.display_service_word = 0U;
            field_runtime_state_.display_service_word = 0U;
            source_boundary("field.command_overview.returned_to_field");
            command_overview_inventory_action_ = InventoryActionMenuState{};
            command_overview_selected_inventory_item_.reset();
            command_overview_discard_confirmation_ = MenuConfirmationState{};
            command_overview_action_message_.clear();
            command_overview_inventory_action_active_ = false;
            command_overview_discard_confirmation_active_ = false;
            command_overview_clips_.close();
            command_overview_cyberware_active_ = false;
            command_overview_cyberware_ = CyberwareMenuModel{};
            pocket_secretary_.open = false;
            posture_dialog_active_ = false;
        } else if (command_overview_.take_action_dispatch_pending()) {
            if (command_overview_.view().source_state.primary_page < 2U) {
                begin_command_overview_inventory_action();
            } else if (command_overview_.view().source_state.primary_page ==
                       2U) {
                begin_command_overview_clips();
            } else if (command_overview_.view().source_state.primary_page ==
                       6U) {
                begin_pocket_secretary();
            } else if (command_overview_.view().source_state.primary_page ==
                       4U) {
                begin_command_overview_cyberware();
            } else {
                // The converted command bridge has identified the remaining
                // action route, but its handler still needs a source-verified
                // native target (Magic and Attributes/Skills).
                source_boundary(
                    "field.command_overview.non_inventory_action_pending");
            }
        }
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

bool FieldSceneRuntime::seek_command_overview_page(const std::uint8_t page) {
    if (!command_overview_.open() || page > 6U) {
        return false;
    }
    // The converted primary-page advance wraps the seven-page selector, so
    // a d-pad left edge moves one page toward zero. Port byte bit 2 is the
    // left direction in the sampled controller word (0x0400 in the expanded
    // events consumed by the bridge).
    const auto current = command_overview_.view().source_state.primary_page;
    const auto steps = (current + 7U - page) % 7U;
    for (std::uint8_t step = 0; step < steps; ++step) {
        set_controller_port_byte(0x04U);
        advance_frame();
        set_controller_port_byte(0U);
        advance_frame();
    }
    return command_overview_.view().source_state.primary_page == page;
}

void FieldSceneRuntime::paint_field_entity(QPainter& painter) const {
    if (!field_entity_.active || !field_entity_.visible) {
        return;
    }
    const auto direction = static_cast<std::size_t>(
        field_entity_.direction % field_entity_idle_images_.size());
    const auto walking = field_entity_walking_;
    const auto walk_frames = field_entity_walk_images_[direction].size();
    const auto frame = walk_frames == 0
        ? 0U
        : bound_field_walk_animation_frame(field_entity_animation_.frame) %
              walk_frames;
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
            return overlay.asset_index < field_hud_faces_.size()
                ? &field_hud_faces_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::physical_bar:
            return overlay.asset_index < field_hud_vertical_counters_.size()
                ? &field_hud_vertical_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::mental_bar:
            return overlay.asset_index < field_hud_horizontal_counters_.size()
                ? &field_hud_horizontal_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::item:
            return overlay.asset_index < field_hud_items_.size()
                ? &field_hud_items_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::unarmed_item:
            return overlay.asset_index < field_hud_unarmed_items_.size()
                ? &field_hud_unarmed_items_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::ammo:
            return overlay.asset_index < field_hud_ammo_counters_.size()
                ? &field_hud_ammo_counters_[overlay.asset_index]
                : nullptr;
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

    const auto& record = active_runner_record();
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
    if (slot >= active_runner_record().inventory_item_codes.size()) {
        return;
    }
    ItemActionDetailInput input{};
    input.item_code = active_runner_record().inventory_item_codes[slot];
    for (std::size_t index = 0;
         index < active_runner_record().inventory_phase_bytes.size();
         ++index) {
        input.phase_bytes[index] =
            active_runner_record().inventory_phase_bytes[index];
    }
    // The ninth byte in the source phase span is active record +0x76, the
    // one-based armor ordinal. Preserve it as typed data for 0x12A8C's
    // item-detail value selection.
    input.phase_bytes[8] = active_runner_record().equipped_armor_index;
    input.phase_slot = 8U;
    command_overview_item_detail_ = render_item_action_details(
        input, us_item_catalog(), command_overview_resource_variant_);
}

void FieldSceneRuntime::paint_command_overview_clips(
    QPainter& painter) const {
    // ROM 0x11B58/0x11E7E clears the center command plane through the shared
    // menu-grid writer. Preserve the runner/inventory/status shell around it,
    // but do not let the higher-mode Attributes/Skills rows leak through.
    painter.fillRect(96, 16, 168, 144, Qt::black);

    draw_command_overview_text(painter, 136, 40,
                               field_menu_text_.clips_title);
    draw_command_overview_text(painter, 136, 48,
                               field_menu_text_.clips_separator);

    const auto draw_lines = [this, &painter](const auto& lines,
                                              const int x,
                                              const int y) {
        for (std::size_t index = 0; index < lines.size(); ++index) {
            draw_command_overview_text(
                painter, x, y + static_cast<int>(index * 8U), lines[index]);
        }
    };

    // This opaque source row replaces all five ordinary command-tab cells.
    // It contains the four action positions plus the source trailing blank
    // cell, so the Clips screen cannot accidentally retain Pocket Secretary.
    painter.drawImage(kCommandOverviewClipsBackgroundX,
                      kCommandOverviewClipsBackgroundY,
                      command_overview_action_background_);

    if (command_overview_clips_.reload_confirmation_open()) {
        draw_lines(field_menu_text_.clips_reload_prompt, 120, 72);
        draw_command_overview_text(
            painter, 112, 200,
            command_overview_clips_.reload_confirmation_yes()
                ? field_menu_text_.action_choice_yes
                : field_menu_text_.action_choice_no);
        return;
    }

    if (command_overview_clips_.trade_open()) {
        // ROM 0x11E34 renders the source trade prompt, then 0x1209A updates
        // the recipient name when the nested selector moves. The source
        // D1B71 ???? text is retained only as the fallback for a record whose
        // projected +0x58 name field is empty.
        draw_command_overview_text(painter, 144, 72,
                                   field_menu_text_.clips_trade_prompt[0]);
        draw_command_overview_text(painter, 144, 80,
                                   field_menu_text_.clips_trade_prompt[1]);
        const auto recipient = command_overview_clips_.trade_recipient_record();
        if (recipient < FieldPartyRosterRuntime::kSlotCount &&
            !field_party_roster_.name(recipient).empty()) {
            draw_command_overview_text(
                painter, 120, 88, field_party_roster_.name(recipient));
        } else {
            draw_command_overview_text(painter, 120, 88,
                                       field_menu_text_.clips_trade_prompt[2]);
        }
        draw_command_overview_text(
            painter, 200, 72,
            std::to_string(command_overview_clips_.trade_quantity()));
        const auto selection = command_overview_clips_.trade_selection();
        if (selection >= static_cast<std::uint8_t>(ClipsCommand::reload) &&
            selection <= static_cast<std::uint8_t>(ClipsCommand::cancel)) {
            const auto focus_index = static_cast<std::size_t>(selection - 2U);
            painter.drawImage(kCommandOverviewClipsFocusX[focus_index],
                              kCommandOverviewClipsFocusY,
                              command_overview_clips_trade_focus_[focus_index]);
            draw_command_overview_text(
                painter, 112, 200,
                field_menu_text_.clips_trade_commands[focus_index]);
        }
        return;
    }

    switch (command_overview_clips_.message()) {
    case ClipsMenuMessage::out_of_ammo:
        draw_command_overview_text(
            painter, 112, 80,
            field_party_roster_.name(0U).empty()
                ? field_menu_text_.runner_names[0]
                : field_party_roster_.name(0U));
        draw_lines(field_menu_text_.clips_out_of_ammo, 112, 88);
        return;
    case ClipsMenuMessage::item_does_not_use_clips:
        draw_lines(field_menu_text_.clips_item_no_clips, 136, 80);
        return;
    case ClipsMenuMessage::reload_success:
        draw_lines(field_menu_text_.clips_reload_success, 112, 80);
        return;
    case ClipsMenuMessage::exchange_complete:
        draw_lines(field_menu_text_.clips_exchange_complete, 112, 80);
        return;
    case ClipsMenuMessage::no_party_members:
        draw_lines(field_menu_text_.clips_no_party, 120, 72);
        return;
    case ClipsMenuMessage::none:
        break;
    }

    draw_command_overview_text(painter, 136, 72,
                               field_menu_text_.clips_counter_label);
    draw_command_overview_text(
        painter, 208, 72,
        std::to_string(active_runner_record().ammo_clips));
    const auto selection = command_overview_clips_.selection();
    if (selection >= static_cast<std::uint8_t>(ClipsCommand::reload) &&
        selection <= static_cast<std::uint8_t>(ClipsCommand::cancel)) {
        const auto focus_index = static_cast<std::size_t>(selection - 2U);
        painter.drawImage(kCommandOverviewClipsFocusX[focus_index],
                          kCommandOverviewClipsFocusY,
                          command_overview_clips_focus_[focus_index]);
        draw_command_overview_text(
            painter, 112, 200,
            field_menu_text_.clips_commands[focus_index]);
    }
}

void FieldSceneRuntime::paint_command_overview(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!command_overview_.open()) {
        return;
    }
    if (command_overview_cyberware_active_) {
        // The 0x11950 destinations are offsets in a 64-column name plane:
        // C1A2 -> (136,24), C328/C31C -> (160/112,48), and each emitted
        // record row advances by 0x100 bytes (16 host pixels vertically).
        painter.drawImage(0, 0, command_overview_background_);
        draw_command_overview_text(
            painter, 136, 24, field_menu_text_.cyberware_title_lines[0]);
        draw_command_overview_text(
            painter, 136, 32, field_menu_text_.cyberware_title_lines[1]);
        if (command_overview_cyberware_.shows_no_cyberware_message) {
            draw_command_overview_text(
                painter, 160, 48, field_menu_text_.cyberware_empty_message);
        } else {
            for (std::size_t row = 0;
                 row < command_overview_cyberware_.installed_row_indices.size();
                 ++row) {
                const auto bit = command_overview_cyberware_
                                     .installed_row_indices[row];
                if (bit >= field_menu_text_.cyberware_entries.size()) {
                    continue;
                }
                draw_command_overview_text(
                    painter, 112, 48 + static_cast<int>(row) * 16,
                    field_menu_text_.cyberware_entries[bit]);
            }
        }
        return;
    }
    if (pocket_secretary_.open) {
        // The Pocket Secretary is its own full-screen 0xC2A6 composition.
        // The FFF0DC-driven row text and selection cursor remain on the
        // visual audit ledger until their source geometry is derived.
        painter.drawImage(0, 0, pocket_secretary_background_);
        return;
    }

    painter.drawImage(0, 0, command_overview_background_);
    if (command_overview_.view().source_state.primary_page == 3U) {
        // 0xD01C draws the Attributes/Skills frame over the base command
        // screen. Its 0xC1A2A label table is stamped in four blocks:
        // words 0xC386/0xC3AC (rows 7+0..3) and 0xC686/0xC6AC (rows
        // 13+0..5) of the 64-column plane, i.e. screen columns 24 and
        // 176 with an eight-pixel row gap at y88-96.
        painter.drawImage(0, 0, command_overview_attributes_background_);
        const auto place = [](const std::size_t label_index) {
            std::array<int, 2> result{};
            if (label_index < 4U) {
                return std::array<int, 2>{24, 56 + static_cast<int>(
                                                      label_index) * 8};
            }
            if (label_index < 8U) {
                return std::array<int, 2>{
                    176, 56 + static_cast<int>(label_index - 4U) * 8};
            }
            if (label_index < 14U) {
                return std::array<int, 2>{
                    24, 104 + static_cast<int>(label_index - 8U) * 8};
            }
            return std::array<int, 2>{
                176, 104 + static_cast<int>(label_index - 14U) * 8};
        };
        for (std::size_t index = 0;
             index < field_menu_text_.attribute_skill_labels.size();
             ++index) {
            if (field_menu_text_.attribute_skill_labels[index].empty()) {
                continue;
            }
            const auto position = place(index);
            draw_command_overview_text(
                painter, position[0], position[1],
                field_menu_text_.attribute_skill_labels[index]);
        }
        // Attribute/skill values and the 0xD27C cursor navigation remain
        // on the visual audit ledger.
        return;
    }
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

    draw_command_overview_text(
        painter, 128, 8,
        field_party_roster_.name(field_party_roster_.active_slot()).empty()
            ? field_menu_text_.runner_names[0]
            : field_party_roster_.name(field_party_roster_.active_slot()));

    // 0x11928/0x11AE4 places the five-state posture bar at CAAC (176,168).
    const auto posture = std::min<std::size_t>(
        active_runner_record().posture,
        command_overview_status_bars_.size() - 1U);
    painter.drawImage(176, 168, command_overview_status_bars_[posture]);
    draw_command_overview_text(painter, 136, 176,
                               field_menu_text_.status_names[posture]);
    if (posture_dialog_active_) {
        // 0x119CE's dialog highlights the posture row while the source
        // loop accepts the raise/lower edges.
        painter.drawImage(176, 168, posture_focus_);
    }

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
                    active_runner_record().equipped_armor_index);
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
            const auto phase = active_runner_record()
                                   .inventory_phase_bytes[*selected_slot];
            if (phase < command_overview_detail_values_.size()) {
                painter.drawImage(120, 56,
                                  command_overview_detail_values_[phase]);
            }
        };
    const auto draw_armor_summary = [this, &painter] {
        const auto armor_index = active_runner_record().equipped_armor_index;
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
    if (view.source_state.primary_page >= 2U &&
        !command_overview_clips_.is_open()) {
        std::string archetype;
        switch (active_runner_record().archetype_index) {
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
                          active_runner_record().delay_meter) * 10U)) +
                "%");
        draw_command_overview_text(
            painter, 208, 48,
            std::to_string(std::min<unsigned>(
                100U, static_cast<unsigned>(
                          active_runner_record().behavior_meter) * 10U)) +
                "%");
        draw_item_detail_rows(active_runner_record().equipped_item_code,
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
    } else if (!command_overview_clips_.is_open()) {
        const auto slot = view.source_state.secondary_page * 2U +
                          view.source_state.primary_page;
        if (slot < active_runner_record().inventory_item_codes.size()) {
            const auto item_code =
                active_runner_record().inventory_item_codes[slot];
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
                                  active_runner_record())
                                  ? 1U
                                  : 0U;
    for (std::size_t slot = 0;
         slot < active_runner_record().inventory_item_codes.size();
         ++slot) {
        const auto item_code = active_runner_record().inventory_item_codes[
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
    if (active_runner_record().equipped_inventory_slot >= 0) {
        draw_equipment_border(static_cast<std::size_t>(
            active_runner_record().equipped_inventory_slot));
    }
    if (active_runner_record().equipped_armor_index != 0U) {
        const auto armor_code = static_cast<std::uint8_t>(
            0x2BU + active_runner_record().equipped_armor_index);
        for (std::size_t slot = 0;
             slot < active_runner_record().inventory_item_codes.size();
             ++slot) {
            if (active_runner_record().inventory_item_codes[slot] ==
                armor_code) {
                draw_equipment_border(slot);
                break;
            }
        }
    }

    if (view.source_state.primary_page < 2U) {
        // ROM 0x11592 builds the cursor SAT entry from the 0xD29F6 list:
        // x = FFF0D0*0x28 + 0x008C, y = FFF0D1*0x18 + 0x00F4. The Genesis
        // SAT origin adds 0x80 on both axes, so the screen position is
        // (primary*40 + 12, secondary*24 + 116). The 24x32 bracket sprite
        // frames the 40x24 inventory cell's icon from those offsets.
        const auto x = 12 + static_cast<int>(
                               view.source_state.primary_page * 40U);
        const auto y = 116 + static_cast<int>(
                               view.source_state.secondary_page * 24U);
        painter.drawImage(x, y, command_overview_cursor_);
    }
    if (command_overview_clips_.is_open()) {
        paint_command_overview_clips(painter);
    } else if (!command_overview_inventory_action_active_ &&
        view.source_state.primary_page >= 2U &&
        view.source_state.primary_page <= 6U) {
        const auto focus_index = static_cast<std::size_t>(
            view.source_state.primary_page - 2U);
        painter.drawImage(128 + static_cast<int>(focus_index * 24U), 184,
                          command_overview_focus_[focus_index]);
    }
    if (command_overview_inventory_action_active_) {
        painter.drawImage(kCommandOverviewActionBackgroundX,
                          kCommandOverviewActionBackgroundY,
                          command_overview_action_background_);
        const auto action_selector =
            command_overview_inventory_action_.action_selector;
        if (action_selector >= 2U && action_selector <= 5U) {
            const auto focus_index = static_cast<std::size_t>(action_selector -
                                                                2U);
            painter.drawImage(kCommandOverviewActionFocusX[focus_index],
                              kCommandOverviewActionFocusY,
                              command_overview_action_focus_[focus_index]);
        }
        if (command_overview_discard_confirmation_active_) {
            draw_command_overview_text(
                painter, 112, 176, field_menu_text_.action_discard_prompt);
            if (command_overview_selected_inventory_item_.has_value() &&
                active_runner_record().equipped_inventory_slot >= 0 &&
                static_cast<std::size_t>(
                    active_runner_record().equipped_inventory_slot) ==
                    command_overview_selected_inventory_item_->slot) {
                draw_command_overview_text(
                    painter, 112, 184,
                    field_menu_text_.action_item_equipped_prefix);
                draw_command_overview_text(
                    painter, 112, 192,
                    field_menu_text_.action_item_equipped_suffix);
            }
            draw_command_overview_text(
                painter, 112, 200,
                command_overview_discard_confirmation_.selected_option == 0U
                    ? field_menu_text_.action_choice_yes
                    : field_menu_text_.action_choice_no);
        } else if (!command_overview_action_message_.empty()) {
            draw_command_overview_text(
                painter, 112, 200, command_overview_action_message_);
        }
    } else if (!command_overview_clips_.is_open() &&
               view.source_state.primary_page <
               field_menu_text_.command_labels.size()) {
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
    if (field_dialogue_.open()) {
        field_dialogue_.paint(painter);
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

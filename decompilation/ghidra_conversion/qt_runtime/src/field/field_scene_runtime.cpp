#include "field_scene_runtime.hpp"

#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"

#include <QPainter>
#include <QColor>
#include <QRandomGenerator>

#include <algorithm>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <sstream>
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
constexpr std::string_view kFieldHudPartyFaceAliasPrefix =
    "field.hud.party.face.";
constexpr std::string_view kFieldHudPartyItemAliasPrefix =
    "field.hud.party.item.";
constexpr std::string_view kFieldHudPartyUnarmedItemAliasPrefix =
    "field.hud.party.unarmed.item.";
constexpr std::string_view kFieldHudMagicItemAliasPrefix =
    "field.hud.magic.item.";
constexpr std::string_view kFieldHudDelayMeterAliasPrefix =
    "field.hud.delay.meter.";
constexpr std::string_view kFieldHudBehaviorMeterPrimaryAliasPrefix =
    "field.hud.behavior.meter.primary.";
constexpr std::string_view kFieldHudBehaviorMeterCompanionAliasPrefix =
    "field.hud.behavior.meter.companion.";
constexpr std::string_view kFieldHudAmmoCounterAliasPrefix =
    "field.hud.ammo.counter.";
constexpr std::string_view kFieldEntityIdleAliasPrefix = "field.entities.idle.";
constexpr std::string_view kFieldEntityWalkAliasPrefix =
    "field.entities.movement.";
constexpr std::string_view kFieldEntityAttackAliasPrefix =
    "field.entities.attack.";
constexpr std::string_view kFieldWeaponProjectileAliasPrefix =
    "field.weapons.projectile.";
constexpr std::string_view kElevatorAnimationAliasPrefix =
    "menu.field.elevator.selector.background.frame.";
constexpr std::string_view kElevatorCurrentFloorAliasPrefix =
    "menu.field.elevator.selector.current.floor.";
constexpr std::string_view kElevatorSelectedFloorAliasPrefix =
    "menu.field.elevator.selector.selected.floor.";
constexpr std::string_view kElevatorFeedbackAliasPrefix =
    "menu.field.elevator.selector.feedback.";
constexpr std::array<std::string_view, 4> kElevatorFeedbackAssetNames{{
    "previous.active", "previous.rest", "next.active", "next.rest"}};
constexpr int kGenesisSpriteOrigin = 0x80;
constexpr std::string_view kCommandOverviewBackgroundAlias =
    "menu.field.background";
constexpr std::string_view kCommandOverviewInventoryBoxesAlias =
    "menu.field.inventory.boxes";
constexpr std::string_view kCommandOverviewCursorAlias = "menu.field.cursor";
constexpr std::string_view kCommandOverviewPortraitAliasPrefix =
    "menu.field.runner.portrait.";
constexpr std::string_view kRandomGridFixedAliasPrefix =
    "menu.field.random.grid.fixed.";
constexpr std::string_view kRandomGridIndexedAliasPrefix =
    "menu.field.random.grid.indexed.";
constexpr std::string_view kCommandOverviewFocusAliasPrefix =
    "menu.field.command.focus.";
constexpr std::string_view kCommandOverviewStatusAliasPrefix =
    "menu.field.status.bar.";
constexpr std::string_view kCommandOverviewSelectedCellAliasPrefix =
    "menu.field.selected.cell.";
constexpr std::string_view kCommandOverviewActionBackgroundAlias =
    "menu.field.action.background";
constexpr std::string_view kCommandOverviewActionConfirmationBackgroundAlias =
    "menu.field.action.confirm.background";
constexpr std::string_view kCommandOverviewClipsBackgroundAlias =
    "menu.field.clips.background";
constexpr std::string_view kCommandOverviewClipsTradeBackgroundAlias =
    "menu.field.clips.trade.background";
constexpr std::string_view kCommandOverviewActionFocusAliasPrefix =
    "menu.field.action.focus.";
constexpr std::string_view kCommandOverviewActionConfirmationFocusAliasPrefix =
    "menu.field.action.confirm.focus.";
constexpr std::string_view kCommandOverviewClipsFocusAliasPrefix =
    "menu.field.clips.focus.";
constexpr std::string_view kCommandOverviewClipsTradeFocusAliasPrefix =
    "menu.field.clips.trade.focus.";
constexpr std::string_view kCommandOverviewDamageBarAliasPrefix =
    "menu.field.damage.bar.";
constexpr std::string_view kCommandOverviewDrainBarAliasPrefix =
    "menu.field.drain.bar.";
constexpr std::string_view kCommandOverviewArmorSummaryBarAliasPrefix =
    "menu.field.armor.summary.bar.";
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
// ROM destination C928 is row 18, column 20 relative to C000 when each
// 0x80-byte name-table row holds 64 two-byte cells: host pixel (160,144).
constexpr int kRunnerPreviewPanelX = 160;
constexpr int kRunnerPreviewPanelY = 144;
constexpr std::array<int, 4> kCommandOverviewActionFocusX{{128, 152, 176,
                                                             200}};
constexpr int kCommandOverviewActionFocusY = 184;
// ROM 0x127FC writes Yes at plane-B EBA2 and No at EBAE, over the
// source CD3B6 confirmation row installed at EB9E.
constexpr std::array<int, 2> kCommandOverviewActionConfirmationFocusX{{136,
                                                                        184}};
constexpr int kCommandOverviewActionConfirmationFocusY = 184;
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

FieldActionCostContext ares_cost_context(
    const std::uint8_t menu_mode, const RunnerRecordFields& runner) {
    // Ares enters the service with FFF0B6 = 1. 1AAC6E[1] is 2, and the
    // source reads the active runner's +0x8C statistic (typed as skills[11]).
    return FieldActionCostContext{menu_mode, 0U, runner.skills[11], 2U};
}

FieldActionPurchaseState make_purchase_state(
    const RunnerRecordFields& runner, const std::uint32_t funds) {
    FieldActionPurchaseState state{};
    state.funds = funds;
    state.runner.action_codes = runner.inventory_item_codes;
    state.runner.companion_values = runner.inventory_phase_bytes;
    state.runner.special_action_count = runner.ammo_clips;
    return state;
}

RunnerActionRemovalState make_sale_state(
    const RunnerRecordFields& runner, const std::uint8_t slot,
    const std::uint32_t funds) {
    RunnerActionRemovalState state{};
    state.runner.action_codes = runner.inventory_item_codes;
    state.runner.companion_values = runner.inventory_phase_bytes;
    state.runner_slot = slot;
    state.selected_runner_slot = runner.equipped_inventory_slot < 0
        ? 0xFFU
        : static_cast<std::uint8_t>(runner.equipped_inventory_slot);
    state.action_suffix_marker = runner.equipped_armor_index;
    state.funds = funds;
    return state;
}

void apply_purchase_state(const FieldActionPurchaseState& state,
                          RunnerRecordFields& runner) {
    runner.inventory_item_codes = state.runner.action_codes;
    runner.inventory_phase_bytes = state.runner.companion_values;
    runner.ammo_clips = state.runner.special_action_count;
}

void apply_sale_state(const RunnerActionRemovalState& state,
                      RunnerRecordFields& runner) {
    runner.inventory_item_codes = state.runner.action_codes;
    runner.inventory_phase_bytes = state.runner.companion_values;
    runner.equipped_armor_index = state.action_suffix_marker;
    if (state.menu_selection_cleared) {
        runner.equipped_inventory_slot = -1;
        runner.equipped_item_code = 0U;
    }
}

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

EntityActionRecord magic_action_record_from_runner(
    const RunnerRecordFields& runner) {
    // 0x1047E / 0x106DA use this subset of the source record for Magical
    // Defense: active byte +0x10 (the high byte of the exposed big-endian
    // flags word), class +0x9B, item-0x18 skill +0x80, +0x77/+0x7C defense
    // attributes, and possession/phase rows +0x66..75.
    EntityActionRecord result{};
    result.flags_10 = static_cast<std::uint8_t>(runner.flags >> 8U);
    result.class_code = runner.archetype_index;
    result.item18_base = runner.skills[0];
    result.posture = runner.posture;
    result.attribute_77 = runner.attributes[0];
    result.attribute_7c = runner.attributes[5];
    result.item_codes = runner.inventory_item_codes;
    result.item_phases = runner.inventory_phase_bytes;
    return result;
}

} // namespace

FieldSceneRuntime::FieldSceneRuntime(
    std::function<void(std::string_view)> source_boundary,
    std::function<void(std::uint16_t)> source_audio_event)
    : source_boundary_callback_(std::move(source_boundary)),
      source_audio_event_callback_(std::move(source_audio_event)),
      field_dialogue_([this](const std::string_view boundary) {
          this->source_boundary(boundary);
      }, StokerMotelArrivalCommandCallbacks{
          [this](const std::uint8_t bound)
              -> std::optional<std::uint8_t> {
              if (bound == 0U) {
                  return std::nullopt;
              }
              const auto sample = sample_source_random_bound(
                  field_random_seed_, static_cast<std::uint16_t>(bound));
              if (sample < 0 || sample >= static_cast<std::int16_t>(bound)) {
                  return std::nullopt;
              }
              return static_cast<std::uint8_t>(sample);
          },
          {}}),
      field_interaction_(
          [this](const std::string_view boundary) {
              this->source_boundary(boundary);
          },
          source_audio_event_callback_) {
    field_dialogue_.set_script_runtime(&game_scripts_);
    field_interaction_.set_script_runtime(&game_scripts_);
    field_dialogue_.set_room_payment_callback([this]()
        -> std::optional<StokerMotelRoomPaymentOutcome> {
        if (!field_funds_.has_value()) {
            return std::nullopt;
        }
        StokerMotelRoomPaymentState payment_state{};
        payment_state.funds = *field_funds_;
        const auto outcome = resolve_stoker_motel_room_payment(payment_state);
        field_funds_ = outcome.remaining_funds;
        return outcome;
    });
    field_dialogue_.set_debt_payment_callback([this]()
        -> std::optional<StokerMotelDebtPaymentOutcome> {
        constexpr std::uint32_t kDebt = 250U;
        if (!field_funds_.has_value() || !notebook_tip_flags_resolved_ ||
            !notebook_referral_flags_.has_value()) {
            // Story and payment state is not restored by the current save
            // adapter, so keep loaded-session writes explicit.
            return std::nullopt;
        }
        const bool paid_branch = *field_funds_ >= kDebt;
        if (paid_branch &&
            (!field_runner_feature_flags_.has_value() ||
             !field_entity_action_phase_roll_counters_.has_value())) {
            // The successful source branch also updates FF018F and the three
            // reaction counters. Those stay unresolved for restored saves.
            return std::nullopt;
        }

        StokerMotelDebtPaymentState payment_state{};
        payment_state.funds = *field_funds_;
        payment_state.world_flags = field_exit_world_flags_;
        // The live Stoker branch is title-service table entry zero at ROM
        // 0x53AB6; 0x53A78 publishes that index in FFDD64 before dispatch.
        payment_state.effect_marker = 0U;
        payment_state.runner_feature_flags =
            field_runner_feature_flags_.value_or(0U);
        payment_state.followup_state.story_flags = notebook_tip_flags_[0];
        payment_state.followup_state.referral_flags =
            *notebook_referral_flags_;
        payment_state.runner_source_bytes =
            field_entity_action_phase_roll_counters_.value_or(
                std::array<std::uint8_t, 3>{});
        const auto outcome =
            resolve_stoker_motel_debt_payment(payment_state);
        field_funds_ = payment_state.funds;
        if (outcome.result == StokerMotelDebtPaymentResult::debt_paid) {
            field_exit_world_flags_ = payment_state.world_flags;
            field_runner_feature_flags_ =
                payment_state.runner_feature_flags;
            field_entity_action_phase_roll_counters_ =
                payment_state.runner_source_bytes;
            cyberdeck_available_ =
                (payment_state.runner_feature_flags & 0x01U) != 0U;
        }
        notebook_tip_flags_[0] = payment_state.followup_state.story_flags;
        notebook_referral_flags_ =
            payment_state.followup_state.referral_flags;
        return outcome;
    });
    field_dialogue_.set_followup_command_callback(
        [this](const StokerMotelFollowupCommand command)
            -> std::optional<StokerMotelFollowupOutcome> {
            StokerMotelFollowupState followup_state{};
            if (command == StokerMotelFollowupCommand::debt_collection) {
                if (!notebook_tip_flags_resolved_) {
                    return std::nullopt;
                }
                followup_state.story_flags = notebook_tip_flags_[0];
            } else {
                if (!notebook_referral_flags_.has_value()) {
                    return std::nullopt;
                }
                followup_state.referral_flags =
                    *notebook_referral_flags_;
            }

            const auto outcome = resolve_stoker_motel_followup_command(
                command, followup_state);
            if (command == StokerMotelFollowupCommand::debt_collection) {
                notebook_tip_flags_[0] = followup_state.story_flags;
            } else {
                notebook_referral_flags_ = followup_state.referral_flags;
            }
            return outcome;
        });
    configure_callbacks();
}

void FieldSceneRuntime::source_boundary(
    const std::string_view boundary) const {
    if (source_boundary_callback_) {
        source_boundary_callback_(boundary);
    }
}

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

bool FieldSceneRuntime::run_field_exit_orchestrator(
    integration::FieldRuntimeCoordinatorState& runtime_state) {
    FieldExitOrchestratorState orchestrator_state{};
    // FF0110 is a graphics-owned gate in the ROM. The field controller has no
    // equivalent host flag yet, so leave it clear rather than treating a
    // controller button as a guessed graphics route.
    orchestrator_state.controller_flags = 0U;
    orchestrator_state.prompt_display_prepared = false;

    const auto& event_metadata = state_.field_scene_buffers.source_metadata;
    const auto event_route = field_map_event_system_.resolve_script_route(
        FieldScriptRouteContext{
            static_cast<std::uint8_t>(runtime_state.scene_id),
            static_cast<std::uint8_t>(runtime_state.field_mode),
            field_movement_transition_state_.mode_event_code,
            event_metadata.event_table_available,
            event_metadata.event_mode_table});
    active_field_script_route_ = event_route;

    FieldExitOrchestratorCallbacks callbacks{};
    callbacks.select_mode_entry_resource = [this] {
        source_boundary("field.exit.select_mode_entry_resource");
    };
    callbacks.run_indexed_title_services =
        [this, &event_route](FieldExitOrchestratorState& state) {
            // ROM 0x53A78 invokes indexed title service 0 before the generic
            // 0x1F65A selector. Its Stoker service is active for scene 2,
            // event 2 while FFFC1E bit 0 is clear and FFF0D5 is zero. Keep
            // that story branch in the event path so repeat visits retain
            // the package offer until collection is complete.
            if (event_route.script !=
                    FieldScriptId::stoker_motel_interaction ||
                field_exit_alternate_result_ != 0) {
                return false;
            }

            game_scripts_.set_location_context(
                "motel", "stoker_motel", "stoker.motel", "stoker");
            if (!field_dialogue_.open_stoker_interaction()) {
                source_boundary("field.dialogue.stoker_story_script_failed");
                state.caller_exit_status = -1;
                return true;
            }
            if ((field_exit_world_flags_ & 0x01U) != 0U &&
                !(stoker_motel_story_state_.michael_debt_paid &&
                  stoker_motel_story_state_.michael_package_collected)) {
                // The payment flag is persistent in the ROM. If this session
                // cannot restore the matching story state, leave the generic
                // selector in control rather than inventing a phase. Preserve
                // the script's first-entry fact update before cancelling it.
                field_dialogue_.close();
                return false;
            }
            field_palette_fade_steps_ = 0U;
            source_boundary("field.exit.prompt.display_palette_refreshed");
            state.caller_exit_status = -1;
            return true;
        };
    callbacks.select_field_exit_prompt =
        [this, &runtime_state, &event_route](
            FieldExitOrchestratorState& state) {
            FieldExitPromptState prompt_state{};
            const auto dialogue_event_code =
                event_route.kind == FieldScriptRouteKind::dialogue
                    ? event_route.event_code
                    : field_movement_transition_state_.mode_event_code;
            const FieldDialogueEventContext event_context{
                runtime_state.field_mode,
                dialogue_event_code,
                field_exit_alternate_result_,
                field_exit_world_flags_};
            const auto action = field_dialogue_event_resolver_.resolve(
                event_context, prompt_state,
                {[this](FieldExitPromptSelection& selection,
                       FieldExitPromptState&) {
                     // 0x1F246/0x1F3C4 is blocking on the Genesis but is a
                     // normal host state transition here. The panel owns the
                     // subsequent controller edges without re-entering field
                     // movement or inventing a second dialogue machine.
                     if (field_dialogue_.open_field_exit_prompt(selection)) {
                         // 0x1F246 rebuilds the mode display and refreshes
                         // its palette after preparing the prompt. The Qt
                         // fade is only a presentation overlay, so it must
                         // not cover the newly prepared dialogue screen.
                         field_palette_fade_steps_ = 0U;
                         source_boundary(
                             "field.exit.prompt.display_palette_refreshed");
                     }
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

    // Direct dialogue and service scripts take priority. Every other event
    // uses the active map's authored event table; the coordinator's ordinary
    // world-activity branch then prepares the selected scene. This keeps the
    // scene-2 C0 doorway (event 0 -> scene 0x14) separate from the Stoker C2
    // interaction and the Ares CB service.
    const auto continues_through_area_event_table = [&event_route] {
        return event_route.kind == FieldScriptRouteKind::area_event;
    };

    // The typed route plan identifies the Ares service entry at ROM 0x1FA78.
    // Its text and optional alternate result are selected by the mode-text
    // provider below; other service routes remain explicit boundaries.
    const auto is_ares_shop_event = [&event_route] {
        return event_route.script == FieldScriptId::ares_weapon_emporium;
    };
    callbacks.prepare_field_mode_exit =
        [this, &is_ares_shop_event, &continues_through_area_event_table](
                                            FieldExitOrchestratorState& state) {
        if (is_ares_shop_event()) {
            return false;
        }
        if (continues_through_area_event_table()) {
            source_boundary("field.exit.area_event_table_continuation");
            return false;
        }
        source_boundary("field.exit.prepare_mode_exit_unresolved");
        state.caller_exit_status = -1;
        return true;
    };
    callbacks.select_field_mode_service =
        [this, &runtime_state, &is_ares_shop_event,
         &continues_through_area_event_table](
            FieldExitOrchestratorState& state) {
            if (continues_through_area_event_table()) {
                return false;
            }
            if (!is_ares_shop_event()) {
                source_boundary("field.exit.mode_service_unresolved");
                state.caller_exit_status = -1;
                return true;
            }

            FieldModeServiceRouteState route_state{};
            route_state.scene_id = runtime_state.field_mode;
            route_state.submode =
                field_movement_transition_state_.mode_event_code;
            route_state.alternate_result = field_exit_alternate_result_;
            bool prompt_opened = false;
            FieldModeServiceRouteCallbacks service_callbacks{};
            service_callbacks.run_mode_exit_services =
                [this, &prompt_opened](
                    FieldModeServiceSelection&,
                    FieldModeServiceRouteState& source_state) {
                    prompt_opened = field_mode_text_.open(
                        FieldModeTextPromptContext{
                            source_state.scene_id, source_state.submode,
                            source_state.alternate_result});
                    if (prompt_opened) {
                        // The source mode-text service rebuilds the display
                        // after the field fade. The Qt fade is a presentation
                        // overlay, so retire it when this full-screen panel
                        // becomes the active screen owner.
                        field_palette_fade_steps_ = 0U;
                        source_boundary(
                            "field.mode_text_prompt.display_palette_refreshed");
                    } else {
                        source_boundary(
                            "field.ares_mode_text_prompt.open_failed");
                    }
                };
            service_callbacks.run_field_mode_service =
                [this, &prompt_opened](FieldModeServiceRouteState&) {
                    if (!prompt_opened) {
                        return;
                    }
                    field_shop_route_pending_ = true;
                    source_boundary("field.shop.ares.service_dispatched");
                };

            const auto action = select_field_mode_service(
                route_state, service_callbacks);
            if (action == FieldModeServiceAction::no_matching_route) {
                source_boundary("field.exit.mode_service_unmatched");
            } else if (action ==
                       FieldModeServiceAction::field_mode_service_unresolved) {
                source_boundary("field.exit.mode_service_callback_unresolved");
            }
            state.caller_exit_status = route_state.caller_exit_status;
            // The generic prompt owns the next input samples. For default
            // result it will open the shop after the prompt returns; alternate
            // result records return to the field without dispatching 0x56AC0.
            return true;
        };
    callbacks.finalize_field_mode_exit =
        [this, &continues_through_area_event_table](
            FieldExitOrchestratorState& state) {
        if (continues_through_area_event_table()) {
            return false;
        }
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
        viewport_state_.normal_render_pending = 1U;
    }
    if (effect.world_effect_y_dirty) {
        field_camera_follow_.layer_y_dirty = true;
        viewport_state_.alternate_render_pending = 1U;
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

bool FieldSceneRuntime::open_field_elevator_selector() {
    if (!field_session_source_assets_ || field_elevator_selector_.is_open()) {
        return false;
    }
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    if (scene_reference >=
        field_session_source_assets_->field_elevator_scene_groups.size()) {
        return false;
    }
    const auto& group = field_session_source_assets_
                            ->field_elevator_scene_groups[scene_reference];
    if (group.count < 2U ||
        group.count > kElevatorSelectorMaximumFloors) {
        return false;
    }
    auto entry_floor = std::uint8_t{0U};
    const auto group_end =
        group.scene_ids.begin() + static_cast<std::ptrdiff_t>(group.count);
    const auto current = std::find(group.scene_ids.begin(), group_end,
                                   scene_reference);
    if (current != group_end) {
        entry_floor = static_cast<std::uint8_t>(
            std::distance(group.scene_ids.begin(), current));
    }
    if (!field_elevator_selector_.open(
            group.scene_ids, group.count, entry_floor)) {
        return false;
    }
    source_boundary("field.elevator.selector.opened");
    return true;
}

bool FieldSceneRuntime::transition_to_elevator_destination(
    const std::uint16_t scene_reference) {
    if (!field_session_source_assets_ ||
        scene_reference >=
            field_session_source_assets_->field_scene_resources.size() ||
        !prepare_runtime_field_scene(scene_reference)) {
        source_boundary("field.elevator.destination_prepare_unresolved");
        return false;
    }

    const auto spawn = state_.field_scene_buffers.source_metadata
                           .initial_field_records[0U];
    field_entity_.world_position = spawn;
    field_entity_.projection = EntityProjectionState{};
    field_entity_.visible = false;
    field_entity_.direction = 0U;
    field_movement_state_.position_x = spawn.x;
    field_movement_state_.position_y = spawn.y;
    field_movement_state_.heading = 0U;
    field_movement_state_.movement_substate = 0U;
    field_target_heading_ = 0U;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
    field_entity_animation_ = EntityAnimationState{};
    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_selector_ =
        select_field_entity_animation_selector(0U, false);
    field_movement_behavior_record_.current_heading = 0U;
    field_movement_behavior_record_.target_heading = 0U;
    field_movement_transition_state_.terrain_collision_code = 0U;
    field_movement_transition_state_.mode_event_code = 0U;
    field_movement_transition_state_.activity_sequence_active = false;
    field_movement_transition_state_.terrain_event_suppressed = false;
    field_movement_transition_state_.scene_id = scene_reference;
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
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    field_world_activity_timeout_state_.scene_id = scene_reference;
    world_activity_routine_state_ = WorldActivityRoutineState{};
    field_weapon_projectiles_.clear();
    field_weapon_slots_occupied_.fill(false);
    field_weapon_collision_targets_pending_reported_ = false;
    last_field_weapon_spawn_result_ = EntitySpawnCycleResult{};
    field_camera_alignment_ready_ = false;
    field_palette_fade_steps_ = 0U;
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    update_source_field_camera();
    update_entity_render_pass(state_.render_entities,
                              callbacks_.entity_render_pass);
    dispatch_field_map_scene_entry(scene_reference);
    source_boundary("field.elevator.destination_loaded");
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
            source_audio_event_callback_(0x25U);
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
    const auto destination = field_elevator_selector_.selected_scene_id();
    source_boundary("field.elevator.selector.confirmed");
    if (field_elevator_selector_.destination_changed() &&
        destination != current_scene) {
        if (field_elevator_scene_transition_.begin_fade_out()) {
            source_boundary("field.elevator.destination_fade_out_started");
        } else {
            source_boundary("field.elevator.destination_transition_busy");
        }
        return;
    }
    field_elevator_selector_.close();
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    source_boundary("field.elevator.selector.returned_to_current_scene");
}

void FieldSceneRuntime::advance_field_elevator_scene_transition() {
    const auto update = field_elevator_scene_transition_.advance();
    if (update.palette_step_changed) {
        source_boundary("field.elevator.destination_fade.palette_step");
    }
    if (update.destination_load_requested) {
        const auto destination = field_elevator_selector_.selected_scene_id();
        if (!transition_to_elevator_destination(destination)) {
            field_elevator_scene_transition_.cancel();
            source_boundary("field.elevator.destination_transition_reopened");
            return;
        }
        field_elevator_selector_.close();
        if (!field_elevator_scene_transition_.begin_fade_in()) {
            field_elevator_scene_transition_.cancel();
            source_boundary("field.elevator.destination_fade_in_unresolved");
            return;
        }
        source_boundary("field.elevator.destination_fade_in_started");
    } else if (update.completed) {
        source_boundary("field.elevator.destination_fade_finished");
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

bool FieldSceneRuntime::load_field_default_actor_assets(
    const ConversionAssetCatalog& catalog) {
    constexpr std::string_view kActorTableAlias =
        "field.entities.default.actors";
    constexpr std::string_view kActorTableHeader =
        "scene\tsource_order\tentity_kind\tworld_x\tworld_y\tinitial_heading"
        "\tpalette_line"
        "\ttemplate_offset\tposition_offset\tlist_offset"
        "\tordered_overrides";
    const auto* table = catalog.find(kActorTableAlias);
    if (table == nullptr || table->kind != "state") {
        failure_ = "missing extracted default field actor table";
        return false;
    }
    std::ifstream input(table->absolute_path, std::ios::binary);
    if (!input) {
        failure_ = "cannot read extracted default field actor table";
        return false;
    }

    field_default_actors_.clear();
    field_default_actor_artwork_ = {};

    std::string line;
    if (!std::getline(input, line) || line != kActorTableHeader) {
        failure_ = "default field actor table header is invalid";
        return false;
    }
    const auto parse_number = [](const std::string& value,
                                 const int base) -> std::uint32_t {
        std::size_t consumed = 0U;
        const auto parsed = std::stoul(value, &consumed, base);
        if (consumed != value.size() || parsed > 0xFFFFFFFFUL) {
            throw std::invalid_argument("invalid default actor number");
        }
        return static_cast<std::uint32_t>(parsed);
    };
    const auto parse_word = [&parse_number](const std::string& value) {
        const auto parsed = parse_number(value, 10);
        if (parsed > 0xFFFFU) {
            throw std::out_of_range("default actor word exceeds 16 bits");
        }
        return static_cast<std::uint16_t>(parsed);
    };
    std::array<std::uint16_t, 2> scene_record_counts{};
    try {
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 11U) {
                failure_ = "default field actor table row is malformed";
                return false;
            }
            DefaultFieldActorPresentation actor;
            actor.scene_reference = parse_word(fields[0]);
            actor.source_order = parse_word(fields[1]);
            actor.entity_kind = parse_word(fields[2]);
            actor.world_position.x = parse_word(fields[3]);
            actor.world_position.y = parse_word(fields[4]);
            actor.initial_heading = parse_word(fields[5]);
            const auto palette_line = parse_word(fields[6]);
            if (palette_line > 0xFFU) {
                failure_ = "default actor palette line exceeds one byte";
                return false;
            }
            actor.palette_line = static_cast<std::uint8_t>(palette_line);
            actor.template_offset = parse_number(fields[7], 0);
            actor.position_offset = parse_number(fields[8], 0);
            actor.list_offset = parse_number(fields[9], 0);
            actor.ordered_overrides = fields[10];
            if ((actor.entity_kind == 5U && actor.palette_line != 2U) ||
                (actor.entity_kind == 11U && actor.palette_line != 3U) ||
                (actor.entity_kind != 5U && actor.entity_kind != 11U)) {
                failure_ = "default actor has an unresolved source art kind";
                return false;
            }

            if (actor.scene_reference == 2U) {
                if (actor.source_order != scene_record_counts[0]) {
                    failure_ = "scene 2 default actor source order changed";
                    return false;
                }
                ++scene_record_counts[0];
            } else if (actor.scene_reference == 0x14U) {
                if (actor.source_order != scene_record_counts[1]) {
                    failure_ = "scene 0x14 default actor source order changed";
                    return false;
                }
                ++scene_record_counts[1];
            }

            field_default_actors_.push_back(std::move(actor));
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse default field actor table: ") +
                   error.what();
        return false;
    }
    if (scene_record_counts[0] != 3U || scene_record_counts[1] != 8U) {
        failure_ = "default field actor table is incomplete";
        return false;
    }

    constexpr std::array<std::string_view, 2> kActorAssetPrefixes{
        "field.entities.actors.corp.guard",
        "field.entities.actors.pedestrian"};
    constexpr std::array<std::uint16_t, 2> kActorKinds{5U, 11U};
    std::array<std::array<bool, 8>, 2> loaded_directions{};
    for (const auto& actor : field_default_actors_) {
        const auto kind_index = actor.entity_kind == 5U ? 0U : 1U;
        const auto direction =
            field_actor_direction_from_heading(actor.initial_heading);
        if (loaded_directions[kind_index][direction]) {
            continue;
        }
        auto& artwork = field_default_actor_artwork_[kind_index];
        const auto direction_suffix = direction == 0U
            ? std::string{}
            : ".dir." + std::to_string(direction);
        const auto idle_alias = std::string(kActorAssetPrefixes[kind_index]) +
                                ".idle" + direction_suffix;
        if (!load_sprite_image(catalog, idle_alias,
                               artwork.idle_images[direction],
                               artwork.idle_origin_x[direction],
                               artwork.idle_origin_y[direction])) {
            return false;
        }
        for (std::size_t frame = 0; frame < 2U; ++frame) {
            const auto entry_alias =
                std::string(kActorAssetPrefixes[kind_index]) + ".entry.dir." +
                std::to_string(direction) + ".frame." +
                std::to_string(frame);
            if (!load_sprite_image(
                    catalog, entry_alias,
                    artwork.entry_images[direction][frame],
                    artwork.entry_origin_x[direction][frame],
                    artwork.entry_origin_y[direction][frame])) {
                return false;
            }
        }
        if (actor.entity_kind != kActorKinds[kind_index]) {
            failure_ = "default actor art kind mapping changed";
            return false;
        }
        loaded_directions[kind_index][direction] = true;
    }
    return true;
}

void FieldSceneRuntime::initialize_default_field_actors(
    const std::uint16_t scene_reference) {
    for (auto& actor : field_default_actors_) {
        if (actor.scene_reference != scene_reference) {
            continue;
        }
        const auto initial_delay = static_cast<std::uint8_t>(
            advance_source_random_seed(field_random_seed_) & 0x000FU);
        actor.behavior = make_field_actor_behavior_state(
            actor.entity_kind, actor.initial_heading, initial_delay);
    }
}

void FieldSceneRuntime::advance_default_field_actors() {
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    for (auto& actor : field_default_actors_) {
        if (actor.scene_reference != current_scene) {
            continue;
        }
        const auto update = advance_field_actor_behavior(actor.behavior);
        if (update.audio_event.has_value() && source_audio_event_callback_) {
            source_audio_event_callback_(*update.audio_event);
        }
    }
}

FieldSceneRuntime::DefaultActorSpriteView
FieldSceneRuntime::default_actor_sprite_view(
    const DefaultFieldActorPresentation& actor) const {
    if (actor.entity_kind != 5U && actor.entity_kind != 11U) {
        return {};
    }
    const auto kind_index = actor.entity_kind == 5U ? 0U : 1U;
    const auto direction = static_cast<std::size_t>(
        actor.behavior.direction & 0x07U);
    const auto& artwork = field_default_actor_artwork_[kind_index];
    if (actor.behavior.animation_phase ==
        FieldActorAnimationPhase::initial_idle) {
        return DefaultActorSpriteView{
            &artwork.idle_images[direction],
            artwork.idle_origin_x[direction],
            artwork.idle_origin_y[direction]};
    }
    const auto frame = static_cast<std::size_t>(
        actor.behavior.animation_frame & 0x01U);
    return DefaultActorSpriteView{
        &artwork.entry_images[direction][frame],
        artwork.entry_origin_x[direction][frame],
        artwork.entry_origin_y[direction][frame]};
}

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

std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_projectile_frame_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return spawn.animation.frame;
        }
    }
    return std::nullopt;
}

std::size_t FieldSceneRuntime::field_weapon_primary_count_for_dump() const
    noexcept {
    std::size_t count = 0U;
    for (const auto& spawn : field_weapon_projectiles_) {
        count += spawn.primary_active ? 1U : 0U;
    }
    return count;
}

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

std::optional<std::uint8_t>
FieldSceneRuntime::field_weapon_primary_retry_count_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return spawn.primary_record.field_0x89;
        }
    }
    return std::nullopt;
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

bool FieldSceneRuntime::load_command_overview_assets(
    const ConversionAssetCatalog& catalog) {
    if (!load_image(catalog, kCommandOverviewBackgroundAlias,
                    command_overview_background_) ||
        !load_image(catalog, kCommandOverviewInventoryBoxesAlias,
                    command_overview_inventory_boxes_) ||
        !load_image(catalog, kCommandOverviewActionBackgroundAlias,
                    command_overview_action_background_) ||
        !load_image(catalog,
                    kCommandOverviewActionConfirmationBackgroundAlias,
                    command_overview_action_confirmation_background_) ||
        !load_image(catalog, kCommandOverviewClipsBackgroundAlias,
                    command_overview_clips_background_) ||
        !load_image(catalog, kCommandOverviewClipsTradeBackgroundAlias,
                    command_overview_clips_trade_background_) ||
        !load_image(catalog, "menu.field.font", command_overview_font_) ||
        !load_image(catalog, "menu.field.magic.font.palette.0",
                    magic_menu_font_) ||
        !load_image(catalog, "menu.field.font.palette.2",
                    hotel_checkout_font_) ||
        !load_image(catalog, "menu.pocket.secretary.background",
                    pocket_secretary_background_) ||
        !load_image(catalog, "menu.pocket.secretary.grid",
                    pocket_secretary_grid_) ||
        !load_image(catalog, "menu.field.hotel.cursor",
                    hotel_options_cursor_) ||
        !load_image(catalog, "menu.field.hotel.checkout.cursor",
                    hotel_checkout_cursor_) ||
        !load_image(catalog, "menu.field.attributes.background",
                    command_overview_attributes_background_) ||
        !load_image(catalog, "menu.field.posture.focus", posture_focus_) ||
        !load_image(catalog, "menu.field.magic.panel.background",
                    command_overview_magic_panel_background_)) {
        return false;
    }
    constexpr std::string_view kRunnerPreviewAliasPrefix =
        "menu.field.runner.preview.table.";
    for (const auto& resource : catalog.records) {
        if (resource.alias.rfind(kRunnerPreviewAliasPrefix.data(), 0U) != 0U) {
            continue;
        }
        QImage image;
        if (!load_image(catalog, resource.alias, image)) {
            return false;
        }
        runner_preview_panels_.emplace(resource.alias, std::move(image));
    }
    for (std::size_t index = 0;
         index < dismiss_runner_random_grid_patterns_.size(); ++index) {
        const auto alias = index < 3U
            ? std::string(kRandomGridFixedAliasPrefix) +
                  std::to_string(index)
            : std::string(kRandomGridIndexedAliasPrefix) +
                  std::to_string(index - 3U);
        if (!load_image(catalog, alias,
                        dismiss_runner_random_grid_patterns_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_spell_icons_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.spell.icon." +
                            std::to_string(index),
                        command_overview_magic_spell_icons_[index])) {
            return false;
        }
    }
    for (std::size_t phase = 0;
         phase < command_overview_magic_cursor_frames_.size(); ++phase) {
        if (!load_image(catalog,
                        "menu.field.magic.cursor.frame." +
                            std::to_string(phase),
                        command_overview_magic_cursor_frames_[phase])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_posture_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.posture.bar." +
                            std::to_string(index),
                        command_overview_magic_posture_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_force_meter_tiles_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.force.meter.tile." +
                            std::to_string(index),
                        command_overview_magic_force_meter_tiles_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_metric_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.metric.bar." +
                            std::to_string(index),
                        command_overview_magic_metric_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_availability_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.availability.bar." +
                            std::to_string(index),
                        command_overview_magic_availability_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_damage_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.damage.bar." +
                            std::to_string(index),
                        command_overview_magic_damage_bars_[index])) {
            return false;
        }
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
         index < command_overview_action_confirmation_focus_.size(); ++index) {
        if (!load_image(
                catalog,
                std::string(
                    kCommandOverviewActionConfirmationFocusAliasPrefix) +
                    std::to_string(index),
                command_overview_action_confirmation_focus_[index])) {
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
         index < command_overview_armor_summary_bars_.size(); ++index) {
        if (!load_sprite(
                std::string(kCommandOverviewArmorSummaryBarAliasPrefix) +
                    std::to_string(index),
                command_overview_armor_summary_bars_[index])) {
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
        [this] {
            source_boundary("field.entities.dispatch_action_candidates");
            advance_default_field_actors();
        };
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
            advance_field_actor_animation();
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
    field_runtime_callbacks_.initialize_world_activity_mode =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            const auto& active_metadata =
                state_.field_scene_buffers.source_metadata;
            if (!active_metadata.available ||
                !active_metadata.event_table_available ||
                !field_session_source_assets_) {
                source_boundary("field.world_activity.source_table_unavailable");
                runtime.field_transition_requested = true;
                return;
            }

            std::optional<std::uint16_t> selected_scene;
            if (active_field_script_route_.has_value() &&
                active_field_script_route_->kind ==
                    FieldScriptRouteKind::area_event &&
                active_field_script_route_->event_code ==
                    field_movement_transition_state_.mode_event_code &&
                active_field_script_route_->area_event_target.has_value()) {
                selected_scene = static_cast<std::uint16_t>(
                    *active_field_script_route_->area_event_target);
            } else {
                selected_scene = resolve_world_activity_scene_id(
                    field_movement_transition_state_.mode_event_code,
                    active_metadata.event_mode_table);
            }
            active_field_script_route_.reset();
            if (!selected_scene.has_value()) {
                source_boundary("field.world_activity.event_code_out_of_range");
                runtime.field_transition_requested = true;
                return;
            }

            WorldActivityRoutineInput input{};
            input.initial_scene_id = *selected_scene;
            input.scene_variant = runtime.field_mode;
            input.timeout_override = runtime.scene_runtime_state;
            input.world_flags = field_exit_world_flags_;
            // 0x167AE clears FFF0C8 before the resource branch.
            input.default_resource_word = 0U;

            std::uint16_t preparation_scene = *selected_scene;
            if (input.timeout_override != 0U) {
                preparation_scene = input.timeout_override == 1U
                                        ? 0x36U
                                        : 0x37U;
            }
            if (preparation_scene <
                field_session_source_assets_->field_scene_resources.size()) {
                const auto& resource =
                    field_session_source_assets_
                        ->field_scene_resources[preparation_scene];
                if (resource.has_value()) {
                    input.resource_random_index_bound =
                        resource->world_activity_random_index_bound;
                    input.resource_cursor_source =
                        resource->world_activity_resource_cursor;
                }
            }

            bool prepared = false;
            WorldActivityRoutineCallbacks activity_callbacks{};
            activity_callbacks.schedule_world_activity_timeout = [this] {
                source_boundary("field.world_activity.schedule_timeout");
            };
            activity_callbacks.publish_world_activity_descriptors = [this] {
                source_boundary("field.world_activity.publish_descriptors");
            };
            activity_callbacks.snapshot_scene_state = [this] {
                source_boundary("field.world_activity.snapshot_scene_state");
            };
            activity_callbacks.prepare_field_scene_buffers =
                [this, &prepared, preparation_scene] {
                    prepared = prepare_runtime_field_scene(preparation_scene);
                };
            activity_callbacks.upload_mode_word_block = [this] {
                source_boundary("field.world_activity.upload_mode_word_block");
            };
            activity_callbacks.sample_source_random_bound =
                [this](const std::uint16_t bound)
                    -> std::optional<std::uint16_t> {
                if (bound == 0U) {
                    return std::nullopt;
                }
                const auto sample = sample_source_random_bound(
                    field_random_seed_, bound);
                if (sample < 0 || sample >= static_cast<std::int16_t>(bound)) {
                    return std::nullopt;
                }
                return static_cast<std::uint16_t>(sample);
            };
            activity_callbacks.advance_source_prng = [this]()
                -> std::optional<std::uint16_t> {
                return advance_source_random_seed(field_random_seed_);
            };

            field_security_floor_index_.reset();
            const auto source_scene_id = runtime.scene_id;
            const auto result = run_world_activity_routine(
                input, world_activity_routine_state_, activity_callbacks);
            runtime.scene_id = world_activity_routine_state_.scene_id;
            field_world_activity_scene_entry_pending_ =
                world_activity_routine_state_.scene_id != source_scene_id;
            runtime.field_scene_gate = static_cast<std::uint16_t>(
                world_activity_routine_state_.mode);
            if (result == WorldActivityResult::resource_activity_ready) {
                field_security_floor_index_ = static_cast<std::uint8_t>(
                    world_activity_routine_state_
                        .selected_resource_random_index);
            }
            if (!prepared) {
                source_boundary("field.world_activity.scene_prepare_unresolved");
                runtime.field_transition_requested = true;
            }
            if (result == WorldActivityResult::resource_activity_bound_unavailable ||
                result ==
                    WorldActivityResult::resource_activity_random_input_unavailable ||
                result ==
                    WorldActivityResult::resource_activity_sample_out_of_range) {
                source_boundary("field.world_activity.resource_selection_unresolved");
                runtime.field_transition_requested = true;
            }
        };
    field_runtime_callbacks_.run_field_exit_orchestrator =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            if (field_opening_auto_walk_.active) {
                // The ROM clears FFE18B after the first field-exit
                // orchestrator pass, regardless of the selected event route.
                field_opening_auto_walk_.active = false;
                source_boundary("field.opening_auto_walk.consumed");
                if (game_scripts_.task_waiting_for_native_operation()) {
                    handle_game_script_task_step(
                        game_scripts_.complete_native_operation(true));
                }
            }
            const auto anchor_refresh_required =
                run_field_exit_orchestrator(runtime);
            source_boundary(anchor_refresh_required
                                ? "field.exit.coordinator_anchor_refresh_required"
                                : "field.exit.coordinator_continue_activity");
            return anchor_refresh_required;
        };
    field_runtime_callbacks_.reset_field_entity_transition_words =
        [this](integration::FieldRuntimeCoordinatorState&) {
            // The source writes its invalid-anchor sentinel to the three
            // actors' previous projection pairs. The native field presenter
            // has one visible controlled actor, so invalidate that projection
            // cache and rebuild it from the actor's typed world position.
            field_entity_.projection = EntityProjectionState{};
            field_entity_.visible = false;
            field_camera_alignment_ready_ = false;
            source_boundary(
                "field.entities.transition_projection_invalidated");
        };
    field_runtime_callbacks_.snapshot_entity_anchors =
        [this](integration::FieldRuntimeCoordinatorState&) {
            field_entity_.projection.world_x =
                field_entity_.world_position.x;
            field_entity_.projection.world_y =
                field_entity_.world_position.y;
            state_.field_scene_entry.camera_x = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_x);
            state_.field_scene_entry.camera_y = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_y);
            state_.field_scene_entry.camera_transition_x =
                static_cast<std::uint16_t>(
                    state_.camera_transition_x);
            state_.field_scene_entry.camera_transition_y =
                static_cast<std::uint16_t>(
                    state_.camera_transition_y);
            source_boundary("field.entities.transition_anchor_snapshotted");
        };
    field_runtime_callbacks_.run_pending_field_scene_handler = [this] {
        const auto continuation = field_activity_continuation_;
        field_activity_continuation_ =
            MovementModeTransitionContinuation::none;
        if (continuation ==
            MovementModeTransitionContinuation::event_29_world_activity) {
            if (!open_field_elevator_selector()) {
                source_boundary("field.elevator.selector_open_unresolved");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        const bool opened = field_interaction_.open(
            continuation,
            state_.field_scene_buffers.scene_reference,
            field_movement_transition_state_.world_mode,
            field_movement_transition_state_.terrain_event_suppressed,
            field_movement_transition_state_.activity_selection,
            field_security_floor_index_,
            &field_world_activity_timeout_state_);
        if (!opened) {
            source_boundary("field.interaction.open_failed");
        } else {
            source_boundary("field.interaction.prompt_opened");
        }
        return integration::PendingFieldSceneHandlerResult::resumed_field;
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
        service_player_weapon_action();
        advance_field_weapon_projectiles();
        advance_field_movement();
        advance_default_field_actors();
    };
    field_runtime_callbacks_.update_field_camera = [this] {
        update_source_field_camera();
    };
    field_runtime_callbacks_.update_entity_render_pass = [this] {
        update_entity_render_pass(state_.render_entities,
                                  callbacks_.entity_render_pass);
        service_pending_field_viewports();
    };
    field_runtime_callbacks_.advance_palette_fade = [this] {
        // ROM 0x16396 services effects on every field frame. When 0xB7B8
        // projected an interaction, FFDF3A/FFDF3C hold the fade gate and its
        // signed frame countdown; expiration publishes FFDF3E for the next
        // coordinator pass.
        PaletteFadeState fade{};
        fade.active =
            field_movement_transition_state_.interaction_transition_pending;
        fade.remaining =
            field_movement_transition_state_.interaction_transition_timer;
        ::shadowrun::ghidra_conversion::advance_palette_fade(
            fade,
            {[this] {
                 source_boundary("field.runtime.service_active_effects");
             },
             [this] {
                 if (field_palette_fade_steps_ < 9U) {
                     ++field_palette_fade_steps_;
                 }
                 source_boundary("field.runtime.step_palette_toward_black");
             }});
        if (fade.dirty) {
            state_.display_dirty_flags = static_cast<std::uint8_t>(
                state_.display_dirty_flags | 0x01U);
            field_runtime_state_.display_dirty_flags =
                static_cast<std::uint8_t>(
                    field_runtime_state_.display_dirty_flags | 0x01U);
        }
        if (field_movement_transition_state_.interaction_transition_pending) {
            field_movement_transition_state_.interaction_transition_timer =
                fade.remaining;
            field_movement_transition_state_.world_interaction_lock =
                fade.active;
        }
        if (fade.completed) {
            field_movement_transition_state_.interaction_transition_pending =
                false;
            field_movement_transition_state_.interaction_transition_timer =
                fade.remaining;
            field_movement_transition_state_.world_interaction_lock = false;
            field_runtime_state_.field_transition_requested = true;
            source_boundary("field.runtime.field_transition_requested");
        }
    };
    field_runtime_callbacks_.advance_world_frame_logic = [this] {
        // FFDF84/FFDF86 are shared by the alert/timeout helper (0x1DBAA)
        // and the per-frame dispatcher (0x1DE00). Keep one native field state
        // authoritative while adapting its words to the world-frame model.
        world_frame_logic_state_.world_active = static_cast<std::uint16_t>(
            field_world_activity_timeout_state_.timeout_marker);
        world_frame_logic_state_.cleanup_countdown = static_cast<std::int16_t>(
            field_world_activity_timeout_state_.timeout_frames);
        advance_world_frame_logic(world_frame_logic_state_,
                                  world_frame_logic_callbacks_);
        field_world_activity_timeout_state_.timeout_marker =
            static_cast<std::int16_t>(world_frame_logic_state_.world_active);
        field_world_activity_timeout_state_.timeout_frames =
            static_cast<std::uint16_t>(
                world_frame_logic_state_.cleanup_countdown);
        // ROM 0x1DDF0 decrements FFE11C every field frame. The native activity
        // gate shares that typed countdown so 0x1E2E0 can accept a later
        // doorway or terminal after the prior activity expires.
        field_world_activity_state_.timeout = static_cast<std::uint8_t>(
            world_frame_logic_state_.resource_countdown);
    };
    field_runtime_callbacks_.handle_mode_action_event = [this] {
        if (source_audio_event_callback_) {
            // ROM 0x391A requests event 0x25 on the field C-button runner
            // selection path.
            source_audio_event_callback_(0x25U);
        }
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
        if (command_overview_.open() || field_interaction_.open() ||
            field_dialogue_.open() || field_elevator_selector_.is_open()) {
            // The source routine returns from the blocking command loop before
            // the field transition leaf can run. The Qt bridge keeps that
            // ordering explicit while the menu owns the next host frames.
            source_boundary(
                field_dialogue_.open()
                    ? "field.runtime.scene_transition_suspended_for_dialogue"
                    : field_elevator_selector_.is_open()
                          ? "field.runtime.scene_transition_suspended_for_elevator"
                          : field_interaction_.open()
                                ? "field.runtime.scene_transition_suspended_for_interaction"
                                : "field.runtime.scene_transition_suspended_for_command_overview");
            return;
        }
        advance_field_scene_transition_after_modal();
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
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    command_overview_first_aid_targets_ = FirstAidTargetList{};
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    command_overview_inventory_selector_suppressed_ = false;
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_inventory_trade_ = InventoryTradeState{};
    command_overview_action_message_ = CommandOverviewActionMessage::none;
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

FieldHudSlotState FieldSceneRuntime::field_hud_slot_state(
    const std::size_t index) const {
    if (index >= field_party_roster_.records().size()) {
        return {};
    }
    const auto& record = field_party_roster_.records()[index];
    FieldHudSlotState slot{};
    slot.flags_byte = static_cast<std::uint8_t>(record.flags >> 8U);
    slot.equipped_inventory_slot = record.equipped_inventory_slot;
    slot.equipped_item_code = record.equipped_item_code;
    slot.class_code = record.archetype_index;
    slot.ammo_rounds = record.ammo_rounds;
    slot.delay_meter = static_cast<std::int8_t>(record.delay_meter);
    slot.behavior_meter = static_cast<std::int8_t>(record.behavior_meter);
    slot.primary_runner = index == field_party_roster_.active_slot();
    slot.asset_family_index =
        field_party_roster_.portrait_asset_family(index);
    slot.portrait_palette_word = record.command_overview_panel_parameter;
    if ((record.cyberware_flags & 0x0008U) != 0U) {
        slot.unarmed_item_variant = 2U;
    } else if ((record.cyberware_flags & 0x0004U) != 0U) {
        slot.unarmed_item_variant = 1U;
    }
    return slot;
}

const QImage* FieldSceneRuntime::runner_preview_panel_for_record(
    const RunnerRecordFields& record) const {
    constexpr std::uint16_t kPaletteLineMask = 0x6000U;
    if ((record.command_overview_panel_parameter &
         static_cast<std::uint16_t>(~kPaletteLineMask)) != 0U ||
        record.hud_portrait_table_pointer == 0U) {
        return nullptr;
    }
    const auto palette_line = static_cast<std::size_t>(
        (record.command_overview_panel_parameter & kPaletteLineMask) >> 13U);
    const auto alias =
        "menu.field.runner.preview.table." +
        std::to_string(record.hud_portrait_table_pointer) + ".offset." +
        std::to_string(record.hud_portrait_table_offset) + ".palette." +
        std::to_string(palette_line);
    const auto preview = runner_preview_panels_.find(alias);
    return preview == runner_preview_panels_.end() ? nullptr
                                                   : &preview->second;
}

void FieldSceneRuntime::refresh_field_hud_runner_slot() {
    auto& records = field_party_roster_.records();
    for (std::size_t slot = 0; slot < field_hud_slots_.size(); ++slot) {
        auto& record = records[slot];
        // ROM 0x39F2 refreshes the three field panels, then 0x38FA calls
        // 0x36FC for each record with D1=0. That call clamps an active
        // clip-using item's +0x4E count to its +0x24 capacity before the
        // resulting HUD frame is presented.
        FieldHudAmmoMeterState ammo{};
        ammo.slot_word = record.record_type;
        ammo.flags_byte = static_cast<std::uint8_t>(record.flags >> 8U);
        ammo.equipped_inventory_slot = record.equipped_inventory_slot;
        ammo.source_type = record.equipped_item_code;
        ammo.ammo = record.ammo_rounds;
        ammo.clip_counts = record.inventory_phase_bytes;
        ammo.clip_aux = record.inventory_item_codes;
        const auto ammo_plan = advance_field_hud_ammo_meter(ammo, 0);
        record.ammo_rounds = ammo.ammo;
        record.equipped_inventory_slot = ammo.equipped_inventory_slot;
        record.equipped_item_code = ammo.source_type;
        record.inventory_phase_bytes = ammo.clip_counts;
        record.inventory_item_codes = ammo.clip_aux;
        if (ammo_plan.reload_assets) {
            source_boundary("field.hud.ammo_refresh.cleared_depleted_item");
        }
        field_hud_slots_[slot] = field_hud_slot_state(slot);
    }
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
    dismiss_runner_ = DismissRunnerMenuState{};
    dismiss_runner_random_grid_effect_ = RandomGridEffectState{};
    dismiss_runner_effect_frames_ = 0U;
    dismiss_runner_random_grid_cells_.fill(-1);
    source_boundary("field.pocket_secretary.opened");
}

bool FieldSceneRuntime::set_notebook_tip_unlocked(
    const std::size_t source_index, const bool unlocked) noexcept {
    if (!notebook_tip_flags_resolved_ || source_index >= 96U) {
        return false;
    }
    const auto group = source_index / 32U;
    const auto mask = std::uint32_t{1} << (source_index % 32U);
    if (unlocked) {
        notebook_tip_flags_[group] |= mask;
    } else {
        notebook_tip_flags_[group] &= ~mask;
    }
    return true;
}

void FieldSceneRuntime::begin_pocket_secretary_dismiss_runner() {
    if (!pocket_secretary_.open || pocket_secretary_.selection != 3U) {
        source_boundary(
            "field.pocket_secretary.dismiss_runner.invalid_route");
        return;
    }

    std::array<std::uint8_t, 2> candidates{};
    std::size_t count = 0U;
    const auto& records = field_party_roster_.records();
    for (std::size_t slot = 1U;
         slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
        const auto active_record =
            (records[slot].flags & 0x0100U) != 0U;
        if (field_party_roster_.slot_active(slot) && active_record) {
            candidates[count++] = static_cast<std::uint8_t>(slot);
        }
    }
    begin_dismiss_runner_menu(dismiss_runner_, candidates, count);
    dismiss_runner_random_grid_effect_ = RandomGridEffectState{};
    dismiss_runner_effect_frames_ = 0U;
    dismiss_runner_random_grid_cells_.fill(-1);
    source_boundary("field.pocket_secretary.dismiss_runner.opened");
}

void FieldSceneRuntime::advance_pocket_secretary_dismiss_runner_effect() {
    advance_random_grid_effect(
        dismiss_runner_random_grid_effect_,
        {[this](const std::uint16_t bound) {
             return sample_source_random_bound(field_random_seed_, bound);
         },
         [this](const RandomGridEffectPlan& plan) {
             const auto branch_zero =
                 plan.primary_source ==
                 RandomGridPrimarySource::branch_zero_table;
             const auto location = branch_zero
                 ? static_cast<std::size_t>(plan.primary_entry_index)
                 : 8U + static_cast<std::size_t>(plan.primary_entry_index);
             const auto pattern = plan.variant_source ==
                     RandomGridVariantSource::fixed_three_variant_resources
                 ? static_cast<std::size_t>(plan.variant_entry_index)
                 : 3U + static_cast<std::size_t>(plan.variant_entry_index);
             if (location < dismiss_runner_random_grid_cells_.size() &&
                 pattern < dismiss_runner_random_grid_patterns_.size()) {
                 // 0x1232 writes into the installed shared frame. Keep
                 // previously changed cells until that panel is rebuilt,
                 // matching the source name table's persistence between
                 // VBlanks.
                 dismiss_runner_random_grid_cells_[location] =
                     static_cast<std::int8_t>(pattern);
             }
         }});
    ++dismiss_runner_effect_frames_;
}

void FieldSceneRuntime::finish_pocket_secretary_dismiss_runner(
    const std::size_t slot) {
    source_boundary(
        "field.pocket_secretary.dismiss_runner.serialize_population");
    if (!field_party_roster_.deactivate_party_member(slot)) {
        source_boundary(
            "field.pocket_secretary.dismiss_runner.invalid_party_marker");
        return;
    }
    source_boundary("field.pocket_secretary.dismiss_runner.party_marker_cleared");

    const auto previous_active_slot = field_party_roster_.active_slot();
    if (slot == previous_active_slot) {
        RunnerSwitchState switch_state{};
        switch_state.records.reserve(FieldPartyRosterRuntime::kSlotCount);
        const auto& records = field_party_roster_.records();
        for (std::size_t index = 0U;
             index < FieldPartyRosterRuntime::kSlotCount; ++index) {
            auto flags = static_cast<std::uint8_t>(records[index].flags >> 8U);
            if (!field_party_roster_.slot_active(index)) {
                flags = static_cast<std::uint8_t>(flags & ~0x01U);
            }
            switch_state.records.push_back(RunnerSwitchRecord{
                static_cast<std::uint16_t>(index), flags});
        }
        switch_state.active_index = previous_active_slot;
        switch_state.suppression_override =
            field_entity_selection_state_.suppression_override;
        const auto decision = advance_runner_switch(switch_state);
        if (decision.result == RunnerSwitchResult::switched) {
            field_party_roster_.set_active_slot(decision.selected_index);
            field_entity_selection_state_ = std::move(switch_state);
            field_entity_behavior_flags_ = static_cast<std::uint8_t>(
                field_entity_selection_state_
                    .records[field_entity_selection_state_.active_index]
                    .flags);
            field_movement_behavior_record_.behavior_flags =
                field_entity_behavior_flags_;
            field_entity_.projection.sprite_offset_y =
                active_runner_record().projection_word_08;
            source_boundary(
                "field.pocket_secretary.dismiss_runner.active_runner_switched");
        } else {
            source_boundary(
                "field.pocket_secretary.dismiss_runner.no_replacement_runner");
        }
    }

    refresh_field_hud_runner_slot();
    refresh_command_overview_combat_bars();
    refresh_command_overview_item_detail();
    source_boundary(
        "field.pocket_secretary.dismiss_runner.effect_asset_refresh_pending");
    source_boundary("field.pocket_secretary.dismiss_runner.dismissed");
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

void FieldSceneRuntime::begin_command_overview_magic() {
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page != 5U) {
        source_boundary("field.command_overview.magic.invalid_route");
        return;
    }

    const auto path = select_magic_menu_path(active_runner_record());
    if (path == MagicMenuPath::spell_panel) {
        const auto* table = find_us_magic_spell_table(
            active_runner_record().magic_table_index);
        if (table == nullptr || table->entry_count == 0U) {
            source_boundary(
                "field.command_overview.magic.spell_table_unavailable");
            return;
        }
        // FBA6 clears both source cursor coordinates on each entry; a second
        // visit starts at the first spell instead of leaking the last cursor.
        command_overview_magic_loop_ = MagicMenuLoopState{};
        command_overview_magic_loop_.first_entry_count =
            static_cast<std::uint16_t>(table->entry_count - 1U);
        command_overview_magic_selected_spell_ = select_magic_spell(
            *table, command_overview_magic_loop_.selection,
            us_magic_spell_catalog());
        if (!command_overview_magic_selected_spell_.has_value()) {
            source_boundary(
                "field.command_overview.magic.initial_spell_unavailable");
            return;
        }
        command_overview_magic_force_ = MagicForceState{};
        prepare_magic_menu_grid_and_select_force(
            command_overview_magic_force_,
            *command_overview_magic_selected_spell_, active_runner_record());
        command_overview_magic_cursor_phase_ = 0U;
        command_overview_magic_cursor_timer_ = 0U;
        command_overview_magic_state_ = CommandOverviewMagicState::spell_panel;
        source_boundary("field.command_overview.magic.spell_panel.opened");
        return;
    }

    std::array<EntityActionRecord, FieldPartyRosterRuntime::kSlotCount>
        ready_entities{};
    const auto& records = field_party_roster_.records();
    for (std::size_t index = 0; index < ready_entities.size(); ++index) {
        ready_entities[index] = magic_action_record_from_runner(records[index]);
    }
    const auto actor = magic_action_record_from_runner(active_runner_record());
    const auto physical =
        prepare_magic_fallback_defense_strip(ready_entities, actor, false);
    const auto mana =
        prepare_magic_fallback_defense_strip(ready_entities, actor, true);
    // The source conversion plan retains addresses for auditing. The Qt
    // runtime keeps only native values and named asset rows for presentation.
    command_overview_magic_physical_defense_ = {
        physical.value, static_cast<std::uint8_t>(physical.row)};
    command_overview_magic_mana_defense_ = {
        mana.value, static_cast<std::uint8_t>(mana.row)};
    command_overview_magic_state_ =
        CommandOverviewMagicState::magical_defense_wait;
    source_boundary("field.command_overview.magic.magical_defense.opened");
}

void FieldSceneRuntime::begin_command_overview_attributes_skills() {
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page != 3U) {
        source_boundary(
            "field.command_overview.attributes_skills.invalid_route");
        return;
    }
    command_overview_attributes_skills_.open(active_runner_record());
    source_boundary(
        "field.command_overview.attributes_skills.opened");
}

void FieldSceneRuntime::begin_command_overview_magic_spell_action() {
    if (command_overview_magic_state_ !=
            CommandOverviewMagicState::spell_panel ||
        !command_overview_magic_selected_spell_.has_value()) {
        source_boundary("field.command_overview.magic.action.invalid_route");
        return;
    }
    const auto* spell = find_us_magic_spell(
        command_overview_magic_selected_spell_->spell_code);
    if (spell == nullptr) {
        source_boundary("field.command_overview.magic.action.spell_missing");
        return;
    }

    MagicActionDetailCallbacks detail_callbacks{};
    detail_callbacks.prepare_menu_grid_then_service = [this] {
        source_boundary("field.command_overview.magic.details.grid_prepared");
    };
    detail_callbacks.render_action_text = [this](const auto region) {
        switch (region) {
        case MagicActionTextRegion::set_force_and_posture_heading:
            source_boundary(
                "field.command_overview.magic.details.heading_rendered");
            break;
        case MagicActionTextRegion::success_label:
            source_boundary(
                "field.command_overview.magic.details.success_label_rendered");
            break;
        case MagicActionTextRegion::force_label:
            source_boundary(
                "field.command_overview.magic.details.force_label_rendered");
            break;
        }
    };
    detail_callbacks.adjust_runner_posture = [this] {
        (void)adjust_runner_posture(active_runner_record(), 0);
        source_boundary("field.command_overview.magic.details.posture_refreshed");
    };
    detail_callbacks.render_spell_availability = [this](const auto& selected) {
        MagicSpellAvailabilityCallbacks availability_callbacks{};
        availability_callbacks.write_repeated_grid = [this] {
            source_boundary(
                "field.command_overview.magic.availability.cleared");
        };
        availability_callbacks.render_spell_text = [this](const auto&) {
            source_boundary(
                "field.command_overview.magic.availability.label_rendered");
        };
        availability_callbacks.write_grid = [this] {
            source_boundary(
                "field.command_overview.magic.availability.bar_rendered");
        };
        render_magic_spell_availability(selected, availability_callbacks);
    };
    detail_callbacks.build_grouped_decimal_glyphs = [this] {
        source_boundary(
            "field.command_overview.magic.details.force_value_prepared");
    };
    prepare_magic_action_details(*spell, detail_callbacks);

    auto& runner = active_runner_record();
    if (runner.equipped_inventory_slot >= 0 &&
        runner.equipped_item_code != 0U) {
        // ROM 0xFF74 copies FFF096 into the active item's +0x4E after the
        // auxiliary marker service and before installing the selected spell.
        EntityAuxiliaryState active_item_auxiliary{runner.ammo_rounds};
        clear_entity_auxiliary_marker(active_item_auxiliary);
        runner.ammo_rounds = active_item_auxiliary.marker;
        runner.ammo_rounds = static_cast<std::uint8_t>(
            command_overview_magic_force_.current_force);
        source_boundary(
            "field.command_overview.magic.action.save_active_force");
    }
    runner.equipped_item_code = static_cast<std::uint8_t>(
        static_cast<unsigned>(spell->spell_code) + 1U);
    command_overview_magic_state_ =
        CommandOverviewMagicState::spell_force_selection;
    source_boundary("field.command_overview.magic.force.opened");
}

void FieldSceneRuntime::finish_command_overview_magic_spell_action() {
    if (!magic_force_selection_open() && !magic_posture_selection_open()) {
        source_boundary("field.command_overview.magic.action.finish.invalid");
        return;
    }
    auto& runner = active_runner_record();
    runner.equipped_inventory_slot = -1;
    runner.magic_force = static_cast<std::uint8_t>(
        command_overview_magic_force_.current_force);
    refresh_field_hud_runner_slot();
    refresh_command_overview_combat_bars();
    command_overview_magic_state_ = CommandOverviewMagicState::spell_panel;
    source_boundary("field.command_overview.magic.spell_equipped");
}

void FieldSceneRuntime::advance_command_overview_magic(
    const std::uint16_t controller_events) {
    if (command_overview_magic_cursor_timer_ == 0U) {
        command_overview_magic_cursor_timer_ = 6U;
        command_overview_magic_cursor_phase_ = static_cast<std::uint8_t>(
            (command_overview_magic_cursor_phase_ + 1U) %
            command_overview_magic_cursor_frames_.size());
    } else {
        --command_overview_magic_cursor_timer_;
    }

    const auto* table = find_us_magic_spell_table(
        active_runner_record().magic_table_index);
    if (table == nullptr) {
        source_boundary("field.command_overview.magic.spell_table_unavailable");
        return;
    }

    if (magic_force_selection_open()) {
        MagicForceSelection selection{
            static_cast<std::uint8_t>(
                std::min<std::uint16_t>(
                    command_overview_magic_force_.current_force, 0xFFU))};
        const auto force_result = apply_magic_force_input(
            selection, controller_events,
            static_cast<std::uint8_t>(std::min<std::uint16_t>(
                command_overview_magic_force_.maximum_force, 0xFFU)));
        if (force_result == MagicForceLoopResult::adjusted) {
            const auto delta = (controller_events & 0x0800U) != 0U ? 1 : -1;
            adjust_magic_force(active_runner_record(),
                               command_overview_magic_force_,
                               static_cast<std::int16_t>(delta));
            source_boundary(
                delta > 0
                    ? "field.command_overview.magic.force.raised"
                    : "field.command_overview.magic.force.lowered");
            return;
        }
        if (force_result == MagicForceLoopResult::finished) {
            source_boundary(
                "field.command_overview.magic.force.final_display_refreshed");
            if ((controller_events & (0x2000U | 0x1000U | 0x4000U)) != 0U) {
                finish_command_overview_magic_spell_action();
            } else if ((controller_events & 0x0200U) != 0U) {
                (void)adjust_runner_posture(active_runner_record(), 0);
                command_overview_magic_state_ =
                    CommandOverviewMagicState::spell_posture_selection;
                source_boundary(
                    "field.command_overview.magic.posture.opened");
            }
        }
        return;
    }

    if (magic_posture_selection_open()) {
        const auto posture_result = apply_runner_posture_input(
            active_runner_record(), controller_events);
        if (posture_result == RunnerPostureLoopResult::adjusted) {
            refresh_command_overview_combat_bars();
            source_boundary(
                "field.command_overview.magic.posture.adjusted");
            return;
        }
        if (posture_result == RunnerPostureLoopResult::finished) {
            source_boundary(
                "field.command_overview.magic.posture.final_display_refreshed");
            if ((controller_events & (0x2000U | 0x1000U | 0x4000U)) != 0U) {
                finish_command_overview_magic_spell_action();
            } else {
                command_overview_magic_state_ =
                    CommandOverviewMagicState::spell_force_selection;
                source_boundary(
                    "field.command_overview.magic.force.reopened");
            }
        }
        return;
    }

    const auto refresh_selection = [this, table] {
        command_overview_magic_selected_spell_ = select_magic_spell(
            *table, command_overview_magic_loop_.selection,
            us_magic_spell_catalog());
        if (!command_overview_magic_selected_spell_.has_value()) {
            source_boundary(
                "field.command_overview.magic.spell_entry_unavailable");
            return;
        }
        prepare_magic_menu_grid_and_select_force(
            command_overview_magic_force_,
            *command_overview_magic_selected_spell_, active_runner_record());
        source_boundary("field.command_overview.magic.spell_selection_changed");
    };
    if ((controller_events & 0x0400U) != 0U) {
        command_overview_magic_loop_.selection = move_magic_selection_column(
            command_overview_magic_loop_.selection, -1,
            command_overview_magic_loop_.first_entry_count);
        refresh_selection();
        command_overview_magic_cursor_timer_ = 0U;
        return;
    }
    if ((controller_events & 0x0800U) != 0U) {
        command_overview_magic_loop_.selection = move_magic_selection_column(
            command_overview_magic_loop_.selection, 1,
            command_overview_magic_loop_.first_entry_count);
        refresh_selection();
        command_overview_magic_cursor_timer_ = 0U;
        return;
    }
    if ((controller_events & 0x0100U) != 0U) {
        command_overview_magic_loop_.selection = move_magic_selection_row(
            command_overview_magic_loop_.selection, -1,
            command_overview_magic_loop_.first_entry_count);
        refresh_selection();
        command_overview_magic_cursor_timer_ = 0U;
        return;
    }
    if ((controller_events & 0x0200U) != 0U) {
        command_overview_magic_loop_.selection = move_magic_selection_row(
            command_overview_magic_loop_.selection, 1,
            command_overview_magic_loop_.first_entry_count);
        refresh_selection();
        command_overview_magic_cursor_timer_ = 0U;
        return;
    }
    if ((controller_events & (0x2000U | 0x1000U | 0x4000U)) != 0U) {
        begin_command_overview_magic_spell_action();
        return;
    }
    if ((controller_events & 0x8000U) != 0U) {
        command_overview_magic_state_ = CommandOverviewMagicState::inactive;
        command_overview_magic_selected_spell_.reset();
        source_boundary("field.command_overview.magic.spell_panel.returned");
    }
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
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
        source_boundary("field.command_overview.clips.selector_moved");
        break;
    case ClipsMenuEvent::confirmation_opened:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        source_boundary(
            "field.command_overview.clips.reload_confirmation_opened");
        break;
    case ClipsMenuEvent::confirmation_toggled:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
        source_boundary(
            command_overview_clips_.reload_confirmation_yes()
                ? "field.command_overview.clips.reload_confirmation_yes"
                : "field.command_overview.clips.reload_confirmation_no");
        break;
    case ClipsMenuEvent::reloaded:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.reloaded");
        break;
    case ClipsMenuEvent::trade_opened:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
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
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
        source_boundary(
            "field.command_overview.clips.trade.selector_moved");
        break;
    case ClipsMenuEvent::trade_quantity_changed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        source_boundary(
            "field.command_overview.clips.trade.quantity_changed");
        break;
    case ClipsMenuEvent::trade_recipient_changed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        source_boundary(
            "field.command_overview.clips.trade.recipient_changed");
        break;
    case ClipsMenuEvent::trade_transferred:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary(
            "field.command_overview.clips.trade.transferred");
        break;
    case ClipsMenuEvent::trade_closed:
        if (source_audio_event_callback_ &&
            (controller_events & 0x7000U) != 0U) {
            source_audio_event_callback_(0x24U);
        }
        source_boundary("field.command_overview.clips.trade.closed");
        break;
    case ClipsMenuEvent::distributed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.distributed");
        break;
    case ClipsMenuEvent::message_shown:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
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
        if (source_audio_event_callback_ &&
            (controller_events & 0x7000U) != 0U) {
            source_audio_event_callback_(0x24U);
        }
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
    const auto source_choice = static_cast<std::uint8_t>(*result.choice);
    if (source_choice < 2U) {
        source_boundary(
            "field.command_overview.inventory_action.choice_unresolved");
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }
    const auto choice_index = static_cast<std::size_t>(source_choice - 2U);
    if (choice_index >=
        field_menu_text_.inventory_action_choice_ids.size()) {
        source_boundary(
            "field.command_overview.inventory_action.choice_unresolved");
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }
    const auto& choice_id =
        field_menu_text_.inventory_action_choice_ids[choice_index];
    if (choice_id == "equip_or_use") {
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
            begin_command_overview_first_aid();
            // First Aid spans multiple controller frames, so its target and
            // result states retain the selected slot until the nested menu
            // returns. Other one-shot item actions clear it below.
            command_overview_inventory_action_active_ = false;
            return;
        } else if (branch == SelectedItemBranch::automatic_use) {
            command_overview_automatic_use_state_ =
                CommandOverviewAutomaticUseState::result_wait;
            command_overview_inventory_action_active_ = false;
            source_boundary(
                "field.command_overview.inventory_action.automatic_use_opened");
            return;
        }
        command_overview_inventory_action_active_ = false;
        command_overview_selected_inventory_item_.reset();
        return;
    }

    if (choice_id == "discard") {
        if (!can_discard_selected_item(*result.item)) {
            command_overview_action_message_ =
                CommandOverviewActionMessage::cannot_discard;
            command_overview_inventory_action_active_ = false;
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

    if (choice_id == "trade") {
        if (!can_trade_selected_item(result.item->item_code)) {
            command_overview_action_message_ =
                CommandOverviewActionMessage::cannot_trade;
            command_overview_inventory_action_active_ = false;
            source_boundary(
                "field.command_overview.inventory_action.trade_rejected");
            return;
        }

        std::array<bool, kInventoryTradePartySlotCount> active_slots{};
        for (std::size_t slot = 0; slot < active_slots.size(); ++slot) {
            active_slots[slot] = field_party_roster_.slot_active(slot);
        }
        const auto trade = begin_inventory_trade(
            field_party_roster_.records(), active_slots,
            field_party_roster_.active_slot(), *result.item,
            command_overview_inventory_trade_);
        command_overview_inventory_action_active_ = false;
        if (trade.event == InventoryTradeEvent::no_recipient) {
            command_overview_selected_inventory_item_.reset();
            source_boundary(
                "field.command_overview.inventory_action.trade_no_recipient");
            return;
        }
        if (trade.event != InventoryTradeEvent::recipient_opened) {
            command_overview_selected_inventory_item_.reset();
            source_boundary(
                "field.command_overview.inventory_action.trade_open_failed");
            return;
        }
        source_boundary(
            trade.recipient_inventory_full
                ? "field.command_overview.inventory_action.trade_recipient_full"
                : "field.command_overview.inventory_action.trade_recipient_opened");
        return;
    }

    if (choice_id != "cancel") {
        source_boundary(
            "field.command_overview.inventory_action.choice_unresolved");
    }
    command_overview_inventory_action_active_ = false;
    command_overview_selected_inventory_item_.reset();
}

void FieldSceneRuntime::advance_command_overview_discard_confirmation(
    const std::uint16_t controller_events) {
    if ((controller_events & 0x0C00U) != 0U) {
        // ROM 0x12704 calls the shared Yes/No writer for either horizontal
        // edge; the source toggles its binary choice rather than assigning a
        // direction-specific value. The prompt starts on No.
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
        command_overview_discard_confirmation_.selected_option =
            command_overview_discard_confirmation_.selected_option == 0U ? 1U
                                                                          : 0U;
        source_boundary(
            command_overview_discard_confirmation_.selected_option == 0U
                ? "field.command_overview.discard_confirmation.select_yes"
                : "field.command_overview.discard_confirmation.select_no");
        return;
    }
    // 0x126F8/0x12750 confirm on A or C. B and Start are ignored in this
    // source loop; neither is a cancel shortcut here.
    if ((controller_events & 0x5000U) == 0U) {
        return;
    }
    if (source_audio_event_callback_) {
        source_audio_event_callback_(0x24U);
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
    const auto choice_index = static_cast<std::size_t>(
        command_overview_discard_confirmation_.selected_option);
    if (choice_index >= field_menu_text_.confirmation_choice_ids.size()) {
        source_boundary(
            "field.command_overview.discard_confirmation.choice_unresolved");
        return;
    }
    const auto& choice_id =
        field_menu_text_.confirmation_choice_ids[choice_index];
    if (choice_id == "yes") {
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
    } else if (choice_id == "no") {
        source_boundary(
            "field.command_overview.discard_confirmation.selected_no");
    } else {
        source_boundary(
            "field.command_overview.discard_confirmation.choice_unresolved");
    }
}

void FieldSceneRuntime::advance_command_overview_inventory_trade(
    const std::uint16_t controller_events) {
    if (!command_overview_inventory_trade_.open) {
        return;
    }
    const auto left = (controller_events & 0x0400U) != 0U;
    const auto right = (controller_events & 0x0800U) != 0U;
    // ROM 0x129A6..0x129FC routes A, B, and C through the same confirm path.
    const auto confirm = (controller_events & 0x7000U) != 0U;
    if (left || right) {
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
    } else if (confirm && source_audio_event_callback_) {
        source_audio_event_callback_(0x24U);
    }

    std::array<bool, kInventoryTradePartySlotCount> active_slots{};
    for (std::size_t slot = 0; slot < active_slots.size(); ++slot) {
        active_slots[slot] = field_party_roster_.slot_active(slot);
    }
    const auto result = advance_inventory_trade(
        field_party_roster_.records(), active_slots,
        command_overview_inventory_trade_,
        InventoryTradeInput{left, right, confirm});
    switch (result.event) {
    case InventoryTradeEvent::no_change:
        break;
    case InventoryTradeEvent::choice_moved:
        source_boundary(
            result.yes_selected
                ? "field.command_overview.inventory_action.trade_select_yes"
                : "field.command_overview.inventory_action.trade_select_no");
        break;
    case InventoryTradeEvent::recipient_advanced:
        source_boundary(
            "field.command_overview.inventory_action.trade_recipient_advanced");
        break;
    case InventoryTradeEvent::full_recipient_advanced:
        source_boundary(
            "field.command_overview.inventory_action.trade_full_recipient_advanced");
        break;
    case InventoryTradeEvent::no_recipient:
    case InventoryTradeEvent::recipient_opened:
        source_boundary(
            "field.command_overview.inventory_action.trade_invalid_transition");
        break;
    case InventoryTradeEvent::finished:
        command_overview_selected_inventory_item_.reset();
        command_overview_inventory_action_active_ = false;
        source_boundary(
            "field.command_overview.inventory_action.trade_finished");
        break;
    case InventoryTradeEvent::transferred:
        command_overview_selected_inventory_item_.reset();
        command_overview_inventory_action_active_ = false;
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
        source_boundary(
            "field.command_overview.inventory_action.trade_transferred");
        break;
    case InventoryTradeEvent::transfer_failed:
        command_overview_selected_inventory_item_.reset();
        command_overview_inventory_action_active_ = false;
        source_boundary(
            "field.command_overview.inventory_action.trade_transfer_failed");
        break;
    }
}

void FieldSceneRuntime::advance_command_overview_inventory_action(
    const std::uint16_t controller_events) {
    if (command_overview_discard_confirmation_active_) {
        advance_command_overview_discard_confirmation(controller_events);
        return;
    }
    if (command_overview_action_message_ !=
        CommandOverviewActionMessage::none) {
        if ((controller_events & 0xFF00U) != 0U) {
            command_overview_action_message_ =
                CommandOverviewActionMessage::none;
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
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
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
        // 0x11C3C requests the command-confirm sound before entering the
        // selected equip/discard/trade/cancel handler.
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x24U);
        }
        dispatch_command_overview_inventory_action(result);
        break;
    case InventoryActionEvent::opened:
    case InventoryActionEvent::no_change:
        break;
    }
}

void FieldSceneRuntime::begin_command_overview_first_aid() {
    if (!command_overview_selected_inventory_item_.has_value()) {
        source_boundary("field.command_overview.first_aid.missing_item");
        return;
    }

    std::array<std::uint8_t, FieldPartyRosterRuntime::kSlotCount> flags{};
    const auto& records = field_party_roster_.records();
    for (std::size_t index = 0; index < records.size(); ++index) {
        // The ROM tests bit zero in the first byte at +0x10. The typed word
        // is big-endian, so that byte is its high byte.
        flags[index] = static_cast<std::uint8_t>(records[index].flags >> 8U);
    }
    command_overview_first_aid_targets_ = build_first_aid_target_list(flags);
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::target_selection;
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    source_boundary("field.command_overview.first_aid.targets_opened");
}

void FieldSceneRuntime::finish_command_overview_first_aid() {
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    command_overview_first_aid_targets_ = FirstAidTargetList{};
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    command_overview_selected_inventory_item_.reset();
    // 0x11928 redraws the inventory row after the nested action; the
    // reference keeps the selected item's detail but clears its gold cursor.
    command_overview_inventory_selector_suppressed_ = true;
}

void FieldSceneRuntime::finish_command_overview_automatic_use() {
    command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    command_overview_inventory_action_active_ = false;
    command_overview_selected_inventory_item_.reset();
    // After the 0x0D94 acknowledgement the source redraws the inventory row
    // through 0x11928, which clears the yellow item selector.
    command_overview_inventory_selector_suppressed_ = true;
}

void FieldSceneRuntime::advance_command_overview_automatic_use(
    const std::uint16_t controller_events) {
    if (command_overview_automatic_use_state_ !=
        CommandOverviewAutomaticUseState::result_wait) {
        return;
    }
    // 0x121DA tail-calls 0x0D94. That shared wait accepts a newly pressed
    // controller event, so held buttons and released frames leave the
    // acknowledgement screen in place.
    if ((controller_events & 0xFF00U) != 0U) {
        finish_command_overview_automatic_use();
        source_boundary(
            "field.command_overview.inventory_action.automatic_use_acknowledged");
    }
}

void FieldSceneRuntime::advance_command_overview_first_aid(
    const std::uint16_t controller_events) {
    if (command_overview_first_aid_state_ ==
        CommandOverviewFirstAidState::inactive) {
        return;
    }

    if (command_overview_first_aid_state_ ==
        CommandOverviewFirstAidState::result_wait) {
        // ROM 0x0D94 waits for a newly pressed high-byte event. A held
        // confirm button must not dismiss the result on the following frame.
        if ((controller_events & 0xFF00U) != 0U) {
            finish_command_overview_first_aid();
            source_boundary(
                "field.command_overview.first_aid.result_acknowledged");
        }
        return;
    }

    if ((controller_events & 0x0100U) != 0U ||
        (controller_events & 0x0200U) != 0U) {
        const auto delta = (controller_events & 0x0100U) != 0U
                               ? std::int8_t{-1}
                               : std::int8_t{1};
        command_overview_first_aid_cursor_ =
            advance_first_aid_target_cursor(
                command_overview_first_aid_cursor_,
                command_overview_first_aid_targets_.active_count, delta);
        if (source_audio_event_callback_) {
            source_audio_event_callback_(0x25U);
        }
        source_boundary("field.command_overview.first_aid.target_moved");
        return;
    }

    // ROM 0x12402 tests only input bit 14 (A). B/C are sampled by the shared
    // controller service but do not confirm a First Aid target.
    if ((controller_events & 0x4000U) == 0U) {
        return;
    }
    if (source_audio_event_callback_) {
        source_audio_event_callback_(0x24U);
    }
    if (command_overview_first_aid_cursor_ ==
        command_overview_first_aid_targets_.cancel_row()) {
        finish_command_overview_first_aid();
        source_boundary("field.command_overview.first_aid.cancelled");
        return;
    }
    if (!command_overview_selected_inventory_item_.has_value() ||
        command_overview_first_aid_cursor_ >=
            command_overview_first_aid_targets_.active_count) {
        source_boundary("field.command_overview.first_aid.target_missing");
        finish_command_overview_first_aid();
        return;
    }

    const auto target_index = command_overview_first_aid_targets_
                                  .active_record_indices[
                                      command_overview_first_aid_cursor_];
    if (target_index >= field_party_roster_.records().size()) {
        source_boundary("field.command_overview.first_aid.target_out_of_range");
        finish_command_overview_first_aid();
        return;
    }

    auto& target_record = field_party_roster_.records()[target_index];
    const auto item = *command_overview_selected_inventory_item_;
    if (item.slot >= active_runner_record().inventory_item_codes.size()) {
        source_boundary("field.command_overview.first_aid.item_slot_invalid");
        finish_command_overview_first_aid();
        return;
    }

    FirstAidTargetState target{
        target_record.delay_meter,
        target_record.behavior_meter,
        target_record.attributes[0],
        static_cast<std::uint8_t>(target_record.flags >> 8U),
        target_record.first_aid_continuation};
    const FirstAidUserState user{active_runner_record().skills[8]};
    const auto roll_service = [this](const std::uint8_t roll_count,
                                     const std::uint8_t difficulty) {
        return count_thresholded_random_rolls(
            roll_count, static_cast<std::int16_t>(difficulty),
            [this](const std::uint16_t bound) {
                return sample_source_random_bound(field_random_seed_, bound);
            });
    };

    auto& item_code = active_runner_record().inventory_item_codes[item.slot];
    auto& item_phase = active_runner_record().inventory_phase_bytes[item.slot];
    const auto resolution = use_first_aid_item(
        item_code, item_phase, user, target, roll_service);
    if (resolution.result == FirstAidItemResult::used) {
        target_record.delay_meter = target.physical_meter;
        target_record.behavior_meter = target.mental_meter;
        target_record.flags = static_cast<std::uint16_t>(
            (target_record.flags & 0x00FFU) |
            (static_cast<std::uint16_t>(target.flags) << 8U));
        target_record.first_aid_continuation = target.continuation;
        command_overview_first_aid_message_ = resolution.message;
        if (target_index == 0U) {
            refresh_field_hud_runner_slot();
        }
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
        source_boundary("field.command_overview.first_aid.item_used");
    } else if (resolution.result == FirstAidItemResult::target_not_injured) {
        command_overview_first_aid_message_ = resolution.message;
        source_boundary("field.command_overview.first_aid.target_not_injured");
    } else {
        source_boundary("field.command_overview.first_aid.item_unavailable");
        finish_command_overview_first_aid();
        return;
    }

    command_overview_selected_inventory_item_->item_code = item_code;
    command_overview_selected_inventory_item_->phase = item_phase;
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::result_wait;
}

bool FieldSceneRuntime::bootstrap(
    const integration::StartupIntegrationState& startup_state) {
    ready_ = false;
    failure_.clear();
    field_shop_menu_.close();
    field_shop_route_pending_ = false;
    field_funds_.reset();
    command_overview_magic_state_ = CommandOverviewMagicState::inactive;
    command_overview_magic_physical_defense_ = MagicDefenseStripView{};
    command_overview_magic_mana_defense_ = MagicDefenseStripView{};
    state_ = integration::FieldSessionBootstrapState{};
    field_session_source_assets_.reset();
    field_asset_catalog_.reset();
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
    // ROM 0x007F0 initializes FFFBFE to 20 on the New Game route before the
    // field is entered. The current save adapter does not restore that global.
    field_funds_ = restored_field_state == nullptr
                       ? std::optional<std::uint32_t>{0x14U}
                       : std::nullopt;
    field_runner_feature_flags_ =
        restored_field_state == nullptr
            ? std::optional<std::uint8_t>{0U}
            : std::nullopt;
    field_entity_action_phase_roll_counters_ =
        restored_field_state == nullptr
            ? std::optional<std::array<std::uint8_t, 3>>{
                  std::array<std::uint8_t, 3>{}}
            : std::nullopt;
    pocket_secretary_ = PocketSecretaryMenuState{};
    dismiss_runner_ = DismissRunnerMenuState{};
    dismiss_runner_random_grid_effect_ = RandomGridEffectState{};
    dismiss_runner_effect_frames_ = 0U;
    dismiss_runner_random_grid_cells_.fill(-1);
    // The 0x59C48 New Game path begins without the FF018F deck-present bit.
    // The loaded-save contract does not yet expose this global, so keep that
    // branch explicit instead of treating an unknown saved bit as clear.
    cyberdeck_available_ = restored_field_state == nullptr
                               ? std::optional<bool>{false}
                               : std::nullopt;
    group_item_flags_ = restored_field_state == nullptr
                            ? std::optional<std::uint32_t>{0U}
                            : std::nullopt;
    // ROM startup 0x084E seeds FFFC30 bit 0 for a fresh New Game.
    notebook_tip_flags_ = {{1U, 0U, 0U}};
    notebook_tip_flags_resolved_ = restored_field_state == nullptr;
    // 0x82A clears FFFC2C/27/2A; New Game setup later calls 0x59C48 to seed
    // the Contacts gate to 0x1C7F. No loaded-save adapter restores these.
    notebook_contacts_flags_ = restored_field_state == nullptr
                                   ? std::optional<std::uint32_t>{0x1C7FU}
                                   : std::nullopt;
    notebook_referral_flags_ = restored_field_state == nullptr
                                   ? std::optional<std::uint8_t>{0U}
                                   : std::nullopt;
    notebook_shadowrunner_flags_ = restored_field_state == nullptr
                                      ? std::optional<std::uint16_t>{0U}
                                      : std::nullopt;
    const auto& assets = *startup_state.field_session_source_assets;
    field_session_source_assets_ =
        std::make_unique<integration::FieldSessionSourceAssets>(assets);
    field_asset_catalog_ = std::make_unique<ConversionAssetCatalog>(
        startup_state.asset_catalog.catalog);
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
    field_random_seed_ = static_cast<std::uint16_t>(
        QRandomGenerator::global()->generate());
    field_opening_auto_walk_ = FieldOpeningAutoWalkState{};
    field_controller_sample_state_ = ControllerSampleState{};
    field_controller_latch_state_ = ControllerLatchState{};
    field_target_heading_ = 0U;
    field_movement_state_ = EntityMovementStepState{};
    field_collision_record_ = EntityCollisionRecord{};
    field_collision_probe_state_ = CollisionProbeState{};
    field_movement_behavior_record_ = EntityBehaviorFrameRecord{};
    field_movement_transition_state_ = EntityMovementModeTransitionState{};
    const auto script_directory =
        startup_state.asset_catalog.catalog.asset_root.parent_path() /
        "gameplay" / "scripts";
    const auto dialogue_text_file =
        startup_state.asset_catalog.catalog.asset_root / "dialogue" /
        "script-text.lua";
    if (!game_scripts_.load_package(script_directory,
                                   dialogue_text_file)) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.gameplay_scripts_missing");
        return false;
    }
    const auto map_rules = game_scripts_.map_event_rules();
    if (!map_rules.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.map_event_scripts_invalid");
        return false;
    }
    field_shop_definition_ =
        game_scripts_.shop_definition("ares_weapon_emporium");
    if (!field_shop_definition_.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.shop_script_invalid");
        return false;
    }
    field_map_event_system_ = FieldMapEventSystem(*map_rules);
    field_map_event_state_flags_ = 0U;
    field_palette_fade_steps_ = 0U;
    field_world_activity_scene_entry_pending_ = false;
    field_world_activity_state_ = WorldActivityState{};
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    world_activity_routine_state_ = WorldActivityRoutineState{};
    field_weapon_projectiles_.clear();
    field_weapon_slots_occupied_.fill(false);
    field_weapon_collision_targets_pending_reported_ = false;
    last_field_weapon_spawn_result_ = EntitySpawnCycleResult{};
    field_security_floor_index_.reset();
    field_activity_continuation_ = MovementModeTransitionContinuation::none;
    field_projected_terrain_mark_count_ = 0U;
    field_terrain_render_dirty_ = false;
    field_terrain_sample_state_ = {};
    field_terrain_boundary_active_ = false;
    field_terrain_outside_bounds_marker_ = false;
    field_action_counter_ = 0U;
    field_entity_behavior_flags_ = 0U;
    field_exit_alternate_result_ = 0;
    field_exit_world_flags_ = 0U;
    stoker_motel_story_state_ = StokerMotelStoryState{};
    game_scripts_.set_story_fact_accessors(
        [this](const std::string_view name) -> std::optional<bool> {
            if (name == "opening_dialogue_seen") {
                return stoker_motel_story_state_.opening_dialogue_seen;
            }
            if (name == "michael_debt_paid") {
                return stoker_motel_story_state_.michael_debt_paid;
            }
            if (name == "michael_package_collected") {
                return stoker_motel_story_state_.michael_package_collected;
            }
            return std::nullopt;
        },
        [this](const std::string_view name, const bool value) {
            if (name == "opening_dialogue_seen") {
                stoker_motel_story_state_.opening_dialogue_seen = value;
                return true;
            }
            if (name == "michael_debt_paid") {
                stoker_motel_story_state_.michael_debt_paid = value;
                return true;
            }
            if (name == "michael_package_collected") {
                stoker_motel_story_state_.michael_package_collected = value;
                return true;
            }
            return false;
        });
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
    hotel_menu_.close();
    hotel_menu_content_ = HotelMenuContent{};
    hotel_karma_skills_.close_to_parent();
    field_mode_text_.close();
    field_interaction_.reset_session();
    field_elevator_selector_ = ElevatorSelectorRuntime{};
    field_elevator_scene_transition_ = ElevatorSceneTransitionRuntime{};
    field_party_roster_.clear();
    field_menu_text_ = FieldMenuTextCatalog{};
    command_overview_resource_variant_ = ResourceVariantState{};
    command_overview_combat_bars_ = EquipmentCombatBarResult{};
    command_overview_item_detail_ = ItemActionDetailResult{};
    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    command_overview_first_aid_targets_ = FirstAidTargetList{};
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    command_overview_inventory_selector_suppressed_ = false;
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_inventory_trade_ = InventoryTradeState{};
    command_overview_action_message_ = CommandOverviewActionMessage::none;
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    command_overview_clips_.close();
    field_joshua_archetype_selection_ = 0U;
    field_hud_slots_ = {};
    field_entity_animation_ = {};
    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_selector_ =
        select_field_entity_animation_selector(0, false);
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
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
    // The controlled runner is copied from the selected 0x7291A archetype or
    // restored save record. Its live +0x10 flags gate field interactions in
    // 0xB7B8; 0x44BC's separate 0x05 default is for spawned world entities.
    field_entity_behavior_flags_ =
        field_entity_.active
            ? static_cast<std::uint8_t>(active_runner_record().flags >> 8U)
            : 0U;
    initialize_field_entity_selection_state();
    refresh_field_hud_runner_slot();

    const auto menu_text = load_field_menu_text_catalog(
        startup_state.asset_catalog.catalog, game_scripts_);
    if (!menu_text.complete()) {
        failure_ = menu_text.errors.empty()
                       ? "field menu text catalog is incomplete"
                       : menu_text.errors.front();
        source_boundary("field.bootstrap.menu_text_missing");
        return false;
    }
    field_menu_text_ = menu_text.catalog;
    if (!load_command_overview_menu_content()) {
        return false;
    }
    if (!load_hotel_menu_content()) {
        return false;
    }
    if (!load_pocket_secretary_menu_content()) {
        return false;
    }
    if (!load_ares_shop_menu_content()) {
        return false;
    }
    if (!load_item_menu_content()) {
        return false;
    }

    // Dialogue is optional during initial field bootstrap because the player
    // does not start inside an NPC prompt. Load it once here so the later
    // 0x1F65A callback can bind direct assets without rebuilding a catalog on
    // every prompt or reaching back into the ROM.
    (void)field_dialogue_.load_assets(startup_state.asset_catalog.catalog);
    field_mode_text_.set_script_runtime(&game_scripts_);
    (void)field_mode_text_.load_assets(startup_state.asset_catalog.catalog);
    if (!field_interaction_.load_assets(startup_state.asset_catalog.catalog)) {
        failure_ = field_interaction_.failure();
        source_boundary("field.bootstrap.interaction_assets_missing");
        return false;
    }

    const auto field_map_alias =
        std::string("field.maps.mode.") + std::to_string(scene_reference);
    if (!load_image(startup_state.asset_catalog.catalog, field_map_alias,
                    field_map_) ||
        !load_image(startup_state.asset_catalog.catalog, kFieldHudAlias,
                    field_hud_) ||
        !load_image(startup_state.asset_catalog.catalog,
                    "menu.field.service.background",
                    field_service_background_) ||
        !load_field_hud_overlays(startup_state.asset_catalog.catalog) ||
        !load_command_overview_assets(startup_state.asset_catalog.catalog) ||
        !load_field_weapon_spawn_assets(
            startup_state.asset_catalog.catalog) ||
        !load_field_elevator_selector_assets(
            startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.direct_assets_missing");
        return false;
    }
    for (std::size_t index = 0; index < field_service_tab_images_.size();
         ++index) {
        const auto alias = "menu.field.service.tabs.selected." +
                           std::to_string(index);
        if (!load_image(startup_state.asset_catalog.catalog, alias,
                        field_service_tab_images_[index])) {
            source_boundary("field.bootstrap.service_tab_asset_missing");
            return false;
        }
    }
    if (!load_image(startup_state.asset_catalog.catalog,
                    "menu.field.service.universal.clips",
                    field_service_universal_clips_icon_)) {
        source_boundary("field.bootstrap.service_clips_icon_missing");
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
        const auto horizontal_flip =
            field_entity_animation_flips_horizontally(
                static_cast<std::uint8_t>(direction));
        if (horizontal_flip) {
            field_entity_idle_images_[direction] =
                field_entity_idle_images_[direction].flipped(
                    Qt::Horizontal);
        }
        const auto attack_alias = std::string(kFieldEntityAttackAliasPrefix) +
                                  std::to_string(direction);
        if (!load_sprite_image(
                startup_state.asset_catalog.catalog, attack_alias,
                field_entity_attack_images_[direction],
                field_entity_attack_origin_x_[direction],
                field_entity_attack_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
        if (horizontal_flip) {
            field_entity_attack_images_[direction] =
                field_entity_attack_images_[direction].flipped(
                    Qt::Horizontal);
        }
        // The ordinary movement poses come from the C062 positive-marker
        // list, extracted as separate host files. C0B2 is a different finite
        // behavior-transition sequence. Direction 5..7 descriptor records
        // set source attribute 0x0800; bake that facing into the native image
        // once when loading each typed direction set.
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
            if (horizontal_flip) {
                walk_image = walk_image.flipped(Qt::Horizontal);
            }
            field_entity_walk_images_[direction].push_back(
                std::move(walk_image));
            field_entity_walk_origin_x_[direction].push_back(walk_origin_x);
            field_entity_walk_origin_y_[direction].push_back(walk_origin_y);
            ++loaded_frames;
        }
        if (loaded_frames != kFieldWalkAnimationFrameCount) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
    }
    if (!load_field_default_actor_assets(
            startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.default_actor_assets_missing");
        return false;
    }
    initialize_default_field_actors(scene_reference);

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
    // New Game enters 0x075E with FFDF40 still clear. The scene-derived
    // interaction mode is assigned later by 0x167AE, after a world-activity
    // handoff. The loaded-save path also clears FFDF40 in 0x594E0 before its
    // 0x03074/0x028C2 re-entry. Do not run the later scene-mode selector during
    // this initial bootstrap: scene 2's motel contact depends on mode 0 here.
    field_movement_transition_state_.world_mode = 0U;
    field_movement_transition_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    field_movement_transition_state_.world_activity_source_word = 0U;
    // ROM 0x59C5E leaves FFE18B set after the New Game opening. 0xACAE
    // consumes that startup state to begin the ordinary heading-zero field
    // continuation (0xB574); a restored save never takes this branch.
    field_opening_auto_walk_ = FieldOpeningAutoWalkState{};
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

    if (startup_state.new_game_opening_completed &&
        restored_field_state == nullptr) {
        field_map_event_state_flags_ |= field_map_event_flag(
            FieldMapEventStateFlag::new_game_opening_completed);
    }
    dispatch_field_map_scene_entry(scene_reference);

    ready_ = true;
    if (field_opening_auto_walk_.active) {
        source_boundary("field.opening_auto_walk.armed");
    }
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
    if (continuation ==
            MovementModeTransitionContinuation::maglock_door_interaction ||
        continuation ==
            MovementModeTransitionContinuation::building_security_terminal ||
        continuation ==
            MovementModeTransitionContinuation::event_29_world_activity) {
        field_runtime_state_.pending_field_scene_handler = true;
    }
    // ROM 0x1E2E0/0x1E2EC stores the typed continuation in the activity
    // timer. Scene-table selection belongs to the later 0x167AE dispatcher;
    // it is not a return value of the timer helper.
    source_boundary("field.world.activity.started");
}

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

bool FieldSceneRuntime::recover_blocked_field_movement(
    const MovementProbeResult& initial_probe) {
    field_movement_transition_state_.terrain_collision_code =
        static_cast<std::uint16_t>(initial_probe.collision_code);
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
    transition_callbacks.dispatch_field_map_event =
        [this](const FieldMapCollisionEventInput& input) {
            const auto dispatch =
                field_map_event_system_.dispatch_collision(input);
            source_boundary("field.map_event.collision_dispatched");
            return dispatch.decision;
        };
    transition_callbacks.begin_world_activity_if_idle =
        [this](const MovementModeTransitionContinuation continuation) {
            const auto started = begin_world_activity_if_idle(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            if (started) {
                world_frame_logic_state_.resource_countdown =
                    field_world_activity_state_.timeout;
                record_field_activity_start(continuation);
            }
            return started;
        };
    transition_callbacks.begin_world_activity =
        [this](const MovementModeTransitionContinuation continuation) {
            begin_world_activity(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            world_frame_logic_state_.resource_countdown =
                field_world_activity_state_.timeout;
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
    last_field_movement_transition_result_ = result;
    if (result.mode_event_code_published) {
        source_boundary("field.world.activity.mode_event_published");
    }
    if (result.kind ==
        EntityMovementModeTransitionResultKind::projected_interaction_pending) {
        state_.display_service_word = 0xFFFFU;
        field_runtime_state_.display_service_word = 0xFFFFU;
        field_palette_fade_steps_ = 0U;
    }
    source_boundary("field.movement.blocked_transition");
    return (field_movement_behavior_record_.route_flags & 0x01U) != 0U;
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y) {
    return evaluate_field_collision(candidate_x, candidate_y,
                                    field_collision_record_,
                                    field_terrain_sample_state_,
                                    field_entity_behavior_flags_, true);
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y,
    EntityCollisionRecord& entity_collision_record,
    std::array<std::uint8_t, 4>& sample_state,
    const std::uint8_t entity_behavior_flags,
    const bool publish_movement_state) {
    const auto collision_evaluator =
        [this, &sample_state, entity_behavior_flags, publish_movement_state](
            const std::uint16_t evaluated_x,
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
                                      entity_behavior_flags,
                                      bounds,
                                      sample_state},
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
            sample_state = field_sample_state_from_result(
                result.packed_sample_state);
            // ROM 0xE7C8 clears and republishes both globals for every probe.
            // 0x14F50 reads FFDFE2 only after this result is available.
            field_terrain_boundary_active_ =
                result.state.directional_collision_marker;
            field_terrain_outside_bounds_marker_ =
                result.state.outside_bounds_marker;
            // ROM 0xE83E publishes the raw in-bounds terrain sample to
            // FFDF64 before its collision classifiers adjust the return code.
            // 0xAC5E consumes this value on the next entity pass to release
            // B7B8's one-shot bit after the runner has moved back onto clear
            // terrain. Outside-map returns leave FFDF64 unchanged.
            if (publish_movement_state &&
                !result.state.outside_bounds_marker) {
                field_movement_transition_state_.terrain_collision_code =
                    static_cast<std::uint16_t>(
                        result.state.raw_terrain_code);
            }
            // Outside the map, 0xE7C8 returns before 0xE8A0, so FFDFE4 stays
            // at the previous sample.
            if (!result.state.outside_bounds_marker) {
                field_collision_marker_index_ =
                    result.state.sample.selected_tile_index;
                // ROM 0xE8A0 publishes the selected map byte as FFDFE4.
                // B994 copies it to FFE11A when a mode-2 event is entered;
                // 0x2C1A then uses that index to advance a negative result.
                if (publish_movement_state) {
                    field_movement_transition_state_.world_activity_source_word =
                        field_collision_marker_index_;
                }
            }
            return CollisionEvaluation{
                result.code, result.packed_sample_state};
        };
    return evaluate_entity_collision(entity_collision_record, candidate_x,
                                     candidate_y, collision_evaluator);
}

void FieldSceneRuntime::advance_field_movement() {
    if (!ready_ || !field_entity_.active) {
        // The source keeps the walking pose only while a direction is held.
        // When the controller goes idle, the heading animation returns to
        // the directional idle list instead of looping the walk cycle while
        // the runner stands still.
        if (field_entity_walking_) {
            apply_field_entity_animation(false);
        }
        return;
    }

    if (field_opening_auto_walk_.active &&
        field_movement_transition_state_.interaction_transition_pending) {
        // Keep the startup marker alive through the fade, as the ROM does,
        // while the projected doorway event owns the field transition.
        apply_field_entity_animation(false);
        return;
    }

    if (!field_opening_auto_walk_.active && field_controller_nibble_ == 0U) {
        if (field_entity_walking_) {
            apply_field_entity_animation(false);
        }
        return;
    }

    const auto input = field_opening_auto_walk_.active
                           ? FieldMovementInputResult{
                                 true,
                                 field_opening_auto_walk_.target_heading,
                                 field_movement_direction_from_heading(
                                     field_opening_auto_walk_.target_heading),
                                 static_cast<std::uint16_t>(
                                     field_movement_direction_from_heading(
                                         field_opening_auto_walk_
                                             .target_heading) *
                                     2U)}
                           : decode_field_movement_input(
                                 field_controller_nibble_,
                                 field_controller_heading_table_);
    if (!input.valid) {
        return;
    }

    field_target_heading_ = input.target_heading;
    EntityCoordinateTarget heading{
        field_movement_state_.heading, field_target_heading_};
    approach_entity_fixed_point_target(heading);
    field_movement_state_.heading = heading.current;
    // The ordinary step presents the slewed +0x2E heading. ROM 0xBADE may
    // replace +0x36 with the direction of its committed diagonal fallback;
    // keep that source-facing value separate and let the fallback update it.
    field_movement_state_.animation_heading = field_movement_state_.heading;

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
        return probe(direction_slot);
    };

    const auto previous_x = field_movement_state_.position_x;
    const auto previous_y = field_movement_state_.position_y;
    const MovementResultPolicy result_policy{
        false,
        state_.field_scene_buffers.scene_reference,
        field_movement_transition_state_.world_mode};
    const auto reset_blocked_movement = [this] {
        // ROM 0xB442 resets the active entity after a blocked directional
        // probe. The field route previously left this callback as a marker
        // only, so a failed diagonal slide retained its partially slewed
        // heading and animation accumulator.
        EntityMovementState reset_state{};
        reset_state.movement_flags =
            field_movement_behavior_record_.route_flags;
        reset_state.current_heading = field_movement_state_.heading;
        reset_state.desired_heading = field_target_heading_;
        reset_state.animation_flags = field_movement_state_.movement_flags;
        reset_state.movement_status = static_cast<std::uint16_t>(
            field_entity_animation_.accumulator);
        reset_state.direction_animation = field_entity_animation_selector_;
        reset_state.collision = field_collision_record_;

        auto sample_state = field_terrain_sample_state_;
        reset_entity_movement_state(
            reset_state, kFieldIdleAnimationSelectors,
            field_movement_state_.position_x,
            field_movement_state_.position_y,
            [this, &reset_state, &sample_state](
                const std::uint16_t x, const std::uint16_t y) {
                const auto result = evaluate_field_collision(
                    x, y, reset_state.collision, sample_state,
                    field_entity_behavior_flags_, true);
                return CollisionEvaluation{
                    result, reset_state.collision.packed_state};
            });

        field_movement_behavior_record_.route_flags =
            reset_state.movement_flags;
        field_movement_state_.heading = reset_state.current_heading;
        field_movement_state_.movement_flags = reset_state.animation_flags;
        field_entity_animation_.accumulator = static_cast<std::int16_t>(
            reset_state.movement_status);
        field_entity_animation_.frame = 0U;
        field_entity_animation_.redraw_requested = true;
        field_entity_animation_selector_ = reset_state.direction_animation;
        field_entity_attack_pose_active_ = false;
        field_collision_record_ = reset_state.collision;
        field_terrain_sample_state_ = sample_state;
        source_boundary("field.movement.reset_blocked_step");
    };
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
        reset_blocked_movement);

    // ROM 0xAC5E checks FFDF64 after the terrain probe. A clear raw sample
    // releases the entity's B7B8 retry guard so a later doorway or terminal
    // can publish its event. Keep the other route bits intact.
    if ((field_entity_behavior_flags_ & 0x08U) != 0U &&
        field_movement_transition_state_.terrain_collision_code == 0U) {
        field_movement_behavior_record_.route_flags =
            static_cast<std::uint8_t>(
                field_movement_behavior_record_.route_flags &
                static_cast<std::uint8_t>(~0x20U));
    }

    field_entity_.world_position.x = field_movement_state_.position_x;
    field_entity_.world_position.y = field_movement_state_.position_y;
    field_entity_.direction =
        field_movement_direction_from_heading(
            field_movement_state_.animation_heading);
    if (previous_x != field_movement_state_.position_x ||
        previous_y != field_movement_state_.position_y) {
        source_boundary("field.movement.accepted_step");
        apply_field_entity_animation(true);
    } else {
        apply_field_entity_animation(false);
    }
}

void FieldSceneRuntime::service_player_weapon_action() {
    if (!ready_ || !field_entity_.active) {
        return;
    }

    auto& runner = active_runner_record();
    advance_active_entity_animation_action_gate(runner);

    // ROM 0xA8EA only calls 0xACAE for an active record with source +0x10 bit
    // 1 set. The ordinary field profile starts at 0x05 (active plus mode-5),
    // so merely standing with an empty weapon must not reload it.
    if ((runner.flags & 0x0200U) == 0U ||
        (field_controller_sample_state_.events & 0x0040U) == 0U) {
        return;
    }

    // ROM 0xACAE routes action codes 1..12 through the weapon spawn/ammo
    // service. Codes 13..15 use the separate effect-record path at 0x15646.
    if (runner.equipped_item_code == 0U || runner.equipped_item_code > 12U ||
        (runner.flags & 0x0100U) == 0U) {
        return;
    }

    const PlayerActionDispatchCallbacks reload_callbacks{
        [this](const std::uint16_t event) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(event);
            }
        }};
    if (runner.ammo_rounds == 0U) {
        const auto reload = dispatch_player_action_if_ready(
            runner, us_item_catalog(), reload_callbacks);
        if (reload.status == PlayerActionDispatchStatus::serviced) {
            refresh_field_hud_runner_slot();
            source_boundary("field.combat.player_auto_reload");
        }
        return;
    }

    // 0x1514E refuses another spawn while the source +0x16 animation window
    // is past 0x3C. B39A installs C042 selectors 0x40..0x5C, so movement must
    // return the runner to idle/walk before another shot can enter the window.
    if ((field_entity_animation_selector_ & 0x07FFU) > 0x003CU) {
        return;
    }

    if (runner.equipped_inventory_slot < 0 ||
        static_cast<std::size_t>(runner.equipped_inventory_slot) >=
            runner.inventory_phase_bytes.size()) {
        return;
    }

    const auto source_status_flags = runner.inventory_phase_bytes[
        static_cast<std::size_t>(runner.equipped_inventory_slot)];
    EntitySpawnCycleState spawn_state{};
    spawn_state.parent.position_x = field_movement_state_.position_x;
    spawn_state.parent.position_y = field_movement_state_.position_y;
    spawn_state.parent.heading = field_movement_state_.heading;
    spawn_state.parent.flags = static_cast<std::uint8_t>(runner.flags >> 8U);
    spawn_state.parent.field_0x56 = static_cast<std::uint8_t>(
        runner.equipped_inventory_slot);
    spawn_state.parent.field_0x57 = runner.equipped_item_code;
    spawn_state.parent.field_0x9d = runner.magic_force;
    spawn_state.equipment = EntityEquipmentState{
        runner.ammo_rounds, static_cast<std::int16_t>(runner.record_type)};
    spawn_state.animation_counter = field_entity_animation_selector_;
    spawn_state.global_mode = static_cast<std::uint8_t>(
        field_runtime_state_.field_scene_gate);
    spawn_state.entity_flags = spawn_state.parent.flags;
    spawn_state.source_status_flags = source_status_flags;

    const auto ammo_before = runner.ammo_rounds;
    std::vector<std::uint16_t> allocated_slots;
    const auto spawn_cycle = process_entity_spawn_cycle(
        spawn_state,
        [] { return true; },
        [this, &allocated_slots]() -> std::optional<std::uint16_t> {
            for (std::size_t index = 0;
                 index < field_weapon_slots_occupied_.size(); ++index) {
                if (!field_weapon_slots_occupied_[index]) {
                    field_weapon_slots_occupied_[index] = true;
                    const auto slot = static_cast<std::uint16_t>(9U + index);
                    allocated_slots.push_back(slot);
                    return slot;
                }
            }
            return std::nullopt;
        },
        [this] {
            // 0x55640 resolves the attacker's action outcome for the primary
            // record. Its damage/target path is not connected yet, but the
            // boundary stays visible in the live route rather than being
            // silently replaced with a guessed hit result.
            source_boundary("field.combat.weapon_hit_resolution_pending");
        },
        [this] {
            (void)schedule_world_activity_timeout(
                field_world_activity_timeout_state_);
        },
        [this](const std::uint16_t event) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(event);
            }
        },
        [this](const std::uint16_t) {
            source_boundary("field.combat.mode_effect_transition_pending");
        },
        [this, &runner](EntityEquipmentState& equipment,
                        const std::uint8_t source_action_code) {
            RunnerAssetOperationResult weapon_operation =
                RunnerAssetOperationResult::no_change;
            const auto ammo_action = service_entity_ammo(
                equipment, source_action_code,
                [this, &runner, &equipment,
                 &weapon_operation](const std::int8_t meter_delta) {
                    weapon_operation = service_entity_ammo(
                        runner, meter_delta, us_item_catalog());
                    equipment.current_ammo = runner.ammo_rounds;
                });

            if (equipment.record_type >= 3) {
                runner.ammo_rounds = equipment.current_ammo;
                return ammo_action;
            }
            if (weapon_operation ==
                RunnerAssetOperationResult::ammo_changed) {
                return ammo_action;
            }
            if (weapon_operation ==
                    RunnerAssetOperationResult::missing_source_data ||
                weapon_operation ==
                    RunnerAssetOperationResult::invalid_equipped_slot) {
                return EntityAmmoServiceAction::weapon_service_unresolved;
            }
            return EntityAmmoServiceAction::no_action;
        });
    last_field_weapon_spawn_result_ = spawn_cycle;
    for (const auto slot : allocated_slots) {
        const bool has_primary_record = spawn_cycle.primary.has_value() &&
            spawn_cycle.primary_slot.has_value() &&
            *spawn_cycle.primary_slot == slot;
        const bool has_secondary_record = spawn_cycle.secondary.has_value() &&
            spawn_cycle.secondary_slot.has_value() &&
            *spawn_cycle.secondary_slot == slot;
        if (!has_primary_record && !has_secondary_record && slot >= 9U &&
            slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(slot - 9U)] = false;
        }
    }
    if (runner.ammo_rounds != ammo_before) {
        refresh_field_hud_runner_slot();
    }

    if (spawn_cycle.primary.has_value() ||
        spawn_cycle.secondary.has_value()) {
        FieldWeaponProjectilePresentation spawn{};
        if (spawn_cycle.primary.has_value()) {
            spawn.primary_record = *spawn_cycle.primary;
            spawn.primary_spawn_position = SceneRunnerPosition{
                spawn.primary_record.position_x,
                spawn.primary_record.position_y};
            spawn.primary_active =
                (spawn.primary_record.flags & 0x01U) != 0U;
            spawn.primary_slot = spawn_cycle.primary_slot;
            spawn.primary_fixed_position = EntityFixedPointPosition{
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_x)) * 0x10000,
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_y)) * 0x10000,
                spawn.primary_record.heading_offset_x,
                spawn.primary_record.heading_offset_y};
            spawn.primary_animation.period = static_cast<std::int16_t>(
                spawn.primary_record.behavior_defaults.behavior_timer);
        }
        if (spawn_cycle.secondary.has_value()) {
            spawn.source_record = *spawn_cycle.secondary;
            spawn.secondary_active = true;
            spawn.secondary_slot = spawn_cycle.secondary_slot;
            spawn.animation.period = static_cast<std::int16_t>(
                spawn.source_record.behavior_defaults.behavior_timer);
        }

        const auto secondary_selector = static_cast<std::uint16_t>(
            spawn.source_record.animation_resource & 0x7FFFU);
        const auto secondary_animation_index = static_cast<std::size_t>(
            secondary_selector / 4U);
        if (spawn.secondary_active &&
            (secondary_animation_index >=
                 field_weapon_projectile_images_.size() ||
             field_weapon_projectile_images_[secondary_animation_index]
                 .empty())) {
            source_boundary("field.combat.weapon_projectile_asset_missing");
            spawn.secondary_active = false;
            if (spawn.secondary_slot.has_value() &&
                *spawn.secondary_slot >= 9U &&
                *spawn.secondary_slot <= 14U) {
                field_weapon_slots_occupied_[
                    static_cast<std::size_t>(*spawn.secondary_slot - 9U)] =
                    false;
            }
        }
        field_weapon_projectiles_.push_back(std::move(spawn));
    }

    const bool complete_spawn =
        spawn_cycle.action == EntitySpawnCycleAction::spawned &&
        spawn_cycle.primary.has_value() &&
        spawn_cycle.secondary.has_value();
    if (!complete_spawn) {
        if (spawn_cycle.action == EntitySpawnCycleAction::no_primary_slot ||
            spawn_cycle.action == EntitySpawnCycleAction::no_secondary_slot) {
            const auto fallback = dispatch_player_action_if_ready(
                runner, us_item_catalog(), reload_callbacks);
            if (fallback.status == PlayerActionDispatchStatus::serviced) {
                refresh_field_hud_runner_slot();
            }
        }
        return;
    }

    HeadingAnimationState attack_pose{};
    const auto spawned_heading = spawn_cycle.primary->heading;
    const auto direction = field_movement_direction_from_heading(
        spawned_heading);
    set_entity_heading_with_field_animation(
        attack_pose, spawned_heading,
        runner.equipped_item_code, kFieldAttackAnimationSelectors,
        [this, &runner](const std::uint16_t delay_mode) {
            EntityDelayState delay{runner.animation_action_gate};
            update_entity_animation_delay(
                delay,
                EntityDelayAttributes{runner.attributes[1U],
                                      runner.attributes[4U],
                                      runner.cyberware_flags},
                decode_entity_delay_scale(delay_mode));
            runner.animation_action_gate = delay.animation_delay;
        },
        [this] {
            field_entity_animation_.accumulator = 0;
            field_entity_animation_.frame = 0U;
            field_entity_animation_.redraw_requested = true;
        });
    field_entity_animation_selector_ = attack_pose.direction_animation;
    field_entity_attack_direction_ = direction;
    field_entity_.direction = direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    source_boundary("field.combat.player_weapon_spawned");
}

void FieldSceneRuntime::apply_field_entity_animation(const bool walking) {
    if (walking) {
        // Accepted movement replaces the one-frame C042 pose with the source
        // C062 walking list. Idle field frames leave the attack pose selected
        // until the source movement path chooses another list.
        field_entity_attack_pose_active_ = false;
    } else if (field_entity_attack_pose_active_) {
        return;
    }
    const auto selector = select_field_entity_animation_selector(
        field_entity_.direction, walking);
    if (selector != field_entity_animation_selector_) {
        field_entity_animation_selector_ = selector;
        // B6DE and B442 replace +0x16 directly. They leave +0x1A and +0x1E
        // intact; 0x600E wraps the positive walk list or resets the single
        // idle frame on the next entity-render pass.
        field_entity_animation_.redraw_requested = true;
    }
    field_entity_walking_ = walking;
}

void FieldSceneRuntime::advance_field_weapon_projectiles() {
    const auto advance_record_animation =
        [this](SpawnEntityRecordState& record,
               EntityAnimationState& animation) {
            const auto selector = static_cast<std::uint16_t>(
                record.animation_resource & 0x7FFFU);
            const auto animation_index =
                static_cast<std::size_t>(selector / 4U);
            if ((selector & 0x0003U) != 0U ||
                animation_index >= field_weapon_projectile_images_.size()) {
                source_boundary(
                    "field.combat.weapon_projectile_selector_invalid");
                return false;
            }

            const auto frame_count =
                field_weapon_projectile_images_[animation_index].size();
            if (frame_count == 0U || frame_count > 16U) {
                source_boundary(
                    "field.combat.weapon_projectile_frames_invalid");
                return false;
            }

            animation.redraw_requested = false;
            advance_entity_animation_accumulator(animation);
            EntitySpriteDescriptorList source_list{};
            source_list.marker = static_cast<std::int16_t>(
                -32768 + static_cast<std::int16_t>(frame_count));
            source_list.auxiliary = 0x270FU;
            source_list.frame_pointers.reserve(frame_count);
            for (std::size_t frame = 0; frame < frame_count; ++frame) {
                source_list.frame_pointers.push_back(
                    static_cast<std::uint32_t>(frame));
            }
            EntitySpriteDescriptorState source_animation{};
            source_animation.kind = record.behavior_defaults.animation_state;
            source_animation.selector = record.animation_resource;
            source_animation.flags = record.record_flags;
            source_animation.frame = animation.frame;
            source_animation.increment = static_cast<std::uint16_t>(
                animation.increment);
            (void)advance_entity_sprite_descriptor(
                source_animation, source_list, 0U);
            animation.frame = source_animation.frame;
            animation.increment = static_cast<std::int16_t>(
                source_animation.increment);
            record.animation_resource = source_animation.selector;
            return true;
        };
    const auto release_slot = [this](
                                   std::optional<std::uint16_t>& slot) {
        if (slot.has_value() && *slot >= 9U && *slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(*slot - 9U)] = false;
        }
        slot.reset();
    };
    const auto word_from_fixed = [](const std::int32_t value) {
        return static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(value) >> 16U);
    };
    const std::vector<CollisionCandidate> live_collision_targets;

    auto spawn = field_weapon_projectiles_.begin();
    while (spawn != field_weapon_projectiles_.end()) {
        if (spawn->primary_active) {
            auto& primary = spawn->primary_record;
            if (!advance_record_animation(primary,
                                          spawn->primary_animation)) {
                primary.flags = static_cast<std::uint8_t>(
                    primary.flags & static_cast<std::uint8_t>(~0x01U));
                spawn->primary_active = false;
                release_slot(spawn->primary_slot);
            } else if ((primary.record_flags & 0x08U) != 0U) {
                // ROM 0x14E0A's stopped branch enters 0x14F1C. Once 0x600E
                // marks the finite impact descriptor complete, 0x14F86
                // decrements +0x41 and retires the primary on its negative
                // result.
                if ((primary.animation_resource & 0x8000U) != 0U) {
                    primary.field_0x41 = static_cast<std::int8_t>(
                        primary.field_0x41 - 1);
                    if (primary.field_0x41 < 0) {
                        primary.flags = static_cast<std::uint8_t>(
                            primary.flags &
                            static_cast<std::uint8_t>(~0x01U));
                        spawn->primary_active = false;
                        release_slot(spawn->primary_slot);
                    } else {
                        source_boundary(
                            "field.combat.weapon_linked_action_followup_pending");
                    }
                }
            } else {
                EntityCollisionRecoveryState recovery{
                    spawn->primary_fixed_position,
                    static_cast<std::int8_t>(primary.field_0x89)};
                EntityCollisionSearchContext collision_context{};
                collision_context.source_flags = primary.flags;

                if (!field_weapon_collision_targets_pending_reported_) {
                    source_boundary(
                        "field.combat.weapon_live_actor_population_pending");
                    field_weapon_collision_targets_pending_reported_ = true;
                }

                // 0x14E0A executes DBF from 3: four 0x14F50 terrain/recovery
                // probes per field update. The scene's live actor table has
                // not been imported yet, so this typed search receives no
                // fabricated collision candidates.
                for (std::uint8_t attempt = 0U; attempt < 4U; ++attempt) {
                    const auto recovery_result =
                        advance_entity_collision_recovery(
                            recovery,
                            [this, &spawn, &primary](
                                const std::int32_t world_x,
                                const std::int32_t world_y) {
                                const auto terrain_code = evaluate_field_collision(
                                    static_cast<std::uint16_t>(
                                        static_cast<std::uint32_t>(world_x) >>
                                        16U),
                                    static_cast<std::uint16_t>(
                                        static_cast<std::uint32_t>(world_y) >>
                                        16U),
                                    spawn->primary_collision_record,
                                    field_terrain_sample_state_,
                                    primary.flags, false);
                                return EntityTerrainProbeSample{
                                    static_cast<std::uint16_t>(terrain_code),
                                    field_terrain_boundary_active_,
                                    field_terrain_outside_bounds_marker_};
                            },
                            collision_context, live_collision_targets);
                    primary.field_0x89 = static_cast<std::uint8_t>(
                        recovery.retry_counter);

                    if (recovery_result.action ==
                            EntityCollisionRecoveryAction::terrain_probe_blocked ||
                        recovery_result.action ==
                            EntityCollisionRecoveryAction::retry_exhausted) {
                        // ROM 0x14E32 marks completion, clears movement
                        // offsets, selects list 0 for the finite impact tail,
                        // and requests event 0x3C when D1 is zero.
                        primary.record_flags = static_cast<std::uint8_t>(
                            primary.record_flags | 0x09U);
                        primary.animation_resource = 0U;
                        primary.heading_offset_x = 0;
                        primary.heading_offset_y = 0;
                        if (recovery_result.outside_bounds_marker) {
                            primary.flags = static_cast<std::uint8_t>(
                                primary.flags &
                                static_cast<std::uint8_t>(~0x01U));
                            spawn->primary_active = false;
                            release_slot(spawn->primary_slot);
                        } else if (source_audio_event_callback_) {
                            source_audio_event_callback_(0x003CU);
                        }
                        break;
                    }

                    spawn->primary_fixed_position = recovery.position;
                    primary.position_x =
                        word_from_fixed(recovery.position.world_x);
                    primary.position_y =
                        word_from_fixed(recovery.position.world_y);
                    if (recovery_result.source_carry) {
                        // A real target will take the ROM's linked-record
                        // action/status path. No such field records are
                        // populated by the current scene importer.
                        source_boundary(
                            "field.combat.weapon_hit_resolution_pending");
                        break;
                    }
                }
                spawn->primary_fixed_position = recovery.position;
                primary.position_x = word_from_fixed(recovery.position.world_x);
                primary.position_y = word_from_fixed(recovery.position.world_y);
                primary.field_0x89 = static_cast<std::uint8_t>(
                    recovery.retry_counter);
            }
        }

        if (spawn->secondary_active) {
            auto& secondary = spawn->source_record;
            if (!advance_record_animation(secondary, spawn->animation)) {
                secondary.flags = static_cast<std::uint8_t>(
                    secondary.flags & static_cast<std::uint8_t>(~0x01U));
                spawn->secondary_active = false;
                release_slot(spawn->secondary_slot);
            } else if ((secondary.animation_resource & 0x8000U) != 0U) {
                // The copied +0xAC4E continuation only retires the finite
                // descriptor; it never advances the source world position.
                secondary.flags = static_cast<std::uint8_t>(
                    secondary.flags & static_cast<std::uint8_t>(~0x01U));
                spawn->secondary_active = false;
                release_slot(spawn->secondary_slot);
            }
        }

        if (!spawn->primary_active && !spawn->secondary_active) {
            spawn = field_weapon_projectiles_.erase(spawn);
        } else {
            ++spawn;
        }
    }
}

void FieldSceneRuntime::advance_field_actor_animation() {
    if (field_entity_attack_pose_active_) {
        // C042 entries are static held poses. 0xABFC owns the source action
        // delay; the host render cadence must not turn this into a walk loop.
        field_entity_animation_.frame = 0U;
        field_entity_animation_.accumulator = 0;
        return;
    }
    // Keep the actor's animation as a small native clock. The source C062
    // positive marker wraps its four poses, while C052's one-pose idle list
    // resolves to frame zero. BABC sets +0xCA=0x28 and 600E consequently uses
    // this 0x0280 increment against 0x1000; no ROM list needs interpreting in
    // the Qt runtime.
    const auto accumulated =
        static_cast<std::int32_t>(field_entity_animation_.accumulator) +
        static_cast<std::int32_t>(field_entity_animation_.increment);
    if (accumulated >= kFieldEntityAnimationPeriod) {
        field_entity_animation_.frame = static_cast<std::uint16_t>(
            field_entity_animation_.frame + 1U);
        field_entity_animation_.redraw_requested = true;
        field_entity_animation_.accumulator = static_cast<std::int16_t>(
            accumulated - kFieldEntityAnimationPeriod);
    } else {
        field_entity_animation_.accumulator =
            static_cast<std::int16_t>(accumulated);
    }

    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.frame = field_entity_walking_
        ? bound_field_walk_animation_frame(field_entity_animation_.frame)
        : 0U;
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

void FieldSceneRuntime::service_pending_field_viewports() {
    // ROM 0x1872 runs from the VBlank entry and services FFE100/FFE102 after
    // the field-camera pass. The Qt host commits the requested viewport
    // redraw after projecting entities, then paints the scene from its native
    // map and state on the next frame.
    if (viewport_state_.normal_render_pending != 0U &&
        callbacks_.field_viewports.render_normal_viewport) {
        callbacks_.field_viewports.render_normal_viewport();
    }
    if (viewport_state_.alternate_render_pending != 0U &&
        callbacks_.field_viewports.render_alternate_viewport) {
        callbacks_.field_viewports.render_alternate_viewport();
    }
}

bool FieldSceneRuntime::prepare_runtime_field_scene(
    const std::uint16_t scene_reference) {
    if (!field_session_source_assets_ || !field_asset_catalog_ ||
        scene_reference >= kFieldSceneProfileCount) {
        return false;
    }

    const auto input = integration::make_field_scene_input(
        *field_session_source_assets_, scene_reference);
    if (!input.has_value()) {
        return false;
    }
    const auto alias =
        std::string("field.maps.mode.") + std::to_string(scene_reference);
    QImage next_field_map;
    if (!load_image(*field_asset_catalog_, alias, next_field_map)) {
        return false;
    }

    const bool same_scene = !field_world_activity_scene_entry_pending_ &&
                            state_.field_scene_buffers.initialized &&
                            state_.field_scene_buffers.scene_reference ==
                                scene_reference;
    const auto prior_world_mode = field_movement_transition_state_.world_mode;
    const auto prior_camera_x = state_.field_scene_buffers.camera_x;
    const auto prior_camera_y = state_.field_scene_buffers.camera_y;
    const auto prior_runner_position = field_entity_.world_position;
    auto next_buffers = state_.field_scene_buffers;
    next_buffers.initialized = false;
    if (!prepare_field_scene_buffers(next_buffers, *input, {})) {
        return false;
    }

    if (same_scene) {
        // A 0x2BD2 same-scene refresh redraws the active scene without
        // resetting the controlled runner to its original entry position.
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

    state_.field_scene_buffers = std::move(next_buffers);
    field_map_ = std::move(next_field_map);
    if (!same_scene) {
        initialize_default_field_actors(scene_reference);
        field_weapon_projectiles_.clear();
        field_weapon_slots_occupied_.fill(false);
        field_weapon_collision_targets_pending_reported_ = false;
        field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    }
    if (!same_scene && field_palette_fade_steps_ != 0U) {
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
    // ROM 0x2BD2 performs a same-scene redraw in place; its visible code does
    // not replace FFDF40. Preserve the active field mode across that modal
    // return so 0xBC36 keeps refreshing collision probes when the source had
    // FFDF40 clear. A genuinely new scene still selects its source mode.
    field_movement_transition_state_.world_mode =
        same_scene ? prior_world_mode : *interaction_mode;
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
    source_boundary("field.world_activity.scene_prepared");
    return true;
}

void FieldSceneRuntime::advance_field_scene_transition_after_modal() {
    // The modal Qt loop is non-blocking. Keep the 0x2BD2 transition pending
    // while the page owns the screen; once it closes, run the ROM-selected
    // same-scene refresh or nonzero scene/profile re-entry in source order.

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
        if (!prepare_runtime_field_scene(
                state_.field_scene_buffers.scene_reference)) {
            source_boundary("field.scene.transition.scene_prepare_unresolved");
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

void FieldSceneRuntime::dispatch_field_map_scene_entry(
    const std::uint16_t scene_id) {
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
            scripting::ScriptNativeOperation::Type::actor_walk_heading ||
        operation.actor_id != "player" || operation.heading > 7U ||
        field_opening_auto_walk_.active) {
        failure_ = "Lua requested an unsupported native actor operation";
        source_boundary("field.map_event.lua_operation_unresolved");
        return;
    }

    field_opening_auto_walk_.active = true;
    field_opening_auto_walk_.target_heading = operation.heading;
    source_boundary("field.map_event.new_game_opening_walk_started");
}

void FieldSceneRuntime::advance_frame() {
    if (!ready_) {
        return;
    }
    if (field_elevator_scene_transition_.is_active()) {
        advance_field_elevator_scene_transition();
        return;
    }
    if (field_elevator_selector_.is_open()) {
        advance_field_elevator_selector();
        return;
    }
    if (field_interaction_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        (void)advance_field_interaction(
            field_controller_sample_state_.events);
        return;
    }
    if (hotel_menu_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if (hotel_karma_skills_.is_open()) {
            (void)process_hotel_karma_screen_events(
                field_controller_sample_state_.events);
            return;
        }
        hotel_menu_.advance_animation(field_random_seed_);
        (void)process_hotel_menu_events(
            field_controller_sample_state_.events);
        return;
    }
    if (field_mode_text_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        const auto result = field_mode_text_.advance_controller_events(
            field_controller_sample_state_.events);
        if (result == FieldModeTextAdvanceResult::returned_to_route) {
            source_boundary("field.mode_text_prompt.returned_to_caller");
            if (field_shop_route_pending_) {
                field_shop_route_pending_ = false;
                if (field_shop_definition_.has_value() &&
                    field_shop_menu_.open(
                        ShopId::ares_weapon_emporium,
                        *field_shop_definition_,
                        field_menu_text_.confirmation_choice_ids)) {
                    source_boundary("field.shop.ares.opened");
                } else {
                    source_boundary("field.shop.ares.open_failed");
                }
            }
        }
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
            advance_field_scene_transition_after_modal();
        } else if (result ==
                   FieldDialogueAdvanceResult::hotel_options_requested) {
            begin_hotel_menu();
        }
        return;
    }
    if (field_shop_menu_.is_open()) {
        // ROM 0x56AC0 services audio and new high-byte input inside its menu
        // loop. Keep that interaction modal so field movement cannot write
        // stale map pixels underneath the full-screen service presentation.
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if (field_shop_menu_.transaction_confirmation_open() ||
            field_shop_menu_.transaction_waiting_for_event()) {
            if (!field_funds_.has_value()) {
                source_boundary("field.shop.ares.transaction_wallet_unavailable");
                field_shop_menu_.clear_pending_action();
                return;
            }
            auto purchase_state = make_purchase_state(
                active_runner_record(), *field_funds_);
            auto sale_state = make_sale_state(
                active_runner_record(), field_shop_menu_.selected_cursor(),
                *field_funds_);
            const auto cost_context = ares_cost_context(
                field_shop_menu_.menu_mode(), active_runner_record());
            const auto confirmation_was_open =
                field_shop_menu_.transaction_confirmation_open();
            const auto transaction_event =
                field_shop_menu_.advance_transaction(
                    field_controller_sample_state_.events, cost_context,
                    purchase_state, sale_state);
            if (transaction_event ==
                ShopTransactionEvent::confirmation_moved) {
                if (source_audio_event_callback_) {
                    source_audio_event_callback_(0x25U);
                }
                source_boundary("field.shop.ares.confirmation_moved");
            } else if (confirmation_was_open &&
                       !field_shop_menu_.transaction_confirmation_open() &&
                       source_audio_event_callback_) {
                // 0x57B8C requests event 0x24 on either source exit button.
                source_audio_event_callback_(0x24U);
            }

            if (transaction_event == ShopTransactionEvent::purchase_applied) {
                apply_purchase_state(purchase_state, active_runner_record());
                field_funds_ = purchase_state.funds;
                refresh_field_hud_runner_slot();
                source_boundary("field.shop.ares.purchase_applied");
            } else if (transaction_event ==
                       ShopTransactionEvent::sale_applied) {
                apply_sale_state(sale_state, active_runner_record());
                field_funds_ = sale_state.funds;
                refresh_field_hud_runner_slot();
                source_boundary("field.shop.ares.sale_applied");
            } else if (transaction_event ==
                       ShopTransactionEvent::message_shown) {
                source_boundary("field.shop.ares.transaction_message");
            } else if (transaction_event ==
                       ShopTransactionEvent::cancelled) {
                source_boundary("field.shop.ares.transaction_cancelled");
            } else if (transaction_event ==
                       ShopTransactionEvent::message_dismissed) {
                source_boundary("field.shop.ares.message_dismissed");
            } else if (transaction_event ==
                       ShopTransactionEvent::unresolved) {
                source_boundary("field.shop.ares.transaction_unresolved");
            }
            return;
        }
        const auto menu_event = field_shop_menu_.advance(
            field_controller_sample_state_.events);
        switch (menu_event) {
        case ShopMenuEvent::selection_moved:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x25U);
            }
            source_boundary("field.shop.ares.selection_moved");
            break;
        case ShopMenuEvent::panel_changed:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x25U);
            }
            source_boundary("field.shop.ares.panel_changed");
            break;
        case ShopMenuEvent::purchase_selected:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x24U);
            }
            if (field_shop_menu_.pending_action().has_value() &&
                field_shop_menu_.pending_action()->offer.has_value() &&
                field_funds_.has_value()) {
                auto purchase_state = make_purchase_state(
                    active_runner_record(), *field_funds_);
                const auto cost_context = ares_cost_context(
                    field_shop_menu_.menu_mode(), active_runner_record());
                const auto transaction_event =
                    field_shop_menu_.begin_purchase(
                        *field_shop_menu_.pending_action()->offer,
                        cost_context, purchase_state);
                if (transaction_event == ShopTransactionEvent::message_shown) {
                    source_boundary("field.shop.ares.purchase_preflight_message");
                } else if (transaction_event ==
                           ShopTransactionEvent::unresolved) {
                    source_boundary("field.shop.ares.purchase_unresolved");
                }
            } else {
                field_shop_menu_.clear_pending_action();
                source_boundary("field.shop.ares.purchase_state_unavailable");
            }
            break;
        case ShopMenuEvent::sale_selected:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x24U);
            }
            if (field_funds_.has_value()) {
                auto sale_state = make_sale_state(
                    active_runner_record(), field_shop_menu_.selected_cursor(),
                    *field_funds_);
                const auto cost_context = ares_cost_context(
                    field_shop_menu_.menu_mode(), active_runner_record());
                const auto transaction_event = field_shop_menu_.begin_sale(
                    cost_context, sale_state);
                if (transaction_event == ShopTransactionEvent::message_shown) {
                    source_boundary("field.shop.ares.sale_preflight_message");
                } else if (transaction_event ==
                           ShopTransactionEvent::action_ignored) {
                    source_boundary("field.shop.ares.sale_empty_slot");
                } else if (transaction_event ==
                           ShopTransactionEvent::unresolved) {
                    source_boundary("field.shop.ares.sale_unresolved");
                }
            } else {
                field_shop_menu_.clear_pending_action();
                source_boundary("field.shop.ares.sale_wallet_unavailable");
            }
            break;
        case ShopMenuEvent::offer_unavailable:
            source_boundary("field.shop.ares.offer_unavailable");
            break;
        case ShopMenuEvent::source_action_boundary:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x24U);
            }
            source_boundary("field.shop.ares.action_boundary");
            field_shop_menu_.finish_action();
            break;
        case ShopMenuEvent::active_runner_change_requested:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x24U);
            }
            handle_field_mode_action_event();
            refresh_field_hud_runner_slot();
            source_boundary("field.shop.ares.active_runner_changed");
            break;
        case ShopMenuEvent::returned_to_field:
            field_shop_route_pending_ = false;
            state_.display_service_word = 0U;
            field_runtime_state_.display_service_word = 0U;
            source_boundary("field.shop.ares.returned_to_field");
            break;
        case ShopMenuEvent::no_change:
            break;
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
        if (command_overview_attributes_skills_.is_open()) {
            const auto menu_event =
                command_overview_attributes_skills_.advance(
                    field_controller_sample_state_.events,
                    active_runner_record(),
                    [this] {
                        handle_field_mode_action_event();
                        refresh_field_hud_runner_slot();
                        source_boundary(
                            "field.command_overview.attributes_skills.runner_selected");
                    });
            switch (menu_event) {
            case AttributesSkillsRuntimeEvent::no_change:
                break;
            case AttributesSkillsRuntimeEvent::selection_moved:
                source_boundary(
                    "field.command_overview.attributes_skills.selection_moved");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_prompt_opened:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_prompt_opened");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_prompt_moved:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_prompt_moved");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_applied:
                refresh_command_overview_combat_bars();
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_applied");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_declined:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_declined");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_blocked:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_blocked");
                break;
            case AttributesSkillsRuntimeEvent::insufficient_karma:
                source_boundary(
                    "field.command_overview.attributes_skills.insufficient_karma");
                break;
            case AttributesSkillsRuntimeEvent::
                active_runner_selection_requested:
                command_overview_attributes_skills_.refresh(
                    active_runner_record());
                refresh_command_overview_combat_bars();
                break;
            case AttributesSkillsRuntimeEvent::returned_to_parent:
                command_overview_attributes_skills_.close_to_parent();
                // ROM 0x10E74 routes the Attributes return back through the
                // 0x10D30 command initializer. Rebuild every shared panel and
                // row here so partial page pixels cannot survive the return.
                command_overview_.close();
                open_command_overview();
                source_boundary(
                    "field.command_overview.attributes_skills.returned_to_command");
                break;
            }
            return;
        }
        if (command_overview_clips_.is_open()) {
            advance_command_overview_clips(
                field_controller_sample_state_.events);
            return;
        }
        if (magic_spell_interface_open()) {
            advance_command_overview_magic(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_automatic_use_state_ !=
            CommandOverviewAutomaticUseState::inactive) {
            advance_command_overview_automatic_use(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_first_aid_state_ !=
            CommandOverviewFirstAidState::inactive) {
            advance_command_overview_first_aid(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_inventory_trade_.open) {
            advance_command_overview_inventory_trade(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_inventory_selector_suppressed_ &&
            (field_controller_sample_state_.events & 0x0F00U) != 0U) {
            command_overview_inventory_selector_suppressed_ = false;
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
        if (command_overview_magic_state_ ==
            CommandOverviewMagicState::magical_defense_wait) {
            // ROM 0x0D94 services VBlank/input until any newly pressed
            // high-byte event arrives, takes one final sample, then 0x11928
            // restores the selected command row.
            if ((field_controller_sample_state_.events & 0xFF00U) != 0U) {
                command_overview_magic_state_ =
                    CommandOverviewMagicState::inactive;
                source_boundary(
                    "field.command_overview.magic.magical_defense.returned");
            }
            return;
        }
        if (command_overview_.take_action_dialog()) {
            // Up on command pages 2..6 arms FFF115 and enters the 0x119CE
            // posture focus. Keep the command page selected underneath it.
            posture_dialog_active_ = true;
            source_boundary("field.posture.dialog_opened");
        }
        if (posture_dialog_active_) {
            const auto events = field_controller_sample_state_.events;
            bool check_increment = false;
            bool check_decrement = false;
            bool close_by_down = false;

            // ROM 0x119CE gives a Down edge special treatment: while Left or
            // Right is held it can still complete that horizontal adjustment;
            // otherwise the armed FFF115 gate returns focus to the command
            // row. With no Down edge, Right raises and Left lowers posture.
            if ((events & 0x0200U) != 0U) {
                if ((events & 0x0008U) != 0U) {
                    check_increment = true;
                } else if ((events & 0x0004U) != 0U) {
                    check_decrement = true;
                } else {
                    close_by_down = true;
                }
            } else {
                check_increment = true;
            }

            int posture_delta = 0;
            if (!close_by_down && check_increment) {
                if ((events & 0x0800U) != 0U) {
                    posture_delta = 1;
                } else {
                    check_decrement = true;
                }
            }
            if (!close_by_down && posture_delta == 0 && check_decrement &&
                (events & 0x0400U) != 0U) {
                posture_delta = -1;
            }

            if (posture_delta != 0) {
                if (source_audio_event_callback_) {
                    source_audio_event_callback_(0x25U);
                }
                (void)adjust_runner_posture(active_runner_record(),
                                            posture_delta);
                refresh_command_overview_combat_bars();
                source_boundary(posture_delta > 0
                                    ? "field.posture.dialog.raised"
                                    : "field.posture.dialog.lowered");
                return;
            }

            if (close_by_down || (events & 0x8000U) != 0U) {
                // The blocking source loop writes its final posture strip
                // through 0x11AE4 with delta zero, then redraws both combat
                // bars before restoring command navigation.
                (void)adjust_runner_posture(active_runner_record(), 0);
                posture_dialog_active_ = false;
                refresh_command_overview_combat_bars();
                source_boundary(close_by_down
                                    ? "field.posture.dialog.deselected"
                                    : "field.posture.dialog.closed");
            }
            return;
        }
        if (pocket_secretary_.open) {
            // C2A6's parent loop and C448's child loop both call E660 once
            // after the VBlank input sample. Its named cell assets are kept
            // in typed panel state and painted with the active secretary UI.
            advance_pocket_secretary_dismiss_runner_effect();
            if (dismiss_runner_.open) {
                const auto event = apply_dismiss_runner_input(
                    dismiss_runner_, field_controller_sample_state_.events);
                switch (event) {
                case DismissRunnerMenuEvent::selection_moved:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(0x25U);
                    }
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.selection_moved");
                    break;
                case DismissRunnerMenuEvent::abort_selected:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(0x24U);
                    }
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.abort_selected");
                    break;
                case DismissRunnerMenuEvent::runner_selected:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(0x24U);
                    }
                    if (dismiss_runner_.selection > 0U &&
                        dismiss_runner_.selection <=
                            dismiss_runner_.candidate_count) {
                        const auto slot = dismiss_runner_.candidate_slots[
                            dismiss_runner_.selection - 1U];
                        finish_pocket_secretary_dismiss_runner(slot);
                    }
                    break;
                case DismissRunnerMenuEvent::returned:
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.returned");
                    break;
                case DismissRunnerMenuEvent::waiting:
                    break;
                }
                if (!dismiss_runner_.open) {
                    // Returning to the parent reconstructs its own menu
                    // panel. Do not let child grid writes survive the redraw.
                    dismiss_runner_random_grid_effect_ =
                        RandomGridEffectState{};
                    dismiss_runner_random_grid_cells_.fill(-1);
                }
                return;
            }
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
                     if (selection >=
                         pocket_secretary_menu_content_.choices.size()) {
                         source_boundary(
                             "field.pocket_secretary.choice_id_unresolved");
                         return true;
                     }
                     const auto& choice_id =
                         pocket_secretary_menu_content_.choices[selection].id;
                     if (choice_id == "notebook") {
                         const auto event_gates =
                             notebook_event_gates();
                         if (!event_gates.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.notebook.event_flags_unresolved");
                         } else {
                             begin_pocket_secretary_notebook(
                                 pocket_secretary_,
                                 notebook_current_run_available_,
                                 *event_gates);
                             source_boundary(
                                 "field.pocket_secretary.notebook.opened");
                         }
                     } else if (choice_id == "cyberdeck") {
                         if (cyberdeck_available_.has_value() &&
                             !*cyberdeck_available_) {
                             begin_pocket_secretary_cyberdeck_unavailable(
                                 pocket_secretary_);
                             source_boundary(
                                 "field.pocket_secretary.cyberdeck_unavailable");
                         } else if (!cyberdeck_available_.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.cyberdeck_flag_unresolved");
                         } else {
                             source_boundary(
                                 "field.pocket_secretary.matrix_path_pending");
                         }
                     } else if (choice_id == "group_items") {
                         if (!group_item_flags_.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.group_items_flags_unresolved");
                         } else {
                             begin_pocket_secretary_group_items(
                                 pocket_secretary_, *group_item_flags_);
                             source_boundary(*group_item_flags_ == 0U
                                                 ? "field.pocket_secretary.group_items_empty"
                                                 : "field.pocket_secretary.group_items_populated");
                         }
                     } else if (choice_id == "dismiss_runner") {
                         begin_pocket_secretary_dismiss_runner();
                     } else if (choice_id == "save_load") {
                         source_boundary(
                             "field.pocket_secretary.save_load_pending");
                     }
                     return true;
                 },
                 [this] {
                     if (source_audio_event_callback_) {
                         source_audio_event_callback_(0x25U);
                     }
                     handle_field_mode_action_event();
                     source_boundary("field.pocket_secretary.mode_action");
                 }});
            switch (event) {
            case PocketSecretaryMenuEvent::notebook_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.mode_action");
                break;
            case PocketSecretaryMenuEvent::notebook_current_run_no_run:
                source_boundary(
                    "field.pocket_secretary.notebook.current_run.unavailable");
                break;
            case PocketSecretaryMenuEvent::notebook_response_dismissed:
                source_boundary(
                    "field.pocket_secretary.notebook.current_run.dismissed");
                break;
            case PocketSecretaryMenuEvent::notebook_returned:
                source_boundary("field.pocket_secretary.notebook.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_action_boundary:
                if (pocket_secretary_.notebook.selection >=
                    pocket_secretary_.notebook.option_count) {
                    source_boundary(
                        "field.pocket_secretary.notebook.choice_id_unresolved");
                    break;
                }
                {
                    const auto option_index = static_cast<std::size_t>(
                        pocket_secretary_.notebook.visible_options[
                            pocket_secretary_.notebook.selection]);
                    if (option_index >=
                        pocket_secretary_menu_content_.notebook_choice_ids
                            .size()) {
                        source_boundary(
                            "field.pocket_secretary.notebook.choice_id_unresolved");
                        break;
                    }
                    const auto& choice_id =
                        pocket_secretary_menu_content_.notebook_choice_ids[
                            option_index];
                    if (choice_id == "tips_and_clues") {
                        if (!notebook_tip_flags_resolved_) {
                            source_boundary(
                                "field.pocket_secretary.notebook.tips.flags_unresolved");
                            break;
                        }
                        begin_pocket_secretary_notebook_tips(
                            pocket_secretary_, notebook_tip_flags_);
                        source_boundary(
                            "field.pocket_secretary.notebook.tips.opened");
                    } else if (choice_id == "mr_johnsons") {
                        if (!notebook_referral_flags_.has_value()) {
                            source_boundary(
                                "field.pocket_secretary.notebook.mr_johnsons.flags_unresolved");
                            break;
                        }
                        begin_pocket_secretary_notebook_mr_johnsons(
                            pocket_secretary_, *notebook_referral_flags_);
                        source_boundary(
                            "field.pocket_secretary.notebook.mr_johnsons.opened");
                    } else if (choice_id == "current_run" ||
                               choice_id == "contacts" ||
                               choice_id == "shadowrunners") {
                        source_boundary(
                            "field.pocket_secretary.notebook.child_pending");
                    } else {
                        source_boundary(
                            "field.pocket_secretary.notebook.choice_id_unresolved");
                    }
                }
                break;
            case PocketSecretaryMenuEvent::notebook_tips_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_tips_returned:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_tips_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.mode_action");
                break;
            case PocketSecretaryMenuEvent::
                notebook_mr_johnsons_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_detail_opened:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.detail_opened");
                break;
            case PocketSecretaryMenuEvent::
                notebook_mr_johnsons_detail_dismissed:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.detail_dismissed");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_returned:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.mode_action");
                break;
            default:
                break;
            }
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
            command_overview_action_message_ =
                CommandOverviewActionMessage::none;
            command_overview_inventory_action_active_ = false;
            command_overview_discard_confirmation_active_ = false;
            command_overview_first_aid_state_ =
                CommandOverviewFirstAidState::inactive;
            command_overview_first_aid_targets_ = FirstAidTargetList{};
            command_overview_first_aid_cursor_ = 0U;
            command_overview_first_aid_message_ = FirstAidMessage::none;
            command_overview_automatic_use_state_ =
                CommandOverviewAutomaticUseState::inactive;
            command_overview_inventory_selector_suppressed_ = false;
            command_overview_clips_.close();
            command_overview_cyberware_active_ = false;
            command_overview_cyberware_ = CyberwareMenuModel{};
            command_overview_magic_state_ =
                CommandOverviewMagicState::inactive;
            command_overview_magic_loop_ = MagicMenuLoopState{};
            command_overview_magic_selected_spell_.reset();
            command_overview_magic_force_ = MagicForceState{};
            pocket_secretary_.open = false;
            dismiss_runner_ = DismissRunnerMenuState{};
            posture_dialog_active_ = false;
            advance_field_scene_transition_after_modal();
        } else if (command_overview_.take_action_dispatch_pending()) {
            // ROM 0x10E4E publishes audio command 0x24 before dispatching the
            // A/C action selected on the command overview. FBA6 itself has
            // no additional sound call in its Magical Defense fallback.
            if (source_audio_event_callback_) {
                source_audio_event_callback_(0x24U);
            }
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
            } else if (command_overview_.view().source_state.primary_page ==
                       5U) {
                begin_command_overview_magic();
            } else if (command_overview_.view().source_state.primary_page ==
                       3U) {
                begin_command_overview_attributes_skills();
            } else {
                source_boundary(
                    "field.command_overview.non_inventory_action_pending");
            }
        }
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
        return;
    }
    if (field_entity_.active) {
        handle_field_map_event_activations(
            field_map_event_system_.evaluate_actor_scope(
                FieldMapEventTriggerContext{
                    state_.field_scene_buffers.scene_reference,
                    field_entity_.world_position.x,
                    field_entity_.world_position.y,
                    field_map_event_state_flags_,
                    true}));
    }
    field_runtime_state_.field_activity_lock =
        state_.field_lock != 0U || field_opening_auto_walk_.active;
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

bool FieldSceneRuntime::open_attributes_skills_for_dump() {
    if (!open_command_overview_for_dump() ||
        !seek_command_overview_page(3U)) {
        return false;
    }
    // Exercise the same command-row A dispatch as the focused Qt window.
    set_controller_port_byte(0x40U);
    advance_frame();
    set_controller_port_byte(0U);
    advance_frame();
    return command_overview_attributes_skills_.is_open();
}

bool FieldSceneRuntime::open_stoker_room_payment_for_dump(
    const std::uint32_t funds) {
    if (!ready_) {
        return false;
    }
    field_funds_ = funds;
    if (!field_dialogue_.open_stoker_prompt_for_dump(
            StokerMotelPromptId::room_offer)) {
        return false;
    }
    return field_dialogue_.advance_controller_events(0x4000U) ==
           FieldDialogueAdvanceResult::prompt_advanced;
}

FieldDialogueAdvanceResult
FieldSceneRuntime::advance_stoker_dialogue_for_dump(
    const std::uint16_t event_word) {
    const auto result = field_dialogue_.advance_controller_events(event_word);
    if (result == FieldDialogueAdvanceResult::hotel_options_requested) {
        begin_hotel_menu();
    }
    return result;
}

FieldInteractionAdvanceResult
FieldSceneRuntime::advance_field_interaction(
    const std::uint16_t event_word) {
    std::array<bool, FieldPartyRosterRuntime::kSlotCount> active_slots{};
    for (std::size_t slot = 0U; slot < active_slots.size(); ++slot) {
        active_slots[slot] = field_party_roster_.slot_active(slot);
    }
    const auto result = field_interaction_.advance_controller_events(
        event_word, field_party_roster_.records(), active_slots,
        field_movement_transition_state_.activity_selection,
        field_random_seed_);
    if (result == FieldInteractionAdvanceResult::returned_to_field) {
        // The source re-enters the field renderer after the prompt callback.
        // This host redraw replaces its full 320x224 image and consumes the
        // coordinator's display-service marker here. ROM 0x2BD2 also commits
        // a negative interaction result to the sampled field event counter;
        // passkey success uses FFE16E = 0xFF to advance the A2 door cell.
        state_.display_service_word = 0U;
        field_runtime_state_.display_service_word = 0U;
        source_boundary("field.interaction.returned_to_field");
        advance_field_scene_transition_after_modal();
    }
    refresh_field_hud_runner_slot();
    return result;
}

void FieldSceneRuntime::begin_hotel_menu() {
    if (!ready_) {
        return;
    }
    std::array<std::string, FieldPartyRosterRuntime::kSlotCount> names{};
    for (std::size_t slot = 0; slot < names.size(); ++slot) {
        names[slot] = field_party_roster_.name(slot);
    }
    hotel_menu_.open(field_party_roster_.records(), names,
                     field_party_roster_.active_slot());
    refresh_field_hud_runner_slot();
    source_boundary("field.hotel.entity_status.opened");
}

bool FieldSceneRuntime::load_command_overview_menu_content() {
    constexpr std::array<std::string_view, kCommandOverviewLabelCount>
        kChoiceIds{{"inventory_left", "inventory_right", "clips",
                    "attributes_skills", "cyberware", "magic",
                    "pocket_secretary"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.command_overview_script_invalid");
        return false;
    };
    const auto definition = game_scripts_.menu(
        "command_overview", "root", "field.command_overview", "root");
    if (!definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "command overview root menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (definition->choices.size() != kChoiceIds.size()) {
        return fail(
            "command overview Lua menu does not match the seven source rows");
    }
    for (std::size_t index = 0; index < kChoiceIds.size(); ++index) {
        const auto& choice = definition->choices[index];
        const auto* label = game_scripts_.dialogue_text(choice.label_key);
        if (choice.id != kChoiceIds[index] || label == nullptr) {
            return fail("command overview Lua choice ID or label is invalid");
        }
        field_menu_text_.command_labels[index] = *label;
    }
    source_boundary("field.command_overview.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_hotel_menu_content() {
    using scripting::ScriptMenuDefinition;
    constexpr std::array<std::string_view, 2> kOptionIds{{
        "karma_screen", "check_out"}};
    constexpr std::array<std::string_view, 2> kCheckoutIds{{
        "checkout_yes", "checkout_no"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.hotel_menu_script_invalid");
        return false;
    };
    const auto options = game_scripts_.menu(
        "hotel", "options", "hotel", "options");
    if (!options.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel options menu script is unavailable"
                        : game_scripts_.failure());
    }
    const auto checkout = game_scripts_.menu(
        "hotel", "checkout", "hotel", "checkout");
    if (!checkout.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel checkout menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (options->choices.size() != kOptionIds.size() ||
        checkout->choices.size() != kCheckoutIds.size()) {
        return fail("Hotel Lua menus do not match the source choice count");
    }
    HotelMenuContent content{};
    const auto copy_text = [this](const std::string_view key,
                                  std::string& destination) {
        const auto* source = game_scripts_.dialogue_text(key);
        if (source == nullptr) {
            return false;
        }
        destination = *source;
        return true;
    };
    const auto* title = game_scripts_.dialogue_text(options->title_key);
    if (title == nullptr || !copy_text("hotel.options.separator",
                                      content.separator)) {
        return fail("Hotel options title or separator text is missing");
    }
    content.title = *title;
    for (std::size_t index = 0; index < kOptionIds.size(); ++index) {
        const auto& choice = options->choices[index];
        if (choice.id != kOptionIds[index] ||
            !copy_text(choice.label_key, content.options[index].label)) {
            return fail("Hotel options Lua choice ID or label is invalid");
        }
        content.options[index].id = choice.id;
    }
    const auto* checkout_question =
        game_scripts_.dialogue_text(checkout->title_key);
    if (checkout_question == nullptr) {
        return fail("Hotel checkout question text is missing");
    }
    const auto question_break = checkout_question->find('|');
    if (question_break == std::string::npos ||
        checkout_question->find('|', question_break + 1U) !=
            std::string::npos) {
        return fail("Hotel checkout question must contain two source rows");
    }
    content.checkout_question[0] =
        checkout_question->substr(0U, question_break);
    content.checkout_question[1] =
        checkout_question->substr(question_break + 1U);
    for (std::size_t index = 0; index < kCheckoutIds.size(); ++index) {
        const auto& choice = checkout->choices[index];
        if (choice.id != kCheckoutIds[index] ||
            !copy_text(choice.label_key,
                       content.checkout_choices[index].label)) {
            return fail("Hotel checkout Lua choice ID or label is invalid");
        }
        content.checkout_choices[index].id = choice.id;
    }
    if (!copy_text("hotel.status.medical_attention_suffix",
                   content.medical_attention_suffix)) {
        return fail("Hotel medical-attention text is missing");
    }
    for (std::size_t index = 2U; index < content.recovery_message.size();
         ++index) {
        if (!copy_text("hotel.status.recovery." + std::to_string(index),
                       content.recovery_message[index])) {
            return fail("Hotel recovery text is incomplete");
        }
    }
    hotel_menu_content_ = std::move(content);
    source_boundary("field.hotel.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_pocket_secretary_menu_content() {
    constexpr std::array<std::string_view,
                         kPocketSecretaryFieldMenuRowCount> kChoiceIds{{
        "notebook", "group_items", "cyberdeck", "dismiss_runner",
        "save_load"}};
    constexpr std::array<std::string_view, kNotebookChoiceCount>
        kNotebookChoiceIds{{"current_run", "tips_and_clues", "contacts",
                            "mr_johnsons", "shadowrunners"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.pocket_secretary_script_invalid");
        return false;
    };
    const auto definition = game_scripts_.menu(
        "pocket_secretary", "root", "pocket_secretary", "root");
    if (!definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Pocket Secretary root menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (definition->choices.size() != kChoiceIds.size()) {
        return fail(
            "Pocket Secretary Lua menu does not match its source rows");
    }
    const auto notebook_definition = game_scripts_.menu(
        "pocket_secretary", "notebook", "notebook", "options");
    if (!notebook_definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Pocket Secretary Notebook menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (notebook_definition->choices.size() != kNotebookChoiceIds.size()) {
        return fail(
            "Pocket Secretary Notebook menu does not match its source rows");
    }
    const auto* title = game_scripts_.dialogue_text(definition->title_key);
    const auto* separator = game_scripts_.dialogue_text(
        "pocket_secretary.root.separator");
    if (title == nullptr || separator == nullptr) {
        return fail("Pocket Secretary root title or separator text is missing");
    }
    PocketSecretaryMenuContent content{};
    content.title = *title;
    content.separator = *separator;
    for (std::size_t index = 0; index < kChoiceIds.size(); ++index) {
        const auto& choice = definition->choices[index];
        const auto* label = game_scripts_.dialogue_text(choice.label_key);
        if (choice.id != kChoiceIds[index] || label == nullptr) {
            return fail("Pocket Secretary Lua choice ID or label is invalid");
        }
        content.choices[index] = PocketSecretaryMenuContent::Choice{
            choice.id, *label};
    }
    for (std::size_t index = 0; index < kNotebookChoiceIds.size(); ++index) {
        const auto& choice = notebook_definition->choices[index];
        if (choice.id != kNotebookChoiceIds[index] ||
            game_scripts_.dialogue_text(choice.label_key) == nullptr) {
            return fail(
                "Pocket Secretary Notebook choice ID or label is invalid");
        }
        content.notebook_choice_ids[index] = choice.id;
    }
    pocket_secretary_menu_content_ = std::move(content);
    source_boundary("field.pocket_secretary.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_ares_shop_menu_content() {
    constexpr std::array<std::string_view, kFieldServiceTitleCount>
        kServiceTitleKeys{{
            "shop.service.title.weapons_world",
            "shop.ares.title",
            "shop.service.title.crime_mall",
            "shop.service.title.medicine_lodge",
            "shop.service.title.riannons",
            "shop.service.title.merlins_lore",
            "shop.service.title.microtronics",
            "shop.service.title.wire_masters",
            "shop.service.title.little_chiba",
            "shop.service.title.dr_bobs",
            "shop.service.title.council_island_hospital",
            "shop.service.title.seattle_general_hospital"}};
    constexpr std::array<std::string_view, 6> kGroupIds{{
        "weapons", "other_items", "armor", "source_action_3",
        "source_action_4", "sell"}};
    constexpr std::array<std::size_t, 6> kGroupFirstTitleIndices{{
        0U, 4U, 2U, 6U, 8U, 10U}};
    constexpr std::array<std::string_view, 20> kOtherServiceActionTitleKeys{{
        "shop.service.action-title.12",
        "shop.service.action-title.13",
        "shop.service.action-title.14",
        "shop.service.action-title.15",
        "shop.service.action-title.16",
        "shop.service.action-title.17",
        "shop.service.action-title.18",
        "shop.service.action-title.19",
        "shop.service.action-title.20",
        "shop.service.action-title.21",
        "shop.service.action-title.22",
        "shop.service.action-title.23",
        "shop.service.action-title.24",
        "shop.service.action-title.25",
        "shop.service.action-title.26",
        "shop.service.action-title.27",
        "shop.service.action-title.28",
        "shop.service.action-title.29",
        "shop.service.action-title.30",
        "shop.service.action-title.31"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.ares_shop_script_invalid");
        return false;
    };
    if (!field_shop_definition_.has_value() ||
        field_shop_definition_->id != "ares_weapon_emporium" ||
        field_shop_definition_->title_key != kServiceTitleKeys[1] ||
        field_shop_definition_->groups.size() != kGroupIds.size()) {
        return fail("Ares shop Lua definition does not match its source page");
    }

    for (std::size_t index = 0; index < kServiceTitleKeys.size(); ++index) {
        const auto* title =
            game_scripts_.dialogue_text(kServiceTitleKeys[index]);
        if (title == nullptr) {
            return fail("Lua shop service title text is incomplete");
        }
        field_menu_text_.field_service_titles[index] = *title;
    }

    std::array<bool, 12> ares_action_titles_found{};
    for (std::size_t group_index = 0; group_index < kGroupIds.size();
         ++group_index) {
        const auto& group = field_shop_definition_->groups[group_index];
        const auto first_title_index = kGroupFirstTitleIndices[group_index];
        if (group.id != kGroupIds[group_index]) {
            return fail("Ares shop Lua categories changed source order");
        }
        for (std::size_t line = 0; line < group.title_keys.size(); ++line) {
            const auto title_index = first_title_index + line;
            const auto* title =
                game_scripts_.dialogue_text(group.title_keys[line]);
            if (title == nullptr || title->size() != 20U ||
                ares_action_titles_found[title_index]) {
                return fail("Ares shop Lua category heading is invalid");
            }
            ares_action_titles_found[title_index] = true;
            field_menu_text_.field_service_action_titles[title_index] = *title;
        }
    }
    if (!std::all_of(ares_action_titles_found.begin(),
                     ares_action_titles_found.end(),
                     [](const bool found) { return found; })) {
        return fail(
            "Ares shop Lua category headings do not cover each source row");
    }
    for (std::size_t index = 0; index < kOtherServiceActionTitleKeys.size();
         ++index) {
        const auto* title = game_scripts_.dialogue_text(
            kOtherServiceActionTitleKeys[index]);
        if (title == nullptr || title->size() != 20U) {
            return fail("Lua service action title text is incomplete");
        }
        field_menu_text_.field_service_action_titles[index + 12U] = *title;
    }

    const auto copy_shop_text = [this](const std::string& key,
                                       std::string& destination) {
        const auto* text = game_scripts_.dialogue_text(key);
        if (text == nullptr) {
            return false;
        }
        destination = *text;
        return true;
    };
    const auto& keys = field_shop_definition_->text_keys;
    if (!copy_shop_text(keys.universal_clips_title,
                        field_menu_text_.field_service_universal_clips_title) ||
        !copy_shop_text(keys.universal_clips_description,
                        field_menu_text_.field_service_universal_clips_description) ||
        !copy_shop_text(keys.transaction_prompt,
                        field_menu_text_.shop_transaction_prompt) ||
        !copy_shop_text(keys.no_inventory_slot,
                        field_menu_text_.shop_no_inventory_slot) ||
        !copy_shop_text(keys.insufficient_funds,
                        field_menu_text_.shop_insufficient_funds) ||
        !copy_shop_text(keys.special_action_limit,
                        field_menu_text_.shop_special_action_limit) ||
        !copy_shop_text(keys.protected_sale,
                        field_menu_text_.shop_protected_sale) ||
        !copy_shop_text(keys.sale_quote_heading,
                        field_menu_text_.shop_sale_quote_heading) ||
        !copy_shop_text(keys.purchase_price_heading,
                        field_menu_text_.shop_purchase_price_heading)) {
        return fail("Ares shop Lua transaction text is incomplete");
    }
    source_boundary("field.ares_shop.menu_content_loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_item_menu_content() {
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.item_script_invalid");
        return false;
    };
    const auto catalog = game_scripts_.item_catalog();
    if (!catalog.has_value() ||
        catalog->items.size() != field_menu_text_.item_names.size() ||
        catalog->detail_text_keys.size() !=
            field_menu_text_.field_action_detail_texts.size()) {
        return fail(game_scripts_.failure().empty()
                        ? "Lua item catalog does not match source table sizes"
                        : game_scripts_.failure());
    }
    for (std::size_t index = 0; index < catalog->items.size(); ++index) {
        const auto& item = catalog->items[index];
        if (item.code != index + 1U) {
            return fail("Lua item catalog changed source item order");
        }
        const auto* name = game_scripts_.dialogue_text(item.name_key);
        const auto* category = game_scripts_.dialogue_text(item.category_key);
        if (name == nullptr || category == nullptr) {
            return fail("Lua item name or category text is incomplete");
        }
        field_menu_text_.item_names[index] = *name;
        field_menu_text_.item_categories[index] = *category;
    }
    for (std::size_t index = 0; index < catalog->detail_text_keys.size();
         ++index) {
        const auto* detail =
            game_scripts_.dialogue_text(catalog->detail_text_keys[index]);
        if (detail == nullptr) {
            return fail("Lua item detail text is incomplete");
        }
        field_menu_text_.field_action_detail_texts[index] = *detail;
    }
    for (std::size_t index = 0;
         index < catalog->detail_label_keys.size(); ++index) {
        const auto* label =
            game_scripts_.dialogue_text(catalog->detail_label_keys[index]);
        if (label == nullptr) {
            return fail("Lua item detail labels are incomplete");
        }
        field_menu_text_.item_detail_labels[index] = *label;
    }
    source_boundary("field.item.menu_content_loaded_from_lua");
    return true;
}

HotelMenuEvent FieldSceneRuntime::process_hotel_menu_events(
    const std::uint16_t events) {
    const auto result = hotel_menu_.advance_controller_events(events);
    switch (result) {
    case HotelMenuEvent::waiting:
        break;
    case HotelMenuEvent::status_acknowledged:
        source_boundary("field.hotel.entity_status.acknowledged");
        break;
    case HotelMenuEvent::selection_moved:
        // The Hotel loop and E660 path do not request a sound event.
        source_boundary("field.hotel.selection_moved");
        break;
    case HotelMenuEvent::checkout_opened:
        source_boundary("field.hotel.checkout.opened");
        break;
    case HotelMenuEvent::stayed_at_hotel:
        source_boundary("field.hotel.checkout.stayed");
        break;
    case HotelMenuEvent::checked_out:
        refresh_field_hud_runner_slot();
        source_boundary("field.hotel.checked_out");
        break;
    case HotelMenuEvent::karma_screen_opened:
        hotel_karma_skills_.open(active_runner_record());
        source_boundary("field.hotel.karma_screen.opened");
        break;
    case HotelMenuEvent::mode_action:
        handle_field_mode_action_event();
        source_boundary("field.hotel.mode_action");
        break;
    }
    return result;
}

HotelMenuEvent FieldSceneRuntime::advance_hotel_menu_for_dump(
    const std::uint16_t events) {
    if (!hotel_menu_.open()) {
        return HotelMenuEvent::waiting;
    }
    if (hotel_karma_skills_.is_open()) {
        (void)process_hotel_karma_screen_events(events);
        return HotelMenuEvent::waiting;
    }
    hotel_menu_.advance_animation(field_random_seed_);
    return process_hotel_menu_events(events);
}

AttributesSkillsRuntimeEvent
FieldSceneRuntime::advance_hotel_karma_screen_for_dump(
    const std::uint16_t events) {
    if (!hotel_menu_.open() || !hotel_karma_skills_.is_open()) {
        return AttributesSkillsRuntimeEvent::no_change;
    }
    return process_hotel_karma_screen_events(events);
}

AttributesSkillsRuntimeEvent
FieldSceneRuntime::process_hotel_karma_screen_events(
    const std::uint16_t events) {
    const auto result = hotel_karma_skills_.advance(
        events, active_runner_record(), [this] {
            handle_field_mode_action_event();
            refresh_field_hud_runner_slot();
            source_boundary("field.hotel.karma_screen.runner_selected");
        });
    switch (result) {
    case AttributesSkillsRuntimeEvent::no_change:
        break;
    case AttributesSkillsRuntimeEvent::selection_moved:
        source_boundary("field.hotel.karma_screen.selection_moved");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_prompt_opened:
        source_boundary("field.hotel.karma_screen.upgrade_prompt_opened");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_prompt_moved:
        source_boundary("field.hotel.karma_screen.upgrade_prompt_moved");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_applied:
        source_boundary("field.hotel.karma_screen.upgrade_applied");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_declined:
        source_boundary("field.hotel.karma_screen.upgrade_declined");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_blocked:
        source_boundary("field.hotel.karma_screen.upgrade_blocked");
        break;
    case AttributesSkillsRuntimeEvent::insufficient_karma:
        source_boundary("field.hotel.karma_screen.insufficient_karma");
        break;
    case AttributesSkillsRuntimeEvent::
        active_runner_selection_requested:
        hotel_karma_skills_.refresh(active_runner_record());
        break;
    case AttributesSkillsRuntimeEvent::returned_to_parent:
        hotel_karma_skills_.close_to_parent();
        source_boundary("field.hotel.karma_screen.returned_to_options");
        break;
    }
    return result;
}

bool FieldSceneRuntime::begin_ares_shop_event_for_dump() {
    if (!ready_) {
        return false;
    }
    // Inject the already source-resolved event tuple into the same converted
    // orchestrator used by field movement. This test seam does not skip the
    // generic prompt or the 0x1FA78 service selector.
    field_runtime_state_.field_mode = 2U;
    field_movement_transition_state_.mode_event_code = 11U;
    field_exit_alternate_result_ = 0;
    field_shop_route_pending_ = false;
    (void)run_field_exit_orchestrator(field_runtime_state_);
    return field_mode_text_.open();
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

void FieldSceneRuntime::paint_field_scene_entities(QPainter& painter) const {
    struct SceneSprite {
        std::int16_t depth = 0;
        std::uint16_t source_order = 0;
        int x = 0;
        int y = 0;
        const QImage* image = nullptr;
    };
    std::vector<SceneSprite> sprites;
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);

    if (field_entity_.active && field_entity_.visible) {
        const auto direction = static_cast<std::size_t>(
            (field_entity_attack_pose_active_ ? field_entity_attack_direction_
                                              : field_entity_.direction) %
            field_entity_idle_images_.size());
        const auto walking = field_entity_walking_;
        const auto movement_frames =
            field_entity_walk_images_[direction].size();
        const auto frame = movement_frames == 0U
            ? 0U
            : bound_field_walk_animation_frame(
                  field_entity_animation_.frame) % movement_frames;
        const auto& image = field_entity_attack_pose_active_
            ? field_entity_attack_images_[direction]
            : walking ? field_entity_walk_images_[direction][frame]
                      : field_entity_idle_images_[direction];
        const auto origin_x = field_entity_attack_pose_active_
            ? field_entity_attack_origin_x_[direction]
            : walking ? field_entity_walk_origin_x_[direction][frame]
                      : field_entity_idle_origin_x_[direction];
        const auto origin_y = field_entity_attack_pose_active_
            ? field_entity_attack_origin_y_[direction]
            : walking ? field_entity_walk_origin_y_[direction][frame]
                      : field_entity_idle_origin_y_[direction];
        EntityProjectionState projection{};
        projection.world_x = field_entity_.world_position.x;
        projection.world_y = field_entity_.world_position.y;
        projection.sprite_offset_y = field_entity_.projection.sprite_offset_y;
        if (project_entity_screen_position_standard(
                projection, camera_x, camera_y).visible) {
            const auto depth = (field_entity_behavior_flags_ & 0x50U) != 0U
                                   ? std::int16_t{0}
                                   : static_cast<std::int16_t>(
                                         field_entity_.world_position.y);
            sprites.push_back(SceneSprite{
                depth,
                0U,
                static_cast<int>(projection.screen_x) -
                    kGenesisSpriteOrigin + origin_x,
                static_cast<int>(projection.screen_y) -
                    kGenesisSpriteOrigin + origin_y,
                &image});
        }
    }

    const auto current_scene =
        state_.field_scene_buffers.scene_reference;
    for (const auto& actor : field_default_actors_) {
        if (actor.scene_reference != current_scene) {
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
        sprites.push_back(SceneSprite{
            static_cast<std::int16_t>(actor.world_position.y),
            static_cast<std::uint16_t>(actor.source_order + 1U),
            static_cast<int>(projection.screen_x) - kGenesisSpriteOrigin +
                sprite.origin_x,
            static_cast<int>(projection.screen_y) - kGenesisSpriteOrigin +
                sprite.origin_y,
            sprite.image});
    }

    const auto append_spawn_record =
        [this, camera_x, camera_y, &sprites](
            const SpawnEntityRecordState& record,
            const std::uint16_t frame,
            const bool active,
            const std::optional<std::uint16_t>& source_slot) {
            if (!active || (record.flags & 0x01U) == 0U) {
                return;
            }
            const auto selector = static_cast<std::uint16_t>(
                record.animation_resource & 0x7FFFU);
            if ((selector & 0x0003U) != 0U) {
                return;
            }
            const auto animation_index =
                static_cast<std::size_t>(selector / 4U);
            if (animation_index >= field_weapon_projectile_images_.size()) {
                return;
            }
            const auto frame_index = static_cast<std::size_t>(frame);
            const auto& images =
                field_weapon_projectile_images_[animation_index];
            if (frame_index >= images.size()) {
                return;
            }

            EntityProjectionState projection{};
            projection.world_x = record.position_x;
            projection.world_y = record.position_y;
            if (!project_entity_screen_position_standard(
                     projection, camera_x, camera_y)
                     .visible) {
                return;
            }
            const auto depth = (record.flags & 0x50U) != 0U
                                   ? std::int16_t{0}
                                   : static_cast<std::int16_t>(
                                         record.position_y);
            sprites.push_back(SceneSprite{
                depth,
                source_slot.value_or(0xFFFFU),
                static_cast<int>(projection.screen_x) -
                    kGenesisSpriteOrigin +
                    field_weapon_projectile_origin_x_[animation_index]
                                                       [frame_index],
                static_cast<int>(projection.screen_y) -
                    kGenesisSpriteOrigin +
                    field_weapon_projectile_origin_y_[animation_index]
                                                       [frame_index],
                &images[frame_index]});
        };

    for (const auto& spawn : field_weapon_projectiles_) {
        append_spawn_record(spawn.primary_record,
                            spawn.primary_animation.frame,
                            spawn.primary_active, spawn.primary_slot);
        append_spawn_record(spawn.source_record, spawn.animation.frame,
                            spawn.secondary_active, spawn.secondary_slot);
    }

    // ROM 0x5B1E stably sorts by descending effective depth, then 0x4658
    // draws records in that order. Flags 0x10/0x40 force depth to zero.
    std::stable_sort(sprites.begin(), sprites.end(),
                     [](const SceneSprite& left,
                        const SceneSprite& right) {
                         if (left.depth != right.depth) {
                             return left.depth > right.depth;
                         }
                         return left.source_order < right.source_order;
                     });
    for (const auto& sprite : sprites) {
        if (sprite.image != nullptr) {
            painter.drawImage(sprite.x, sprite.y, *sprite.image);
        }
    }
}

void FieldSceneRuntime::paint_field_hud_overlays(QPainter& painter) const {
    const auto image_for = [this](const FieldHudOverlay& overlay)
        -> const QImage* {
        switch (overlay.kind) {
        case FieldHudOverlayKind::portrait:
            return overlay.asset_index < field_hud_party_faces_.size() &&
                    overlay.palette_variant <
                        field_hud_party_faces_[overlay.asset_index].size()
                ? &field_hud_party_faces_[overlay.asset_index]
                                 [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::physical_bar:
            return overlay.asset_index < field_hud_delay_meter_counters_.size()
                ? &field_hud_delay_meter_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::mental_bar:
            return overlay.asset_index <
                    field_hud_behavior_meter_counters_.size()
                ? &field_hud_behavior_meter_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::item:
            return overlay.asset_index < field_hud_party_items_.size() &&
                    overlay.palette_variant <
                        field_hud_party_items_[overlay.asset_index].size()
                ? &field_hud_party_items_[overlay.asset_index]
                                  [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::unarmed_item:
            return overlay.asset_index <
                       field_hud_party_unarmed_items_.size() &&
                    overlay.palette_variant <
                        field_hud_party_unarmed_items_[overlay.asset_index]
                            .size()
                ? &field_hud_party_unarmed_items_[overlay.asset_index]
                                           [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::magic_item:
            return overlay.asset_index < field_hud_magic_items_.size() &&
                    overlay.palette_variant <
                        field_hud_magic_items_[overlay.asset_index].size()
                ? &field_hud_magic_items_[overlay.asset_index]
                                 [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::ammo:
            return overlay.asset_index < field_hud_ammo_counters_.size()
                ? &field_hud_ammo_counters_[overlay.asset_index]
                : nullptr;
        }
        return nullptr;
    };
    auto live_slots = field_hud_slots_;
    for (std::size_t slot = 0; slot < live_slots.size(); ++slot) {
        live_slots[slot] = field_hud_slot_state(slot);
    }
    for (const auto& overlay : compose_field_hud_overlays(live_slots)) {
        const auto* image = image_for(overlay);
        if (image == nullptr || image->isNull()) {
            continue;
        }
        painter.drawImage(overlay.x, overlay.y, *image);
    }
}

void FieldSceneRuntime::paint_field_shop(QPainter& painter) const {
    if (field_service_background_.isNull()) {
        return;
    }
    painter.drawImage(0, 0, field_service_background_);

    // 0x56AC0 selects Ares title record 1 from the extracted source table.
    // The full-screen service asset replaces the preceding dialogue and map;
    // dynamic labels, item art, icons, and runner HUD are layered once over it.
    draw_command_overview_text(painter, 64, 16,
                               field_menu_text_.field_service_titles[1]);
    const auto group = field_shop_menu_.selected_group();
    const auto group_index = static_cast<std::size_t>(group);
    if (group_index < field_service_tab_images_.size()) {
        painter.drawImage(64, 128, field_service_tab_images_[group_index]);
    }
    std::size_t first_title = 0U;
    switch (group) {
    case ShopActionGroup::weapons:
        first_title = 0U;
        break;
    case ShopActionGroup::other_items:
        first_title = 4U;
        break;
    case ShopActionGroup::armor:
        first_title = 2U;
        break;
    case ShopActionGroup::source_action_3:
        first_title = 6U;
        break;
    case ShopActionGroup::source_action_4:
        first_title = 8U;
        break;
    case ShopActionGroup::sell:
        first_title = 10U;
        break;
    }

    const auto& action_titles = field_menu_text_.field_service_action_titles;
    if (first_title + 1U < action_titles.size()) {
        if (field_shop_menu_.offers_panel_active()) {
            const bool transaction_replaces_detail =
                field_shop_menu_.transaction_confirmation_open() ||
                field_shop_menu_.transaction_message() !=
                    ShopTransactionMessage::none;
            const auto cursor = field_shop_menu_.selected_cursor();
            const auto selected_offer = group == ShopActionGroup::sell
                ? std::optional<FieldActionCostRecord>{}
                : field_shop_menu_.offer(group, cursor);
            std::uint8_t selected_item_code = 0U;
            if (group == ShopActionGroup::sell) {
                if (cursor < active_runner_record().inventory_item_codes.size()) {
                    selected_item_code =
                        active_runner_record().inventory_item_codes[cursor];
                }
            } else if (selected_offer.has_value() &&
                       selected_offer->type !=
                           static_cast<std::uint8_t>('E')) {
                selected_item_code = static_cast<std::uint8_t>(
                    selected_offer->type + 1U);
            }

            if (selected_offer.has_value() &&
                selected_offer->type == static_cast<std::uint8_t>('E')) {
                // ROM 0x586F6 uses the dedicated CE28E grid, 1AACE2
                // description, 1AACCE title, and record quantity for the
                // Universal Gun Clips row. These are source text/assets, not
                // a fabricated inventory item-code mapping.
                painter.drawImage(128, 32,
                                  field_service_universal_clips_icon_);
                if (!transaction_replaces_detail) {
                    draw_command_overview_text(
                        painter, 24, 56,
                        field_menu_text_
                            .field_service_universal_clips_description);
                }
                draw_command_overview_text(
                    painter, 104, 144,
                    field_menu_text_.field_service_universal_clips_title);
                if (!transaction_replaces_detail) {
                    draw_command_overview_text(
                        painter, 192, 112,
                        std::to_string(selected_offer->quantity));
                }
            }
            else if (selected_item_code > 0U &&
                     selected_item_code <= field_menu_text_.item_names.size()) {
                const auto item_index =
                    static_cast<std::size_t>(selected_item_code - 1U);
                const auto& item_name = field_menu_text_.item_names[item_index];
                const auto& item_category =
                    field_menu_text_.item_categories[item_index];
                const auto space = item_name.find(' ');
                if (!item_category.empty()) {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_category.size() * 8U)) / 2,
                        152, item_category);
                } else if (space != std::string::npos && space > 0U &&
                           space + 1U < item_name.size()) {
                    const auto first_line =
                        std::string_view(item_name).substr(0U, space);
                    const auto second_line =
                        std::string_view(item_name).substr(space + 1U);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(first_line.size() * 8U)) / 2,
                        144, first_line);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(second_line.size() * 8U)) / 2,
                        152, second_line);
                } else {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                }
                if (selected_item_code <=
                    command_overview_item_details_.size()) {
                    painter.drawImage(
                        124, 32,
                        command_overview_item_details_[item_index]);
                }

                if (!transaction_replaces_detail) {
                    const auto detail_label_index = selected_item_code <= 15U
                        ? 0U
                        : selected_item_code <= 22U ? 1U : 2U;
                    draw_command_overview_text(
                        painter, 72, 56,
                        field_menu_text_
                            .item_detail_labels[detail_label_index]);
                    const auto detail_text = select_field_action_detail_text(
                        static_cast<std::uint8_t>(selected_item_code - 1U));
                    if (detail_text.has_value() &&
                        detail_text->table_index <
                            field_menu_text_.field_action_detail_texts.size()) {
                        draw_command_overview_text(
                            painter, 24, 80,
                            field_menu_text_.field_action_detail_texts[
                                detail_text->table_index]);
                    }
                }

                const auto grouped_decimal = [](std::uint32_t amount) {
                    auto value = std::to_string(amount);
                    for (auto position = value.size(); position > 3U;
                         position -= 3U) {
                        value.insert(position - 3U, ",");
                    }
                    return value;
                };
                if (!transaction_replaces_detail &&
                    group != ShopActionGroup::sell &&
                    selected_offer.has_value()) {
                    draw_command_overview_text(
                        painter, 80, 112,
                        field_menu_text_.shop_purchase_price_heading);
                    const auto amount = grouped_decimal(
                        calculate_field_action_record_cost(
                            *selected_offer,
                            ares_cost_context(field_shop_menu_.menu_mode(),
                                              active_runner_record()),
                            us_item_catalog()));
                    const auto x = 184 -
                        static_cast<int>(amount.size() * 8U);
                    draw_command_overview_text(painter, x, 112, amount);
                }
            }
        } else {
            draw_command_overview_text(painter, 64, 144,
                                       action_titles[first_title]);
            draw_command_overview_text(painter, 64, 152,
                                       action_titles[first_title + 1U]);
        }
    }

    const auto count = group == ShopActionGroup::sell
        ? active_runner_record().inventory_item_codes.size()
        : field_shop_menu_.offer_count(group);
    for (std::size_t index = 0; index < count && index < 8U; ++index) {
        if (group == ShopActionGroup::sell) {
            const auto item_code =
                active_runner_record().inventory_item_codes[index];
            if (item_code > 0U &&
                item_code <= command_overview_inventory_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    command_overview_inventory_icons_[item_code - 1U][0]);
            }
            continue;
        }
        const auto offer = field_shop_menu_.offer(group, index);
        if (!offer.has_value()) {
            continue;
        }
        if (offer->type == static_cast<std::uint8_t>('E')) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                field_service_universal_clips_icon_);
            continue;
        }
        const auto item_code = static_cast<std::uint8_t>(offer->type + 1U);
        if (item_code > 0U &&
            item_code <= command_overview_inventory_icons_.size()) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                command_overview_inventory_icons_[item_code - 1U][0]);
        }
    }

    if (field_shop_menu_.offers_panel_active()) {
        // The Genesis selector hugs each 24x16 item image. Draw a single
        // warm-yellow pixel around that footprint; the surrounding 32x24
        // cell and neighboring controls remain clear.
        const auto cursor = field_shop_menu_.selected_cursor();
        const auto selectable = group != ShopActionGroup::sell ||
            (cursor < active_runner_record().inventory_item_codes.size() &&
             active_runner_record().inventory_item_codes[cursor] != 0U);
        if (selectable && cursor < count && cursor < 8U) {
            constexpr int kIconWidth = 24;
            constexpr int kIconHeight = 16;
            const auto x = 16 + static_cast<int>(cursor) * 32;
            constexpr int y = 168;
            const QColor selector_color(196, 190, 39);
            painter.fillRect(x, y, kIconWidth, 1, selector_color);
            painter.fillRect(x, y + kIconHeight - 1, kIconWidth, 1,
                             selector_color);
            painter.fillRect(x, y, 1, kIconHeight, selector_color);
            painter.fillRect(x + kIconWidth - 1, y, 1, kIconHeight,
                             selector_color);
        }
    }

    paint_field_hud_overlays(painter);
    if (field_funds_.has_value()) {
        auto amount = std::to_string(*field_funds_);
        for (auto position = amount.size(); position > 3U; position -= 3U) {
            amount.insert(position - 3U, ",");
        }
        draw_command_overview_text(painter, 144, 200, amount);
        draw_command_overview_text(painter, 200, 200, "$");
    }

    if (field_shop_menu_.transaction_confirmation_open()) {
        draw_command_overview_text(
            painter, 24, 80, field_menu_text_.shop_transaction_prompt);
        if (group == ShopActionGroup::sell) {
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_sale_quote_heading);
            auto amount = std::to_string(
                field_shop_menu_.transaction_amount());
            for (auto position = amount.size(); position > 3U;
                 position -= 3U) {
                amount.insert(position - 3U, ",");
            }
            const auto x = 224 - static_cast<int>(amount.size() * 8U);
            draw_command_overview_text(painter, x, 88, amount);
        }
        draw_command_overview_text(
            painter, 128, 96, field_menu_text_.action_choice_yes);
        draw_command_overview_text(
            painter, 128, 112, field_menu_text_.action_choice_no);
        draw_command_overview_text(
            painter, 112,
            field_shop_menu_.transaction_confirmation().selected_option == 0U
                ? 96
                : 112,
            ">");
    } else {
        switch (field_shop_menu_.transaction_message()) {
        case ShopTransactionMessage::no_inventory_slot:
            draw_command_overview_text(
                painter, 96, 88, field_menu_text_.shop_no_inventory_slot);
            break;
        case ShopTransactionMessage::insufficient_funds:
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_insufficient_funds);
            break;
        case ShopTransactionMessage::special_action_limit:
            draw_command_overview_text(
                painter, 56, 88,
                field_menu_text_.shop_special_action_limit);
            break;
        case ShopTransactionMessage::protected_sale:
            draw_command_overview_text(
                painter, 48, 96, field_menu_text_.shop_protected_sale);
            break;
        case ShopTransactionMessage::none:
            break;
        }
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
    auto cursor_y = y;
    for (const unsigned char code : text) {
        if (code == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        if (code < 0x20U || code > 0x7EU) {
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
        painter.drawImage(cursor, cursor_y, tinted_glyph);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_field_menu_font_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const QImage& font) const {
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > font.width() || source_y + 8 > font.height()) {
            cursor += 8;
            continue;
        }
        painter.drawImage(cursor, cursor_y, font,
                          source_x, source_y, 8, 8);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_hotel_menu_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text,
    const HotelMenuPaletteLine palette_line) const {
    const auto& font = palette_line == HotelMenuPaletteLine::checkout
                           ? hotel_checkout_font_
                           : command_overview_font_;
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 <= font.width() &&
            source_y + 8 <= font.height()) {
            // Hotel pages select the CRAM line in the source text writer.
            // Keep the extracted RGBA colors intact for both green options
            // text and the purple nested checkout prompt.
            painter.drawImage(cursor, cursor_y, font, source_x, source_y, 8,
                              8);
        }
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
    // ROM 0x12C36 starts at name-table C118 (screen 96,16) and clears the
    // source 20x20 cell region. The field HUD is composed afterward as a
    // separate shared layer.
    painter.fillRect(96, 16, 160, 160, Qt::black);

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

    if (command_overview_clips_.reload_confirmation_open()) {
        // ROM 0x11D06/0x127B0 replaces CD33E with the dedicated CD3B6 Yes/No
        // row; 0x127FC selects one of its two source focus blocks.
        painter.drawImage(kCommandOverviewClipsBackgroundX,
                          kCommandOverviewClipsBackgroundY,
                          command_overview_action_confirmation_background_);
        const auto choice_index =
            command_overview_clips_.reload_confirmation_yes() ? 0U : 1U;
        painter.drawImage(
            kCommandOverviewActionConfirmationFocusX[choice_index],
            kCommandOverviewActionConfirmationFocusY,
            command_overview_action_confirmation_focus_[choice_index]);
        draw_lines(field_menu_text_.clips_reload_prompt, 120, 72);
        draw_command_overview_text(
            painter, 112, 200,
            command_overview_clips_.reload_confirmation_yes()
                ? field_menu_text_.action_choice_yes
                : field_menu_text_.action_choice_no);
        return;
    }

    if (command_overview_clips_.trade_open()) {
        // ROM 0x11E7E installs the nested trade row from CD37A.
        painter.drawImage(kCommandOverviewClipsBackgroundX,
                          kCommandOverviewClipsBackgroundY,
                          command_overview_clips_trade_background_);
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

    // ROM 0x11B98 installs the top-level Clips row from CD33E. It has the
    // same footprint as the inventory action row but different button art.
    painter.drawImage(kCommandOverviewClipsBackgroundX,
                      kCommandOverviewClipsBackgroundY,
                      command_overview_clips_background_);

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

void FieldSceneRuntime::paint_attributes_skills_menu(
    QPainter& painter, const AttributesSkillsMenuController& menu) const {
    // ROM 0xD01C clears both planes and installs the command-menu view. The
    // Hotel option enters the shared page body at 0xD024, where the upgrade
    // detail fields are enabled by the live selection-data state.
    // Paint it as the page's base so no command buttons or stale field HUD
    // pixels can remain underneath the new menu.
    painter.drawImage(0, 0, command_overview_attributes_background_);

    const auto& record = active_runner_record();
    const auto slot = field_party_roster_.active_slot();
    const auto runner_name = field_party_roster_.name(slot).empty()
                                 ? field_menu_text_.runner_names[0]
                                 : field_party_roster_.name(slot);
    draw_command_overview_text(painter, 80, 24, runner_name);
    if (record.species_index < field_menu_text_.race_names.size()) {
        draw_command_overview_text(
            painter, 80, 32,
            field_menu_text_.race_names[record.species_index]);
    }
    if (record.archetype_index < field_menu_text_.class_names.size()) {
        draw_command_overview_text(
            painter, 128, 32,
            field_menu_text_.class_names[record.archetype_index]);
    }
    // The source calls 0x110EC with VDP destination E132. That is the same
    // runner panel family used at E104 on the command page, shifted to x=200.
    if (slot < field_hud_faces_.size() &&
        !field_hud_faces_[slot].isNull()) {
        painter.drawImage(200, 16, field_hud_faces_[slot]);
    }

    const auto grid = menu.grid(record);
    const auto cursor = menu.state().cursor;
    for (std::size_t column = 0; column < kAttributesSkillsColumnCount;
         ++column) {
        for (std::size_t row = 0; row < kAttributesSkillsRowCount; ++row) {
            const auto& cell = grid[column][row];
            if (cell.label_index >=
                field_menu_text_.attribute_skill_labels.size()) {
                continue;
            }
            const auto x = column == 0U ? 24 : 176;
            const auto y = row < 4U
                               ? 56 + static_cast<int>(row) * 8
                               : 104 + static_cast<int>(row - 4U) * 8;
            if (cursor.column == column && cursor.row == row) {
                // 0xD5DC paints the focused label and value after the base
                // grid. Leave this cell clear here so transparent glyphs
                // from the green base text cannot show through the purple
                // selection layer.
                continue;
            }
            draw_command_overview_text(
                painter, x, y,
                field_menu_text_.attribute_skill_labels[cell.label_index]);
            // The source places the static Essence value one tile to the
            // left of the other right-column values. When Essence is focused,
            // 0xD5DC replaces it with the live and secondary values below.
            const auto value_x = cell.label_index == 6U
                                     ? 272
                                     : (column == 0U ? 136 : 280);
            draw_command_overview_text(
                painter, value_x, y, std::to_string(cell.value));
        }
    }

    const auto draw_detail_text = [&](const int x, const int y,
                                      const std::string_view text,
                                      const std::uint16_t text_mode) {
        const auto palette_line = text_mode == 0x4000U
                                      ? HotelMenuPaletteLine::checkout
                                      : HotelMenuPaletteLine::status_and_options;
        draw_hotel_menu_text(painter, x, y, text, palette_line);
    };
    CyberwareDetailCallbacks detail_callbacks{};
    detail_callbacks.render_selected_label =
        [&](const CyberwareDetailLabel& label,
            const std::uint16_t text_mode) {
            std::size_t label_index = 0U;
            int x = 24;
            int y = 56;
            if (label.section == CyberwareDetailLabelSection::attribute) {
                label_index = label.index;
                const auto column = static_cast<int>(label.index / 4U);
                const auto row = static_cast<int>(label.index % 4U);
                x = column == 0 ? 24 : 176;
                y = 56 + row * 8;
            } else {
                label_index = 8U + label.index;
                const auto column = static_cast<int>(label.index / 6U);
                const auto row = static_cast<int>(label.index % 6U);
                x = column == 0 ? 24 : 176;
                y = 104 + row * 8;
            }
            if (label_index < field_menu_text_.attribute_skill_labels.size()) {
                draw_detail_text(
                    x, y, field_menu_text_.attribute_skill_labels[label_index],
                    text_mode);
            }
        };
    detail_callbacks.render_numeric_value =
        [&](const CyberwareDetailNumericField field, const std::uint16_t value,
            const std::uint16_t text_mode) {
            int x = 0;
            int y = 0;
            switch (field) {
            case CyberwareDetailNumericField::selected_attribute_or_skill:
            case CyberwareDetailNumericField::magic:
                x = cursor.column == 0U ? 136 : 280;
                y = cursor.row < 4U
                        ? 56 + static_cast<int>(cursor.row) * 8
                        : 104 + static_cast<int>(cursor.row - 4U) * 8;
                break;
            case CyberwareDetailNumericField::essence:
                x = 272;
                y = 72;
                break;
            case CyberwareDetailNumericField::essence_secondary:
                x = 288;
                y = 72;
                break;
            case CyberwareDetailNumericField::quickness_cyberware_modifier:
                x = 160;
                y = 64;
                break;
            case CyberwareDetailNumericField::strength_cyberware_modifier:
                x = 160;
                y = 72;
                break;
            case CyberwareDetailNumericField::body_cyberware_modifier:
                x = 160;
                y = 56;
                break;
            }
            draw_detail_text(x, y, std::to_string(value), text_mode);
        };
    detail_callbacks.render_cyberware_modifier_label =
        [&](const CyberwareDetailModifierField field,
            const std::uint16_t text_mode) {
            const auto y = field == CyberwareDetailModifierField::body
                               ? 56
                               : field == CyberwareDetailModifierField::quickness
                                     ? 64
                                     : 72;
            // The ROM resource at 0xC259D is the one-character cyberware
            // marker "c". The enum identifies its stat row; it is not a
            // visible word label.
            draw_detail_text(144, y, "c", text_mode);
        };
    const auto detail_values = read_cyberware_detail_values(record);
    (void)render_attribute_skill_detail_panel(
        detail_values, {cursor.column, cursor.row}, 0x4000U,
        detail_callbacks);

    draw_command_overview_text(
        painter, 120, 160,
        std::to_string(static_cast<unsigned>(record.auxiliary[2U])));

    const auto& menu_state = menu.state();
    const auto& selected = selected_attribute_skill_cell(
        grid, menu_state.cursor);
    if (selected.label_index <
        field_menu_text_.attribute_skill_labels.size()) {
        draw_hotel_menu_text(
            painter, 24, 168,
            field_menu_text_.attribute_skill_labels[selected.label_index],
            HotelMenuPaletteLine::checkout);
    }

    if (menu.allows_karma_upgrades()) {
        const auto upgrade = menu.preview(record);
        if (upgrade.status == AttributeSkillUpgradeStatus::available) {
            draw_hotel_menu_text(
                painter, 64, 200,
                field_menu_text_.attributes_upgrade_header,
                HotelMenuPaletteLine::checkout);
            draw_hotel_menu_text(
                painter, 160, 200,
                std::to_string(static_cast<unsigned>(upgrade.next_value)),
                HotelMenuPaletteLine::checkout);
            draw_hotel_menu_text(
                painter, 248, 200,
                std::to_string(static_cast<unsigned>(upgrade.karma_cost)),
                HotelMenuPaletteLine::checkout);
        }
    }

    switch (menu.allows_karma_upgrades()
                ? menu.message()
                : AttributesSkillsMenuMessage::none) {
    case AttributesSkillsMenuMessage::none:
        break;
    case AttributesSkillsMenuMessage::maximum_level:
        draw_hotel_menu_text(
            painter, 48, 200,
            field_menu_text_.attributes_upgrade_maximum_level_message,
            HotelMenuPaletteLine::checkout);
        break;
    case AttributesSkillsMenuMessage::mage_only_skill:
        draw_hotel_menu_text(
            painter, 72, 176,
            field_menu_text_.attributes_upgrade_mage_only_message,
            HotelMenuPaletteLine::checkout);
        break;
    case AttributesSkillsMenuMessage::insufficient_karma:
        draw_hotel_menu_text(
            painter, 72, 176,
            field_menu_text_.attributes_upgrade_insufficient_karma_message,
            HotelMenuPaletteLine::checkout);
        break;
    }

    if (menu.allows_karma_upgrades() && menu.upgrade_prompt_open()) {
        // D53C writes its choice text at CAb2 and the cursor at CAAE.
        // Keep the existing Attributes/Skills detail beneath this nested
        // prompt, matching the source's partial redraw of the lower panel.
        draw_hotel_menu_text(
            painter, 96, 176, field_menu_text_.attributes_upgrade_prompt,
            HotelMenuPaletteLine::checkout);
        draw_hotel_menu_text(painter, 200, 168,
                             field_menu_text_.action_choice_yes,
                             HotelMenuPaletteLine::checkout);
        draw_hotel_menu_text(painter, 200, 184,
                             field_menu_text_.action_choice_no,
                             HotelMenuPaletteLine::checkout);
        const auto cursor_y =
            menu.upgrade_prompt_selection() == 0U
                ? 168
                : 184;
        painter.drawImage(184, cursor_y, hotel_checkout_cursor_);
    }
}

void FieldSceneRuntime::paint_command_overview_magic(
    QPainter& painter) const {
    // FBA6 clears Plane A before installing this 34x28 layer. The command
    // overview shell is therefore absent beneath transparent Magic cells;
    // its separately presented right-hand field HUD is composed afterward.
    painter.drawImage(0, 0, command_overview_magic_panel_background_);
    const auto* table = find_us_magic_spell_table(
        active_runner_record().magic_table_index);
    if (table != nullptr) {
        for (std::size_t index = 0;
             index < table->entry_count &&
             index < kMagicSpellTableEntryCapacity;
             ++index) {
            const auto spell_code = table->entries[index].spell_code;
            if (spell_code >= command_overview_magic_spell_icons_.size()) {
                continue;
            }
            const auto column = index % 7U;
            const auto row = index / 7U;
            painter.drawImage(
                32 + static_cast<int>(column * 32U),
                168 + static_cast<int>(row * 24U),
                command_overview_magic_spell_icons_[spell_code]);
        }
    }

    if (!command_overview_magic_selected_spell_.has_value()) {
        return;
    }
    const auto spell_code =
        command_overview_magic_selected_spell_->spell_code;
    const auto* spell = find_us_magic_spell(spell_code);
    if (spell_code < field_menu_text_.magic_spell_names.size()) {
        // ROM 0x10276 sends the selected D4EA0 glyph triplet to C194, which
        // is the 24x24 cell beside the spell title. The menu row below uses
        // the same named glyph assets at its separate list destinations.
        if (spell_code < command_overview_magic_spell_icons_.size()) {
            painter.drawImage(
                80, 24, command_overview_magic_spell_icons_[spell_code]);
        }
        draw_field_menu_font_text(
            painter, 112, 32,
            field_menu_text_.magic_spell_names[spell_code],
            magic_menu_font_);
        if (magic_force_selection_open() ||
            magic_posture_selection_open()) {
            // ROM 0x102D4 replaces the spell description with the action
            // heading at C390 while Force/Posture selection is active.
            // paint() starts from the named panel background each frame, so
            // omitting the old description also clears both stale text rows.
            draw_field_menu_font_text(
                painter, 64, 56,
                field_menu_text_.magic_force_posture_heading,
                magic_menu_font_);
        } else {
            draw_field_menu_font_text(
                painter, 32, 48,
                field_menu_text_.magic_spell_descriptions[spell_code],
                magic_menu_font_);
        }
    }
    // ROM 0x102EC passes each complete encoded label block to C0DA. The
    // 0x80 bytes inside them preserve the line breaks and empty Force-to-
    // Posture row; this is not the fallback heading used by 0x102D4.
    draw_field_menu_font_text(
        painter, 32, 80, field_menu_text_.magic_panel_metric_labels,
        magic_menu_font_);
    draw_field_menu_font_text(
        painter, 160, 104, field_menu_text_.magic_force_posture_labels,
        magic_menu_font_);
    draw_field_menu_font_text(
        painter, 216, 104,
        std::to_string(command_overview_magic_force_.current_force),
        magic_menu_font_);

    const auto& runner = active_runner_record();
    MagicMenuValueRefreshInput success_input{};
    success_input.current_force =
        command_overview_magic_force_.current_force;
    success_input.action_cap = runner.auxiliary[0];
    success_input.active_entity = magic_action_record_from_runner(runner);
    MagicMenuValueRefreshCallbacks success_callbacks{};
    success_callbacks.resolve_menu_metric =
        [this, &runner, &success_input](
        const std::uint16_t value) -> std::optional<std::uint16_t> {
        const auto* spell_table = find_us_magic_spell_table(
            runner.magic_table_index);
        if (spell_table == nullptr ||
            !command_overview_magic_selected_spell_.has_value()) {
            return std::nullopt;
        }

        // FBA6 copies this count-and-pair table into FFF9FA. 0x104C8 then
        // selects an even byte using its source row*6+column expression;
        // preserve that source indexing through the typed lookup helper.
        std::vector<std::uint8_t> source_action_indices;
        source_action_indices.reserve(2U +
                                      spell_table->entry_count * 2U);
        source_action_indices.push_back(0U);
        source_action_indices.push_back(static_cast<std::uint8_t>(
            spell_table->entry_count - 1U));
        for (std::size_t index = 0;
             index < spell_table->entry_count &&
             index < kMagicSpellTableEntryCapacity;
             ++index) {
            source_action_indices.push_back(
                spell_table->entries[index].spell_code);
            source_action_indices.push_back(
                spell_table->entries[index].force_limit);
        }

        MenuMetricLookupInput metric_input{};
        metric_input.menu_row = command_overview_magic_loop_.selection.row;
        metric_input.menu_column =
            command_overview_magic_loop_.selection.column;
        metric_input.source_action_indices =
            std::move(source_action_indices);
        metric_input.value = value;
        metric_input.entity = success_input.active_entity;
        metric_input.item_catalog = &us_item_catalog();
        metric_input.selector_table =
            &us_special_item_modifier_selector_table();
        const auto metric = resolve_menu_metric_action(metric_input);
        if (!metric.has_value() || metric->modifiers.source_data_missing) {
            return std::nullopt;
        }
        return metric->modifiers.value;
    };
    const auto success =
        refresh_magic_menu_success_or_defense_values(success_input,
                                                     success_callbacks);
    if (!success.grid_requests.empty() &&
        success.grid_requests.front().row <
            command_overview_magic_metric_bars_.size()) {
        painter.drawImage(
            32, 88,
            command_overview_magic_metric_bars_[
                success.grid_requests.front().row]);
    }

    MagicMenuValueRefreshInput defense_input{};
    defense_input.refresh_defense_strips = true;
    defense_input.active_entity = success_input.active_entity;
    defense_input.attribute_77 = defense_input.active_entity.attribute_77;
    defense_input.attribute_7c = defense_input.active_entity.attribute_7c;
    const auto& party_records = field_party_roster_.records();
    for (std::size_t index = 0;
         index < defense_input.ready_entities.size(); ++index) {
        defense_input.ready_entities[index] =
            magic_action_record_from_runner(party_records[index]);
    }
    const auto defense =
        refresh_magic_menu_success_or_defense_values(defense_input);
    for (const auto& request : defense.grid_requests) {
        if (request.row >= command_overview_magic_metric_bars_.size()) {
            continue;
        }
        if (request.strip == MagicMenuValueStrip::defense_physical) {
            painter.drawImage(32, 128,
                              command_overview_magic_metric_bars_[request.row]);
        } else if (request.strip == MagicMenuValueStrip::defense_mana) {
            painter.drawImage(32, 144,
                              command_overview_magic_metric_bars_[request.row]);
        }
    }

    // 0x1088C draws the spell's availability label and seven-tile row only
    // for nonnegative +0x25 values. The negative branch clears that region.
    if (spell != nullptr && spell->availability_byte >= 0) {
        draw_field_menu_font_text(
            painter, 160, 80, field_menu_text_.magic_availability_label,
            magic_menu_font_);
        const auto availability = static_cast<std::size_t>(
            spell->availability_byte);
        if (availability < command_overview_magic_availability_bars_.size()) {
            painter.drawImage(
                160, 88,
                command_overview_magic_availability_bars_[availability]);
        }
    }

    // 0x105E6 writes `None` at C68C for nonpositive residuals, otherwise the
    // selected seven-tile C86F8 row at C688. The typed calculation consumes
    // the active runner and spell records; the extracted rows supply pixels.
    if (spell != nullptr) {
        const MagicActionEffectValueInput effect_input{
            command_overview_magic_force_.current_force,
            magic_action_record_from_runner(runner), *spell};
        const auto effect = refresh_magic_action_effect_value(effect_input);
        if (!effect.residual_valid || effect.residual_effect <= 0) {
            draw_field_menu_font_text(painter, 48, 104,
                                      field_menu_text_.none_label,
                                      magic_menu_font_);
        } else if (effect.damage_grid.has_value() &&
                   effect.damage_grid->effect_row <
                       command_overview_magic_damage_bars_.size()) {
            painter.drawImage(
                32, 104,
                command_overview_magic_damage_bars_[
                    effect.damage_grid->effect_row]);
        }
    }

    const auto posture = std::min<std::size_t>(
        runner.posture, command_overview_magic_posture_bars_.size() - 1U);
    painter.drawImage(160, 136,
                      command_overview_magic_posture_bars_[posture]);
    if (posture < field_menu_text_.status_names.size()) {
        draw_field_menu_font_text(
            painter, 160, 144, field_menu_text_.status_names[posture],
            magic_menu_font_);
    }

    // ROM 0x10172 lays out the endpoint, current-force fill, marker, one
    // separator cell, remaining fill, and final endpoint at C728. These are
    // direct source tile assets; only their positions depend on typed state.
    const auto maximum_force = std::max<std::uint16_t>(
        1U, command_overview_magic_force_.maximum_force);
    const auto current_force = std::clamp<std::uint16_t>(
        command_overview_magic_force_.current_force, 1U, maximum_force);
    const auto force_tile_x = [](const std::uint16_t index) {
        return 160 + static_cast<int>(index) * 8;
    };
    painter.drawImage(force_tile_x(0U), 112,
                      command_overview_magic_force_meter_tiles_[0]);
    for (std::uint16_t index = 1U; index < current_force; ++index) {
        painter.drawImage(force_tile_x(index), 112,
                          command_overview_magic_force_meter_tiles_[2]);
    }
    const auto marker = magic_force_selection_open() ? 3U : 4U;
    painter.drawImage(force_tile_x(current_force), 112,
                      command_overview_magic_force_meter_tiles_[marker]);
    for (std::uint16_t index = 0U;
         index < maximum_force - current_force; ++index) {
        painter.drawImage(
            force_tile_x(static_cast<std::uint16_t>(current_force + 2U + index)),
            112, command_overview_magic_force_meter_tiles_[2]);
    }
    painter.drawImage(force_tile_x(
                          static_cast<std::uint16_t>(maximum_force + 2U)),
                      112, command_overview_magic_force_meter_tiles_[1]);

    const auto selection = command_overview_magic_loop_.selection;
    const auto phase = static_cast<std::size_t>(
        command_overview_magic_cursor_phase_ %
        command_overview_magic_cursor_frames_.size());
    painter.drawImage(
        static_cast<int>(selection.column) * 32,
        static_cast<int>(selection.row) * 24,
        command_overview_magic_cursor_frames_[phase]);
}

void FieldSceneRuntime::paint_command_overview(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!command_overview_.open()) {
        return;
    }
    if (magic_spell_interface_open()) {
        // The ROM cleared the previous command Plane A before writing Magic.
        // Transparent cells expose the cleared/black underlay; the HUD plane
        // remains a separate pass in paint().
        paint_command_overview_magic(painter);
        return;
    }
    if (command_overview_attributes_skills_.is_open()) {
        paint_attributes_skills_menu(
            painter, command_overview_attributes_skills_.controller());
        return;
    }
    if (pocket_secretary_.open) {
        // ROM 0xC2A6 writes C1418 at C216 with C0DA. The embedded 0x80
        // markers advance eight pixels per row, leaving one blank row
        // between options. C376 writes its right cursor tile at C3A2, x=136;
        // C1320 maps tile 0x05FF to the source lowercase-x glyph.
        painter.drawImage(0, 0, pocket_secretary_background_);
        // ROM 0xC3E6 fills 28x21 name words from C186; this direct asset is
        // the extracted C1320 space tile repeated with palette line 3.
        painter.drawImage(24, 24, pocket_secretary_grid_);
        // ROM 0xE660's C13E8/C13F8 destinations map to these twelve 8x8
        // cells in the lower shared frame. Each call replaces one adjacent
        // 16x8 pair; all previous pairs remain until the panel is rebuilt.
        constexpr std::array<std::array<int, 2>, 12> kEffectCellOrigins{{
            {{40, 200}}, {{72, 200}}, {{184, 200}}, {{216, 200}},
            {{40, 208}}, {{72, 208}}, {{184, 208}}, {{216, 208}},
            {{56, 200}}, {{200, 200}}, {{56, 208}}, {{200, 208}},
        }};
        for (std::size_t location = 0;
             location < dismiss_runner_random_grid_cells_.size();
             ++location) {
            const auto pattern = dismiss_runner_random_grid_cells_[location];
            if (pattern < 0 ||
                static_cast<std::size_t>(pattern) >=
                    dismiss_runner_random_grid_patterns_.size()) {
                continue;
            }
            painter.drawImage(
                kEffectCellOrigins[location][0],
                kEffectCellOrigins[location][1],
                dismiss_runner_random_grid_patterns_[
                    static_cast<std::size_t>(pattern)]);
        }
        if (pocket_secretary_.notebook.open) {
            if (pocket_secretary_.notebook.mr_johnsons.open) {
                const auto& page =
                    pocket_secretary_.notebook.mr_johnsons;
                draw_command_overview_text(
                    painter, 88, 32, field_menu_text_.notebook_labels[4]);
                draw_command_overview_text(
                    painter, 72, 40,
                    field_menu_text_.notebook_tips_separator);
                if (page.detail_wait) {
                    // ROM F404 writes the 28x17 lower inner-panel grid at
                    // C306 before C0DA places the selected detail at C38C.
                    // This clears the old list and cursor while preserving
                    // the title/separator and shared outer frame.
                    painter.drawImage(QRect(24, 48, 224, 136),
                                      pocket_secretary_grid_,
                                      QRect(0, 24, 224, 136));
                    if (page.selection < page.visible_count) {
                        const auto source_index =
                            page.visible_source_indices[page.selection];
                        if (source_index <
                            field_menu_text_.notebook_mr_johnson_details
                                .size()) {
                            draw_command_overview_text(
                                painter, 48, 56,
                                field_menu_text_
                                    .notebook_mr_johnson_details[source_index]);
                        }
                    }
                    return;
                }
                for (std::size_t row = 0; row < page.visible_count; ++row) {
                    const auto source_index =
                        page.visible_source_indices[row];
                    if (source_index >=
                        field_menu_text_.notebook_mr_johnson_names.size()) {
                        continue;
                    }
                    const auto y = 56 + static_cast<int>(row) * 16;
                    draw_command_overview_text(
                        painter, 88, y,
                        field_menu_text_
                            .notebook_mr_johnson_names[source_index]);
                }
                if (page.selection < page.visible_count) {
                    draw_command_overview_text(
                        painter, 72,
                        56 + static_cast<int>(page.selection) * 16, "x");
                }
                return;
            }
            if (pocket_secretary_.notebook.tips.open) {
                const auto& tips = pocket_secretary_.notebook.tips;
                draw_command_overview_text(
                    painter, 88, 32, field_menu_text_.notebook_tips_title);
                draw_command_overview_text(
                    painter, 72, 40,
                    field_menu_text_.notebook_tips_separator);
                draw_command_overview_text(
                    painter, 136, 184,
                    field_menu_text_.notebook_tips_page_count_label);
                if (tips.visible_count != 0U &&
                    tips.selection < tips.visible_count) {
                    const auto source_index =
                        tips.visible_source_indices[tips.selection];
                    draw_command_overview_text(
                        painter, 40, 56,
                        field_menu_text_.notebook_tips[source_index]);
                }
                const auto draw_right_aligned_decimal =
                    [this, &painter](const int right_edge,
                                     const int y,
                                     const std::size_t value) {
                        const auto digits = std::to_string(value);
                        const auto left_edge = right_edge -
                            static_cast<int>(digits.size() - 1U) * 8;
                        draw_command_overview_text(
                            painter, left_edge, y, digits);
                    };
                draw_right_aligned_decimal(
                    184, 184,
                    static_cast<std::size_t>(tips.selection) + 1U);
                draw_right_aligned_decimal(232, 184, tips.visible_count);
                return;
            }
            if (pocket_secretary_.notebook.no_current_run_wait) {
                for (std::size_t line = 0;
                     line < field_menu_text_.notebook_no_run_lines.size();
                     ++line) {
                    draw_command_overview_text(
                        painter, 48, 48 + static_cast<int>(line) * 8,
                        field_menu_text_.notebook_no_run_lines[line]);
                }
                return;
            }
            // ROM 0xF33E clears through C3E6, writes C567E at C216, and
            // writes its separator at C292. C0DA advances 0x80 source bytes
            // (one eight-pixel row) at each marker. Current Run and Tips &
            // Clues therefore start at y=56 and y=72; F33E places each
            // enabled event-gated row another 16 pixels below the previous
            // row. F236's C376 cursor base is C392, eight pixels left of the
            // first option, and uses the same 16-pixel row stride.
            draw_command_overview_text(
                painter, 88, 32, field_menu_text_.notebook_labels[0]);
            draw_command_overview_text(
                painter, 72, 40, pocket_secretary_menu_content_.separator);
            for (std::size_t row = 0;
                 row < pocket_secretary_.notebook.option_count; ++row) {
                const auto label_index = static_cast<std::size_t>(
                    pocket_secretary_.notebook.visible_options[row]) + 1U;
                if (label_index >= field_menu_text_.notebook_labels.size()) {
                    continue;
                }
                const auto y = 56 + static_cast<int>(row) * 16;
                draw_command_overview_text(
                    painter, 88, y,
                    field_menu_text_.notebook_labels[label_index]);
            }
            if (pocket_secretary_.notebook.selection <
                pocket_secretary_.notebook.option_count) {
                draw_command_overview_text(
                    painter, 72,
                    56 + static_cast<int>(
                             pocket_secretary_.notebook.selection) * 16,
                    "x");
            }
            return;
        }
        if (dismiss_runner_.open) {
            // ROM C448 writes its fixed C1499/C14A4/Separator/C14BD block
            // through C0DA at C216. Abort remains at C416 (y=64); each active
            // FF0200/FF0300 name is written one row below it, at y=80+16n.
            // Redrawing the cleared shared inner grid first prevents the old
            // Pocket Secretary buttons from remaining beneath this child.
            draw_command_overview_text(
                painter, 88, 32, field_menu_text_.dismiss_runner_labels[0]);
            draw_command_overview_text(
                painter, 88, 40, field_menu_text_.dismiss_runner_labels[1]);
            draw_command_overview_text(
                painter, 72, 48, field_menu_text_.dismiss_runner_separator);
            draw_command_overview_text(
                painter, 88, 64, field_menu_text_.dismiss_runner_labels[2]);
            for (std::size_t row = 0;
                 row < dismiss_runner_.candidate_count; ++row) {
                const auto slot = dismiss_runner_.candidate_slots[row];
                draw_command_overview_text(
                    painter, 88, 80 + static_cast<int>(row) * 16,
                    field_party_roster_.name(slot));
            }
            if (dismiss_runner_.selection <=
                dismiss_runner_.candidate_count) {
                draw_command_overview_text(
                    painter, 72,
                    64 + static_cast<int>(dismiss_runner_.selection) * 16,
                    "x");
            }
            if (const auto slot = dismiss_runner_preview_slot();
                slot.has_value() &&
                *slot < field_party_roster_.records().size()) {
                const auto& record = field_party_roster_.records()[*slot];
                constexpr std::uint16_t kPaletteLineMask = 0x6000U;
                const auto panel_attributes =
                    record.command_overview_panel_parameter;
                // ROM 0x110EC maps +0x3E values through <=0, ==1, and >=2
                // layout branches. All three select a portrait-panel origin;
                // recruited records commonly carry 3 or 4, so larger values
                // must remain valid here.
                if ((panel_attributes &
                            static_cast<std::uint16_t>(~kPaletteLineMask)) !=
                           0U) {
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.preview_attributes_unresolved");
                } else if (record.hud_portrait_table_pointer == 0U) {
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.preview_table_unresolved");
                } else {
                    const auto* preview =
                        runner_preview_panel_for_record(record);
                    if (preview == nullptr) {
                        source_boundary(
                            "field.pocket_secretary.dismiss_runner.preview_asset_unavailable");
                    } else {
                        painter.drawImage(kRunnerPreviewPanelX,
                                          kRunnerPreviewPanelY, *preview);
                    }
                }
            }
            return;
        }
        if (pocket_secretary_.cyberdeck_unavailable_wait) {
            // 0x18150 calls C3E6 before writing this response at C40E, which
            // replaces the parent menu plane contents until the wait ends.
            for (std::size_t line = 0;
                 line < field_menu_text_
                           .pocket_secretary_cyberdeck_unavailable_lines.size();
                 ++line) {
                draw_command_overview_text(
                    painter, 56, 64 + static_cast<int>(line) * 8,
                    field_menu_text_
                        .pocket_secretary_cyberdeck_unavailable_lines[line]);
            }
            return;
        }
        if (pocket_secretary_.group_items_wait) {
            if (pocket_secretary_.group_item_flags == 0U) {
                for (std::size_t line = 0;
                     line < field_menu_text_
                                .pocket_secretary_group_items_empty_lines.size();
                     ++line) {
                    draw_command_overview_text(
                        painter, 56, 80 + static_cast<int>(line) * 8,
                        field_menu_text_
                            .pocket_secretary_group_items_empty_lines[line]);
                }
            } else {
                const auto display_order =
                    pocket_secretary_group_item_display_order(
                        pocket_secretary_.group_item_flags);
                for (std::size_t row = 0; row < display_order.size(); ++row) {
                    const auto item_index = display_order[row];
                    if (item_index >= field_menu_text_
                                          .pocket_secretary_group_item_names
                                          .size()) {
                        continue;
                    }
                    draw_command_overview_text(
                        painter, 32, 32 + static_cast<int>(row) * 16,
                        field_menu_text_
                            .pocket_secretary_group_item_names[item_index]);
                }
            }
            return;
        }
        draw_command_overview_text(
            painter, 88, 32, pocket_secretary_menu_content_.title);
        draw_command_overview_text(
            painter, 88, 40, pocket_secretary_menu_content_.separator);
        for (std::size_t row = 0;
             row < pocket_secretary_menu_content_.choices.size(); ++row) {
            draw_command_overview_text(
                painter, 88, 56 + static_cast<int>(row) * 16,
                pocket_secretary_menu_content_.choices[row].label);
        }
        if (pocket_secretary_.selection <
            pocket_secretary_menu_content_.choices.size()) {
            draw_command_overview_text(
                painter, 136,
                56 + static_cast<int>(pocket_secretary_.selection) * 16,
                "x");
        }
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
        // 0x119CE writes the six-tile POSTURE focus row at CA9E (120,168).
        // The five-state strip starts separately at CAAC (176,168), so keep
        // it visible and replace only the static label beneath the selector.
        painter.drawImage(120, 168, posture_focus_);
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
    const auto draw_armor_summary = [this, &painter](
                                        const std::uint8_t armor_index) {
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
        if (armor->secondary_modifier <
            command_overview_armor_summary_bars_.size()) {
            painter.drawImage(176, 144,
                              command_overview_armor_summary_bars_[
                                  armor->secondary_modifier]);
        }
        if (armor->primary_modifier <
            command_overview_armor_summary_bars_.size()) {
            painter.drawImage(176, 152,
                              command_overview_armor_summary_bars_[
                                  armor->primary_modifier]);
        }
    };

    const auto& view = command_overview_.view();
    if (view.source_state.primary_page >= 2U &&
        !command_overview_clips_.is_open()) {
        const auto race = active_runner_record().species_index;
        const auto character_class = active_runner_record().archetype_index;
        if (race < field_menu_text_.race_names.size()) {
            draw_command_overview_text(
                painter, 128, 24, field_menu_text_.race_names[race]);
        }
        if (character_class < field_menu_text_.class_names.size()) {
            draw_command_overview_text(
                painter, 176, 24,
                field_menu_text_.class_names[character_class]);
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
        draw_armor_summary(active_runner_record().equipped_armor_index);
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
                // ROM 0x12A8C sends selected armor to 0x11352. It previews
                // the selected D4D50 row in the lower armor summary at
                // C820/C922/C92C/C9AC, then restores the equipped record.
                // Repaint that panel from typed state rather than placing a
                // second Wpns/Guns pair beside the selected-item text.
                if (item_code >= 0x2CU && item_code <= 0x32U) {
                    const auto* armor =
                        find_us_armor_definition_for_item_code(item_code);
                    if (armor != nullptr) {
                        draw_armor_summary(armor->armor_index);
                    }
                }
            }
        }
    }

    // The equipment-cell blocks are opaque name-table records installed by
    // 0x11624 before the inventory item glyph rows. Paint them under the
    // direct item sprites so the selected/equipped frame cannot erase its
    // icon.
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

    if (view.source_state.primary_page < 2U &&
        !command_overview_inventory_selector_suppressed_) {
        // ROM 0x11592 builds the cursor SAT entry from the 0xD29F6 list:
        // x = FFF0D0*0x28 + 0x008C, y = FFF0D1*0x18 + 0x00F4. The Genesis
        // SAT origin adds 0x80 on both axes. The 24x16 item graphic starts
        // four pixels inside that 32x24 sprite footprint. Section 06 frames
        // 2580, 2610, and 2640 show a one-pixel warm-gold outline hugging the
        // item graphic; outlining the larger SAT footprint leaves the broad
        // rectangular margin that is absent from those captures.
        const auto x = 16 + static_cast<int>(
                               view.source_state.primary_page * 40U);
        const auto y = 120 + static_cast<int>(
                                view.source_state.secondary_page * 24U);
        constexpr int kOutlineWidth = 24;
        constexpr int kOutlineHeight = 16;
        const QColor selector_color(196, 190, 39);
        painter.fillRect(x, y, kOutlineWidth, 1, selector_color);
        painter.fillRect(x, y + kOutlineHeight - 1,
                         kOutlineWidth, 1, selector_color);
        painter.fillRect(x, y, 1, kOutlineHeight, selector_color);
        painter.fillRect(x + kOutlineWidth - 1, y, 1, kOutlineHeight,
                         selector_color);
    }

    if (command_overview_cyberware_active_) {
        // ROM 0x11950 rebuilds the center name-plane region before writing
        // C1A2/C328/C31C. Keep the command screen's separate portrait,
        // inventory, posture, tab, and right-HUD layers in the composition;
        // replace only the central text panel so old status text cannot show
        // through the Cyberware rows.
        painter.drawImage(96, 16, command_overview_background_, 96, 16,
                          168, 144);
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
        if (command_overview_discard_confirmation_active_) {
            painter.drawImage(
                kCommandOverviewActionBackgroundX,
                kCommandOverviewActionBackgroundY,
                command_overview_action_confirmation_background_);
            const auto choice_index = static_cast<std::size_t>(
                command_overview_discard_confirmation_.selected_option == 0U
                    ? 0U
                    : 1U);
            painter.drawImage(
                kCommandOverviewActionConfirmationFocusX[choice_index],
                kCommandOverviewActionConfirmationFocusY,
                command_overview_action_confirmation_focus_[choice_index]);
            // 0x126A2 writes D1CD3 at Plane A C7A0 (128,120).
            draw_command_overview_text(
                painter, 128, 120, field_menu_text_.action_discard_prompt);
            if (command_overview_selected_inventory_item_.has_value() &&
                active_runner_record().equipped_inventory_slot >= 0 &&
                static_cast<std::size_t>(
                    active_runner_record().equipped_inventory_slot) ==
                    command_overview_selected_inventory_item_->slot) {
                // 0x126AE writes D1CBB at C620; its 0x80 line break separates
                // the warning at y=96 from the suffix at y=104.
                draw_command_overview_text(
                    painter, 128, 96,
                    field_menu_text_.action_item_equipped_prefix);
                draw_command_overview_text(
                    painter, 128, 104,
                    field_menu_text_.action_item_equipped_suffix);
            }
            draw_command_overview_text(
                painter, 112, 200,
                command_overview_discard_confirmation_.selected_option == 0U
                    ? field_menu_text_.action_choice_yes
                    : field_menu_text_.action_choice_no);
        } else {
            painter.drawImage(kCommandOverviewActionBackgroundX,
                              kCommandOverviewActionBackgroundY,
                              command_overview_action_background_);
            const auto action_selector =
                command_overview_inventory_action_.action_selector;
            if (action_selector >= 2U && action_selector <= 5U) {
                const auto focus_index = static_cast<std::size_t>(
                    action_selector - 2U);
                painter.drawImage(kCommandOverviewActionFocusX[focus_index],
                                  kCommandOverviewActionFocusY,
                                  command_overview_action_focus_[focus_index]);
            }
        }
    } else if (command_overview_inventory_trade_.open) {
        painter.drawImage(kCommandOverviewActionBackgroundX,
                          kCommandOverviewActionBackgroundY,
                          command_overview_action_confirmation_background_);
        const auto choice_index = command_overview_inventory_trade_.yes_selected
            ? 0U
            : 1U;
        painter.drawImage(
            kCommandOverviewActionConfirmationFocusX[choice_index],
            kCommandOverviewActionConfirmationFocusY,
            command_overview_action_confirmation_focus_[choice_index]);
        if (command_overview_inventory_trade_.recipient_inventory_full) {
            const auto recipient =
                command_overview_inventory_trade_.recipient_slot;
            if (recipient < FieldPartyRosterRuntime::kSlotCount) {
                draw_command_overview_text(
                    painter, 120, 104, field_party_roster_.name(recipient));
            }
            draw_command_overview_text(
                painter, 120, 112,
                field_menu_text_.action_trade_full_inventory_message);
        } else {
            draw_command_overview_text(
                painter, 152, 120,
                field_menu_text_.action_trade_recipient_label);
            const auto recipient =
                command_overview_inventory_trade_.recipient_slot;
            if (recipient < FieldPartyRosterRuntime::kSlotCount) {
                draw_command_overview_text(
                    painter, 128, 128, field_party_roster_.name(recipient));
                const auto& recipient_record =
                    field_party_roster_.records()[recipient];
                const auto& panel_attributes =
                    recipient_record.command_overview_panel_parameter;
                constexpr std::uint16_t kPaletteLineMask = 0x6000U;
                if ((panel_attributes &
                     static_cast<std::uint16_t>(~kPaletteLineMask)) != 0U) {
                    source_boundary(
                        "field.inventory.trade.preview_attributes_unresolved");
                } else if (recipient_record.hud_portrait_table_pointer == 0U) {
                    source_boundary(
                        "field.inventory.trade.preview_table_unresolved");
                } else if (const auto* preview =
                               runner_preview_panel_for_record(
                                   recipient_record);
                           preview == nullptr) {
                    source_boundary(
                        "field.inventory.trade.preview_asset_unavailable");
                } else {
                    // ROM 0x1287E reaches 0x110E8 for a recipient with room;
                    // the same typed 32x32 extracted panel used by Dismiss
                    // Runner is projected at the source HUD coordinate.
                    painter.drawImage(kRunnerPreviewPanelX,
                                      kRunnerPreviewPanelY, *preview);
                }
            }
            if (command_overview_selected_inventory_item_.has_value() &&
                active_runner_record().equipped_inventory_slot >= 0 &&
                static_cast<std::size_t>(
                    active_runner_record().equipped_inventory_slot) ==
                    command_overview_selected_inventory_item_->slot) {
                draw_command_overview_text(
                    painter, 128, 96,
                    field_menu_text_.action_item_equipped_prefix);
                draw_command_overview_text(
                    painter, 128, 104,
                    field_menu_text_.action_item_equipped_suffix);
            }
        }
        draw_command_overview_text(
            painter, 112, 200,
            command_overview_inventory_trade_.yes_selected
                ? field_menu_text_.action_choice_yes
                : field_menu_text_.action_choice_no);
    } else if (command_overview_action_message_ !=
               CommandOverviewActionMessage::none) {
        if (command_overview_action_message_ ==
            CommandOverviewActionMessage::cannot_discard) {
            draw_command_overview_text(
                painter, 128, 96, field_menu_text_.action_cannot_discard);
        } else if (command_overview_action_message_ ==
                   CommandOverviewActionMessage::cannot_trade) {
            draw_command_overview_text(
                painter, 136, 96, field_menu_text_.action_cannot_trade);
        }
    } else if (!command_overview_clips_.is_open() &&
               command_overview_first_aid_state_ ==
                   CommandOverviewFirstAidState::inactive &&
               view.source_state.primary_page <
               field_menu_text_.command_labels.size()) {
        draw_command_overview_text(
            painter, 112, 200,
            field_menu_text_.command_labels[view.source_state.primary_page]);
    }

    if (command_overview_magic_state_ ==
        CommandOverviewMagicState::magical_defense_wait) {
        // ROM 0xFBA6 writes C89C0 at name-plane C31C (screen 112,48), then
        // 0x106DA writes 13-tile strips at C51E/C69E (120,80 and 120,104).
        // Keep the page and runner/HUD below these overlays; this fallback
        // does not call 0x0FFE to clear Plane A.
        draw_command_overview_text(painter, 112, 48,
                                   field_menu_text_.magic_defense_title);
        const auto draw_strip = [this, &painter](
                                    const MagicDefenseStripView& strip,
                                    const int y) {
            if (strip.bar_row < command_overview_defense_bars_.size()) {
                painter.drawImage(
                    120, y, command_overview_defense_bars_[strip.bar_row]);
            }
        };
        draw_strip(command_overview_magic_physical_defense_, 80);
        draw_strip(command_overview_magic_mana_defense_, 104);
    }

    if (command_overview_first_aid_state_ !=
        CommandOverviewFirstAidState::inactive) {
        if (command_overview_first_aid_state_ ==
            CommandOverviewFirstAidState::target_selection) {
            draw_command_overview_text(
                painter, 128, 88, field_menu_text_.first_aid_prompt);
            for (std::size_t row = 0;
                 row < command_overview_first_aid_targets_.active_count;
                 ++row) {
                const auto record_index = command_overview_first_aid_targets_
                                              .active_record_indices[row];
                if (record_index < FieldPartyRosterRuntime::kSlotCount) {
                    draw_command_overview_text(
                        painter, 128, 104 + static_cast<int>(row) * 8,
                        field_party_roster_.name(record_index));
                }
            }
            draw_command_overview_text(
                painter, 152,
                104 + static_cast<int>(
                          command_overview_first_aid_targets_.active_count) * 8,
                field_menu_text_.first_aid_cancel);
            const auto selected_row = command_overview_first_aid_cursor_;
            const auto cursor_x = selected_row ==
                                          command_overview_first_aid_targets_
                                              .cancel_row()
                                      ? 144
                                      : 120;
            draw_command_overview_text(
                painter, cursor_x,
                104 + static_cast<int>(selected_row) * 8, ">");
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::medkit_used) {
            draw_command_overview_text(
                painter, 144, 104,
                field_menu_text_.first_aid_medkit_used);
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::slap_patch_used) {
            draw_command_overview_text(
                painter, 144, 104,
                field_menu_text_.first_aid_slap_patch_used);
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::target_not_injured) {
            draw_command_overview_text(
                painter, 104, 104,
                field_menu_text_.first_aid_not_injured);
        }

        if (command_overview_first_aid_cursor_ <
            command_overview_first_aid_targets_.active_count) {
            const auto target_index = command_overview_first_aid_targets_
                                          .active_record_indices[
                                              command_overview_first_aid_cursor_];
            // This First Aid child panel currently projects the selected
            // Joshua face. The separate three-slot HUD asset families do not
            // imply that recruited target portraits are wired into this page.
            if (target_index == 0U &&
                field_joshua_archetype_selection_ <
                    field_hud_faces_.size() &&
                !field_hud_faces_[field_joshua_archetype_selection_].isNull()) {
                painter.drawImage(
                    144, 144,
                    field_hud_faces_[field_joshua_archetype_selection_]);
            }
        }
        draw_command_overview_text(
            painter, 104, 200, field_menu_text_.first_aid_label);
    }
    if (command_overview_automatic_use_state_ ==
        CommandOverviewAutomaticUseState::result_wait) {
        draw_command_overview_text(
            painter, 128, 96,
            field_menu_text_.action_automatic_use_message);
    }
}

void FieldSceneRuntime::paint(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!ready_ || field_map_.isNull()) {
        return;
    }
    const auto apply_palette_fade = [this, &painter] {
        const auto transition_alpha = static_cast<int>(
            field_elevator_scene_transition_.black_overlay_alpha());
        const auto alpha = std::max(
            transition_alpha,
            std::min(255, static_cast<int>(field_palette_fade_steps_) * 32));
        if (alpha == 0) {
            return;
        }
        painter.fillRect(0, 0, 320, 224, QColor(0, 0, 0, alpha));
    };
    if (field_elevator_selector_.is_open()) {
        const auto animation_frame = static_cast<std::size_t>(
            field_elevator_selector_.animation_frame());
        const auto current_floor = static_cast<std::size_t>(
            field_elevator_selector_.current_floor());
        const auto selected_floor = static_cast<std::size_t>(
            field_elevator_selector_.selected_floor());
        painter.drawImage(
            0, 0, field_elevator_selector_backgrounds_[animation_frame]);
        switch (field_elevator_selector_.navigation_feedback()) {
        case ElevatorSelectorNavigationFeedback::previous_floor_active:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[0U]);
            break;
        case ElevatorSelectorNavigationFeedback::previous_floor_rest:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[1U]);
            break;
        case ElevatorSelectorNavigationFeedback::next_floor_active:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[2U]);
            break;
        case ElevatorSelectorNavigationFeedback::next_floor_rest:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[3U]);
            break;
        case ElevatorSelectorNavigationFeedback::none:
            break;
        }
        // The ROM writes the arrow feedback after the floor name-table
        // updates, but the selector labels occupy the higher-priority Plane-B
        // layer. Keep both digits visible above the temporary rail graphics.
        painter.drawImage(
            0, 0, field_elevator_selector_current_floor_layers_[current_floor]);
        painter.drawImage(
            0, 0,
            field_elevator_selector_selected_floor_layers_[selected_floor]);
        painter.drawImage(0, 0, field_hud_);
        paint_field_hud_overlays(painter);
        apply_palette_fade();
        return;
    }
    if (field_interaction_.open()) {
        field_interaction_.paint(painter);
        apply_palette_fade();
        return;
    }
    if (hotel_karma_skills_.is_open()) {
        paint_attributes_skills_menu(painter,
                                     hotel_karma_skills_.controller());
        apply_palette_fade();
        return;
    }
    if (hotel_menu_.open()) {
        hotel_menu_.paint(
            painter, pocket_secretary_grid_, hotel_options_cursor_,
            hotel_checkout_cursor_, dismiss_runner_random_grid_patterns_,
            hotel_menu_content_,
            [this](QPainter& target, const int x, const int y,
                   const std::string_view text,
                   const HotelMenuPaletteLine palette_line) {
                draw_hotel_menu_text(target, x, y, text, palette_line);
            });
        apply_palette_fade();
        return;
    }
    if (field_mode_text_.open()) {
        field_mode_text_.paint(painter);
        apply_palette_fade();
        return;
    }
    if (field_shop_menu_.is_open()) {
        paint_field_shop(painter);
        apply_palette_fade();
        return;
    }
    if (field_dialogue_.open()) {
        field_dialogue_.paint(painter);
        apply_palette_fade();
        return;
    }
    if (command_overview_.open()) {
        paint_command_overview(painter);
        if (command_overview_attributes_skills_.is_open()) {
            apply_palette_fade();
            return;
        }
        // The field command loop updates its menu plane without replacing
        // the field's right-hand HUD plane. Preserve the current runner face,
        // item, and counters above the command panel. Pocket Secretary is a
        // separate full-screen composition and owns its own backdrop.
        if (!pocket_secretary_.open) {
            painter.drawImage(0, 0, field_hud_);
            paint_field_hud_overlays(painter);
        }
        apply_palette_fade();
        return;
    }
    paint_field_scene(painter);
    apply_palette_fade();
}

void FieldSceneRuntime::paint_field_scene_for_dump(
    QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!ready_ || field_map_.isNull()) {
        return;
    }
    paint_field_scene(painter);
}

std::size_t FieldSceneRuntime::default_actor_count_for_dump() const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference;
        }));
}

std::size_t
FieldSceneRuntime::default_actor_transition_started_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.behavior.transition_started;
        }));
}

std::size_t
FieldSceneRuntime::default_actor_transition_complete_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.behavior.transition_complete;
        }));
}

std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_direction_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->behavior.direction};
}

std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_animation_frame_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{
                     actor->behavior.animation_frame};
}

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

void FieldSceneRuntime::paint_field_scene(QPainter& painter) const {
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
    paint_field_scene_entities(painter);

    // 0x334C supplies the static chrome. 0x35D4/0x35F0/0x36FC/0x3674 stamp
    // the live face, item, and counter overlays at the $F000 nametable
    // cells. New-game keeps slots 1 and 2 inactive.
    painter.drawImage(0, 0, field_hud_);
    paint_field_hud_overlays(painter);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

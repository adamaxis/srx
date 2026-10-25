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

void FieldSceneRuntime::synchronize_command_overview_magic_from_route(
    const scripting::ScriptMenuRouteSnapshot& route_state) {
    menu_route_magic_view_.reset();
    menu_route_magic_physical_defense_ = MagicDefenseStripView{};
    menu_route_magic_mana_defense_ = MagicDefenseStripView{};
    if (!route_state.open) {
        command_overview_magic_state_ = CommandOverviewMagicState::inactive;
        command_overview_magic_selected_spell_.reset();
        return;
    }

    switch (static_cast<srx::MenuScreen>(route_state.top_screen)) {
    case srx::MenuScreen::MAGIC:
        command_overview_magic_state_ = CommandOverviewMagicState::spell_panel;
        break;
    case srx::MenuScreen::MAGICAL_DEFENSE:
        command_overview_magic_state_ =
            CommandOverviewMagicState::magical_defense_wait;
        break;
    case srx::MenuScreen::MAGIC_FORCE:
        command_overview_magic_state_ =
            CommandOverviewMagicState::spell_force_selection;
        break;
    case srx::MenuScreen::MAGIC_POSTURE:
        command_overview_magic_state_ =
            CommandOverviewMagicState::spell_posture_selection;
        break;
    default:
        command_overview_magic_state_ = CommandOverviewMagicState::inactive;
        command_overview_magic_selected_spell_.reset();
        return;
    }

    if (!route_state.magic_view.has_value() ||
        !route_state.magic_view->present) {
        command_overview_magic_state_ = CommandOverviewMagicState::inactive;
        source_boundary("field.command_overview.magic.lua_view_missing");
        return;
    }
    const auto& view = *route_state.magic_view;
    menu_route_magic_view_ = view;
    command_overview_magic_loop_.selection = {
        static_cast<std::uint8_t>(std::clamp(view.column, 0, 0xFF)),
        static_cast<std::uint8_t>(std::clamp(view.row, 0, 0xFF))};
    command_overview_magic_force_.current_force =
        static_cast<std::uint16_t>(std::clamp(view.force, 0, 0xFFFF));
    command_overview_magic_force_.maximum_force =
        static_cast<std::uint16_t>(std::clamp(view.maximum_force, 0, 0xFFFF));
    command_overview_magic_cursor_phase_ =
        static_cast<std::uint8_t>(std::clamp(view.cursor_phase, 0, 0xFF));
    menu_route_magic_physical_defense_ = {
        static_cast<std::uint16_t>(std::clamp(
            view.physical_defense_value, 0, 0xFFFF)),
        static_cast<std::uint8_t>(std::clamp(
            view.physical_defense_row, 0, 0xFF))};
    menu_route_magic_mana_defense_ = {
        static_cast<std::uint16_t>(std::clamp(
            view.mana_defense_value, 0, 0xFFFF)),
        static_cast<std::uint8_t>(std::clamp(
            view.mana_defense_row, 0, 0xFF))};
    command_overview_magic_physical_defense_ =
        menu_route_magic_physical_defense_;
    command_overview_magic_mana_defense_ = menu_route_magic_mana_defense_;

    if (view.spell_code < 0 || view.spell_code > 0xFF) {
        command_overview_magic_selected_spell_.reset();
        return;
    }
    const auto* spell = find_us_magic_spell(
        static_cast<std::uint8_t>(view.spell_code));
    if (spell == nullptr) {
        command_overview_magic_selected_spell_.reset();
        source_boundary("field.command_overview.magic.lua_spell_missing");
        return;
    }
    command_overview_magic_selected_spell_ = MagicSpellSelection{
        static_cast<std::uint16_t>(std::clamp(
            view.table_entry_index, 0, 0xFFFF)),
        static_cast<std::uint8_t>(view.spell_code),
        static_cast<std::uint8_t>(std::clamp(view.maximum_force, 0, 0xFF)),
        spell->availability_byte,
        spell->availability_byte >= 0
            ? MagicSpellAvailability::available
            : MagicSpellAvailability::unavailable};
}

void FieldSceneRuntime::synchronize_pocket_secretary_notebook_from_route(
    const scripting::ScriptMenuRouteSnapshot& route_state) {
    const auto screen = static_cast<srx::MenuScreen>(route_state.top_screen);
    const bool is_message = screen == srx::MenuScreen::MESSAGE;
    const auto message = route_state.top_message.value_or(0);
    const bool no_current_run =
        is_message && message == static_cast<int>(srx::MenuMessage::NO_CURRENT_RUN);
    const bool johnson_detail =
        is_message && message ==
                          static_cast<int>(srx::MenuMessage::MR_JOHNSON_DETAIL);
    const bool contacts_detail =
        is_message && message == static_cast<int>(
                                      srx::MenuMessage::NOTEBOOK_CONTACT_DETAIL);
    const bool shadowrunners_detail =
        is_message && message == static_cast<int>(
                                      srx::MenuMessage::
                                          NOTEBOOK_SHADOWRUNNER_DETAIL);
    const bool contacts_page =
        screen == srx::MenuScreen::NOTEBOOK_CONTACTS;
    const bool shadowrunners_page =
        screen == srx::MenuScreen::NOTEBOOK_SHADOWRUNNERS;
    const bool current_run_page =
        screen == srx::MenuScreen::NOTEBOOK_CURRENT_RUN;
    auto& notebook = pocket_secretary_.notebook;
    if (screen != srx::MenuScreen::NOTEBOOK &&
        screen != srx::MenuScreen::NOTEBOOK_TIPS &&
        screen != srx::MenuScreen::MR_JOHNSONS && !no_current_run &&
        !current_run_page &&
        !johnson_detail && !contacts_page && !shadowrunners_page &&
        !contacts_detail && !shadowrunners_detail) {
        notebook = NotebookMenuState{};
        return;
    }

    notebook.open = true;
    notebook.no_current_run_wait = no_current_run;
    notebook.current_run.open = current_run_page;
    if (current_run_page &&
        route_state.top_notebook_current_run.has_value()) {
        const auto& source_view =
            *route_state.top_notebook_current_run;
        auto& current_run = notebook.current_run;
        current_run.source_initializer = source_view.source_initializer;
        current_run.context = source_view.context;
        current_run.run_type = source_view.run_type;
        current_run.payment_amount = source_view.payment_amount;
        current_run.johnson_label = source_view.johnson_label;
        current_run.johnson_name = source_view.johnson_name;
        current_run.run_type_label = source_view.run_type_label;
        current_run.run_type_name = source_view.run_type_name;
        current_run.payment_label = source_view.payment_label;
        current_run.currency_symbol = source_view.currency_symbol;
        current_run.body_lines = source_view.body_lines;
    } else if (!current_run_page) {
        notebook.current_run = NotebookMenuState::CurrentRunState{};
    }
    notebook.tips.open = screen == srx::MenuScreen::NOTEBOOK_TIPS;
    notebook.mr_johnsons.open = screen == srx::MenuScreen::MR_JOHNSONS ||
                                johnson_detail;
    notebook.mr_johnsons.detail_wait = johnson_detail;
    notebook.contacts.open = contacts_page || contacts_detail;
    notebook.contacts.detail_wait = contacts_detail;
    notebook.shadowrunners.open = shadowrunners_page || shadowrunners_detail;
    notebook.shadowrunners.detail_wait = shadowrunners_detail;

    if (screen == srx::MenuScreen::NOTEBOOK) {
        notebook.option_count = static_cast<std::uint8_t>(
            std::min<std::size_t>(route_state.top_visible_option_ids.size(),
                                  notebook.visible_options.size()));
        for (std::size_t index = 0; index < notebook.option_count; ++index) {
            notebook.visible_options[index] =
                static_cast<NotebookMenuOption>(
                    route_state.top_visible_option_ids[index]);
        }
        notebook.selection = static_cast<std::uint8_t>(
            std::clamp(route_state.top_row.value_or(0), 0,
                       std::max(0, static_cast<int>(notebook.option_count) - 1)));
    }

    if (notebook.tips.open) {
        auto& tips = notebook.tips;
        tips.visible_count = static_cast<std::uint8_t>(
            std::min<std::size_t>(route_state.top_visible_source_ids.size(),
                                  tips.visible_source_indices.size()));
        for (std::size_t index = 0; index < tips.visible_count; ++index) {
            tips.visible_source_indices[index] = static_cast<std::uint8_t>(
                route_state.top_visible_source_ids[index]);
        }
        tips.selection = static_cast<std::uint8_t>(
            std::clamp(route_state.top_page.value_or(0), 0,
                       std::max(0, static_cast<int>(tips.visible_count) - 1)));
    } else {
        notebook.tips.open = false;
    }

    if (notebook.mr_johnsons.open) {
        auto& page = notebook.mr_johnsons;
        page.visible_count = static_cast<std::uint8_t>(
            std::min<std::size_t>(route_state.top_visible_source_ids.size(),
                                  page.visible_source_indices.size()));
        for (std::size_t index = 0; index < page.visible_count; ++index) {
            page.visible_source_indices[index] = static_cast<std::uint8_t>(
                route_state.top_visible_source_ids[index]);
        }
        const auto selected_row = johnson_detail
                                      ? route_state.top_detail_row.value_or(0)
                                      : route_state.top_row.value_or(0);
        page.selection = static_cast<std::uint8_t>(
            std::clamp(selected_row, 0,
                       std::max(0, static_cast<int>(page.visible_count) - 1)));
    }
    const auto synchronize_record_page =
        [&route_state](NotebookMenuState::RecordPageState& page,
                       const bool detail_wait) {
            if (!page.open) return;
            page.visible_count = static_cast<std::uint8_t>(
                std::min<std::size_t>(route_state.top_visible_source_ids.size(),
                                      page.visible_source_indices.size()));
            for (std::size_t index = 0; index < page.visible_count; ++index) {
                page.visible_source_indices[index] =
                    static_cast<std::uint8_t>(
                        route_state.top_visible_source_ids[index]);
            }
            const auto selected_row = detail_wait
                                          ? route_state.top_detail_row.value_or(0)
                                          : route_state.top_row.value_or(0);
            page.selection = static_cast<std::uint8_t>(std::clamp(
                selected_row, 0,
                std::max(0, static_cast<int>(page.visible_count) - 1)));
        };
    synchronize_record_page(notebook.contacts, contacts_detail);
    synchronize_record_page(notebook.shadowrunners, shadowrunners_detail);
}

void FieldSceneRuntime::synchronize_command_overview_clips_from_route(
    const scripting::ScriptMenuRouteSnapshot& route_state) {
    constexpr int kClipsPage = static_cast<int>(srx::MenuPage::CLIPS);
    constexpr std::uint16_t kClips =
        static_cast<std::uint16_t>(srx::MenuScreen::CLIPS);
    constexpr std::uint16_t kReloadConfirm =
        static_cast<std::uint16_t>(srx::MenuScreen::RELOAD_CONFIRM);
    constexpr std::uint16_t kClipTrade =
        static_cast<std::uint16_t>(srx::MenuScreen::CLIP_TRADE);
    constexpr std::uint16_t kMessage =
        static_cast<std::uint16_t>(srx::MenuScreen::MESSAGE);
    const bool clips_route =
        route_state.open && route_state.page == kClipsPage &&
        (route_state.top_screen == kClips ||
         route_state.top_screen == kReloadConfirm ||
         route_state.top_screen == kClipTrade ||
         route_state.top_screen == kMessage);
    if (!clips_route) {
        if (command_overview_clips_.is_open()) {
            command_overview_clips_.close();
        }
        return;
    }

    if (!command_overview_clips_.is_open()) {
        command_overview_clips_.open();
    }
    auto message = ClipsMenuMessage::none;
    if (route_state.top_screen == kMessage &&
        route_state.top_message.has_value()) {
        using srx::MenuMessage;
        switch (static_cast<MenuMessage>(*route_state.top_message)) {
        case MenuMessage::OUT_OF_AMMO:
            message = ClipsMenuMessage::out_of_ammo; break;
        case MenuMessage::ITEM_NO_CLIPS:
            message = ClipsMenuMessage::item_does_not_use_clips; break;
        case MenuMessage::NO_PARTY:
            message = ClipsMenuMessage::no_party_members; break;
        case MenuMessage::RELOADED:
            message = ClipsMenuMessage::reload_success; break;
        case MenuMessage::EXCHANGE_COMPLETE:
            message = ClipsMenuMessage::exchange_complete; break;
        case MenuMessage::FUNCTION_ABORTED:
            message = ClipsMenuMessage::function_aborted; break;
        default: break;
        }
    }
    const auto u8_or = [](const std::optional<int>& value,
                          const std::uint8_t fallback) {
        return static_cast<std::uint8_t>(
            value.has_value() ? std::clamp(*value, 0, 0xFF) : fallback);
    };
    const bool reload_confirmation = route_state.top_screen == kReloadConfirm;
    const bool trade = route_state.top_screen == kClipTrade;
    command_overview_clips_.synchronize_route_presentation(
        u8_or(route_state.top_selection,
              command_overview_clips_.selection()),
        reload_confirmation,
        reload_confirmation && route_state.top_yes.value_or(false), trade,
        u8_or(route_state.top_command, 2U),
        u8_or(route_state.top_quantity, active_runner_record().ammo_clips),
        route_state.top_recipient.has_value() && *route_state.top_recipient >= 0
            ? static_cast<std::size_t>(*route_state.top_recipient)
            : 0U,
        message);
}

void FieldSceneRuntime::synchronize_command_overview_return_transition(
    const scripting::ScriptMenuRouteSnapshot& route_state) {
    if (route_state.field_return_transition_phase ==
        scripting::ScriptFieldReturnTransitionPhase::none) {
        return;
    }

    const auto was_active = command_overview_return_transition_active_;
    command_overview_return_transition_active_ =
        route_state.field_return_transition_active;
    command_overview_return_transition_elapsed_frames_ =
        route_state.field_return_transition_elapsed_frames;
    command_overview_return_palette_step_ =
        route_state.field_return_transition_palette_step;
    command_overview_return_transition_phase_ =
        route_state.field_return_transition_phase;

    if (command_overview_return_transition_phase_ ==
        scripting::ScriptFieldReturnTransitionPhase::field_fade_in) {
        field_palette_fade_steps_ =
            command_overview_return_palette_step_;
    } else {
        field_palette_fade_steps_ = 0U;
    }
    if (!command_overview_return_transition_active_ &&
        command_overview_return_transition_phase_ ==
            scripting::ScriptFieldReturnTransitionPhase::complete) {
        field_palette_fade_steps_ = 0U;
        command_overview_return_image_ = QImage{};
        field_entity_animation_.redraw_requested = true;
        if (was_active) {
            source_boundary("field.command_overview.return_fade_completed");
        }
    }
}

void FieldSceneRuntime::advance_command_overview_return_transition() {
    field_modal_return_blocked_inputs_ = static_cast<std::uint8_t>(
        field_modal_return_blocked_inputs_ & field_controller_port_byte_);
    ::shadowrun::ghidra_conversion::sample_controller_input(
        field_controller_sample_state_, field_controller_port_byte_);
    field_modal_return_blocked_inputs_ = static_cast<std::uint8_t>(
        field_modal_return_blocked_inputs_ |
        static_cast<std::uint8_t>(field_controller_sample_state_.held));
    field_controller_sample_state_.events = static_cast<std::uint16_t>(
        static_cast<std::uint8_t>(field_controller_sample_state_.held));
    field_controller_latch_state_ = ControllerLatchState{};

    game_scripts_.advance_menu_route(0U);
    if (!game_scripts_.failure().empty()) {
        failure_ = game_scripts_.failure();
        return;
    }
    synchronize_command_overview_return_transition(
        game_scripts_.menu_route_snapshot());
}

void FieldSceneRuntime::synchronize_command_overview_inventory_from_route(
    const scripting::ScriptMenuRouteSnapshot& route_state) {
    command_overview_inventory_detail_pending_ =
        route_state.root_inventory_detail_pending.value_or(false);
    command_overview_inventory_cursor_hidden_ =
        route_state.root_inventory_cursor_hidden.value_or(false);
    command_overview_inventory_picture_partial_ =
        route_state.root_inventory_picture_partial.value_or(false);
    command_overview_inventory_ammo_count_ =
        route_state.root_inventory_ammo_count;

    const auto action = static_cast<std::uint16_t>(
        srx::MenuScreen::ITEM_ACTION);
    const auto discard = static_cast<std::uint16_t>(
        srx::MenuScreen::DISCARD_CONFIRM);
    const auto trade = static_cast<std::uint16_t>(
        srx::MenuScreen::INVENTORY_TRADE);
    const auto first_aid = static_cast<std::uint16_t>(
        srx::MenuScreen::FIRST_AID);
    const auto message = static_cast<std::uint16_t>(
        srx::MenuScreen::MESSAGE);
    const bool inventory_page = route_state.page.has_value() &&
                                *route_state.page >= 0 &&
                                *route_state.page < 2;
    const bool item_message = route_state.top_screen == message &&
                              route_state.top_item_slot.has_value();
    const bool inventory_screen = route_state.top_screen == action ||
                                  route_state.top_screen == discard ||
                                  route_state.top_screen == trade ||
                                  route_state.top_screen == first_aid ||
                                  item_message;
    const bool routed = route_state.open && inventory_page &&
                        inventory_screen;
    const bool was_routed = command_overview_inventory_lua_route_active_;
    command_overview_inventory_lua_route_active_ = routed;
    if (!routed) {
        if (was_routed) {
            const bool source_wait_clears_selector =
                command_overview_first_aid_state_ !=
                    CommandOverviewFirstAidState::inactive ||
                command_overview_automatic_use_state_ !=
                    CommandOverviewAutomaticUseState::inactive;
            command_overview_inventory_action_ = InventoryActionMenuState{};
            command_overview_selected_inventory_item_.reset();
            command_overview_inventory_action_active_ = false;
            command_overview_discard_confirmation_active_ = false;
            command_overview_discard_confirmation_ = MenuConfirmationState{};
            command_overview_inventory_trade_ = InventoryTradeState{};
            command_overview_first_aid_state_ =
                CommandOverviewFirstAidState::inactive;
            command_overview_first_aid_targets_ = FirstAidTargetList{};
            command_overview_first_aid_cursor_ = 0U;
            command_overview_first_aid_message_ = FirstAidMessage::none;
            command_overview_automatic_use_state_ =
                CommandOverviewAutomaticUseState::inactive;
            command_overview_action_message_ =
                CommandOverviewActionMessage::none;
            // The First Aid and utility wait tails return through 0x11928,
            // which clears the yellow selector. Ordinary equipment, discard,
            // Trade, and no-recipient returns keep the inventory cursor state.
            if (source_wait_clears_selector) {
                command_overview_inventory_selector_suppressed_ = true;
            }
        }
        return;
    }

    command_overview_action_message_ = CommandOverviewActionMessage::none;
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    command_overview_first_aid_targets_ = FirstAidTargetList{};
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    command_overview_discard_confirmation_active_ = false;
    command_overview_inventory_trade_ = InventoryTradeState{};

    if (route_state.top_item_slot.has_value() &&
        route_state.top_item_code.has_value() &&
        *route_state.top_item_slot >= 0 &&
        *route_state.top_item_slot < 8 &&
        *route_state.top_item_code > 0 &&
        *route_state.top_item_code <= 0xFF) {
        command_overview_selected_inventory_item_ = SelectedInventoryItem{
            static_cast<std::size_t>(*route_state.top_item_slot),
            static_cast<std::uint8_t>(*route_state.top_item_code),
            static_cast<std::uint8_t>(std::clamp(
                route_state.top_item_phase.value_or(0), 0, 0xFF))};
    } else {
        command_overview_selected_inventory_item_.reset();
    }

    if (route_state.top_screen == action ||
        route_state.top_screen == discard) {
        command_overview_inventory_action_active_ = true;
        command_overview_inventory_action_.open = true;
        command_overview_inventory_action_.column = static_cast<std::uint8_t>(
            std::clamp(route_state.top_column.value_or(0), 0, 1));
        command_overview_inventory_action_.row = static_cast<std::uint8_t>(
            std::clamp(route_state.top_row.value_or(0), 0, 3));
        command_overview_inventory_action_.action_selector =
            static_cast<std::uint8_t>(std::clamp(
                route_state.top_selection.value_or(2), 2, 5));
    } else {
        command_overview_inventory_action_active_ = false;
    }

    if (route_state.top_screen == discard) {
        command_overview_discard_confirmation_active_ = true;
        command_overview_discard_confirmation_.selected_option =
            route_state.top_yes.value_or(false) ? 0U : 1U;
    }
    if (route_state.top_screen == trade) {
        auto& state = command_overview_inventory_trade_;
        state.open = true;
        if (command_overview_selected_inventory_item_.has_value()) {
            state.selected_item = *command_overview_selected_inventory_item_;
        }
        state.source_slot = static_cast<std::size_t>(std::clamp(
            route_state.top_source.value_or(
                static_cast<int>(field_party_roster_.active_slot())),
            0, static_cast<int>(FieldPartyRosterRuntime::kSlotCount - 1U)));
        state.recipient_slot = static_cast<std::size_t>(std::clamp(
            route_state.top_recipient.value_or(0), 0,
            static_cast<int>(FieldPartyRosterRuntime::kSlotCount - 1U)));
        state.recipient_inventory_slot = static_cast<std::size_t>(
            std::clamp(route_state.top_destination_slot.value_or(0), 0, 7));
        state.recipient_inventory_full =
            route_state.top_recipient_full.value_or(false);
        state.yes_selected = route_state.top_yes.value_or(false);
    }
    if (route_state.top_screen == first_aid) {
        command_overview_first_aid_state_ =
            CommandOverviewFirstAidState::target_selection;
        auto& targets = command_overview_first_aid_targets_;
        for (const auto slot : route_state.top_targets) {
            if (slot < 0 ||
                slot >= static_cast<int>(FieldPartyRosterRuntime::kSlotCount) ||
                targets.active_count >= targets.active_record_indices.size()) {
                continue;
            }
            targets.active_record_indices[targets.active_count++] =
                static_cast<std::size_t>(slot);
        }
        command_overview_first_aid_cursor_ = static_cast<std::size_t>(
            std::clamp(route_state.top_row.value_or(0), 0,
                       static_cast<int>(targets.cancel_row())));
    }
    if (route_state.top_screen == message &&
        route_state.top_message.has_value()) {
        switch (*route_state.top_message) {
        case 6:
            command_overview_action_message_ =
                CommandOverviewActionMessage::cannot_discard;
            break;
        case 7:
            command_overview_action_message_ =
                CommandOverviewActionMessage::cannot_trade;
            break;
        case 8:
            command_overview_automatic_use_state_ =
                CommandOverviewAutomaticUseState::result_wait;
            break;
        case 9:
        case 10:
        case 11: {
            command_overview_first_aid_state_ =
                CommandOverviewFirstAidState::result_wait;
            command_overview_first_aid_message_ =
                *route_state.top_message == 9
                    ? FirstAidMessage::medkit_used
                    : (*route_state.top_message == 10
                           ? FirstAidMessage::slap_patch_used
                           : FirstAidMessage::target_not_injured);
            for (const auto slot : route_state.top_targets) {
                if (slot < 0 ||
                    slot >= static_cast<int>(
                               FieldPartyRosterRuntime::kSlotCount) ||
                    command_overview_first_aid_targets_.active_count >=
                        command_overview_first_aid_targets_
                            .active_record_indices.size()) {
                    continue;
                }
                command_overview_first_aid_targets_
                    .active_record_indices[
                        command_overview_first_aid_targets_.active_count++] =
                    static_cast<std::size_t>(slot);
            }
            if (route_state.top_target.has_value()) {
                const auto target = std::find(
                    route_state.top_targets.begin(),
                    route_state.top_targets.end(), *route_state.top_target);
                if (target != route_state.top_targets.end()) {
                    command_overview_first_aid_cursor_ =
                        static_cast<std::size_t>(std::distance(
                            route_state.top_targets.begin(), target));
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void FieldSceneRuntime::start_command_menu_legacy_screen(
    const std::uint16_t screen,
    const scripting::ScriptMenuRouteArgs& args) {
    auto page = static_cast<int>(
        command_overview_.view().source_state.primary_page);
    auto row = static_cast<int>(
        command_overview_.view().source_state.secondary_page);
    if (args.page.has_value()) {
        page = *args.page;
    }
    if (args.column.has_value()) {
        page = *args.column;
    }
    if (args.row.has_value()) {
        row = *args.row;
    }
    if (page >= 0 && page < 7 && row >= 0 && row < 4) {
        command_overview_.sync_script_selection(
            static_cast<std::uint8_t>(page), static_cast<std::uint8_t>(row));
    }

    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::ITEM_ACTION)) {
        begin_command_overview_inventory_action();
    } else if (screen == static_cast<std::uint16_t>(srx::MenuScreen::CLIPS)) {
        begin_command_overview_clips();
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::ATTRIBUTES_SKILLS)) {
        begin_command_overview_attributes_skills();
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::CYBERWARE)) {
        begin_command_overview_cyberware();
    } else if (screen == static_cast<std::uint16_t>(srx::MenuScreen::MAGIC)) {
        begin_command_overview_magic();
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::POCKET_SECRETARY)) {
        begin_pocket_secretary();
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::NOTEBOOK)) {
        const auto event_gates = notebook_event_gates();
        if (event_gates.has_value()) {
            begin_pocket_secretary_notebook(
                pocket_secretary_, notebook_current_run_available_,
                *event_gates);
        } else {
            source_boundary(
                "field.pocket_secretary.notebook.event_flags_unresolved");
        }
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::GROUP_ITEMS)) {
        if (group_item_flags_.has_value()) {
            begin_pocket_secretary_group_items(
                pocket_secretary_, *group_item_flags_);
        } else {
            source_boundary(
                "field.pocket_secretary.group_items_flags_unresolved");
        }
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::CYBERDECK)) {
        if (cyberdeck_available_.has_value() &&
            !*cyberdeck_available_) {
            begin_pocket_secretary_cyberdeck_unavailable(
                pocket_secretary_);
        } else if (cyberdeck_available_.has_value()) {
            source_boundary("field.pocket_secretary.matrix_path_pending");
        } else {
            source_boundary(
                "field.pocket_secretary.cyberdeck_flag_unresolved");
        }
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::DISMISS_RUNNER)) {
        begin_pocket_secretary_dismiss_runner();
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::SAVE_LOAD)) {
        source_boundary("field.pocket_secretary.save_load_pending");
    } else if (screen == static_cast<std::uint16_t>(
                            srx::MenuScreen::DEBUG_MENU)) {
        source_boundary("field.pocket_secretary.debug_menu_route_missing");
    } else {
        source_boundary("field.command_overview.lua_legacy_route_unmapped");
    }
}

bool FieldSceneRuntime::command_menu_legacy_screen_active(
    const std::uint16_t screen) const {
    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::ITEM_ACTION)) {
        return command_overview_inventory_action_active_ ||
               command_overview_discard_confirmation_active_ ||
               command_overview_inventory_trade_.open ||
               command_overview_action_message_ !=
                   CommandOverviewActionMessage::none ||
               command_overview_first_aid_state_ !=
                   CommandOverviewFirstAidState::inactive ||
               command_overview_automatic_use_state_ !=
                   CommandOverviewAutomaticUseState::inactive;
    }
    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::CLIPS)) {
        return command_overview_clips_.is_open();
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::ATTRIBUTES_SKILLS)) {
        return command_overview_attributes_skills_.is_open();
    }
    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::CYBERWARE)) {
        return command_overview_cyberware_active_;
    }
    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::MAGIC)) {
        return command_overview_magic_state_ !=
               CommandOverviewMagicState::inactive;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::POCKET_SECRETARY)) {
        return pocket_secretary_.open || dismiss_runner_.open;
    }
    if (screen == static_cast<std::uint16_t>(srx::MenuScreen::NOTEBOOK)) {
        return pocket_secretary_.notebook.open;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::NOTEBOOK_CONTACTS)) {
        return pocket_secretary_.notebook.contacts.open;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::NOTEBOOK_SHADOWRUNNERS)) {
        return pocket_secretary_.notebook.shadowrunners.open;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::GROUP_ITEMS)) {
        return pocket_secretary_.group_items_wait;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::CYBERDECK)) {
        return pocket_secretary_.cyberdeck_unavailable_wait;
    }
    if (screen == static_cast<std::uint16_t>(
                      srx::MenuScreen::DISMISS_RUNNER)) {
        return dismiss_runner_.open;
    }
    return false;
}

void FieldSceneRuntime::open_command_overview(
    const std::uint16_t root_screen,
    scripting::ScriptMenuRouteStartArguments start_arguments) {
    if (!ready_ || command_overview_.open()) {
        return;
    }

    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_inventory_lua_route_active_ = false;
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
    if (root_screen == static_cast<std::uint16_t>(
                           srx::MenuScreen::CYBERWARE)) {
        // A direct CYBERWARE root has no parent COMMAND route to supply its
        // owning page. Seed the typed parent view from the ROM page identity
        // before the wait screen's legacy presenter is opened.
        command_overview_.sync_script_selection(4U, 0U);
    }
    field_matrix_route_active_ =
        root_screen == static_cast<std::uint16_t>(
                           srx::MenuScreen::MATRIX_ENTRY) ||
        root_screen == static_cast<std::uint16_t>(
                           srx::MenuScreen::MATRIX_SEARCH);
    field_taxi_route_active_ =
        root_screen == static_cast<std::uint16_t>(
                           srx::MenuScreen::TAXI_SELECTOR);
    matrix_return_to_field_on_close_ =
        field_matrix_route_active_ && start_arguments.return_to_field;
    matrix_field_return_transition_pending_ = false;
    menu_route_active_ = game_scripts_.open_menu_route(
        root_screen, std::move(start_arguments));
    if (!menu_route_active_) {
        field_matrix_route_active_ = false;
        field_taxi_route_active_ = false;
        matrix_return_to_field_on_close_ = false;
        source_boundary("field.command_overview.lua_route_unavailable");
        menu_route_screen_ = 0U;
    } else {
        const auto route_state = game_scripts_.menu_route_snapshot();
        menu_route_screen_ = route_state.top_screen;
        posture_dialog_active_ =
            menu_route_screen_ ==
            static_cast<std::uint16_t>(srx::MenuScreen::POSTURE);
        if (route_state.page.has_value() && route_state.row.has_value()) {
            command_overview_.sync_script_selection(
                static_cast<std::uint8_t>(*route_state.page),
                static_cast<std::uint8_t>(*route_state.row));
        }
        synchronize_command_overview_clips_from_route(route_state);
        synchronize_command_overview_inventory_from_route(route_state);
        synchronize_command_overview_magic_from_route(route_state);
        synchronize_pocket_secretary_notebook_from_route(route_state);
    }
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
    FieldHudAmmoMeterState ammo_meter{};
    ammo_meter.slot_word = record.record_type;
    ammo_meter.flags_byte = slot.flags_byte;
    ammo_meter.equipped_inventory_slot = record.equipped_inventory_slot;
    ammo_meter.source_type = record.equipped_item_code;
    ammo_meter.ammo = record.ammo_rounds;
    ammo_meter.clip_counts = record.inventory_phase_bytes;
    ammo_meter.clip_aux = record.inventory_item_codes;
    // The displayed counter is the 0x36FC D2 result, not always the live
    // +0x4E value. In particular, an unequipped record keeps its ammo
    // percentage in +0x4E while the ROM draws the zero counter beside the
    // unarmed/cyberware icon.
    slot.ammo_display_value =
        advance_field_hud_ammo_meter(ammo_meter, 0).displayed;
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
    // ROM 0xC2A6 normally uses FFF101 to choose four or five labeled rows.
    // The temporary test override exposes its sixth, unlabeled debug row.
    initialize_pocket_secretary_menu(
        pocket_secretary_, debug_menu_unlocked_,
        {[this] { source_boundary("field.pocket_secretary.transfer_menu_assets"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_grid"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_text"); },
         [this] { source_boundary("field.pocket_secretary.render_menu_numbers"); },
         {}, {}});
    dismiss_runner_ = DismissRunnerMenuState{};
    dismiss_runner_random_grid_effect_ = RandomGridEffectState{};
    dismiss_runner_effect_frames_ = 0U;
    dismiss_runner_random_grid_cells_.fill(-1);
    field_matrix_route_active_ = false;
    matrix_return_to_field_on_close_ = false;
    matrix_field_return_transition_pending_ = false;
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
    // Dismiss Runner redraws only the inner panel. E660's lower-bezel cells
    // remain installed until the Pocket Secretary panel is rebuilt.
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
    // Entering this top-level child replaces the shared command button row.
    // Drop any stale inventory-action presentation so it cannot be layered
    // over Clips if a prior menu route left that state behind.
    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_discard_confirmation_active_ = false;
    command_overview_inventory_trade_ = InventoryTradeState{};
    command_overview_action_message_ = CommandOverviewActionMessage::none;
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
        const auto* table = field_magic_spell_table(
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

    const auto* table = field_magic_spell_table(
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
    const auto capacity = weapon_magazine_capacity_for_item(
        active_runner_record().equipped_item_code).value_or(0U);
    const auto event = command_overview_clips_.advance(
        controller_events, field_party_roster_.records(),
        field_party_roster_.active_slot(), capacity);
    switch (event) {
    case ClipsMenuEvent::selection_moved:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        source_boundary("field.command_overview.clips.selector_moved");
        break;
    case ClipsMenuEvent::confirmation_opened:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        source_boundary(
            "field.command_overview.clips.reload_confirmation_opened");
        break;
    case ClipsMenuEvent::confirmation_toggled:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        source_boundary(
            command_overview_clips_.reload_confirmation_yes()
                ? "field.command_overview.clips.reload_confirmation_yes"
                : "field.command_overview.clips.reload_confirmation_no");
        break;
    case ClipsMenuEvent::reloaded:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.reloaded");
        break;
    case ClipsMenuEvent::trade_opened:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        source_boundary(
            "field.command_overview.clips.trade.selector_moved");
        break;
    case ClipsMenuEvent::trade_quantity_changed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        source_boundary(
            "field.command_overview.clips.trade.quantity_changed");
        break;
    case ClipsMenuEvent::trade_recipient_changed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        source_boundary(
            "field.command_overview.clips.trade.recipient_changed");
        break;
    case ClipsMenuEvent::trade_transferred:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary(
            "field.command_overview.clips.trade.transferred");
        break;
    case ClipsMenuEvent::trade_closed:
        if (source_audio_event_callback_ &&
            (controller_events & 0x7000U) != 0U) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        source_boundary("field.command_overview.clips.trade.closed");
        break;
    case ClipsMenuEvent::distributed:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        source_boundary("field.command_overview.clips.distributed");
        break;
    case ClipsMenuEvent::message_shown:
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            const auto item_factor = [this](const std::uint8_t item_code) {
                return weapon_magazine_capacity_for_item(item_code)
                    .value_or(0U);
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
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
        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
    } else if (confirm && source_audio_event_callback_) {
        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
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
        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
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


} // namespace shadowrun::ghidra_conversion::qt_runtime

#include "field_command_overview_bridge.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

void FieldCommandOverviewBridge::begin(
    const CommandOverviewInput& input,
    const CommandOverviewPreparationInput& preparation_input,
    const CommandOverviewHudPanelInput& hud_panel_input,
    FieldCommandOverviewCallbacks callbacks) {
    input_ = input;
    callbacks_ = std::move(callbacks);
    state_ = CommandOverviewState{};
    state_.primary_page = 6U;
    state_.secondary_page = 0U;
    state_.action_marker = -1;
    state_.exit_marker = 0;
    state_.panel_marker = 0;
    state_.graphics_marker = 0;
    state_.resource_variant = 0;
    state_.refresh_marker = 0;
    state_.action_dialog_gate = 0;
    view_ = FieldCommandOverviewView{};
    view_.open = true;
    missing_asset_reported_ = false;

    // Keep the converted preparation routine in the call path. Its source
    // tile-table aliases are checked here, while the Qt presenter consumes
    // the named direct background and sprite files below.
    view_.preparation = prepare_command_overview(
        preparation_input,
        {[this] {},
         [this](const CommandOverviewGridPlan& plan) {
             mark_missing_source_asset(plan.asset_alias);
         },
         {},
         [this](const CommandOverviewResourceUploadPlan& plan) {
             mark_missing_source_asset(plan.asset_alias);
         },
         [this](const CommandOverviewBlockTransferPlan& plan) {
             mark_missing_source_asset(plan.asset_alias);
         }});
    state_.primary_page = view_.preparation.primary_page;
    state_.secondary_page = view_.preparation.secondary_page;
    state_.resource_variant = static_cast<std::int8_t>(
        view_.preparation.resource_variant ==
                CommandOverviewResourceVariant::alternate
            ? 1
            : 0);
    if (callbacks_.preparation_complete) {
        callbacks_.preparation_complete(view_.preparation);
    }

    draw_command_overview_hud_panel(
        hud_panel_input, [this](const CommandOverviewHudPanelPlan& plan) {
            view_.hud_panel = plan;
            view_.hud_panel_ready = true;
            if (callbacks_.hud_panel_prepared) {
                callbacks_.hud_panel_prepared(plan);
            }
        });

    refresh_view();
}

bool FieldCommandOverviewBridge::asset_available(
    const std::string_view alias) const {
    return !callbacks_.asset_available || callbacks_.asset_available(alias);
}

void FieldCommandOverviewBridge::source_boundary(
    const std::string_view boundary) const {
    if (callbacks_.source_boundary) {
        callbacks_.source_boundary(boundary);
    }
}

void FieldCommandOverviewBridge::mark_missing_source_asset(
    const std::string_view alias) {
    if (asset_available(alias)) {
        return;
    }
    view_.source_tile_data_pending = true;
    if (!missing_asset_reported_) {
        missing_asset_reported_ = true;
        source_boundary("field.command_overview.source_tile_data_pending");
    }
}

void FieldCommandOverviewBridge::refresh_view() {
    view_.source_state = state_;
    view_.selected_title_text =
        select_command_overview_title_text(state_.primary_page);
    view_.selected_title_text_available =
        view_.selected_title_text.has_value() &&
        asset_available(view_.selected_title_text->source_alias);
    if (view_.selected_title_text.has_value() &&
        !view_.selected_title_text_available) {
        mark_missing_source_asset(view_.selected_title_text->source_alias);
    }

    CommandOverviewActionRefreshState action_state{
        state_.primary_page, state_.secondary_page, 1U, false, 0U};
    CommandOverviewActionRefreshCallbacks action_callbacks;
    action_callbacks.build_title_sprite_records =
        [this](const CommandOverviewSpriteBuildPlan& plan) {
            if (!asset_available(plan.source_alias)) {
                mark_missing_source_asset(plan.source_alias);
                return std::size_t{0};
            }
            // The extracted cursor is one host sprite record even though the
            // source builder emits several SAT words around it.
            return std::size_t{1};
        };
    action_callbacks.clear_last_sprite_sequence = [] {};
    action_callbacks.prepare_menu_grid_variant_b = [] {};
    action_callbacks.read_entity_action_code = [this] {
        return callbacks_.read_active_action_code
                   ? callbacks_.read_active_action_code()
                   : 0U;
    };
    action_callbacks.render_item_action_details = [this] {
        view_.action_dispatch_pending = true;
        source_boundary("field.command_overview.item_details_pending");
    };
    action_callbacks.service_entity_action_menu = [this] {
        view_.action_dispatch_pending = true;
        source_boundary("field.command_overview.entity_action_pending");
    };
    action_callbacks.prepare_menu_dialog = [this] {
        view_.action_dialog_pending = true;
        source_boundary("field.command_overview.dialog_pending");
    };
    action_callbacks.render_action_title_text =
        [this](const CommandOverviewActionTitleTextPlan& plan) {
            mark_missing_source_asset(plan.source_table);
        };
    view_.action_refresh = refresh_command_overview_action_details(
        action_state, action_callbacks);
    if (callbacks_.action_refresh_complete && view_.action_refresh.has_value()) {
        callbacks_.action_refresh_complete(*view_.action_refresh);
    }
}

void FieldCommandOverviewBridge::advance_primary(const std::int8_t delta) {
    CommandOverviewPrimaryPageState primary{
        state_.primary_page, state_.panel_marker};
    advance_command_overview_primary_page(
        primary,
        {[this, delta] {
             source_boundary("field.command_overview.upload_cursor_reset");
             return delta;
         },
         [this] {
             source_boundary("field.command_overview.vdp_upload_reset");
         },
         [this] {
             source_boundary("field.command_overview.tile_upload");
         },
         [] {}});
    state_.primary_page = primary.page;
    state_.panel_marker = primary.auxiliary_marker;
    refresh_view();
}

void FieldCommandOverviewBridge::advance_secondary(
    const std::int8_t delta, const std::uint16_t controller_events) {
    CommandOverviewSecondaryPageState secondary{
        state_.primary_page, state_.secondary_page, state_.action_dialog_gate,
        controller_events};
    advance_command_overview_secondary_page(
        secondary,
        {[this, delta] {
             source_boundary("field.command_overview.secondary_upload_reset");
             return delta;
         },
         [this] {
             view_.action_dialog_pending = true;
             source_boundary("field.command_overview.dialog_pending");
         },
         [this] {
             source_boundary("field.command_overview.secondary_tile_upload");
         },
         [] {}});
    state_.secondary_page = secondary.secondary_page;
    state_.action_dialog_gate = secondary.action_dialog_gate;
    refresh_view();
}

void FieldCommandOverviewBridge::advance(
    const std::uint16_t controller_events) {
    if (!view_.open) {
        return;
    }

    // Preserve the source priority in 0x10DD8..0x10F2A. In particular, a
    // d-pad edge wins over A/B and Start when a host frame contains both.
    if ((controller_events & 0x0400U) != 0U) {
        advance_primary(-1);
        return;
    }
    if ((controller_events & 0x0100U) != 0U) {
        advance_secondary(-1, controller_events);
        return;
    }
    if ((controller_events & 0x0800U) != 0U) {
        advance_primary(1);
        return;
    }
    if ((controller_events & 0x0200U) != 0U) {
        advance_secondary(1, controller_events);
        return;
    }
    if ((controller_events & 0x4000U) != 0U ||
        (controller_events & 0x1000U) != 0U) {
        view_.action_dispatch_pending = true;
        source_boundary("field.command_overview.action_dispatch_pending");
        return;
    }
    if ((controller_events & 0x2000U) != 0U) {
        source_boundary("field.command_overview.mode_action_pending");
        state_.primary_page = 6U;
        state_.secondary_page = 0U;
        state_.action_dialog_gate = -1;
        refresh_view();
        return;
    }
    if ((controller_events & 0x8000U) != 0U) {
        close();
    }
}

void FieldCommandOverviewBridge::close() noexcept {
    view_.open = false;
    state_.action_marker = 0;
    state_.exit_marker = 0;
    view_.source_state = state_;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

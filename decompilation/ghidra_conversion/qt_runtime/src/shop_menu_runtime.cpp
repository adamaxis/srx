#include "shop_menu_runtime.hpp"

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_selection_navigation_conversion.hpp"

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::uint16_t kActionCursorDecrement = 0x0400U;
constexpr std::uint16_t kActionPanelDecrement = 0x0100U;
constexpr std::uint16_t kActionCursorIncrement = 0x0800U;
constexpr std::uint16_t kActionPanelIncrement = 0x0200U;
constexpr std::uint16_t kRestartSetup = 0x2000U;
constexpr std::uint16_t kConfirm = 0x4000U;
constexpr std::uint16_t kAlternateConfirm = 0x1000U;
constexpr std::uint16_t kReturnToField = 0x8000U;

constexpr std::uint8_t kAresMenuMode = 1U;
constexpr std::uint8_t kUnavailable = 0xFFU;

using OfferGroup = std::array<FieldActionCostRecord, 8>;

// US ROM records at 0x1AAC9E, 0x1AADD7, and 0x1AAD8F. Source action indices
// 0, 1, and 2 select weapons, other items, and armor respectively at 0x56DC4.
constexpr std::array<OfferGroup, 3> kAresOfferGroups{{
    {{{0x01U, 0x00U, 0x00U}, {0x02U, 0x00U, 0x00U},
      {0x06U, 0x00U, 0x0FU}, {0x04U, 0x00U, 0x00U},
      {0x05U, 0x00U, 0x0DU}, {0x0DU, 0x06U, 0x14U},
      {0x0EU, 0x06U, 0x00U},
      {kUnavailable, kUnavailable, kUnavailable}}},
    {{{0x45U, 0x00U, 0x19U}, {0x0FU, 0x06U, 0x00U},
      {0x10U, 0x04U, 0x07U}, {0x11U, 0x04U, 0x07U},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable}}},
    {{{0x2EU, 0x00U, 0x00U}, {0x2DU, 0x00U, 0x0CU},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable},
      {kUnavailable, kUnavailable, kUnavailable}}},
}};

bool is_action_confirmed(const std::uint16_t events) {
    return (events & (kConfirm | kAlternateConfirm)) != 0U;
}

} // namespace

bool ShopMenuController::open(const ShopId shop) noexcept {
    if (shop != ShopId::ares_weapon_emporium) {
        return false;
    }
    shop_ = shop;
    menu_mode_ = kAresMenuMode;
    source_state_ = FieldModeServiceState{};
    source_state_.action_selection.menu_result = -1;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel = 0;
    source_state_.action_selection.selected_action = 0;
    source_state_.selection_marker = -1;
    pending_action_.reset();
    open_ = true;
    return true;
}

void ShopMenuController::close() noexcept {
    open_ = false;
    pending_action_.reset();
    source_state_ = FieldModeServiceState{};
    menu_mode_ = 0U;
}

void ShopMenuController::finish_action() noexcept {
    if (!open_) {
        return;
    }
    pending_action_.reset();
    source_state_.service_result = 0;
    source_state_.selection_marker = -1;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel = 0;
    source_state_.action_selection.selected_action = 0;
}

void ShopMenuController::publish_selected_group() noexcept {
    source_state_.action_selection.selected_action =
        source_state_.action_selection.action_cursor;
}

ShopActionGroup ShopMenuController::selected_group() const noexcept {
    const auto group = source_state_.action_selection.selected_action;
    if (group < 0 || group > 5) {
        return ShopActionGroup::source_action_3;
    }
    return static_cast<ShopActionGroup>(group);
}

std::size_t ShopMenuController::offer_count(
    const ShopId shop, const ShopActionGroup group) noexcept {
    if (shop != ShopId::ares_weapon_emporium ||
        group < ShopActionGroup::weapons || group > ShopActionGroup::armor) {
        return 0U;
    }
    const auto& rows = kAresOfferGroups[static_cast<std::size_t>(group)];
    std::size_t count = 0U;
    while (count < rows.size() && rows[count].type != kUnavailable) {
        ++count;
    }
    return count;
}

std::optional<FieldActionCostRecord> ShopMenuController::offer(
    const ShopId shop, const ShopActionGroup group,
    const std::size_t index) noexcept {
    if (index >= offer_count(shop, group)) {
        return std::nullopt;
    }
    return kAresOfferGroups[static_cast<std::size_t>(group)][index];
}

ShopMenuEvent ShopMenuController::advance(
    const std::uint16_t controller_events) {
    if (!open_ || pending_action_.has_value()) {
        return ShopMenuEvent::no_change;
    }

    auto& selection = source_state_.action_selection;
    FieldActionSelectionNavigationCallbacks navigation_callbacks{};
    if ((controller_events & kActionCursorDecrement) != 0U) {
        const auto before = selection.action_cursor;
        advance_field_action_selection_cursor(selection, -1,
                                              navigation_callbacks);
        if (selection.action_cursor != before) {
            if (selection.action_panel == 0) {
                publish_selected_group();
            }
            return ShopMenuEvent::selection_moved;
        }
        return ShopMenuEvent::no_change;
    }
    if ((controller_events & kActionPanelDecrement) != 0U) {
        const auto before = selection.action_panel;
        advance_field_action_selection_panel(selection, -1,
                                             navigation_callbacks);
        if (selection.action_panel != before) {
            publish_selected_group();
            return ShopMenuEvent::panel_changed;
        }
        return ShopMenuEvent::no_change;
    }
    if ((controller_events & kActionCursorIncrement) != 0U) {
        const auto before = selection.action_cursor;
        advance_field_action_selection_cursor(selection, 1,
                                              navigation_callbacks);
        if (selection.action_cursor != before) {
            if (selection.action_panel == 0) {
                publish_selected_group();
            }
            return ShopMenuEvent::selection_moved;
        }
        return ShopMenuEvent::no_change;
    }
    if ((controller_events & kActionPanelIncrement) != 0U) {
        const auto before = selection.action_panel;
        const auto selected_group = selection.action_cursor;
        advance_field_action_selection_panel(selection, 1,
                                             navigation_callbacks);
        if (selection.action_panel != before) {
            source_state_.action_selection.selected_action = selected_group;
            return ShopMenuEvent::panel_changed;
        }
        return ShopMenuEvent::no_change;
    }

    if (is_action_confirmed(controller_events)) {
        if (selection.action_panel == 0) {
            publish_selected_group();
            return ShopMenuEvent::no_change;
        }

        ShopActionRequest request{};
        request.group = selected_group();
        request.cursor = static_cast<std::uint8_t>(selection.action_cursor);
        if (request.group >= ShopActionGroup::weapons &&
            request.group <= ShopActionGroup::armor) {
            request.offer = offer(shop_, request.group, request.cursor);
            if (!request.offer.has_value()) {
                return ShopMenuEvent::offer_unavailable;
            }
            pending_action_ = request;
            return ShopMenuEvent::purchase_selected;
        }
        if (request.group == ShopActionGroup::sell) {
            pending_action_ = request;
            return ShopMenuEvent::sale_selected;
        }
        pending_action_ = request;
        return ShopMenuEvent::source_action_boundary;
    }

    if ((controller_events & kRestartSetup) != 0U) {
        finish_action();
        return ShopMenuEvent::active_runner_change_requested;
    }
    if ((controller_events & kReturnToField) != 0U) {
        close();
        return ShopMenuEvent::returned_to_field;
    }
    return ShopMenuEvent::no_change;
}

FieldActionApplicationResult ShopTransactionService::purchase(
    const FieldActionCostRecord& offer,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& state,
    const bool confirmed) const {
    FieldActionApplicationCallbacks callbacks{};
    callbacks.confirm_purchase = [confirmed] { return confirmed; };
    return apply_selected_field_action(offer, cost_context, state,
                                       us_item_catalog(), callbacks);
}

ShopSaleOutcome ShopTransactionService::sell(
    const FieldActionCostContext& cost_context,
    RunnerActionRemovalState& state,
    const bool confirmed) const {
    ShopSaleOutcome outcome{};
    if (state.runner_slot >= state.runner.action_codes.size()) {
        outcome.result = ShopSaleOutcome::Result::empty_slot;
        return outcome;
    }

    const auto item_code = state.runner.action_codes[state.runner_slot];
    if (item_code == 0U || item_code == 0x29U || item_code == 0x2AU ||
        item_code == 0x2BU) {
        RunnerActionRemovalCallbacks callbacks{};
        callbacks.confirm_removal = [confirmed] { return confirmed; };
        switch (remove_runner_action(state, callbacks)) {
        case RunnerActionRemovalResult::removed:
            outcome.result = ShopSaleOutcome::Result::removed;
            break;
        case RunnerActionRemovalResult::empty_slot:
            outcome.result = ShopSaleOutcome::Result::empty_slot;
            break;
        case RunnerActionRemovalResult::protected_action:
            outcome.result = ShopSaleOutcome::Result::protected_action;
            break;
        case RunnerActionRemovalResult::cancelled:
            outcome.result = ShopSaleOutcome::Result::cancelled;
            break;
        }
        return outcome;
    }

    const auto quote = quote_sale(item_code, cost_context);
    if (!quote.has_value()) {
        outcome.result = ShopSaleOutcome::Result::missing_item_record;
        return outcome;
    }
    outcome.quoted_refund = *quote;
    state.refund = *quote;

    RunnerActionRemovalCallbacks callbacks{};
    callbacks.confirm_removal = [confirmed] { return confirmed; };
    switch (remove_runner_action(state, callbacks)) {
    case RunnerActionRemovalResult::removed:
        outcome.result = ShopSaleOutcome::Result::removed;
        break;
    case RunnerActionRemovalResult::empty_slot:
        outcome.result = ShopSaleOutcome::Result::empty_slot;
        break;
    case RunnerActionRemovalResult::protected_action:
        outcome.result = ShopSaleOutcome::Result::protected_action;
        break;
    case RunnerActionRemovalResult::cancelled:
        outcome.result = ShopSaleOutcome::Result::cancelled;
        break;
    }
    return outcome;
}

std::optional<std::uint32_t> ShopTransactionService::quote_sale(
    const std::uint8_t item_code,
    const FieldActionCostContext& cost_context) noexcept {
    const auto* item = find_us_item_definition(item_code);
    if (item == nullptr) {
        return std::nullopt;
    }
    const auto half_price = static_cast<std::uint32_t>(item->base_cost / 2U);
    return apply_runner_cost_surcharge(half_price, cost_context);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

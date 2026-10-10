#include "shop_menu_runtime.hpp"

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_selection_navigation_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <utility>

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

std::string_view group_id(const ShopActionGroup group) {
    switch (group) {
    case ShopActionGroup::weapons:
        return "weapons";
    case ShopActionGroup::other_items:
        return "other_items";
    case ShopActionGroup::armor:
        return "armor";
    case ShopActionGroup::source_action_3:
        return "source_action_3";
    case ShopActionGroup::source_action_4:
        return "source_action_4";
    case ShopActionGroup::sell:
        return "sell";
    }
    return {};
}

std::optional<ShopActionGroup> group_from_id(const std::string_view id) {
    for (std::int8_t index = 0; index <= 5; ++index) {
        const auto group = static_cast<ShopActionGroup>(index);
        if (group_id(group) == id) {
            return group;
        }
    }
    return std::nullopt;
}

bool is_action_confirmed(const std::uint16_t events) {
    return (events & (kConfirm | kAlternateConfirm)) != 0U;
}

} // namespace

bool ShopMenuController::open(
    const ShopId shop,
    const scripting::ScriptShopDefinition& definition,
    const std::array<std::string, 2>& confirmation_choice_ids) {
    if (shop != ShopId::ares_weapon_emporium ||
        definition.id != "ares_weapon_emporium" ||
        definition.groups.size() != group_ids_.size() ||
        confirmation_choice_ids[0] != "yes" ||
        confirmation_choice_ids[1] != "no") {
        return false;
    }

    std::array<std::string, 6> group_ids{};
    std::array<std::vector<FieldActionCostRecord>, 6> offer_groups{};
    for (std::size_t index = 0; index < definition.groups.size(); ++index) {
        const auto& scripted_group = definition.groups[index];
        const auto previous = std::find(
            group_ids.begin(), group_ids.begin() + index,
            scripted_group.id);
        if (!group_from_id(scripted_group.id).has_value() ||
            previous != group_ids.begin() + index ||
            scripted_group.offers.size() > 8U) {
            return false;
        }
        group_ids[index] = scripted_group.id;
        auto& offers = offer_groups[index];
        offers.reserve(scripted_group.offers.size());
        for (const auto& offer : scripted_group.offers) {
            offers.push_back(FieldActionCostRecord{
                offer.type, offer.value, offer.quantity});
        }
    }
    shop_ = shop;
    group_ids_ = std::move(group_ids);
    confirmation_choice_ids_ = confirmation_choice_ids;
    offer_groups_ = std::move(offer_groups);
    menu_mode_ = kAresMenuMode;
    source_state_ = FieldModeServiceState{};
    source_state_.action_selection.menu_result = -1;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel = 0;
    source_state_.action_selection.selected_action = 0;
    source_state_.selection_marker = -1;
    pending_action_.reset();
    reset_transaction();
    open_ = true;
    return true;
}

void ShopMenuController::close() noexcept {
    open_ = false;
    pending_action_.reset();
    reset_transaction();
    source_state_ = FieldModeServiceState{};
    group_ids_ = {};
    confirmation_choice_ids_ = {};
    offer_groups_ = {};
    menu_mode_ = 0U;
}

void ShopMenuController::finish_action() noexcept {
    if (!open_) {
        return;
    }
    pending_action_.reset();
    reset_transaction();
    source_state_.service_result = 0;
    source_state_.selection_marker = -1;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel = 0;
    source_state_.action_selection.selected_action = 0;
}

void ShopMenuController::clear_pending_action() noexcept {
    pending_action_.reset();
}

void ShopMenuController::reset_transaction() noexcept {
    transaction_confirmation_ = MenuConfirmationState{};
    transaction_kind_ = TransactionKind::none;
    transaction_message_ = ShopTransactionMessage::none;
    transaction_amount_ = 0U;
    transaction_confirmation_open_ = false;
    transaction_waiting_for_event_ = false;
}

ShopTransactionEvent ShopMenuController::begin_purchase(
    const FieldActionCostRecord& selected_offer,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& state,
    const FieldActionBaseCostLookup& base_cost_lookup) {
    reset_transaction();
    transaction_kind_ = TransactionKind::purchase;
    last_purchase_result_ = ShopTransactionService{}.purchase(
        selected_offer, cost_context, state, false, base_cost_lookup);
    if (last_purchase_result_ == FieldActionApplicationResult::cancelled) {
        transaction_amount_ =
            selected_offer.type == static_cast<std::uint8_t>('E')
                ? apply_field_action_cost_discounts(
                      selected_offer.quantity, cost_context)
                : base_cost_lookup
                      ? calculate_field_action_record_cost(
                            selected_offer, cost_context, base_cost_lookup)
                      : calculate_field_action_record_cost(
                            selected_offer, cost_context, us_item_catalog());
        transaction_confirmation_ = MenuConfirmationState{};
        transaction_confirmation_open_ = true;
        return ShopTransactionEvent::confirmation_opened;
    }
    if (last_purchase_result_ == FieldActionApplicationResult::no_inventory_slot) {
        transaction_message_ = ShopTransactionMessage::no_inventory_slot;
    } else if (last_purchase_result_ ==
               FieldActionApplicationResult::insufficient_funds) {
        transaction_message_ = ShopTransactionMessage::insufficient_funds;
    } else {
        transaction_kind_ = TransactionKind::none;
        clear_pending_action();
        return ShopTransactionEvent::unresolved;
    }
    transaction_waiting_for_event_ = true;
    return ShopTransactionEvent::message_shown;
}

ShopTransactionEvent ShopMenuController::begin_sale(
    const FieldActionCostContext& cost_context,
    RunnerActionRemovalState& state,
    const FieldActionBaseCostLookup& base_cost_lookup) {
    reset_transaction();
    transaction_kind_ = TransactionKind::sale;
    last_sale_outcome_ = ShopTransactionService{}.sell(
        cost_context, state, false, base_cost_lookup);
    if (last_sale_outcome_.result == ShopSaleOutcome::Result::cancelled) {
        transaction_amount_ = last_sale_outcome_.quoted_refund;
        transaction_confirmation_ = MenuConfirmationState{};
        transaction_confirmation_open_ = true;
        return ShopTransactionEvent::confirmation_opened;
    }
    if (last_sale_outcome_.result ==
        ShopSaleOutcome::Result::protected_action) {
        transaction_message_ = ShopTransactionMessage::protected_sale;
        transaction_waiting_for_event_ = true;
        return ShopTransactionEvent::message_shown;
    }
    transaction_kind_ = TransactionKind::none;
    clear_pending_action();
    return last_sale_outcome_.result == ShopSaleOutcome::Result::empty_slot
               ? ShopTransactionEvent::action_ignored
               : ShopTransactionEvent::unresolved;
}

ShopTransactionEvent ShopMenuController::advance_transaction(
    const std::uint16_t controller_events,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& purchase_state,
    RunnerActionRemovalState& sale_state,
    const FieldActionBaseCostLookup& base_cost_lookup) {
    if (transaction_waiting_for_event_) {
        if (controller_events == 0U) {
            return ShopTransactionEvent::no_change;
        }
        reset_transaction();
        clear_pending_action();
        return ShopTransactionEvent::message_dismissed;
    }
    if (!transaction_confirmation_open_) {
        return ShopTransactionEvent::no_change;
    }

    const auto confirmation_step = advance_menu_confirmation_prompt(
        transaction_confirmation_, controller_events);
    if (confirmation_step == MenuConfirmationStep::option_moved) {
        return ShopTransactionEvent::confirmation_moved;
    }
    if (confirmation_step != MenuConfirmationStep::closed) {
        return ShopTransactionEvent::no_change;
    }
    transaction_confirmation_open_ = false;
    if (transaction_confirmation_.selected_option >=
        confirmation_choice_ids_.size()) {
        clear_pending_action();
        reset_transaction();
        return ShopTransactionEvent::unresolved;
    }
    const auto& choice_id = confirmation_choice_ids_[
        transaction_confirmation_.selected_option];
    if (choice_id == "no") {
        last_purchase_result_ = FieldActionApplicationResult::cancelled;
        last_sale_outcome_.result = ShopSaleOutcome::Result::cancelled;
        clear_pending_action();
        reset_transaction();
        return ShopTransactionEvent::cancelled;
    }
    if (choice_id != "yes") {
        clear_pending_action();
        reset_transaction();
        return ShopTransactionEvent::unresolved;
    }

    if (transaction_kind_ == TransactionKind::purchase && pending_action_ &&
        pending_action_->offer.has_value()) {
        last_purchase_result_ = ShopTransactionService{}.purchase(
            *pending_action_->offer, cost_context, purchase_state, true,
            base_cost_lookup);
        if (last_purchase_result_ == FieldActionApplicationResult::applied) {
            clear_pending_action();
            reset_transaction();
            return ShopTransactionEvent::purchase_applied;
        }
        if (last_purchase_result_ ==
            FieldActionApplicationResult::special_action_limit) {
            transaction_message_ =
                ShopTransactionMessage::special_action_limit;
        } else if (last_purchase_result_ ==
                   FieldActionApplicationResult::no_inventory_slot) {
            transaction_message_ = ShopTransactionMessage::no_inventory_slot;
        } else if (last_purchase_result_ ==
                   FieldActionApplicationResult::insufficient_funds) {
            transaction_message_ = ShopTransactionMessage::insufficient_funds;
        } else {
            clear_pending_action();
            reset_transaction();
            return ShopTransactionEvent::unresolved;
        }
        transaction_kind_ = TransactionKind::none;
        transaction_waiting_for_event_ = true;
        return ShopTransactionEvent::message_shown;
    }

    if (transaction_kind_ == TransactionKind::sale && pending_action_) {
        last_sale_outcome_ = ShopTransactionService{}.sell(
            cost_context, sale_state, true, base_cost_lookup);
        if (last_sale_outcome_.result == ShopSaleOutcome::Result::removed) {
            clear_pending_action();
            reset_transaction();
            return ShopTransactionEvent::sale_applied;
        }
        if (last_sale_outcome_.result ==
            ShopSaleOutcome::Result::protected_action) {
            transaction_message_ = ShopTransactionMessage::protected_sale;
            transaction_kind_ = TransactionKind::none;
            transaction_waiting_for_event_ = true;
            return ShopTransactionEvent::message_shown;
        }
    }
    clear_pending_action();
    reset_transaction();
    return ShopTransactionEvent::unresolved;
}

void ShopMenuController::publish_selected_group() noexcept {
    source_state_.action_selection.selected_action =
        source_state_.action_selection.action_cursor;
}

ShopActionGroup ShopMenuController::selected_group() const noexcept {
    const auto group = source_state_.action_selection.selected_action;
    if (group < 0 || static_cast<std::size_t>(group) >= group_ids_.size()) {
        return ShopActionGroup::source_action_3;
    }
    const auto selected = group_from_id(
        group_ids_[static_cast<std::size_t>(group)]);
    return selected.value_or(ShopActionGroup::source_action_3);
}

std::size_t ShopMenuController::offer_count(
    const ShopActionGroup group) const noexcept {
    if (!open_ || group < ShopActionGroup::weapons ||
        group > ShopActionGroup::armor) {
        return 0U;
    }
    const auto id = group_id(group);
    const auto found = std::find(group_ids_.begin(), group_ids_.end(), id);
    if (found == group_ids_.end()) {
        return 0U;
    }
    const auto group_index = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return offer_groups_[group_index].size();
}

std::optional<FieldActionCostRecord> ShopMenuController::offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= offer_count(group)) {
        return std::nullopt;
    }
    const auto id = group_id(group);
    const auto found = std::find(group_ids_.begin(), group_ids_.end(), id);
    if (found == group_ids_.end()) {
        return std::nullopt;
    }
    const auto group_index = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return offer_groups_[group_index][index];
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
            request.offer = offer(request.group, request.cursor);
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
    const bool confirmed,
    const FieldActionBaseCostLookup& base_cost_lookup) const {
    FieldActionApplicationCallbacks callbacks{};
    callbacks.confirm_purchase = [confirmed] { return confirmed; };
    return base_cost_lookup
               ? apply_selected_field_action(offer, cost_context, state,
                                             base_cost_lookup, callbacks)
               : apply_selected_field_action(offer, cost_context, state,
                                             us_item_catalog(), callbacks);
}

ShopSaleOutcome ShopTransactionService::sell(
    const FieldActionCostContext& cost_context,
    RunnerActionRemovalState& state,
    const bool confirmed,
    const FieldActionBaseCostLookup& base_cost_lookup) const {
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

    const auto quote = quote_sale(item_code, cost_context, base_cost_lookup);
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
    const FieldActionCostContext& cost_context,
    const FieldActionBaseCostLookup& base_cost_lookup) {
    if (item_code == 0U || item_code > 50U) {
        return std::nullopt;
    }
    std::uint32_t base_cost = 0U;
    if (base_cost_lookup) {
        base_cost = base_cost_lookup(
            static_cast<std::uint8_t>(item_code - 1U));
    } else {
        const auto* item = find_us_item_definition(item_code);
        if (item == nullptr) {
            return std::nullopt;
        }
        base_cost = item->base_cost;
    }
    const auto half_price = base_cost / 2U;
    return apply_runner_cost_surcharge(half_price, cost_context);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

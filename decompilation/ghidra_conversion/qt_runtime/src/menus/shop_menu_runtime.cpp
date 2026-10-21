#include "shop_menu_runtime.hpp"

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

std::string_view group_id(const ShopActionGroup group) {
    switch (group) {
    case ShopActionGroup::weapons: return "weapons";
    case ShopActionGroup::other_items: return "other_items";
    case ShopActionGroup::armor: return "armor";
    case ShopActionGroup::source_action_3: return "source_action_3";
    case ShopActionGroup::source_action_4: return "source_action_4";
    case ShopActionGroup::sell: return "sell";
    case ShopActionGroup::magic_spells: return "magic_spells";
    case ShopActionGroup::magic_items: return "magic_items";
    case ShopActionGroup::magic_foci: return "magic_foci";
    }
    return {};
}

} // namespace

bool ShopMenuController::open(
    const ShopId shop,
    const scripting::ScriptShopDefinition& definition,
    const std::array<std::string, 2>& confirmation_choice_ids) {
    const bool shop_id_matches_definition =
        (shop == ShopId::ares_weapon_emporium &&
         definition.id == "ares_weapon_emporium") ||
        (shop == ShopId::medicine_lodge &&
         definition.id == "medicine_lodge");
    if (!shop_id_matches_definition || definition.groups.empty() ||
        definition.groups.size() > group_ids_.size() ||
        confirmation_choice_ids[0] != "yes" ||
        confirmation_choice_ids[1] != "no") {
        return false;
    }

    std::array<std::string, 6> group_ids{};
    std::array<std::vector<FieldActionCostRecord>, 6> offer_groups{};
    std::array<std::vector<scripting::ScriptShopProgressionOffer>, 6>
        progression_offer_groups{};
    for (std::size_t index = 0; index < definition.groups.size(); ++index) {
        const auto& scripted_group = definition.groups[index];
        const auto previous = std::find(
            group_ids.begin(), group_ids.begin() + index,
            scripted_group.id);
        if (scripted_group.id.empty() ||
            previous != group_ids.begin() + index ||
            scripted_group.offers.size() > 8U ||
            scripted_group.progression_offers.size() > 8U ||
            (!scripted_group.offers.empty() &&
             !scripted_group.progression_offers.empty())) {
            return false;
        }
        group_ids[index] = scripted_group.id;
        auto& offers = offer_groups[index];
        offers.reserve(scripted_group.offers.size());
        for (const auto& offer : scripted_group.offers) {
            offers.push_back(FieldActionCostRecord{
                offer.type, offer.value, offer.quantity});
        }
        progression_offer_groups[index] = scripted_group.progression_offers;
    }
    shop_ = shop;
    definition_id_ = definition.id;
    group_count_ = definition.groups.size();
    group_ids_ = std::move(group_ids);
    displayed_offer_group_ =
        ShopActionGroup::weapons;
    displayed_offer_group_source_index_ = 0U;
    confirmation_choice_ids_ = confirmation_choice_ids;
    offer_groups_ = std::move(offer_groups);
    progression_offer_groups_ = std::move(progression_offer_groups);
    menu_mode_ = definition.menu_mode;
    source_state_ = FieldModeServiceState{};
    source_state_.action_selection.menu_result = definition.menu_result;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel = 0;
    source_state_.action_selection.selected_action = 0;
    displayed_offer_group_ = selected_group();
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
    definition_id_.clear();
    group_ids_ = {};
    group_count_ = 0U;
    displayed_offer_group_ = ShopActionGroup::weapons;
    displayed_offer_group_source_index_ = 0U;
    confirmation_choice_ids_ = {};
    offer_groups_ = {};
    progression_offer_groups_ = {};
    menu_mode_ = 0U;
}

void ShopMenuController::finish_action() noexcept {
    if (!open_) {
        return;
    }
    pending_action_.reset();
    reset_transaction();
}

void ShopMenuController::clear_pending_action() noexcept {
    pending_action_.reset();
}

void ShopMenuController::show_progression_message(
    const ShopTransactionMessage message) noexcept {
    reset_transaction();
    if (message == ShopTransactionMessage::none) {
        return;
    }
    transaction_message_ = message;
    transaction_waiting_for_event_ = true;
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
    const FieldActionBaseCostLookup& resale_base_lookup) {
    reset_transaction();
    transaction_kind_ = TransactionKind::sale;
    last_sale_outcome_ = ShopTransactionService{}.sell(
        cost_context, state, false, resale_base_lookup);
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
    const FieldActionBaseCostLookup& purchase_base_lookup,
    const FieldActionBaseCostLookup& resale_base_lookup) {
    if (transaction_waiting_for_event_) {
        // ROM 0x5794E calls the 0xD94 wait after drawing rejected-purchase
        // feedback. 0xD94 exits only when FFDD22's new-press high byte is set;
        // the held-button low byte must not dismiss the message.
        if ((controller_events & 0xFF00U) == 0U) {
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
            purchase_base_lookup);
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
            cost_context, sale_state, true, resale_base_lookup);
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

ShopActionGroup ShopMenuController::selected_group() const noexcept {
    const auto index = selected_group_index();
    if (index >= group_count_) {
        return ShopActionGroup::weapons;
    }
    const auto& id = group_ids_[index];
    if (id == "weapons") return ShopActionGroup::weapons;
    if (id == "other_items") return ShopActionGroup::other_items;
    if (id == "armor") return ShopActionGroup::armor;
    if (id == "source_action_3") return ShopActionGroup::source_action_3;
    if (id == "source_action_4") return ShopActionGroup::source_action_4;
    if (id == "sell") return ShopActionGroup::sell;
    if (id == "magic_spells") return ShopActionGroup::magic_spells;
    if (id == "magic_items") return ShopActionGroup::magic_items;
    if (id == "magic_foci") return ShopActionGroup::magic_foci;
    return static_cast<ShopActionGroup>(index);
}

std::size_t ShopMenuController::selected_group_index() const noexcept {
    const auto index = source_state_.action_selection.selected_action;
    return index < 0 ? group_count_ : static_cast<std::size_t>(index);
}

std::size_t ShopMenuController::displayed_offer_group_index() const noexcept {
    return displayed_offer_group_source_index_ < group_count_
               ? displayed_offer_group_source_index_
               : group_count_;
}

std::string_view ShopMenuController::selected_group_id() const noexcept {
    const auto index = selected_group_index();
    return index < group_count_
               ? std::string_view(group_ids_[index])
               : std::string_view{};
}

std::string_view ShopMenuController::displayed_offer_group_id() const noexcept {
    const auto index = displayed_offer_group_index();
    return index < group_count_
               ? std::string_view(group_ids_[index])
               : std::string_view{};
}

std::size_t ShopMenuController::offer_count(
    const ShopActionGroup group) const noexcept {
    if (!open_) {
        return 0U;
    }
    const auto id = group_id(group);
    const auto found = std::find(
        group_ids_.begin(), group_ids_.begin() +
                                static_cast<std::ptrdiff_t>(group_count_),
        id);
    if (found == group_ids_.begin() +
                     static_cast<std::ptrdiff_t>(group_count_)) {
        return 0U;
    }
    const auto index = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return offer_groups_[index].size();
}

std::optional<FieldActionCostRecord> ShopMenuController::offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= offer_count(group)) {
        return std::nullopt;
    }
    const auto id = group_id(group);
    const auto found = std::find(
        group_ids_.begin(), group_ids_.begin() +
                                static_cast<std::ptrdiff_t>(group_count_),
        id);
    if (found == group_ids_.begin() +
                     static_cast<std::ptrdiff_t>(group_count_)) {
        return std::nullopt;
    }
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return offer_groups_[group_slot][index];
}

std::size_t ShopMenuController::progression_offer_count(
    const ShopActionGroup group) const noexcept {
    if (!open_) {
        return 0U;
    }
    const auto id = group_id(group);
    const auto found = std::find(
        group_ids_.begin(), group_ids_.begin() +
                                static_cast<std::ptrdiff_t>(group_count_),
        id);
    if (found == group_ids_.begin() +
                     static_cast<std::ptrdiff_t>(group_count_)) {
        return 0U;
    }
    const auto index = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return progression_offer_groups_[index].size();
}

std::optional<scripting::ScriptShopProgressionOffer>
ShopMenuController::progression_offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= progression_offer_count(group)) {
        return std::nullopt;
    }
    const auto id = group_id(group);
    const auto found = std::find(
        group_ids_.begin(), group_ids_.begin() +
                                static_cast<std::ptrdiff_t>(group_count_),
        id);
    if (found == group_ids_.begin() +
                     static_cast<std::ptrdiff_t>(group_count_)) {
        return std::nullopt;
    }
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return progression_offer_groups_[group_slot][index];
}

std::optional<ShopMenuEvent> ShopMenuController::apply_script_snapshot(
    const scripting::ScriptShopMenuSnapshot& snapshot) {
    if (!open_ || snapshot.shop_id != definition_id_ ||
        snapshot.menu_mode != menu_mode_ ||
        snapshot.selected_action >= group_count_) {
        return std::nullopt;
    }

    ShopMenuEvent event = ShopMenuEvent::no_change;
    if (snapshot.event == "selection_moved") {
        event = ShopMenuEvent::selection_moved;
    } else if (snapshot.event == "panel_changed") {
        event = ShopMenuEvent::panel_changed;
    } else if (snapshot.event == "purchase_selected") {
        event = ShopMenuEvent::purchase_selected;
    } else if (snapshot.event == "sale_selected") {
        event = ShopMenuEvent::sale_selected;
    } else if (snapshot.event == "offer_unavailable") {
        event = ShopMenuEvent::offer_unavailable;
    } else if (snapshot.event == "progression_selected") {
        event = ShopMenuEvent::progression_selected;
    } else if (snapshot.event == "source_action_boundary") {
        event = ShopMenuEvent::source_action_boundary;
    } else if (snapshot.event == "active_runner_change_requested") {
        event = ShopMenuEvent::active_runner_change_requested;
    } else if (snapshot.event == "returned_to_field") {
        event = ShopMenuEvent::returned_to_field;
    } else if (snapshot.event != "opened" &&
               snapshot.event != "no_change") {
        return std::nullopt;
    }

    source_state_.action_selection.menu_result = snapshot.menu_result;
    source_state_.action_selection.action_cursor =
        static_cast<std::int8_t>(snapshot.action_cursor);
    source_state_.action_selection.action_panel =
        static_cast<std::int8_t>(snapshot.action_panel);
    source_state_.action_selection.selected_action =
        static_cast<std::int8_t>(snapshot.selected_action);
    if (event == ShopMenuEvent::panel_changed && snapshot.action_panel != 0U) {
        displayed_offer_group_ = selected_group();
        displayed_offer_group_source_index_ = selected_group_index();
    }

    pending_action_.reset();
    if (event == ShopMenuEvent::purchase_selected ||
        event == ShopMenuEvent::sale_selected ||
        event == ShopMenuEvent::progression_selected ||
        event == ShopMenuEvent::source_action_boundary) {
        ShopActionRequest request{};
        request.group = selected_group();
        request.cursor = static_cast<std::uint8_t>(snapshot.action_cursor);
        if (event == ShopMenuEvent::purchase_selected) {
            request.offer = offer(request.group, request.cursor);
            if (!request.offer.has_value()) {
                return std::nullopt;
            }
        } else if (event == ShopMenuEvent::progression_selected) {
            request.progression_offer =
                progression_offer(request.group, request.cursor);
            if (!request.progression_offer.has_value()) {
                return std::nullopt;
            }
        }
        pending_action_ = request;
    }

    if (!snapshot.open) {
        if (event != ShopMenuEvent::returned_to_field) {
            return std::nullopt;
        }
        close();
    }
    return event;
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
    const FieldActionBaseCostLookup& resale_base_lookup) const {
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

    const auto quote = quote_sale(item_code, cost_context, resale_base_lookup);
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
    const FieldActionBaseCostLookup& resale_base_lookup) {
    if (item_code == 0U || item_code > 50U) {
        return std::nullopt;
    }
    std::uint32_t resale_base = 0U;
    if (resale_base_lookup) {
        resale_base = resale_base_lookup(
            static_cast<std::uint8_t>(item_code - 1U));
    } else {
        const auto* item = find_us_item_definition(item_code);
        if (item == nullptr) {
            return std::nullopt;
        }
        resale_base = item->base_cost / 2U;
    }
    return apply_runner_cost_surcharge(resale_base, cost_context);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

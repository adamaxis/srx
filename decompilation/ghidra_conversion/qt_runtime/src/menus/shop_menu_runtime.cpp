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
    case ShopActionGroup::cyberware: return "cyberware";
    case ShopActionGroup::cyberdecks: return "cyberdecks";
    case ShopActionGroup::cyberdeck_upgrades: return "cyberdeck_upgrades";
    case ShopActionGroup::cyberdeck_upgrades_second:
        return "cyberdeck_upgrades_second";
    }
    return {};
}

} // namespace

bool ShopMenuController::open(
    const ShopId shop,
    const scripting::ScriptShopDefinition& definition,
    const std::array<std::string, 2>& confirmation_choice_ids) {
    const bool shop_id_matches_definition =
        (shop == ShopId::field_store && definition.menu_result == -1 &&
         definition.groups.size() == 6U) ||
        (shop == ShopId::field_store && definition.menu_result == 0 &&
         definition.groups.size() == 4U) ||
        (shop == ShopId::medicine_lodge &&
         definition.id == "medicine_lodge" &&
         definition.menu_result == 0 && definition.groups.size() == 4U) ||
        (shop == ShopId::wire_masters &&
         definition.id == "wire_masters" && definition.menu_mode == 7U &&
         definition.menu_result == 1 && definition.groups.size() == 1U &&
         definition.equipment_upgrade_meter_thresholds.size() == 16U &&
         definition.groups.front().equipment_upgrade_offers.size() == 8U) ||
        (shop == ShopId::microtronics &&
         definition.id == "microtronics" && definition.menu_mode == 6U &&
         definition.menu_result == 2 && definition.groups.size() == 6U &&
         definition.groups[3U].id == "cyberdecks" &&
         definition.groups[3U].cyberdeck_model_offers.size() == 3U &&
         definition.groups[4U].id == "cyberdeck_upgrades" &&
         definition.groups[4U].cyberdeck_stat_upgrade_offers.size() == 4U &&
         definition.groups[5U].id == "cyberdeck_upgrades_second" &&
         definition.groups[5U].cyberdeck_rating_upgrade_offers.size() == 5U);
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
    std::array<std::vector<scripting::ScriptShopEquipmentUpgradeOffer>, 6>
        equipment_upgrade_offer_groups{};
    std::array<std::vector<scripting::ScriptShopCyberdeckModelOffer>, 6>
        cyberdeck_model_offer_groups{};
    std::array<std::vector<scripting::ScriptShopCyberdeckStatUpgradeOffer>, 6>
        cyberdeck_stat_upgrade_offer_groups{};
    std::array<std::vector<scripting::ScriptShopCyberdeckRatingUpgradeOffer>, 6>
        cyberdeck_rating_upgrade_offer_groups{};
    for (std::size_t index = 0; index < definition.groups.size(); ++index) {
        const auto& scripted_group = definition.groups[index];
        const auto previous = std::find(
            group_ids.begin(), group_ids.begin() + index,
            scripted_group.id);
        if (scripted_group.id.empty() ||
            previous != group_ids.begin() + index ||
            scripted_group.offers.size() > 8U ||
            scripted_group.progression_offers.size() > 8U ||
            scripted_group.equipment_upgrade_offers.size() > 8U ||
            scripted_group.cyberdeck_model_offers.size() > 8U ||
            scripted_group.cyberdeck_stat_upgrade_offers.size() > 8U ||
            scripted_group.cyberdeck_rating_upgrade_offers.size() > 8U ||
            static_cast<unsigned>(!scripted_group.offers.empty()) +
                    static_cast<unsigned>(
                        !scripted_group.progression_offers.empty()) +
                    static_cast<unsigned>(
                        !scripted_group.equipment_upgrade_offers.empty()) +
                    static_cast<unsigned>(
                        !scripted_group.cyberdeck_model_offers.empty()) +
                    static_cast<unsigned>(
                        !scripted_group.cyberdeck_stat_upgrade_offers.empty()) +
                    static_cast<unsigned>(
                        !scripted_group.cyberdeck_rating_upgrade_offers.empty()) >
                1U) {
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
        equipment_upgrade_offer_groups[index] =
            scripted_group.equipment_upgrade_offers;
        cyberdeck_model_offer_groups[index] =
            scripted_group.cyberdeck_model_offers;
        cyberdeck_stat_upgrade_offer_groups[index] =
            scripted_group.cyberdeck_stat_upgrade_offers;
        cyberdeck_rating_upgrade_offer_groups[index] =
            scripted_group.cyberdeck_rating_upgrade_offers;
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
    equipment_upgrade_offer_groups_ =
        std::move(equipment_upgrade_offer_groups);
    cyberdeck_model_offer_groups_ =
        std::move(cyberdeck_model_offer_groups);
    cyberdeck_stat_upgrade_offer_groups_ =
        std::move(cyberdeck_stat_upgrade_offer_groups);
    cyberdeck_rating_upgrade_offer_groups_ =
        std::move(cyberdeck_rating_upgrade_offer_groups);
    menu_mode_ = definition.menu_mode;
    source_state_ = FieldModeServiceState{};
    source_state_.action_selection.menu_result = definition.menu_result;
    source_state_.action_selection.action_cursor = 0;
    source_state_.action_selection.action_panel =
        definition.menu_result == 1 ? 1 : 0;
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
    equipment_upgrade_offer_groups_ = {};
    cyberdeck_model_offer_groups_ = {};
    cyberdeck_stat_upgrade_offer_groups_ = {};
    cyberdeck_rating_upgrade_offer_groups_ = {};
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

ShopTransactionEvent ShopMenuController::begin_equipment_upgrade_confirmation(
    const std::uint32_t cost) {
    if (!open_ || shop_ != ShopId::wire_masters || !pending_action_ ||
        !pending_action_->equipment_upgrade_offer.has_value()) {
        return ShopTransactionEvent::unresolved;
    }
    reset_transaction();
    transaction_kind_ = TransactionKind::equipment_upgrade;
    transaction_amount_ = cost;
    transaction_confirmation_ = MenuConfirmationState{};
    transaction_confirmation_open_ = true;
    return ShopTransactionEvent::confirmation_opened;
}

ShopTransactionEvent
ShopMenuController::begin_cyberdeck_model_upgrade_confirmation(
    const std::uint32_t cost) {
    if (!open_ || shop_ != ShopId::microtronics || !pending_action_ ||
        !pending_action_->cyberdeck_model_offer.has_value()) {
        return ShopTransactionEvent::unresolved;
    }
    reset_transaction();
    transaction_kind_ = TransactionKind::cyberdeck_model_upgrade;
    transaction_amount_ = cost;
    transaction_confirmation_ = MenuConfirmationState{};
    transaction_confirmation_open_ = true;
    return ShopTransactionEvent::confirmation_opened;
}

ShopTransactionEvent
ShopMenuController::begin_cyberdeck_stat_upgrade_confirmation(
    const std::uint32_t cost) {
    if (!open_ || shop_ != ShopId::microtronics || !pending_action_ ||
        !pending_action_->cyberdeck_stat_upgrade_offer.has_value()) {
        return ShopTransactionEvent::unresolved;
    }
    reset_transaction();
    transaction_kind_ = TransactionKind::cyberdeck_stat_upgrade;
    transaction_amount_ = cost;
    transaction_confirmation_ = MenuConfirmationState{};
    transaction_confirmation_open_ = true;
    return ShopTransactionEvent::confirmation_opened;
}

ShopTransactionEvent
ShopMenuController::begin_cyberdeck_rating_upgrade_confirmation(
    const std::uint32_t cost) {
    if (!open_ || shop_ != ShopId::microtronics || !pending_action_ ||
        !pending_action_->cyberdeck_rating_upgrade_offer.has_value()) {
        return ShopTransactionEvent::unresolved;
    }
    reset_transaction();
    transaction_kind_ = TransactionKind::cyberdeck_rating_upgrade;
    transaction_amount_ = cost;
    transaction_confirmation_ = MenuConfirmationState{};
    transaction_confirmation_open_ = true;
    return ShopTransactionEvent::confirmation_opened;
}

void ShopMenuController::show_transaction_message(
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
    if (transaction_kind_ == TransactionKind::equipment_upgrade &&
        pending_action_ &&
        pending_action_->equipment_upgrade_offer.has_value()) {
        reset_transaction();
        return ShopTransactionEvent::equipment_upgrade_confirmed;
    }
    if (transaction_kind_ == TransactionKind::cyberdeck_model_upgrade &&
        pending_action_ &&
        pending_action_->cyberdeck_model_offer.has_value()) {
        reset_transaction();
        return ShopTransactionEvent::cyberdeck_model_upgrade_confirmed;
    }
    if (transaction_kind_ == TransactionKind::cyberdeck_stat_upgrade &&
        pending_action_ &&
        pending_action_->cyberdeck_stat_upgrade_offer.has_value()) {
        reset_transaction();
        return ShopTransactionEvent::cyberdeck_stat_upgrade_confirmed;
    }
    if (transaction_kind_ == TransactionKind::cyberdeck_rating_upgrade &&
        pending_action_ &&
        pending_action_->cyberdeck_rating_upgrade_offer.has_value()) {
        reset_transaction();
        return ShopTransactionEvent::cyberdeck_rating_upgrade_confirmed;
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
    if (id == "cyberware") return ShopActionGroup::cyberware;
    if (id == "cyberdecks") return ShopActionGroup::cyberdecks;
    if (id == "cyberdeck_upgrades") {
        return ShopActionGroup::cyberdeck_upgrades;
    }
    if (id == "cyberdeck_upgrades_second") {
        return ShopActionGroup::cyberdeck_upgrades_second;
    }
    return static_cast<ShopActionGroup>(index);
}

std::size_t ShopMenuController::selected_group_index() const noexcept {
    if (source_state_.action_selection.menu_result == 1) {
        return group_count_ == 0U ? group_count_ : 0U;
    }
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

std::size_t ShopMenuController::equipment_upgrade_offer_count(
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
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return equipment_upgrade_offer_groups_[group_slot].size();
}

std::optional<scripting::ScriptShopEquipmentUpgradeOffer>
ShopMenuController::equipment_upgrade_offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= equipment_upgrade_offer_count(group)) {
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
    return equipment_upgrade_offer_groups_[group_slot][index];
}

std::size_t ShopMenuController::cyberdeck_model_offer_count(
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
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return cyberdeck_model_offer_groups_[group_slot].size();
}

std::optional<scripting::ScriptShopCyberdeckModelOffer>
ShopMenuController::cyberdeck_model_offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= cyberdeck_model_offer_count(group)) {
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
    return cyberdeck_model_offer_groups_[group_slot][index];
}

std::size_t ShopMenuController::cyberdeck_stat_upgrade_offer_count(
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
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return cyberdeck_stat_upgrade_offer_groups_[group_slot].size();
}

std::optional<scripting::ScriptShopCyberdeckStatUpgradeOffer>
ShopMenuController::cyberdeck_stat_upgrade_offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= cyberdeck_stat_upgrade_offer_count(group)) {
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
    return cyberdeck_stat_upgrade_offer_groups_[group_slot][index];
}

std::size_t ShopMenuController::cyberdeck_rating_upgrade_offer_count(
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
    const auto group_slot = static_cast<std::size_t>(
        std::distance(group_ids_.begin(), found));
    return cyberdeck_rating_upgrade_offer_groups_[group_slot].size();
}

std::optional<scripting::ScriptShopCyberdeckRatingUpgradeOffer>
ShopMenuController::cyberdeck_rating_upgrade_offer(
    const ShopActionGroup group, const std::size_t index) const noexcept {
    if (index >= cyberdeck_rating_upgrade_offer_count(group)) {
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
    return cyberdeck_rating_upgrade_offer_groups_[group_slot][index];
}

std::optional<ShopMenuEvent> ShopMenuController::apply_script_snapshot(
    const scripting::ScriptShopMenuSnapshot& snapshot) {
    const bool equipment_selector = snapshot.menu_result == 1;
    if (!open_ || snapshot.shop_id != definition_id_ ||
        snapshot.menu_mode != menu_mode_) {
        return std::nullopt;
    }
    if (equipment_selector) {
        if (shop_ != ShopId::wire_masters || snapshot.action_panel != 1U ||
            snapshot.action_cursor >= 8U || snapshot.selected_action >= 8U) {
            return std::nullopt;
        }
    } else if (snapshot.selected_action >= group_count_) {
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
    } else if (snapshot.event == "equipment_upgrade_selected") {
        event = ShopMenuEvent::equipment_upgrade_selected;
    } else if (snapshot.event == "cyberdeck_model_selected") {
        event = ShopMenuEvent::cyberdeck_model_selected;
    } else if (snapshot.event == "cyberdeck_stat_upgrade_selected") {
        event = ShopMenuEvent::cyberdeck_stat_upgrade_selected;
    } else if (snapshot.event == "cyberdeck_rating_upgrade_selected") {
        event = ShopMenuEvent::cyberdeck_rating_upgrade_selected;
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
        event == ShopMenuEvent::equipment_upgrade_selected ||
        event == ShopMenuEvent::cyberdeck_model_selected ||
        event == ShopMenuEvent::cyberdeck_stat_upgrade_selected ||
        event == ShopMenuEvent::cyberdeck_rating_upgrade_selected ||
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
        } else if (event == ShopMenuEvent::equipment_upgrade_selected) {
            request.equipment_upgrade_offer = equipment_upgrade_offer(
                request.group, request.cursor);
            if (!request.equipment_upgrade_offer.has_value()) {
                return std::nullopt;
            }
        } else if (event == ShopMenuEvent::cyberdeck_model_selected) {
            request.cyberdeck_model_offer = cyberdeck_model_offer(
                request.group, request.cursor);
            if (shop_ != ShopId::microtronics ||
                snapshot.menu_result != 2 || snapshot.action_panel != 1U ||
                snapshot.selected_action != 3U ||
                snapshot.action_cursor >= 3U ||
                request.group != ShopActionGroup::cyberdecks ||
                !request.cyberdeck_model_offer.has_value()) {
                return std::nullopt;
            }
        } else if (event == ShopMenuEvent::cyberdeck_stat_upgrade_selected) {
            request.cyberdeck_stat_upgrade_offer =
                cyberdeck_stat_upgrade_offer(request.group, request.cursor);
            if (shop_ != ShopId::microtronics ||
                snapshot.menu_result != 2 || snapshot.action_panel != 1U ||
                snapshot.selected_action != 4U ||
                snapshot.action_cursor >= 4U ||
                request.group != ShopActionGroup::cyberdeck_upgrades ||
                !request.cyberdeck_stat_upgrade_offer.has_value()) {
                return std::nullopt;
            }
        } else if (event ==
                   ShopMenuEvent::cyberdeck_rating_upgrade_selected) {
            request.cyberdeck_rating_upgrade_offer =
                cyberdeck_rating_upgrade_offer(request.group, request.cursor);
            if (shop_ != ShopId::microtronics ||
                snapshot.menu_result != 2 || snapshot.action_panel != 1U ||
                snapshot.selected_action != 5U ||
                snapshot.action_cursor >= 5U ||
                request.group != ShopActionGroup::cyberdeck_upgrades_second ||
                !request.cyberdeck_rating_upgrade_offer.has_value()) {
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

FieldCyberdeckModelUpgradeResult apply_field_cyberdeck_model_upgrade(
    const scripting::ScriptShopCyberdeckModelOffer& offer,
    FieldCyberdeckModelUpgradeState& state, const bool confirmed) {
    auto& deck = state.deck;
    if (!deck.model_index.has_value() || !deck.mpcp.has_value() ||
        !deck.hardening.has_value() || !deck.memory_megapulses.has_value() ||
        !deck.storage_megapulses.has_value() ||
        !deck.load_io_speed.has_value() || !deck.response.has_value()) {
        return FieldCyberdeckModelUpgradeResult::state_unavailable;
    }
    if (*deck.model_index >= static_cast<std::int8_t>(offer.model_index)) {
        return FieldCyberdeckModelUpgradeResult::model_unavailable;
    }
    if (state.funds < offer.cost) {
        return FieldCyberdeckModelUpgradeResult::insufficient_funds;
    }
    if (!confirmed) {
        return FieldCyberdeckModelUpgradeResult::confirmation_required;
    }

    *deck.mpcp = offer.mpcp;
    *deck.hardening = offer.hardening;
    *deck.memory_megapulses = std::max(
        *deck.memory_megapulses, offer.memory_megapulses);
    *deck.storage_megapulses = std::max(
        *deck.storage_megapulses, offer.storage_megapulses);
    *deck.load_io_speed = offer.load_io_speed;
    *deck.response = offer.response;
    *deck.model_index = static_cast<std::int8_t>(offer.model_index);
    state.funds -= offer.cost;
    return FieldCyberdeckModelUpgradeResult::applied;
}

std::uint8_t field_cyberdeck_stat_upgrade_increment(
    const scripting::ScriptShopCyberdeckUpgradeStat stat) noexcept {
    switch (stat) {
    case scripting::ScriptShopCyberdeckUpgradeStat::memory_megapulses:
        return 0x0AU;
    case scripting::ScriptShopCyberdeckUpgradeStat::storage_megapulses:
        return 0x19U;
    case scripting::ScriptShopCyberdeckUpgradeStat::load_io_speed:
        return 0x05U;
    case scripting::ScriptShopCyberdeckUpgradeStat::response:
        return 1U;
    }
    return 0U;
}

FieldCyberdeckStatUpgradeResult apply_field_cyberdeck_stat_upgrade(
    const scripting::ScriptShopCyberdeckStatUpgradeOffer& offer,
    FieldCyberdeckUpgradeState& state, const bool confirmed) {
    auto& deck = state.deck;
    if (!deck.mpcp.has_value()) {
        return FieldCyberdeckStatUpgradeResult::state_unavailable;
    }

    const auto signed_word = [](const std::uint16_t value) {
        return value < 0x8000U
            ? static_cast<std::int32_t>(value)
            : static_cast<std::int32_t>(value) - 0x10000;
    };
    const auto signed_byte = [](const std::uint8_t value) {
        return value < 0x80U
            ? static_cast<std::int32_t>(value)
            : static_cast<std::int32_t>(value) - 0x100;
    };

    bool below_source_limit = false;
    switch (offer.stat) {
    case scripting::ScriptShopCyberdeckUpgradeStat::memory_megapulses:
        if (!deck.memory_megapulses.has_value()) {
            return FieldCyberdeckStatUpgradeResult::state_unavailable;
        }
        // ROM 0x5766A compares MPCP*0x28 against FFFBBE+2, then uses BLE.
        below_source_limit = signed_word(
            static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(*deck.mpcp) * 0x28U)) >
            signed_word(*deck.memory_megapulses);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::storage_megapulses:
        if (!deck.storage_megapulses.has_value()) {
            return FieldCyberdeckStatUpgradeResult::state_unavailable;
        }
        // The 0x50 storage comparison at 0x576AE..0x576B6 also uses BLE.
        below_source_limit = signed_word(
            static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(*deck.mpcp) * 0x50U)) >
            signed_word(*deck.storage_megapulses);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::load_io_speed:
        if (!deck.load_io_speed.has_value()) {
            return FieldCyberdeckStatUpgradeResult::state_unavailable;
        }
        // ROM 0x57702..0x5770A compares the low byte of MPCP*10 and
        // branches with BLE, so both byte operands are signed for the test.
        below_source_limit = signed_byte(
            static_cast<std::uint8_t>(
                static_cast<std::uint16_t>(*deck.mpcp) * 0x0AU)) >
            signed_byte(*deck.load_io_speed);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::response:
        if (!deck.response.has_value()) {
            return FieldCyberdeckStatUpgradeResult::state_unavailable;
        }
        // ROM 0x576CE..0x576D4 is CMPi.B #3,(A0+7) / BMI. The source
        // branches on the subtraction's N bit only, so retain its 8-bit
        // wrap instead of replacing it with an ordinary signed comparison.
        below_source_limit = static_cast<std::uint8_t>(
            *deck.response - 0x03U) >= 0x80U;
        if (!below_source_limit) {
            return FieldCyberdeckStatUpgradeResult::response_maximum;
        }
        break;
    }
    if (!below_source_limit) {
        return FieldCyberdeckStatUpgradeResult::mpcp_maximum;
    }
    if (state.funds < offer.cost) {
        return FieldCyberdeckStatUpgradeResult::insufficient_funds;
    }
    if (!confirmed) {
        return FieldCyberdeckStatUpgradeResult::confirmation_required;
    }

    const auto increment = field_cyberdeck_stat_upgrade_increment(offer.stat);
    if (increment == 0U) {
        return FieldCyberdeckStatUpgradeResult::state_unavailable;
    }
    switch (offer.stat) {
    case scripting::ScriptShopCyberdeckUpgradeStat::memory_megapulses:
        *deck.memory_megapulses = static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(*deck.memory_megapulses) + increment);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::storage_megapulses:
        *deck.storage_megapulses = static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(*deck.storage_megapulses) + increment);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::load_io_speed:
        *deck.load_io_speed = static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(*deck.load_io_speed) + increment);
        break;
    case scripting::ScriptShopCyberdeckUpgradeStat::response:
        *deck.response = static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(*deck.response) + increment);
        break;
    }
    state.funds -= offer.cost;
    return FieldCyberdeckStatUpgradeResult::applied;
}

FieldCyberdeckRatingUpgradeResult apply_field_cyberdeck_rating_upgrade(
    const scripting::ScriptShopCyberdeckRatingUpgradeQuote& quote,
    FieldCyberdeckUpgradeState& state, const bool confirmed) {
    if (quote.status !=
        scripting::ScriptShopCyberdeckRatingUpgradeStatus::available) {
        return FieldCyberdeckRatingUpgradeResult::state_unavailable;
    }

    auto& deck = state.deck;
    std::uint8_t current_rating = 0U;
    std::size_t persona_index = 0U;
    const bool mpcp_upgrade =
        quote.stat ==
            scripting::ScriptShopCyberdeckRatingUpgradeStat::mpcp_repair;
    switch (quote.stat) {
    case scripting::ScriptShopCyberdeckRatingUpgradeStat::mpcp_repair:
        if (!deck.mpcp.has_value()) {
            return FieldCyberdeckRatingUpgradeResult::state_unavailable;
        }
        current_rating = *deck.mpcp;
        break;
    case scripting::ScriptShopCyberdeckRatingUpgradeStat::body:
    case scripting::ScriptShopCyberdeckRatingUpgradeStat::evasion:
    case scripting::ScriptShopCyberdeckRatingUpgradeStat::masking:
    case scripting::ScriptShopCyberdeckRatingUpgradeStat::sensors: {
        if (!deck.persona.has_value()) {
            return FieldCyberdeckRatingUpgradeResult::state_unavailable;
        }
        persona_index =
            static_cast<std::size_t>(quote.stat) -
            static_cast<std::size_t>(
                scripting::ScriptShopCyberdeckRatingUpgradeStat::body);
        current_rating = (*deck.persona)[persona_index];
        break;
    }
    }
    if (current_rating != quote.current_rating) {
        return FieldCyberdeckRatingUpgradeResult::state_unavailable;
    }
    if (state.funds < quote.cost) {
        return FieldCyberdeckRatingUpgradeResult::insufficient_funds;
    }
    if (!confirmed) {
        return FieldCyberdeckRatingUpgradeResult::confirmation_required;
    }
    if (mpcp_upgrade) {
        *deck.mpcp = quote.next_rating;
    } else {
        (*deck.persona)[persona_index] = quote.next_rating;
    }
    state.funds -= quote.cost;
    return FieldCyberdeckRatingUpgradeResult::applied;
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

#include "shop_menu_runtime.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    const auto ares = ShopId::ares_weapon_emporium;
    if (!expect(ShopMenuController::offer_count(
                    ares, ShopActionGroup::weapons) == 7U) ||
        !expect(ShopMenuController::offer_count(
                    ares, ShopActionGroup::armor) == 2U) ||
        !expect(ShopMenuController::offer_count(
                    ares, ShopActionGroup::other_items) == 4U) ||
        !expect(!ShopMenuController::offer(
                     ares, ShopActionGroup::weapons, 7U)
                     .has_value())) {
        return 1;
    }

    const auto weapon_offer = ShopMenuController::offer(
        ares, ShopActionGroup::weapons, 2U);
    const auto armor_offer = ShopMenuController::offer(
        ares, ShopActionGroup::armor, 1U);
    const auto other_offer = ShopMenuController::offer(
        ares, ShopActionGroup::other_items, 0U);
    if (!expect(weapon_offer.has_value() && weapon_offer->type == 6U &&
                weapon_offer->quantity == 15U) ||
        !expect(armor_offer.has_value() && armor_offer->type == 0x2DU &&
                armor_offer->quantity == 12U) ||
        !expect(other_offer.has_value() && other_offer->type == 0x45U &&
                other_offer->quantity == 25U)) {
        return 2;
    }

    ShopMenuController menu;
    if (!expect(menu.open(ares)) || !expect(menu.menu_mode() == 1U) ||
        !expect(menu.source_state().action_selection.menu_result == -1) ||
        !expect(menu.advance(0x0800U) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_group() == ShopActionGroup::armor) ||
        !expect(menu.advance(0x0200U) == ShopMenuEvent::panel_changed) ||
        !expect(menu.offers_panel_active()) ||
        !expect(menu.selected_group() == ShopActionGroup::armor) ||
        !expect(menu.advance(0x4000U) == ShopMenuEvent::purchase_selected) ||
        !expect(menu.pending_action().has_value()) ||
        !expect(menu.pending_action()->offer->type == 0x2EU)) {
        return 3;
    }
    menu.finish_action();

    if (!expect(menu.advance(0x0200U) == ShopMenuEvent::panel_changed) ||
        !expect(menu.advance(0x0800U) == ShopMenuEvent::selection_moved) ||
        !expect(menu.advance(0x0800U) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_cursor() == 2U) ||
        !expect(menu.advance(0x4000U) == ShopMenuEvent::purchase_selected) ||
        !expect(menu.pending_action()->offer->type == 6U) ||
        !expect(menu.pending_action()->offer->quantity == 15U)) {
        return 4;
    }

    menu.finish_action();
    for (int step = 0; step < 5; ++step) {
        if (!expect(menu.advance(0x0800U) ==
                    ShopMenuEvent::selection_moved)) {
            return 5;
        }
    }
    if (!expect(menu.selected_group() == ShopActionGroup::sell) ||
        !expect(menu.advance(0x0200U) == ShopMenuEvent::panel_changed) ||
        !expect(menu.advance(0x0800U) == ShopMenuEvent::selection_moved) ||
        !expect(menu.advance(0x0800U) == ShopMenuEvent::selection_moved) ||
        !expect(menu.advance(0x4000U) == ShopMenuEvent::sale_selected) ||
        !expect(menu.pending_action()->cursor == 2U)) {
        return 6;
    }

    menu.finish_action();
    for (int step = 0; step < 3; ++step) {
        menu.advance(0x0800U);
    }
    if (!expect(menu.advance(0x0200U) == ShopMenuEvent::panel_changed) ||
        !expect(menu.advance(0x4000U) ==
                ShopMenuEvent::source_action_boundary)) {
        return 7;
    }
    menu.finish_action();
    if (!expect(menu.advance(0x8000U) ==
                ShopMenuEvent::returned_to_field) ||
        !expect(!menu.is_open())) {
        return 8;
    }

    ShopTransactionService transactions;
    const FieldActionCostContext cost_context{1U, 0U, 3U, 2U};
    FieldActionPurchaseState purchase{};
    purchase.funds = 1000U;
    if (!expect(transactions.purchase(*weapon_offer, cost_context, purchase,
                                      true) ==
                FieldActionApplicationResult::applied) ||
        !expect(purchase.funds == 346U) ||
        !expect(purchase.runner.action_codes[0] == 7U) ||
        !expect(purchase.runner.companion_values[0] == 0U)) {
        return 9;
    }

    FieldActionPurchaseState cancelled_purchase{};
    cancelled_purchase.funds = 1000U;
    if (!expect(transactions.purchase(*weapon_offer, cost_context,
                                      cancelled_purchase, false) ==
                FieldActionApplicationResult::cancelled) ||
        !expect(cancelled_purchase.funds == 1000U) ||
        !expect(cancelled_purchase.runner.action_codes[0] == 0U)) {
        return 10;
    }

    FieldActionPurchaseState special_purchase{};
    special_purchase.funds = 100U;
    if (!expect(transactions.purchase(*other_offer, cost_context,
                                      special_purchase, true) ==
                FieldActionApplicationResult::applied) ||
        !expect(special_purchase.funds == 75U) ||
        !expect(special_purchase.runner.special_action_count == 1U)) {
        return 11;
    }

    RunnerActionRemovalState sale{};
    sale.runner.action_codes[0] = 2U;
    sale.runner.companion_values[0] = 0x44U;
    sale.runner_slot = 0U;
    sale.selected_runner_slot = 0U;
    sale.funds = 50U;
    const auto sale_result = transactions.sell(cost_context, sale, true);
    if (!expect(sale_result.result == ShopSaleOutcome::Result::removed) ||
        !expect(sale_result.quoted_refund == 180U) ||
        !expect(sale.funds == 230U) ||
        !expect(sale.runner.action_codes[0] == 0U) ||
        !expect(sale.runner.companion_values[0] == 0U) ||
        !expect(sale.menu_selection_cleared)) {
        return 12;
    }

    RunnerActionRemovalState protected_sale{};
    protected_sale.runner.action_codes[0] = 0x29U;
    protected_sale.runner_slot = 0U;
    protected_sale.funds = 50U;
    const auto protected_result =
        transactions.sell(cost_context, protected_sale, true);
    if (!expect(protected_result.result ==
                ShopSaleOutcome::Result::protected_action) ||
        !expect(protected_sale.funds == 50U) ||
        !expect(protected_sale.runner.action_codes[0] == 0x29U)) {
        return 13;
    }

    RunnerActionRemovalState cancelled_sale{};
    cancelled_sale.runner.action_codes[0] = 2U;
    cancelled_sale.runner_slot = 0U;
    cancelled_sale.funds = 50U;
    const auto cancelled_result =
        transactions.sell(cost_context, cancelled_sale, false);
    if (!expect(cancelled_result.result ==
                ShopSaleOutcome::Result::cancelled) ||
        !expect(cancelled_sale.funds == 50U) ||
        !expect(cancelled_sale.runner.action_codes[0] == 2U)) {
        return 14;
    }

    if (!expect(!ShopTransactionService::quote_sale(0U, cost_context)
                     .has_value()) ||
        !expect(!ShopTransactionService::quote_sale(51U, cost_context)
                     .has_value())) {
        return 15;
    }

    return 0;
}

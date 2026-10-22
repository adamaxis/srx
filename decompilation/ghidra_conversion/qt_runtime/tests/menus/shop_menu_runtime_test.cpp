#include "shop_menu_runtime.hpp"
#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <algorithm>
#include <optional>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    const FieldActionCostContext merlins_lore_discount{5U, 0x80U, 0U, 2U};
    if (!expect(apply_field_action_cost_discounts(
                    1000U, merlins_lore_discount) == 750U)) {
        return 50;
    }

    const auto ares = ShopId::field_store;
    const std::array<std::string, 2> confirmation_choice_ids{{"yes", "no"}};
    const auto snapshot = [](
        const std::string_view shop_id, const std::uint8_t menu_mode,
        const std::int8_t menu_result, const std::uint8_t cursor,
        const std::uint8_t panel, const std::uint8_t selected_action,
        const std::string_view event, const bool open = true) {
        scripting::ScriptShopMenuSnapshot value{};
        value.open = open;
        value.shop_id = shop_id;
        value.menu_mode = menu_mode;
        value.menu_result = menu_result;
        value.action_cursor = cursor;
        value.action_panel = panel;
        value.selected_action = selected_action;
        value.event = event;
        return value;
    };
    shadowrun::ghidra_conversion::scripting::ScriptShopDefinition
        ares_definition;
    ares_definition.id = "ares_weapon_emporium";
    ares_definition.menu_mode = 1U;
    ares_definition.menu_result = -1;
    ares_definition.groups = {
        {"weapons", {}, {{0x01U, 0x00U, 0x00U}, {0x02U, 0x00U, 0x00U},
                          {0x06U, 0x00U, 0x0FU}, {0x04U, 0x00U, 0x00U},
                          {0x05U, 0x00U, 0x0DU}, {0x0DU, 0x06U, 0x14U},
                          {0x0EU, 0x06U, 0x00U}}},
        {"armor", {}, {{0x2EU, 0x00U, 0x00U},
                        {0x2DU, 0x00U, 0x0CU}}},
        {"other_items", {}, {{0x45U, 0x00U, 0x19U},
                             {0x0FU, 0x06U, 0x00U},
                             {0x10U, 0x04U, 0x07U},
                             {0x11U, 0x04U, 0x07U}}},
        {"source_action_3", {}, {}}, {"source_action_4", {}, {}},
        {"sell", {}, {}}};
    ShopMenuController menu;
    if (!expect(menu.open(ares, ares_definition, confirmation_choice_ids)) ||
        !expect(menu.offer_count(ShopActionGroup::weapons) == 7U) ||
        !expect(menu.offer_count(ShopActionGroup::armor) == 2U) ||
        !expect(menu.offer_count(ShopActionGroup::other_items) == 4U) ||
        !expect(!menu.offer(ShopActionGroup::weapons, 7U).has_value())) {
        return 1;
    }
    ShopMenuController invalid_confirmation_menu;
    if (!expect(!invalid_confirmation_menu.open(
            ares, ares_definition,
            std::array<std::string, 2>{"no", "yes"}))) {
        return 18;
    }

    auto weapons_world_definition = ares_definition;
    weapons_world_definition.id = "weapons_world";
    weapons_world_definition.menu_mode = 0U;
    weapons_world_definition.groups[0].offers = {
        {0x03U, 0x00U, 0x0FU}, {0x04U, 0x00U, 0x0BU},
        {0x05U, 0x00U, 0x0EU}, {0x06U, 0x00U, 0x0CU},
        {0x0BU, 0x00U, 0x0FU}, {0x0EU, 0x06U, 0x00U}};
    weapons_world_definition.groups[1].offers = {
        {0x2BU, 0x00U, 0x00U}, {0x2CU, 0x00U, 0x00U}};
    weapons_world_definition.groups[2].offers = {
        {0x45U, 0x00U, 0x28U}, {0x0FU, 0x06U, 0x14U},
        {0x11U, 0x04U, 0x00U}, {0x10U, 0x04U, 0x00U}};
    ShopMenuController weapons_world_menu;
    if (!expect(weapons_world_menu.open(
            ShopId::field_store, weapons_world_definition,
            confirmation_choice_ids))) {
        return 28;
    }
    const auto weapons_world_first_offer =
        weapons_world_menu.offer(ShopActionGroup::weapons, 0U);
    const auto weapons_world_last_armor_offer =
        weapons_world_menu.offer(ShopActionGroup::armor, 1U);
    if (!expect(weapons_world_menu.menu_mode() == 0U) ||
        !expect(weapons_world_menu.menu_result() == -1) ||
        !expect(weapons_world_menu.selected_group() ==
                ShopActionGroup::weapons) ||
        !expect(weapons_world_menu.offer_count(ShopActionGroup::weapons) ==
                6U) ||
        !expect(weapons_world_menu.offer_count(ShopActionGroup::armor) ==
                2U) ||
        !expect(weapons_world_menu.offer_count(
                    ShopActionGroup::other_items) == 4U) ||
        !expect(weapons_world_first_offer.has_value() &&
                    weapons_world_first_offer->type == 0x03U &&
                    weapons_world_first_offer->quantity == 0x0FU) ||
        !expect(weapons_world_last_armor_offer.has_value() &&
                    weapons_world_last_armor_offer->type == 0x2CU)) {
        return 28;
    }

    auto merlins_lore_definition = ares_definition;
    merlins_lore_definition.id = "merlins_lore";
    merlins_lore_definition.menu_mode = 5U;
    merlins_lore_definition.menu_result = 0;
    merlins_lore_definition.groups = {
        {"magic_spells", {}, {}, {
            {0x07U, 0x06U, 1000U}, {0x08U, 0x06U, 1000U},
            {0x09U, 0x06U, 1000U}, {0x06U, 0x06U, 1000U},
            {0x0AU, 0x06U, 1000U}, {0x0BU, 0x06U, 1000U}}},
        {"magic_items", {}, {{0x12U, 0x0AU, 0x0DU}}, {}},
        {"magic_foci", {}, {{0x25U, 0x03U, 0x12U}}, {}},
        {"sell", {}, {}, {}}};
    ShopMenuController merlins_lore_menu;
    const bool merlins_lore_opened = merlins_lore_menu.open(
        ShopId::field_store, merlins_lore_definition,
        confirmation_choice_ids);
    const auto merlins_lore_last_progression =
        merlins_lore_menu.progression_offer(
            ShopActionGroup::magic_spells, 5U);
    if (!expect(merlins_lore_opened) ||
        !expect(merlins_lore_menu.menu_mode() == 5U) ||
        !expect(merlins_lore_menu.menu_result() == 0) ||
        !expect(merlins_lore_menu.selected_group() ==
                ShopActionGroup::magic_spells) ||
        !expect(merlins_lore_menu.progression_offer_count(
                    ShopActionGroup::magic_spells) == 6U) ||
        !expect(merlins_lore_last_progression.has_value() &&
                merlins_lore_last_progression->action_code == 0x0BU)) {
        return 29;
    }

    shadowrun::ghidra_conversion::scripting::ScriptShopDefinition
        medicine_definition;
    medicine_definition.id = "medicine_lodge";
    medicine_definition.menu_mode = 3U;
    medicine_definition.menu_result = 0;
    medicine_definition.groups = {
        {"magic_spells", {}, {}, {{0x03U, 0x06U, 1000U}}},
        {"magic_items", {}, {{0x12U, 0x0AU, 0x00U}}, {}},
        {"magic_foci", {}, {{0x1BU, 0x02U, 0x00U}}, {}},
        {"sell", {}, {}, {}}};
    ShopMenuController medicine_menu;
    if (!expect(medicine_menu.open(
                    ShopId::medicine_lodge, medicine_definition,
                    confirmation_choice_ids)) ||
        !expect(medicine_menu.menu_mode() == 3U) ||
        !expect(medicine_menu.source_state().action_selection.menu_result ==
                0) ||
        !expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 0U, 0U, 0U, "opened")) ==
                ShopMenuEvent::no_change) ||
        !expect(medicine_menu.selected_group_id() == "magic_spells") ||
        !expect(medicine_menu.progression_offer_count(
                    ShopActionGroup::magic_spells) == 1U) ||
        !expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 0U, 1U, 0U, "panel_changed")) ==
                ShopMenuEvent::panel_changed) ||
        !expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 0U, 1U, 0U,
                    "progression_selected")) ==
                ShopMenuEvent::progression_selected) ||
        !expect(medicine_menu.pending_action().has_value()) ||
        !expect(medicine_menu.pending_action()->progression_offer.has_value()) ||
        !expect(medicine_menu.pending_action()->progression_offer->action_code ==
                0x03U)) {
        return 22;
    }
    if (!expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 1U, 0U, 1U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(medicine_menu.selected_group_id() == "magic_items") ||
        !expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 0U, 1U, 1U, "panel_changed")) ==
                ShopMenuEvent::panel_changed) ||
        !expect(medicine_menu.apply_script_snapshot(snapshot(
                    "medicine_lodge", 3U, 0, 0U, 1U, 1U,
                    "purchase_selected")) ==
                ShopMenuEvent::purchase_selected) ||
        !expect(medicine_menu.pending_action()->offer->type == 0x12U)) {
        return 23;
    }
    medicine_menu.close();
    auto reordered_definition = ares_definition;
    std::swap(reordered_definition.groups[0],
              reordered_definition.groups[1]);
    ShopMenuController reordered_menu;
    if (!expect(reordered_menu.open(
                    ares, reordered_definition, confirmation_choice_ids)) ||
        !expect(reordered_menu.selected_group() ==
                ShopActionGroup::armor) ||
        !expect(reordered_menu.displayed_offer_group() ==
                ShopActionGroup::armor) ||
        !expect(reordered_menu.offer_count(
                    ShopActionGroup::armor) == 2U)) {
        return 17;
    }

    const auto weapon_offer = menu.offer(ShopActionGroup::weapons, 2U);
    const auto other_offer = menu.offer(ShopActionGroup::other_items, 0U);
    const auto armor_offer = menu.offer(ShopActionGroup::armor, 1U);
    if (!expect(weapon_offer.has_value() && weapon_offer->type == 6U &&
                weapon_offer->quantity == 15U) ||
        !expect(armor_offer.has_value() && armor_offer->type == 0x2DU &&
                armor_offer->quantity == 12U) ||
        !expect(other_offer.has_value() && other_offer->type == 0x45U &&
                other_offer->quantity == 25U)) {
        return 2;
    }

    if (!expect(menu.menu_mode() == 1U) ||
        !expect(menu.source_state().action_selection.menu_result == -1) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 1U, 0U, 1U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_group() == ShopActionGroup::armor) ||
        !expect(menu.displayed_offer_group() == ShopActionGroup::weapons) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 1U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(menu.offers_panel_active()) ||
        !expect(menu.selected_group() == ShopActionGroup::armor) ||
        !expect(menu.displayed_offer_group() == ShopActionGroup::armor) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 1U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected) ||
        !expect(menu.pending_action().has_value()) ||
        !expect(menu.pending_action()->offer->type == 0x2EU)) {
        return 3;
    }
    menu.finish_action();
    if (!expect(menu.selected_group() == ShopActionGroup::armor) ||
        !expect(menu.displayed_offer_group() == ShopActionGroup::armor)) {
        return 20;
    }

    ShopMenuController retained_display_menu;
    if (!expect(retained_display_menu.open(
                    ares, ares_definition, confirmation_choice_ids)) ||
        !expect(retained_display_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 1U, 0U, 1U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(retained_display_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 1U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(retained_display_menu.selected_group() ==
                ShopActionGroup::armor) ||
        !expect(retained_display_menu.displayed_offer_group() ==
                ShopActionGroup::armor) ||
        !expect(retained_display_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 0U, 0U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(!retained_display_menu.offers_panel_active()) ||
        !expect(retained_display_menu.selected_group() ==
                ShopActionGroup::weapons) ||
        !expect(retained_display_menu.displayed_offer_group() ==
                ShopActionGroup::armor)) {
        return 21;
    }

    if (!expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 0U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 1U, 0U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_cursor() == 2U) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 1U, 0U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected) ||
        !expect(menu.pending_action()->offer->type == 6U) ||
        !expect(menu.pending_action()->offer->quantity == 15U)) {
        return 4;
    }

    menu.finish_action();
    if (!expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 0U, 2U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 0U, 2U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_group() == ShopActionGroup::other_items) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected) ||
        !expect(menu.pending_action().has_value()) ||
        !expect(menu.pending_action()->group == ShopActionGroup::other_items) ||
        !expect(menu.pending_action()->offer->type == 0x45U)) {
        return 16;
    }

    menu.finish_action();
    if (!expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 5U, 0U, 5U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.selected_group() == ShopActionGroup::sell) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 5U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 1U, 5U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 1U, 5U,
                    "sale_selected")) == ShopMenuEvent::sale_selected) ||
        !expect(menu.pending_action()->cursor == 2U)) {
        return 6;
    }

    menu.finish_action();
    if (!expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 3U, 0U, 3U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 3U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 3U,
                    "source_action_boundary")) ==
                ShopMenuEvent::source_action_boundary)) {
        return 7;
    }
    menu.finish_action();
    if (!expect(menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 0U, 0U,
                    "returned_to_field", false)) ==
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

    const FieldActionCostContext medicine_cost_context{3U, 0U, 0U, 2U};
    const auto& medicine_item = medicine_definition.groups[1].offers[0];
    const auto& medicine_focus = medicine_definition.groups[2].offers[0];
    FieldActionPurchaseState medicine_transactions{};
    medicine_transactions.funds = 25000U;
    const auto medicine_item_result = transactions.purchase(
        FieldActionCostRecord{medicine_item.type, medicine_item.value,
                              medicine_item.quantity},
        medicine_cost_context, medicine_transactions, true);
    const auto medicine_focus_result = transactions.purchase(
        FieldActionCostRecord{medicine_focus.type, medicine_focus.value,
                              medicine_focus.quantity},
        medicine_cost_context, medicine_transactions, true);
    if (!expect(medicine_item_result == FieldActionApplicationResult::applied) ||
        !expect(medicine_focus_result == FieldActionApplicationResult::applied) ||
        !expect(medicine_transactions.runner.action_codes[0] == 0x13U) ||
        !expect(medicine_transactions.runner.companion_values[0] == 0x0AU) ||
        !expect(medicine_transactions.runner.action_codes[1] == 0x1CU) ||
        !expect(medicine_transactions.runner.companion_values[1] == 0x02U) ||
        !expect(medicine_transactions.funds == 20500U)) {
        return 24;
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
    const auto template_sale_quote = ShopTransactionService::quote_sale(
        2U, cost_context,
        [](const std::uint8_t catalog_index) {
            return catalog_index == 1U ? 175U : 0U;
        });
    if (!expect(template_sale_quote == std::optional<std::uint32_t>{180U})) {
        return 19;
    }

    ShopMenuController transaction_menu;
    if (!expect(transaction_menu.open(
                    ares, ares_definition, confirmation_choice_ids)) ||
        !expect(transaction_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 2U, 0U, 2U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(transaction_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(transaction_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected)) {
        return 17;
    }
    FieldActionPurchaseState live_special_purchase{};
    live_special_purchase.funds = 100U;
    const auto special_begin = transaction_menu.begin_purchase(
        *transaction_menu.pending_action()->offer, cost_context,
        live_special_purchase);
    if (!expect(special_begin ==
                ShopTransactionEvent::confirmation_opened) ||
        !expect(transaction_menu.transaction_confirmation_open()) ||
        !expect(transaction_menu.transaction_confirmation().selected_option ==
                1U) ||
        !expect(live_special_purchase.funds == 100U &&
                live_special_purchase.runner.special_action_count == 0U) ||
        !expect(transaction_menu.advance_transaction(
                    0x0100U, cost_context, live_special_purchase, sale) ==
                ShopTransactionEvent::confirmation_moved) ||
        !expect(transaction_menu.transaction_confirmation().selected_option ==
                0U) ||
        !expect(transaction_menu.advance_transaction(
                    0x1000U, cost_context, live_special_purchase, sale) ==
                ShopTransactionEvent::purchase_applied) ||
        !expect(live_special_purchase.funds == 75U &&
                live_special_purchase.runner.special_action_count == 1U) ||
        !expect(transaction_menu.selected_group() ==
                ShopActionGroup::other_items) ||
        !expect(transaction_menu.selected_cursor() == 0U) ||
        !expect(transaction_menu.offers_panel_active())) {
        return 18;
    }

    if (!expect(transaction_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected)) {
        return 19;
    }
    FieldActionPurchaseState default_no_purchase{};
    default_no_purchase.funds = 100U;
    if (!expect(transaction_menu.begin_purchase(
                    *transaction_menu.pending_action()->offer, cost_context,
                    default_no_purchase) ==
                ShopTransactionEvent::confirmation_opened) ||
        !expect(transaction_menu.advance_transaction(
                    0x1000U, cost_context, default_no_purchase, sale) ==
                ShopTransactionEvent::cancelled) ||
        !expect(default_no_purchase.funds == 100U &&
                default_no_purchase.runner.special_action_count == 0U)) {
        return 20;
    }

    if (!expect(transaction_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 2U,
                    "purchase_selected")) == ShopMenuEvent::purchase_selected)) {
        return 21;
    }
    FieldActionPurchaseState poor_purchase{};
    poor_purchase.funds = 0U;
    if (!expect(transaction_menu.begin_purchase(
                    *transaction_menu.pending_action()->offer, cost_context,
                    poor_purchase) == ShopTransactionEvent::message_shown) ||
        !expect(transaction_menu.transaction_waiting_for_event()) ||
        !expect(transaction_menu.transaction_message() ==
                ShopTransactionMessage::insufficient_funds) ||
        !expect(poor_purchase.funds == 0U) ||
        !expect(transaction_menu.advance_transaction(
                    0x0040U, cost_context, poor_purchase, sale) ==
                ShopTransactionEvent::no_change) ||
        !expect(transaction_menu.transaction_waiting_for_event()) ||
        !expect(transaction_menu.transaction_message() ==
                ShopTransactionMessage::insufficient_funds) ||
        !expect(transaction_menu.advance_transaction(
                    0x4000U, cost_context, poor_purchase, sale) ==
                ShopTransactionEvent::message_dismissed) ||
        !expect(transaction_menu.transaction_message() ==
                ShopTransactionMessage::none) ||
        !expect(transaction_menu.selected_group() ==
                ShopActionGroup::other_items)) {
        return 22;
    }

    ShopMenuController sale_menu;
    if (!expect(sale_menu.open(
                    ares, ares_definition, confirmation_choice_ids))) {
        return 23;
    }
    if (!expect(sale_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 5U, 0U, 5U,
                    "selection_moved")) == ShopMenuEvent::selection_moved) ||
        !expect(sale_menu.selected_group() == ShopActionGroup::sell) ||
        !expect(sale_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 5U,
                    "panel_changed")) == ShopMenuEvent::panel_changed) ||
        !expect(sale_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 5U,
                    "sale_selected")) == ShopMenuEvent::sale_selected)) {
        return 25;
    }
    RunnerActionRemovalState live_sale{};
    live_sale.runner.action_codes[0] = 2U;
    live_sale.runner_slot = 0U;
    live_sale.funds = 50U;
    if (!expect(sale_menu.begin_sale(cost_context, live_sale) ==
                ShopTransactionEvent::confirmation_opened) ||
        !expect(sale_menu.transaction_amount() == 180U) ||
        !expect(sale_menu.advance_transaction(
                    0x4000U, cost_context, live_special_purchase, live_sale) ==
                ShopTransactionEvent::cancelled) ||
        !expect(live_sale.funds == 50U &&
                live_sale.runner.action_codes[0] == 2U)) {
        return 26;
    }
    if (!expect(sale_menu.apply_script_snapshot(snapshot(
                    "ares_weapon_emporium", 1U, -1, 0U, 1U, 5U,
                    "sale_selected")) == ShopMenuEvent::sale_selected) ||
        !expect(sale_menu.begin_sale(cost_context, live_sale) ==
                ShopTransactionEvent::confirmation_opened) ||
        !expect(sale_menu.advance_transaction(
                    0x0100U, cost_context, live_special_purchase, live_sale) ==
                ShopTransactionEvent::confirmation_moved) ||
        !expect(sale_menu.advance_transaction(
                    0x1000U, cost_context, live_special_purchase, live_sale) ==
                ShopTransactionEvent::sale_applied) ||
        !expect(live_sale.funds == 230U &&
                live_sale.runner.action_codes[0] == 0U) ||
        !expect(sale_menu.selected_group() == ShopActionGroup::sell) ||
        !expect(sale_menu.selected_cursor() == 0U)) {
        return 27;
    }

    return 0;
}

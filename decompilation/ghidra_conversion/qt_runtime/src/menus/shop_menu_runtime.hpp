#pragma once

#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_removal_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class ShopId : std::uint8_t {
    ares_weapon_emporium,
};

enum class ShopActionGroup : std::int8_t {
    // These values are the source action indices in FFF0B8. ROM 0x56DC4
    // dispatches 0 to weapons, 1 to other items, and 2 to armor.
    weapons = 0,
    other_items = 1,
    armor = 2,
    source_action_3 = 3,
    source_action_4 = 4,
    sell = 5,
};

struct ShopActionRequest {
    ShopActionGroup group = ShopActionGroup::weapons;
    std::uint8_t cursor = 0;
    std::optional<FieldActionCostRecord> offer;
};

struct ShopSaleOutcome {
    enum class Result : std::uint8_t {
        removed,
        empty_slot,
        protected_action,
        cancelled,
        missing_item_record,
    } result = Result::empty_slot;
    std::uint32_t quoted_refund = 0;
};

enum class ShopTransactionMessage : std::uint8_t {
    none,
    no_inventory_slot,
    insufficient_funds,
    special_action_limit,
    protected_sale,
};

enum class ShopTransactionEvent : std::uint8_t {
    no_change,
    confirmation_opened,
    confirmation_moved,
    purchase_applied,
    sale_applied,
    cancelled,
    message_shown,
    message_dismissed,
    action_ignored,
    unresolved,
};

enum class ShopMenuEvent : std::uint8_t {
    no_change,
    selection_moved,
    panel_changed,
    purchase_selected,
    sale_selected,
    offer_unavailable,
    source_action_boundary,
    active_runner_change_requested,
    returned_to_field,
};

// A non-blocking controller for the source 0x56AC0 field service menu. It
// keeps the converted selection record authoritative and reports transaction
// requests to the field/session owner instead of maintaining another wallet
// or inventory copy.
class ShopMenuController final {
public:
    bool open(ShopId shop,
              const scripting::ScriptShopDefinition& definition,
              const std::array<std::string, 2>& confirmation_choice_ids);
    void close() noexcept;
    void finish_action() noexcept;

    ShopMenuEvent advance(std::uint16_t controller_events);

    bool is_open() const noexcept { return open_; }
    ShopId shop() const noexcept { return shop_; }
    std::uint8_t menu_mode() const noexcept { return menu_mode_; }
    const FieldModeServiceState& source_state() const noexcept {
        return source_state_;
    }
    bool offers_panel_active() const noexcept {
        return source_state_.action_selection.action_panel != 0;
    }
    bool transaction_confirmation_open() const noexcept {
        return transaction_confirmation_open_;
    }
    bool transaction_waiting_for_event() const noexcept {
        return transaction_waiting_for_event_;
    }
    const MenuConfirmationState& transaction_confirmation() const noexcept {
        return transaction_confirmation_;
    }
    ShopTransactionMessage transaction_message() const noexcept {
        return transaction_message_;
    }
    std::uint32_t transaction_amount() const noexcept {
        return transaction_amount_;
    }
    FieldActionApplicationResult last_purchase_result() const noexcept {
        return last_purchase_result_;
    }
    const ShopSaleOutcome& last_sale_outcome() const noexcept {
        return last_sale_outcome_;
    }
    ShopActionGroup selected_group() const noexcept;
    ShopActionGroup displayed_offer_group() const noexcept {
        return displayed_offer_group_;
    }
    std::uint8_t selected_cursor() const noexcept {
        return static_cast<std::uint8_t>(
            source_state_.action_selection.action_cursor);
    }
    const std::optional<ShopActionRequest>& pending_action() const noexcept {
        return pending_action_;
    }

    std::size_t offer_count(ShopActionGroup group) const noexcept;
    std::optional<FieldActionCostRecord> offer(
        ShopActionGroup group, std::size_t index) const noexcept;

    ShopTransactionEvent begin_purchase(
        const FieldActionCostRecord& selected_offer,
        const FieldActionCostContext& cost_context,
        FieldActionPurchaseState& state,
        const FieldActionBaseCostLookup& base_cost_lookup = {});
    ShopTransactionEvent begin_sale(
        const FieldActionCostContext& cost_context,
        RunnerActionRemovalState& state,
        const FieldActionBaseCostLookup& resale_base_lookup = {});
    ShopTransactionEvent advance_transaction(
        std::uint16_t controller_events,
        const FieldActionCostContext& cost_context,
        FieldActionPurchaseState& purchase_state,
        RunnerActionRemovalState& sale_state,
        const FieldActionBaseCostLookup& purchase_base_lookup = {},
        const FieldActionBaseCostLookup& resale_base_lookup = {});
    void clear_pending_action() noexcept;

private:
    enum class TransactionKind : std::uint8_t { none, purchase, sale };
    void reset_transaction() noexcept;
    void publish_selected_group() noexcept;

    ShopId shop_ = ShopId::ares_weapon_emporium;
    std::uint8_t menu_mode_ = 0;
    FieldModeServiceState source_state_{};
    std::optional<ShopActionRequest> pending_action_;
    std::array<std::string, 6> group_ids_{};
    ShopActionGroup displayed_offer_group_ = ShopActionGroup::weapons;
    std::array<std::string, 2> confirmation_choice_ids_{};
    std::array<std::vector<FieldActionCostRecord>, 6> offer_groups_{};
    MenuConfirmationState transaction_confirmation_{};
    TransactionKind transaction_kind_ = TransactionKind::none;
    ShopTransactionMessage transaction_message_ = ShopTransactionMessage::none;
    FieldActionApplicationResult last_purchase_result_ =
        FieldActionApplicationResult::cancelled;
    ShopSaleOutcome last_sale_outcome_{};
    std::uint32_t transaction_amount_ = 0U;
    bool transaction_confirmation_open_ = false;
    bool transaction_waiting_for_event_ = false;
    bool open_ = false;
};

// Transactions reuse the same typed source conversions as the field action
// menu. The caller supplies the owning session state and controls when a
// pending confirmation is accepted.
class ShopTransactionService final {
public:
    FieldActionApplicationResult purchase(
        const FieldActionCostRecord& offer,
        const FieldActionCostContext& cost_context,
        FieldActionPurchaseState& state,
        bool confirmed,
        const FieldActionBaseCostLookup& base_cost_lookup = {}) const;

    ShopSaleOutcome sell(
        const FieldActionCostContext& cost_context,
        RunnerActionRemovalState& state,
        bool confirmed,
        const FieldActionBaseCostLookup& resale_base_lookup = {}) const;

    static std::optional<std::uint32_t> quote_sale(
        std::uint8_t item_code,
        const FieldActionCostContext& cost_context,
        const FieldActionBaseCostLookup& resale_base_lookup = {});
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

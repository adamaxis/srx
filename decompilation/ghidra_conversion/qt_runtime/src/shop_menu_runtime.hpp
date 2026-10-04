#pragma once

#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_removal_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

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
    bool open(ShopId shop) noexcept;
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
    ShopActionGroup selected_group() const noexcept;
    std::uint8_t selected_cursor() const noexcept {
        return static_cast<std::uint8_t>(
            source_state_.action_selection.action_cursor);
    }
    const std::optional<ShopActionRequest>& pending_action() const noexcept {
        return pending_action_;
    }

    static std::size_t offer_count(ShopId shop,
                                   ShopActionGroup group) noexcept;
    static std::optional<FieldActionCostRecord> offer(
        ShopId shop, ShopActionGroup group, std::size_t index) noexcept;

private:
    void publish_selected_group() noexcept;

    ShopId shop_ = ShopId::ares_weapon_emporium;
    std::uint8_t menu_mode_ = 0;
    FieldModeServiceState source_state_{};
    std::optional<ShopActionRequest> pending_action_;
    bool open_ = false;
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

// Transactions reuse the same typed source conversions as the field action
// menu. The caller supplies the owning session state and controls when a
// pending confirmation is accepted.
class ShopTransactionService final {
public:
    FieldActionApplicationResult purchase(
        const FieldActionCostRecord& offer,
        const FieldActionCostContext& cost_context,
        FieldActionPurchaseState& state,
        bool confirmed) const;

    ShopSaleOutcome sell(
        const FieldActionCostContext& cost_context,
        RunnerActionRemovalState& state,
        bool confirmed) const;

    static std::optional<std::uint32_t> quote_sale(
        std::uint8_t item_code,
        const FieldActionCostContext& cost_context) noexcept;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime

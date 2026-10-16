#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldQuantityPurchaseState {
    std::uint8_t category = 0;             // source FFF0D0
    std::uint8_t quantity = 0;             // source FFF0D6
    std::uint32_t preview_cost = 0;        // source FFF0BE
    std::uint32_t funds = 0;               // source FFFBFE
    std::int8_t availability_marker = 0;  // source FFF0B3
    std::int8_t service_result = 0;        // source FFF112

    // Category 0 writes FFFBBE. Categories 1..4 write the source's
    // FFFBBE+7..FFFBBE+10 byte positions;
    // these are the five quantity slots selected by the source offset rule.
    std::array<std::uint8_t, 5> purchased_quantities{};
};

struct FieldQuantityPurchaseCallbacks {
    // 0x577D0 enters the quantity-purchase presentation with grid variant B,
    // the purchase title, quantity, and preview-cost values in that order.
    std::function<void()> write_quantity_purchase_grid;
    std::function<void()> render_purchase_title;
    std::function<void()> render_quantity;
    std::function<void()> render_preview_cost;
    std::function<void()> show_insufficient_funds;
    std::function<void()> wait_for_controller_event;

    // 0x57B8C is the shared two-choice confirmation prompt. An absent
    // callback accepts the purchase for non-interactive conversion tests.
    std::function<bool()> confirm_purchase;
};

enum class FieldQuantityPurchaseResult {
    applied,
    invalid_category,
    unavailable,
    insufficient_funds,
    cancelled,
};

// ROM branch target 0x577BA. The original Ghidra export did not emit this
// branch target as a separate function even though 0x56DC4 calls it as the
// quantity-purchase path. The host result preserves the source service
// marker, funds, and category-slot writes without retaining FFFBBE addresses.
FieldQuantityPurchaseResult apply_field_quantity_purchase(
    FieldQuantityPurchaseState& state,
    const FieldQuantityPurchaseCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

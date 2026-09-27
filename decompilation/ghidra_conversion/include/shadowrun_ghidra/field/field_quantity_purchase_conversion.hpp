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

    // Category 0 writes FFFBBE. Categories 1..4 write the source's
    // FFFBBE+7..FFFBBE+10 byte positions;
    // these are the five quantity slots selected by the source offset rule.
    std::array<std::uint8_t, 5> purchased_quantities{};
};

struct FieldQuantityPurchaseCallbacks {
    // 0x57C46 refreshes the field-service detail panel.
    std::function<void()> refresh_detail_panel;
    std::function<void()> show_insufficient_funds;

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
// category-5 field-service path.
FieldQuantityPurchaseResult apply_field_quantity_purchase(
    FieldQuantityPurchaseState& state,
    const FieldQuantityPurchaseCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// The field-service purchase path reads one selected category, a requested
// quantity, and four progress counters from the source's FFFBBE block.  The
// host model exposes those values as state instead of retaining the source
// RAM layout.
struct FieldResourcePurchaseState {
    std::uint8_t category = 0;             // source FFF0D0
    std::uint8_t requested_quantity = 0;   // source FFFBBE
    std::uint16_t progress_category_0 = 0; // source FFFBC0
    std::uint16_t progress_category_1 = 0; // source FFFBC2
    std::uint8_t progress_category_2 = 0;  // source FFFBC4
    std::uint8_t progress_category_3 = 0;  // source FFFBC5
    std::uint32_t funds = 0;               // source FFFBFE
    std::uint32_t preview_cost = 0;        // source FFF0BE
};

struct FieldResourcePurchaseCallbacks {
    // 0x57C46 refreshes the selected field-service detail panel before the
    // availability decision.
    std::function<void()> refresh_detail_panel;

    // These callbacks represent the three user-visible early exits in the
    // routine.  The exact text remains source-table/UI work; the behavior is
    // already distinct in the ROM by its message/layout addresses.
    std::function<void()> show_category_satisfied;
    std::function<void()> show_category_limit;
    std::function<void()> show_insufficient_funds;

    // 0x57B8C is the shared two-choice confirmation prompt.  An absent
    // callback accepts the purchase for non-interactive conversion tests.
    std::function<bool()> confirm_purchase;
};

enum class FieldResourcePurchaseResult {
    applied,
    invalid_category,
    category_satisfied,
    category_limit,
    insufficient_funds,
    cancelled,
};

// ROM 0x5766A.  Apply the selected field-service resource purchase when the
// category's progress is below the requested threshold.  The four costs are
// the words read from source table 0x1AC8E0, in category order.
FieldResourcePurchaseResult apply_field_resource_purchase(
    FieldResourcePurchaseState& state,
    const std::array<std::uint16_t, 4>& category_costs,
    const FieldResourcePurchaseCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

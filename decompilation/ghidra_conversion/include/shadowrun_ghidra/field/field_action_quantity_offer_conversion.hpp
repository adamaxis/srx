#pragma once

#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// ROM 0x1AB190 is the six-byte next-rating table used by the MPCP branch of
// the quantity panel. It is extracted source data, not a runtime address.
constexpr std::array<std::uint8_t, 6> kMasterPersonaNextRatings{
    3U, 4U, 6U, 8U, 10U, 12U};

struct FieldActionQuantityOfferState {
    std::uint8_t category = 0; // FFF0D0: 0 = MPCP, 1..4 = attributes
    std::uint8_t quantity = 0; // FFF0D6, retained when a limit blocks advance
    std::uint32_t cost = 0;    // FFF0BE, written by 0x5787C
    std::int8_t availability_marker = 0; // FFF0B3

    std::uint8_t matrix_capacity = 0; // FFFBBE + 0
    std::uint8_t master_persona_level = 0; // FFFBCB
    // FFFBC6..FFFBC9, selected by category 1..4.
    std::array<std::uint8_t, 4> attribute_ratings{};
};

enum class FieldActionQuantityOfferKind {
    offered,
    master_persona_limit,
    attribute_limit,
    invalid_category,
    missing_source_data,
};

struct FieldActionQuantityOfferResult {
    FieldActionQuantityOfferKind kind =
        FieldActionQuantityOfferKind::invalid_category;
    std::uint8_t category = 0;
    std::uint8_t quantity = 0;
    std::uint32_t cost = 0;
    std::int8_t availability_marker = 0;
};

struct FieldActionQuantityOfferCallbacks {
    // Limit notices occur before the common category heading.
    std::function<void()> render_master_persona_limit_notice;
    std::function<void()> render_attribute_limit_notice;

    // 0x58394 renders one of these category-specific label/name sequences.
    std::function<void()> render_master_persona_label;
    std::function<void()> render_master_persona_name;
    std::function<void()> render_attribute_label;
    std::function<void(std::uint8_t)> render_attribute_name;
    std::function<void(std::uint8_t)> render_category_heading;

    // Available upgrades render the shared upgrade label, quantity, and cost.
    std::function<void()> render_upgrade_label;
    std::function<void(std::uint8_t)> render_quantity;
    std::function<void(std::uint32_t)> render_cost;

    // The final source grid is written for both available and limit outcomes;
    // non-MPCP categories then finish through the common tail text.
    std::function<void(std::uint8_t)> write_quantity_grid;
    std::function<void()> render_tail_text;
};

// ROM 0x58394, the positive-result selection-5 quantity panel embedded in
// 0x57C7A. Select the next MPCP/attribute rating, preserve the source limit
// markers and stale-quantity behavior, calculate 0x5787C's exact cost, and
// expose the source presentation order as typed callbacks.
FieldActionQuantityOfferResult render_field_action_quantity_offer(
    FieldActionQuantityOfferState& state,
    const FieldActionQuantityOfferCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion

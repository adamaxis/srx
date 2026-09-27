#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::size_t kFixedOfferCount = 18U;
constexpr std::size_t kMenuModeOfferCount = 8U;
constexpr std::uint8_t kFirstDynamicMenuMode = 3U;
constexpr std::uint8_t kLastDynamicMenuMode = 5U;

bool offer_is_available(const FieldActionProgressionOffer& offer) {
    // 0x56E5A and 0x57D7E use a signed-byte BPL test on the offer marker.
    return static_cast<std::int8_t>(offer.action_code) >= 0;
}

} // namespace

std::optional<FieldActionProgressionOffer>
select_field_action_progression_offer(
    const FieldActionProgressionTables& tables,
    const std::uint8_t menu_mode,
    const std::int8_t saved_selection,
    const std::int8_t primary_cursor) {
    if (primary_cursor < 0) {
        return std::nullopt;
    }

    const auto cursor = static_cast<std::size_t>(primary_cursor);
    if (saved_selection == 0) {
        if (menu_mode < kFirstDynamicMenuMode ||
            menu_mode > kLastDynamicMenuMode ||
            cursor >= kMenuModeOfferCount) {
            return std::nullopt;
        }
        return tables.saved_selection_zero_by_menu_mode
            [static_cast<std::size_t>(menu_mode - kFirstDynamicMenuMode)][cursor];
    }

    if (saved_selection == 1) {
        if (cursor >= kFixedOfferCount) {
            return std::nullopt;
        }
        return tables.saved_selection_one[cursor];
    }

    if (cursor >= kFixedOfferCount) {
        return std::nullopt;
    }
    return tables.saved_selection_two_or_more[cursor];
}

FieldActionProgressionPurchaseResult purchase_field_action_progression(
    const FieldActionProgressionTables& tables,
    FieldActionProgressionPurchaseState& state,
    const FieldActionProgressionPurchaseCallbacks& callbacks) {
    if (state.saved_selection == 3) {
        // The dispatcher sends this selection to the separate removal leaf.
        return FieldActionProgressionPurchaseResult::not_neutral_selection;
    }

    if (state.runner_archetype_status < 2) {
        if (callbacks.show_runner_unavailable) {
            callbacks.show_runner_unavailable();
        }
        return FieldActionProgressionPurchaseResult::runner_unavailable;
    }

    if (callbacks.render_action_panel) {
        callbacks.render_action_panel();
    }

    const auto offer = select_field_action_progression_offer(
        tables, state.menu_mode, state.saved_selection, state.primary_cursor);
    if (!offer) {
        return FieldActionProgressionPurchaseResult::invalid_offer_selection;
    }
    if (!offer_is_available(*offer)) {
        state.service_result = -1;
        return FieldActionProgressionPurchaseResult::offer_unavailable;
    }

    auto existing = state.ledger.records.end();
    for (auto iterator = state.ledger.records.begin();
         iterator != state.ledger.records.end(); ++iterator) {
        if (iterator->action_code == offer->action_code) {
            existing = iterator;
            break;
        }
    }

    std::uint32_t base_cost = offer->base_cost;
    std::uint16_t next_progression_level = 1U;
    if (existing != state.ledger.records.end()) {
        if (existing->progression_level == 6U) {
            if (callbacks.show_progression_maxed) {
                callbacks.show_progression_maxed();
            }
            return FieldActionProgressionPurchaseResult::progression_maxed;
        }

        // The source compares the current level against +0x7F with a signed
        // byte branch.  Valid game levels are small, but keeping the signed
        // comparison here preserves the source's limit behavior.
        if (static_cast<std::int8_t>(existing->progression_level) >=
            state.runner_progression_limit) {
            if (callbacks.show_progression_limit) {
                callbacks.show_progression_limit();
            }
            return FieldActionProgressionPurchaseResult::progression_limit;
        }

        next_progression_level = static_cast<std::uint16_t>(
            existing->progression_level) + 1U;
        base_cost *= next_progression_level;
    }

    auto cost_context = state.cost_context;
    // 0x57A6E reads FFF0B6 directly for the mode-specific discount branch.
    cost_context.mode = state.menu_mode;
    state.selected_action_code = offer->action_code;
    state.pending_cost = apply_field_action_cost_discounts(
        base_cost, cost_context);

    if (state.pending_cost > state.funds) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return FieldActionProgressionPurchaseResult::insufficient_funds;
    }

    if (callbacks.confirm_purchase && !callbacks.confirm_purchase()) {
        return FieldActionProgressionPurchaseResult::cancelled;
    }

    state.funds -= state.pending_cost;
    if (existing == state.ledger.records.end()) {
        // A newly selected record starts at zero in the source's working
        // record area, then 0x56E5A stores the action and increments it.
        state.ledger.records.push_back(
            {state.selected_action_code, 1U});
        ++state.ledger.source_record_count_word;
    }
    else {
        if (static_cast<std::int8_t>(existing->progression_level) <= 0) {
            ++state.ledger.source_record_count_word;
        }
        existing->action_code = state.selected_action_code;
        existing->progression_level = static_cast<std::uint8_t>(
            existing->progression_level + 1U);
    }

    return FieldActionProgressionPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion

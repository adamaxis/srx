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

FieldActionProgressionQuoteOutcome prepare_field_action_progression_quote(
    const FieldActionProgressionTables& tables,
    FieldActionProgressionPurchaseState& state) {
    const auto offer = select_field_action_progression_offer(
        tables, state.menu_mode, state.saved_selection, state.primary_cursor);
    if (!offer) {
        return {FieldActionProgressionQuoteResult::invalid_offer_selection,
                std::nullopt};
    }
    if (!offer_is_available(*offer)) {
        return {FieldActionProgressionQuoteResult::offer_unavailable,
                std::nullopt};
    }

    std::optional<std::size_t> existing_record_index;
    for (std::size_t index = 0; index < state.ledger.records.size(); ++index) {
        if (state.ledger.records[index].action_code == offer->action_code) {
            existing_record_index = index;
            break;
        }
    }

    std::uint32_t base_cost = offer->base_cost;
    std::uint16_t next_progression_level = 1U;
    if (existing_record_index) {
        const auto& existing = state.ledger.records[*existing_record_index];
        if (existing.progression_level == 6U) {
            return {FieldActionProgressionQuoteResult::progression_maxed,
                    std::nullopt};
        }

        // The source compares the current level against +0x7F with a signed
        // byte branch. Valid game levels are small, but keeping the signed
        // comparison here preserves the source's limit behavior.
        if (static_cast<std::int8_t>(existing.progression_level) >=
            state.runner_progression_limit) {
            return {FieldActionProgressionQuoteResult::progression_limit,
                    std::nullopt};
        }

        next_progression_level = static_cast<std::uint16_t>(
            existing.progression_level) + 1U;
        base_cost *= next_progression_level;
    }

    auto cost_context = state.cost_context;
    // 0x57A6E reads FFF0B6 directly for the mode-specific discount branch.
    cost_context.mode = state.menu_mode;
    state.selected_action_code = offer->action_code;
    state.pending_cost = apply_field_action_cost_discounts(
        base_cost, cost_context);

    return {
        FieldActionProgressionQuoteResult::quoted,
        FieldActionProgressionQuote{
            *offer, existing_record_index, next_progression_level,
            state.pending_cost}};
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

    const auto quote = prepare_field_action_progression_quote(tables, state);
    switch (quote.result) {
    case FieldActionProgressionQuoteResult::invalid_offer_selection:
        return FieldActionProgressionPurchaseResult::invalid_offer_selection;
    case FieldActionProgressionQuoteResult::offer_unavailable:
        state.service_result = -1;
        return FieldActionProgressionPurchaseResult::offer_unavailable;
    case FieldActionProgressionQuoteResult::progression_maxed:
        if (callbacks.show_progression_maxed) {
            callbacks.show_progression_maxed();
        }
        return FieldActionProgressionPurchaseResult::progression_maxed;
    case FieldActionProgressionQuoteResult::progression_limit:
        if (callbacks.show_progression_limit) {
            callbacks.show_progression_limit();
        }
        return FieldActionProgressionPurchaseResult::progression_limit;
    case FieldActionProgressionQuoteResult::quoted:
        break;
    }

    if (!quote.quote) {
        return FieldActionProgressionPurchaseResult::invalid_offer_selection;
    }

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
    if (!quote.quote->existing_record_index) {
        // A newly selected record starts at zero in the source's working
        // record area, then 0x56E5A stores the action and increments it.
        state.ledger.records.push_back(
            {state.selected_action_code, 1U});
        ++state.ledger.source_record_count_word;
    }
    else {
        auto& existing =
            state.ledger.records[*quote.quote->existing_record_index];
        if (static_cast<std::int8_t>(existing.progression_level) <= 0) {
            ++state.ledger.source_record_count_word;
        }
        existing.action_code = state.selected_action_code;
        existing.progression_level = static_cast<std::uint8_t>(
            existing.progression_level + 1U);
    }

    return FieldActionProgressionPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion

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

bool runner_action_progression_is_blocked(const std::int8_t status) {
    const auto delta = static_cast<std::int8_t>(static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(status) - 2U));
    return delta >= 0;
}

std::size_t progression_record_scan_count(
    const FieldActionProgressionLedger& ledger) {
    // The source uses DBF with FFF9FA as its initial value, so a header word
    // of zero still scans one two-byte record.  The word is therefore the
    // last scanned-record index, not a vector length.
    return static_cast<std::size_t>(ledger.source_record_count_word) + 1U;
}

FieldActionProgressionGlyphVariant glyph_variant_for_action(
    const std::uint8_t action_code) {
    // 0x57EBC selects the source's low/high glyph transfer variant at action
    // code six.  The host exposes the visual variant instead of its VDP word.
    return action_code < 6U
               ? FieldActionProgressionGlyphVariant::low_action
               : FieldActionProgressionGlyphVariant::high_action;
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
    if (saved_selection == 1) {
        if (cursor >= kFixedOfferCount) {
            return std::nullopt;
        }
        return tables.saved_selection_one[cursor];
    }

    // 0x56E98 compares the saved-selection byte with one and takes BMI for
    // the dynamic family. Preserve the wrapped N flag instead of treating
    // only literal zero as that source branch.
    const auto selection_byte = static_cast<std::uint8_t>(saved_selection);
    const bool compare_is_negative =
        (static_cast<std::uint8_t>(selection_byte - 1U) & 0x80U) != 0U;
    if (compare_is_negative) {
        if (menu_mode < kFirstDynamicMenuMode ||
            menu_mode > kLastDynamicMenuMode ||
            cursor >= kMenuModeOfferCount) {
            return std::nullopt;
        }
        return tables.saved_selection_zero_by_menu_mode
            [static_cast<std::size_t>(menu_mode - kFirstDynamicMenuMode)][cursor];
    }

    if (cursor >= kFixedOfferCount) {
        return std::nullopt;
    }
    return tables.saved_selection_two_or_more[cursor];
}

FieldActionProgressionQuoteOutcome prepare_field_action_progression_quote(
    const FieldActionProgressionTables& tables,
    FieldActionProgressionPurchaseState& state,
    const FieldActionProgressionQuoteCallbacks& callbacks) {
    if (runner_action_progression_is_blocked(state.runner_archetype_status)) {
        if (callbacks.render_runner_unavailable_title) {
            callbacks.render_runner_unavailable_title();
        }
        return {FieldActionProgressionQuoteResult::runner_unavailable,
                std::nullopt};
    }

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

    if (callbacks.render_offer_heading) {
        callbacks.render_offer_heading();
    }
    if (callbacks.render_action_name) {
        callbacks.render_action_name(offer->action_code);
    }
    state.selected_action_code = offer->action_code;

    std::optional<std::size_t> existing_record_index;
    const auto scanned_record_count =
        progression_record_scan_count(state.ledger);
    for (std::size_t index = 0;
         index < scanned_record_count && index < state.ledger.records.size();
         ++index) {
        if (state.ledger.records[index].action_code == offer->action_code) {
            existing_record_index = index;
            break;
        }
    }

    const auto pending_record_index =
        existing_record_index.value_or(scanned_record_count);
    if (pending_record_index >= state.ledger.records.size()) {
        // The ROM always has a contiguous record slot after the DBF scan. A
        // host extraction that omitted that slot is incomplete; do not invent
        // one while pretending the quote was source-derived.
        state.pending_record_index.reset();
        return {FieldActionProgressionQuoteResult::missing_record_slot,
                std::nullopt};
    }
    state.pending_record_index = pending_record_index;

    std::uint32_t base_cost = offer->base_cost;
    std::uint16_t next_progression_level = 1U;
    if (existing_record_index) {
        const auto& existing = state.ledger.records[*existing_record_index];
        // The source compares the current level against +0x7F with a signed
        // byte branch. Valid game levels are small, but keeping the signed
        // comparison here preserves the source's limit behavior.
        const auto level_delta = static_cast<std::int8_t>(
            static_cast<std::uint8_t>(existing.progression_level) -
            static_cast<std::uint8_t>(state.runner_progression_limit));
        if (level_delta >= 0) {
            if (callbacks.render_progression_limit) {
                callbacks.render_progression_limit();
            }
            if (callbacks.transfer_action_glyph && state.saved_selection == 0) {
                callbacks.transfer_action_glyph(
                    offer->action_code,
                    glyph_variant_for_action(offer->action_code));
            }
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

    if (callbacks.render_cost_heading) {
        callbacks.render_cost_heading();
    }
    if (callbacks.render_next_level_label) {
        callbacks.render_next_level_label();
    }
    if (callbacks.render_next_level) {
        callbacks.render_next_level(next_progression_level);
    }
    if (callbacks.render_discounted_cost) {
        callbacks.render_discounted_cost(state.pending_cost);
    }
    if (callbacks.transfer_action_glyph && state.saved_selection == 0) {
        callbacks.transfer_action_glyph(
            offer->action_code, glyph_variant_for_action(offer->action_code));
    }

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
    if (state.saved_selection == 3U) {
        // 0x56E5A diverts exactly saved selection 3 to the separate removal
        // leaf at 0x56F4A. Saved selections 0, 1, and 2 all continue through
        // the progression offer families and the shared application path.
        return FieldActionProgressionPurchaseResult::removal_selection;
    }

    if (runner_action_progression_is_blocked(state.runner_archetype_status)) {
        if (callbacks.write_action_feedback_grid) {
            callbacks.write_action_feedback_grid();
        }
        if (callbacks.show_runner_unavailable) {
            callbacks.show_runner_unavailable();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return FieldActionProgressionPurchaseResult::runner_unavailable;
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

    if (!state.pending_record_index ||
        *state.pending_record_index >= state.ledger.records.size()) {
        return FieldActionProgressionPurchaseResult::missing_record_slot;
    }

    auto& pending_record = state.ledger.records[*state.pending_record_index];
    if (pending_record.progression_level == 6U) {
        // 0x56E5A checks FFF0C2[1] and returns before the funds test or any
        // record write. The level-six behavior is a purchase guard, not a
        // 0x57D7E quote result.
        return FieldActionProgressionPurchaseResult::progression_maxed;
    }

    if (state.pending_cost > state.funds) {
        if (callbacks.write_action_feedback_grid) {
            callbacks.write_action_feedback_grid();
        }
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return FieldActionProgressionPurchaseResult::insufficient_funds;
    }

    state.funds -= state.pending_cost;
    if (static_cast<std::int8_t>(pending_record.progression_level) <= 0) {
        // A zero-level slot is promoted into the active record count by the
        // source before its action/level bytes are written.
        ++state.ledger.source_record_count_word;
    }
    pending_record.action_code = state.selected_action_code;
    pending_record.progression_level = static_cast<std::uint8_t>(
        pending_record.progression_level + 1U);

    return FieldActionProgressionPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion

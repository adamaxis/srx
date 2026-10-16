#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionProgressionTables tables{};
    // These records mirror the first three dynamic blocks at 0x1AAF40:
    // menu mode 3/category 2 is action 0x0C with a 1000-unit base cost.
    tables.saved_selection_zero_by_menu_mode[0][2] = {0x0CU, 0x06U, 1000U};
    tables.saved_selection_one[0] = {0x12U, 0x0AU, 0x0026U};
    tables.saved_selection_two_or_more[0] = {0x1BU, 0x02U, 0x001BU};

    FieldActionProgressionPurchaseState state{};
    state.menu_mode = 3U;
    state.saved_selection = 0;
    state.primary_cursor = 2;
    state.runner_archetype_status = 1;
    state.runner_progression_limit = 6;
    state.funds = 5000U;
    // FFF9FA starts with a last-record index of zero, so 0x57D7E scans slot
    // zero and publishes slot one as the first insertion target.
    state.ledger.records = {{0U, 0U}, {0U, 0U}};

    std::vector<std::string> quote_events;
    std::vector<std::pair<std::uint16_t, std::uint32_t>> quote_values;
    std::vector<std::pair<std::uint8_t, FieldActionProgressionGlyphVariant>>
        glyph_transfers;
    const FieldActionProgressionQuoteCallbacks quote_callbacks{
        [&] { quote_events.emplace_back("runner-unavailable"); },
        [&] { quote_events.emplace_back("offer-heading"); },
        [&](const std::uint8_t) { quote_events.emplace_back("action-name"); },
        [&] { quote_events.emplace_back("progression-limit"); },
        [&] { quote_events.emplace_back("cost-heading"); },
        [&] { quote_events.emplace_back("next-level-label"); },
        [&](const std::uint16_t level) {
            quote_events.emplace_back("next-level");
            quote_values.emplace_back(level, 0U);
        },
        [&](const std::uint32_t cost) {
            quote_events.emplace_back("discounted-cost");
            quote_values.back().second = cost;
        },
        [&](const std::uint8_t action_code,
            const FieldActionProgressionGlyphVariant variant) {
            quote_events.emplace_back("action-glyph");
            glyph_transfers.emplace_back(action_code, variant);
        },
    };

    const auto first_quote = prepare_field_action_progression_quote(
        tables, state, quote_callbacks);
    if (!expect(first_quote.result == FieldActionProgressionQuoteResult::quoted &&
                first_quote.quote.has_value() &&
                first_quote.quote->existing_record_index == std::nullopt &&
                state.pending_record_index == std::optional<std::size_t>(1U) &&
                state.pending_cost == 1000U && state.selected_action_code == 0x0CU &&
                quote_events ==
                    std::vector<std::string>{
                        "offer-heading", "action-name", "cost-heading",
                        "next-level-label", "next-level", "discounted-cost",
                        "action-glyph"} &&
                quote_values ==
                    std::vector<std::pair<std::uint16_t, std::uint32_t>>{{1U,
                                                                          1000U}} &&
                glyph_transfers.size() == 1U &&
                glyph_transfers[0].first == 0x0CU &&
                glyph_transfers[0].second ==
                    FieldActionProgressionGlyphVariant::high_action)) {
        return 1;
    }

    std::vector<std::string> purchase_events;
    const FieldActionProgressionPurchaseCallbacks purchase_callbacks{
        [&] { purchase_events.emplace_back("grid"); },
        [&] { purchase_events.emplace_back("unavailable"); },
        [&] { purchase_events.emplace_back("insufficient"); },
        [&] { purchase_events.emplace_back("wait"); },
    };

    // 0x56E5A consumes the state published by the preview; it does not call
    // the preview again and does not ask for a second confirmation.
    if (!expect(purchase_field_action_progression(
                    tables, state, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 4000U && state.ledger.source_record_count_word == 1U &&
                state.ledger.records[1].action_code == 0x0CU &&
                state.ledger.records[1].progression_level == 1U &&
                purchase_events.empty())) {
        return 2;
    }

    // Re-rendering the same selection now finds the second slot and quotes
    // the next level at twice the base cost.
    quote_events.clear();
    quote_values.clear();
    glyph_transfers.clear();
    const auto second_quote = prepare_field_action_progression_quote(
        tables, state, quote_callbacks);
    if (!expect(second_quote.result == FieldActionProgressionQuoteResult::quoted &&
                second_quote.quote->existing_record_index ==
                    std::optional<std::size_t>(1U) &&
                state.pending_record_index == std::optional<std::size_t>(1U) &&
                state.pending_cost == 2000U &&
                quote_values ==
                    std::vector<std::pair<std::uint16_t, std::uint32_t>>{{2U,
                                                                          2000U}})) {
        return 3;
    }
    if (!expect(purchase_field_action_progression(
                    tables, state, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 2000U &&
                state.ledger.records[1].progression_level == 2U &&
                state.ledger.source_record_count_word == 1U &&
                purchase_events.empty())) {
        return 4;
    }

    state.runner_progression_limit = 2;
    quote_events.clear();
    quote_values.clear();
    glyph_transfers.clear();
    const auto limited_quote = prepare_field_action_progression_quote(
        tables, state, quote_callbacks);
    if (!expect(limited_quote.result ==
                    FieldActionProgressionQuoteResult::progression_limit &&
                !limited_quote.quote.has_value() &&
                quote_events == std::vector<std::string>{
                                     "offer-heading", "action-name",
                                     "progression-limit", "action-glyph"})) {
        return 5;
    }

    // Level six is not rejected by 0x57D7E when the runner limit permits the
    // preview.  0x56E5A is the routine that refuses to mutate that slot.
    state.runner_progression_limit = 7;
    state.ledger.records[1].progression_level = 6U;
    quote_events.clear();
    quote_values.clear();
    glyph_transfers.clear();
    const auto maxed_quote = prepare_field_action_progression_quote(
        tables, state, quote_callbacks);
    if (!expect(maxed_quote.result == FieldActionProgressionQuoteResult::quoted &&
                maxed_quote.quote->next_progression_level == 7U &&
                state.pending_cost == 7000U &&
                purchase_field_action_progression(
                    tables, state, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::progression_maxed &&
                state.funds == 2000U && purchase_events.empty())) {
        return 6;
    }

    FieldActionProgressionPurchaseState poor{};
    poor.menu_mode = 3U;
    poor.saved_selection = 0;
    poor.primary_cursor = 2;
    poor.runner_archetype_status = 1;
    poor.runner_progression_limit = 6;
    poor.funds = 999U;
    poor.ledger.records = {{0U, 0U}, {0U, 0U}};
    quote_events.clear();
    quote_values.clear();
    glyph_transfers.clear();
    if (!expect(prepare_field_action_progression_quote(
                    tables, poor, quote_callbacks)
                    .result == FieldActionProgressionQuoteResult::quoted &&
                purchase_field_action_progression(
                    tables, poor, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::insufficient_funds &&
                poor.ledger.records[1].progression_level == 0U &&
                poor.funds == 999U &&
                purchase_events == std::vector<std::string>{
                                        "grid", "insufficient", "wait"})) {
        return 7;
    }

    FieldActionProgressionPurchaseState selection_one{};
    selection_one.menu_mode = 3U;
    selection_one.saved_selection = 1;
    selection_one.primary_cursor = 0;
    selection_one.runner_archetype_status = 1;
    selection_one.runner_progression_limit = 6;
    selection_one.funds = 1000U;
    selection_one.ledger.records = {{0U, 0U}, {0U, 0U}};
    quote_events.clear();
    quote_values.clear();
    glyph_transfers.clear();
    if (!expect(prepare_field_action_progression_quote(
                    tables, selection_one, quote_callbacks)
                    .result == FieldActionProgressionQuoteResult::quoted &&
                selection_one.selected_action_code == 0x12U &&
                selection_one.pending_cost == 0x26U &&
                purchase_field_action_progression(
                    tables, selection_one, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                selection_one.funds == 0x3C2U &&
                selection_one.ledger.records[1].action_code == 0x12U &&
                selection_one.ledger.records[1].progression_level == 1U)) {
        return 8;
    }

    FieldActionProgressionPurchaseState removal_selection = selection_one;
    removal_selection.saved_selection = 3U;
    removal_selection.funds = 5000U;
    purchase_events.clear();
    if (!expect(purchase_field_action_progression(
                    tables, removal_selection, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::removal_selection &&
                removal_selection.funds == 5000U && purchase_events.empty())) {
        return 9;
    }

    FieldActionProgressionPurchaseState unavailable{};
    unavailable.menu_mode = 3U;
    unavailable.saved_selection = 0;
    unavailable.primary_cursor = 2;
    unavailable.runner_archetype_status = 2;
    unavailable.ledger.records = {{0U, 0U}, {0U, 0U}};
    quote_events.clear();
    if (!expect(prepare_field_action_progression_quote(
                    tables, unavailable, quote_callbacks)
                    .result == FieldActionProgressionQuoteResult::runner_unavailable &&
                quote_events == std::vector<std::string>{"runner-unavailable"})) {
        return 10;
    }
    purchase_events.clear();
    if (!expect(purchase_field_action_progression(
                    tables, unavailable, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::runner_unavailable &&
                purchase_events == std::vector<std::string>{
                                        "grid", "unavailable", "wait"})) {
        return 11;
    }

    tables.saved_selection_zero_by_menu_mode[0][2].action_code = 0xFFU;
    state.ledger.records[1] = {0U, 0U};
    state.pending_record_index.reset();
    state.funds = 5000U;
    state.service_result = 0;
    quote_events.clear();
    if (!expect(prepare_field_action_progression_quote(
                    tables, state, quote_callbacks)
                    .result == FieldActionProgressionQuoteResult::offer_unavailable &&
                purchase_field_action_progression(
                    tables, state, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::offer_unavailable &&
                state.service_result < 0 && state.funds == 5000U)) {
        return 12;
    }

    tables.saved_selection_zero_by_menu_mode[0][2].action_code = 0x0CU;
    FieldActionProgressionPurchaseState missing_slot{};
    missing_slot.menu_mode = 3U;
    missing_slot.saved_selection = 0;
    missing_slot.primary_cursor = 2;
    missing_slot.runner_archetype_status = 1;
    missing_slot.runner_progression_limit = 6;
    missing_slot.ledger.records = {{0U, 0U}};
    if (!expect(prepare_field_action_progression_quote(
                    tables, missing_slot, quote_callbacks)
                    .result == FieldActionProgressionQuoteResult::missing_record_slot &&
                purchase_field_action_progression(
                    tables, missing_slot, purchase_callbacks) ==
                    FieldActionProgressionPurchaseResult::missing_record_slot)) {
        return 13;
    }

    if (!expect(select_field_action_progression_offer(
                    tables, 2U, 0, 2) == std::nullopt &&
                select_field_action_progression_offer(
                    tables, 3U, 1, 0)->action_code == 0x12U &&
                select_field_action_progression_offer(
                    tables, 3U, 2, 0)->action_code == 0x1BU &&
                select_field_action_progression_offer(
                    tables, 3U, static_cast<std::int8_t>(0xFFU), 2)
                        ->action_code == 0x0CU)) {
        return 15;
    }

    return 0;
}

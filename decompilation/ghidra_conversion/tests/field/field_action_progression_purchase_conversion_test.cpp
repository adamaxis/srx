#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"

#include <string>
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

    std::vector<std::string> events;
    const FieldActionProgressionPurchaseCallbacks callbacks{
        [&] { events.emplace_back("grid"); },
        [&] { events.emplace_back("unavailable"); },
        [&] { events.emplace_back("limit"); },
        [&] { events.emplace_back("maxed"); },
        [&] { events.emplace_back("insufficient"); },
        [&] { events.emplace_back("wait"); },
    };

    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 4000U && state.pending_cost == 1000U &&
                state.selected_action_code == 0x0CU &&
                state.ledger.source_record_count_word == 1U &&
                state.ledger.records.size() == 1U &&
                state.ledger.records[0].progression_level == 1U &&
                events.empty())) {
        return 1;
    }

    // Existing records pay for the next level and do not increment the
    // leading count word while their current level is positive.
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 2000U && state.pending_cost == 2000U &&
                state.ledger.records[0].progression_level == 2U &&
                state.ledger.source_record_count_word == 1U &&
                events.empty())) {
        return 2;
    }

    state.ledger.records[0].progression_level = 2U;
    state.runner_progression_limit = 2;
    events.clear();
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::progression_limit &&
                state.funds == 2000U &&
                events == std::vector<std::string>{"limit"})) {
        return 3;
    }

    state.runner_progression_limit = 7;
    state.ledger.records[0].progression_level = 6U;
    events.clear();
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::progression_maxed &&
                state.funds == 2000U &&
                events == std::vector<std::string>{"maxed"})) {
        return 4;
    }

    FieldActionProgressionPurchaseState poor{};
    poor.menu_mode = 3U;
    poor.saved_selection = 0;
    poor.primary_cursor = 2;
    poor.runner_archetype_status = 1;
    poor.runner_progression_limit = 6;
    poor.funds = 999U;
    events.clear();
    if (!expect(purchase_field_action_progression(tables, poor, callbacks) ==
                    FieldActionProgressionPurchaseResult::insufficient_funds &&
                poor.ledger.records.empty() && poor.funds == 999U &&
                events == std::vector<std::string>{"grid", "insufficient",
                                                   "wait"})) {
        return 5;
    }

    FieldActionProgressionPurchaseState non_neutral = poor;
    non_neutral.saved_selection = 1;
    non_neutral.funds = 5000U;
    events.clear();
    if (!expect(purchase_field_action_progression(
                    tables, non_neutral, callbacks) ==
                    FieldActionProgressionPurchaseResult::not_neutral_selection &&
                non_neutral.ledger.records.empty() &&
                non_neutral.funds == 5000U && events.empty())) {
        return 6;
    }

    FieldActionProgressionPurchaseState unavailable{};
    unavailable.menu_mode = 3U;
    unavailable.saved_selection = 0;
    unavailable.primary_cursor = 2;
    unavailable.runner_archetype_status = 2;
    events.clear();
    const FieldActionProgressionPurchaseCallbacks unavailable_callbacks{
        [&] { events.emplace_back("grid"); },
        [&] { events.emplace_back("unavailable"); },
        {}, {}, {},
        [&] { events.emplace_back("wait"); },
    };
    if (!expect(purchase_field_action_progression(
                    tables, unavailable, unavailable_callbacks) ==
                    FieldActionProgressionPurchaseResult::runner_unavailable &&
                unavailable.funds == 0U &&
                events == std::vector<std::string>{"grid", "unavailable",
                                                   "wait"})) {
        return 7;
    }

    tables.saved_selection_zero_by_menu_mode[0][2].action_code = 0xFFU;
    state.ledger.records.clear();
    state.funds = 5000U;
    state.service_result = 0;
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::offer_unavailable &&
                state.service_result < 0 && state.funds == 5000U)) {
        return 8;
    }

    if (!expect(select_field_action_progression_offer(
                    tables, 2U, 0, 2) == std::nullopt &&
                select_field_action_progression_offer(
                    tables, 3U, 1, 0)->action_code == 0x12U &&
                select_field_action_progression_offer(
                    tables, 3U, 2, 0)->action_code == 0x1BU)) {
        return 9;
    }

    return 0;
}

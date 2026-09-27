#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"

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
    state.runner_archetype_status = 2;
    state.runner_progression_limit = 6;
    state.funds = 5000U;

    int panel_renders = 0;
    int limit_messages = 0;
    int maxed_messages = 0;
    int poor_messages = 0;
    const FieldActionProgressionPurchaseCallbacks callbacks{
        [&] { ++panel_renders; },
        {},
        [&] { ++limit_messages; },
        [&] { ++maxed_messages; },
        [&] { ++poor_messages; },
        [] { return true; },
    };

    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 4000U && state.pending_cost == 1000U &&
                state.selected_action_code == 0x0CU &&
                state.ledger.source_record_count_word == 1U &&
                state.ledger.records.size() == 1U &&
                state.ledger.records[0].progression_level == 1U &&
                panel_renders == 1)) {
        return 1;
    }

    // Existing records pay for the next level and do not increment the
    // leading count word while their current level is positive.
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::applied &&
                state.funds == 2000U && state.pending_cost == 2000U &&
                state.ledger.records[0].progression_level == 2U &&
                state.ledger.source_record_count_word == 1U)) {
        return 2;
    }

    state.ledger.records[0].progression_level = 2U;
    state.runner_progression_limit = 2;
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::progression_limit &&
                state.funds == 2000U && limit_messages == 1)) {
        return 3;
    }

    state.runner_progression_limit = 7;
    state.ledger.records[0].progression_level = 6U;
    if (!expect(purchase_field_action_progression(tables, state, callbacks) ==
                    FieldActionProgressionPurchaseResult::progression_maxed &&
                state.funds == 2000U && maxed_messages == 1)) {
        return 4;
    }

    FieldActionProgressionPurchaseState poor{};
    poor.menu_mode = 3U;
    poor.saved_selection = 0;
    poor.primary_cursor = 2;
    poor.runner_archetype_status = 2;
    poor.runner_progression_limit = 6;
    poor.funds = 999U;
    if (!expect(purchase_field_action_progression(tables, poor, callbacks) ==
                    FieldActionProgressionPurchaseResult::insufficient_funds &&
                poor.ledger.records.empty() && poor.funds == 999U &&
                poor_messages == 1)) {
        return 5;
    }

    FieldActionProgressionPurchaseState cancelled = poor;
    cancelled.funds = 5000U;
    const FieldActionProgressionPurchaseCallbacks cancel_callbacks{
        {}, {}, {}, {}, {}, [] { return false; },
    };
    if (!expect(purchase_field_action_progression(
                    tables, cancelled, cancel_callbacks) ==
                    FieldActionProgressionPurchaseResult::cancelled &&
                cancelled.ledger.records.empty() && cancelled.funds == 5000U)) {
        return 6;
    }

    FieldActionProgressionPurchaseState unavailable{};
    unavailable.menu_mode = 3U;
    unavailable.saved_selection = 0;
    unavailable.primary_cursor = 2;
    unavailable.runner_archetype_status = 1;
    int unavailable_messages = 0;
    const FieldActionProgressionPurchaseCallbacks unavailable_callbacks{
        {}, [&] { ++unavailable_messages; }, {}, {}, {},
    };
    if (!expect(purchase_field_action_progression(
                    tables, unavailable, unavailable_callbacks) ==
                    FieldActionProgressionPurchaseResult::runner_unavailable &&
                unavailable_messages == 1 && unavailable.funds == 0U)) {
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

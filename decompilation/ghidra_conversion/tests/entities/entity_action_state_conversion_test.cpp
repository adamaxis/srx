#include "shadowrun_ghidra/entities/entity_action_state_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityActionState state{9U, 8U, 7U, 6U};
    std::vector<int> order;
    set_entity_action_state(state, 0U, 4U,
                            [&order] { order.push_back(1); },
                            [&order] { order.push_back(2); });
    if (state.action_code != 0U ||
        state.state_code != kUnequippedActionType ||
        state.reset_marker != 0U || state.active_marker != 7U ||
        state.scaled_ammo != 0U || order != std::vector<int>{1, 2}) {
        return 1;
    }

    set_entity_action_state(state, 5U, 4U,
                            [&order] { order.push_back(3); },
                            [&order] { order.push_back(4); });
    if (state.action_code != 5U || state.state_code != 4U ||
        state.active_marker != 0U || state.reset_marker != 0U ||
        order != std::vector<int>{1, 2, 3, 4}) {
        return 2;
    }
    EntityActionState scaled{0U, 0U, 10U, 1U};
    set_entity_action_state(scaled, 1U, 3U, {}, {});
    EntityActionState no_round{0U, 0U, 8U, 1U};
    set_entity_action_state(no_round, 1U, 3U, {}, {});
    EntityActionState round_up{0U, 0U, 9U, 1U};
    set_entity_action_state(round_up, 1U, 3U, {}, {});
    if (scaled.action_code != 1U || scaled.state_code != 3U ||
        scaled.active_marker != 0U || scaled.reset_marker != 0U ||
        scaled.scaled_ammo != 1U || no_round.scaled_ammo != 0U ||
        round_up.scaled_ammo != 1U) {
        return 8;
    }

    EntityActionCandidateDispatchState dispatch{9U, 0x4BU, false};
    dispatch_entity_action_candidates(
        dispatch, [&order] { order.push_back(5); },
        [&order] { order.push_back(6); });
    if (dispatch.service_marker != 0U ||
        order != std::vector<int>{1, 2, 3, 4, 5}) {
        return 3;
    }

    dispatch.alternate_mode = true;
    dispatch_entity_action_candidates(
        dispatch, [&order] { order.push_back(7); },
        [&order] { order.push_back(8); });
    if (order != std::vector<int>{1, 2, 3, 4, 5, 7, 8}) {
        return 4;
    }

    EntityActionState scanned{9U, 8U, 0U, 6U};
    advance_entity_action_threshold(
        scanned, {0x15U, 0x12U, 0x0CU, 0x04U, 0x01U, 0x00U, 0x00U, 0x00U},
        [&order] { order.push_back(9); },
        [&order] { order.push_back(10); });
    if (scanned.action_code != 0x0CU || scanned.state_code != 2U ||
        order != std::vector<int>{1, 2, 3, 4, 5, 7, 8, 9, 10}) {
        return 5;
    }

    if (select_entity_action_lane({4U, 4U, 0U, false, false}) != 1U ||
        select_entity_action_lane({4U, 4U, 4U, false, false}) != 2U ||
        select_entity_action_lane({2U, 9U, 9U, false, false}) != 0U ||
        select_entity_action_lane({4U, 0U, 0U, true, true}) != 2U) {
        return 6;
    }
    EntityActionRecordTable records;
    collect_entity_action_records(
        {{0U, 1U}, {2U, 2U}, {7U, 3U}, {11U, 9U}, {8U, 4U}}, records);
    if (records.selected.size() != 3U ||
        records.selected[0].code != 0U || records.selected[0].value != 1U ||
        records.selected[1].code != 7U || records.selected[1].value != 3U ||
        records.selected[2].code != 8U || records.selected[2].value != 4U ||
        records.special_value != 9U || !records.terminated) {
        return 7;
    }
    return 0;
}

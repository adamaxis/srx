#include "shadowrun_ghidra/menus/first_aid_conversion.hpp"

#include <cstdint>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FirstAidUserState user{4U};
    FirstAidTargetState full{10U, 10U, 9U, 0x51U, 0x1234U};
    bool called = false;
    const auto full_result = apply_first_aid_menu_effect(
        user, full, [&](const std::uint8_t, const std::uint8_t) {
            called = true;
            return 3U;
        });
    if (!expect(full_result.result == FirstAidResult::target_already_full &&
                !called && full.physical_meter == 10U &&
                full.mental_meter == 10U && full.flags == 0x51U &&
                full.continuation == 0x1234U)) {
        return 1;
    }

    FirstAidTargetState injured{3U, 2U, 9U, 0x51U, 0U};
    FirstAidRollRequest seen{};
    const auto treated = apply_first_aid_menu_effect(
        user, injured,
        [&](const std::uint8_t roll_count, const std::uint8_t difficulty) {
            seen = {roll_count, difficulty};
            return 0U;
        });
    if (!expect(treated.result == FirstAidResult::treated &&
                seen.roll_count == 4U && seen.difficulty == 5U &&
                treated.successes == 0U && treated.applied_delta == 1U &&
                injured.physical_meter == 4U && injured.mental_meter == 3U &&
                injured.flags == 0x01U && injured.continuation == 0xB45CU)) {
        return 2;
    }

    FirstAidTargetState inactive{8U, 8U, 0U, 0U, 0U};
    const auto inactive_result = apply_first_aid_menu_effect(
        FirstAidUserState{0U}, inactive,
        [](const std::uint8_t, const std::uint8_t) { return 2U; });
    if (!expect(inactive_result.result == FirstAidResult::treated &&
                inactive_result.roll.difficulty == 4U &&
                inactive_result.applied_delta == 2U &&
                inactive.physical_meter == 0U && inactive.mental_meter == 0U &&
                inactive.flags == 0x01U && inactive.continuation == 0xB45CU)) {
        return 3;
    }

    FirstAidTargetState capped{9U, 9U, 0U, 0x01U, 0U};
    const auto capped_result = apply_first_aid_menu_effect(
        FirstAidUserState{2U}, capped,
        [](const std::uint8_t, const std::uint8_t) { return 3U; });
    if (!expect(capped_result.roll.difficulty == 4U &&
                capped.physical_meter == 10U && capped.mental_meter == 10U)) {
        return 4;
    }

    FirstAidTargetState unavailable{2U, 3U, 0U, 0x01U, 0U};
    const auto unavailable_result =
        apply_first_aid_menu_effect(user, unavailable, {});
    if (!expect(unavailable_result.result ==
                    FirstAidResult::roll_service_unavailable &&
                unavailable.physical_meter == 2U &&
                unavailable.mental_meter == 3U && unavailable.flags == 0x01U)) {
        return 5;
    }

    const auto targets = build_first_aid_target_list(
        std::array<std::uint8_t, 3>{0x01U, 0x00U, 0x41U});
    if (!expect(targets.active_count == 2U &&
                targets.active_record_indices[0] == 0U &&
                targets.active_record_indices[1] == 2U &&
                targets.cancel_row() == 2U && targets.row_count() == 3U &&
                advance_first_aid_target_cursor(0U, 2U, -1) == 2U &&
                advance_first_aid_target_cursor(2U, 2U, 1) == 0U)) {
        return 6;
    }

    FirstAidTargetState mental_target{4U, 7U, 0U, 0x51U, 0U};
    std::uint8_t mental_item = 0x11U;
    std::uint8_t mental_phase = 2U;
    const auto mental_patch = use_first_aid_item(
        mental_item, mental_phase, user, mental_target, {});
    if (!expect(mental_patch.result == FirstAidItemResult::used &&
                mental_patch.message == FirstAidMessage::slap_patch_used &&
                mental_patch.applied_delta == 2U &&
                mental_target.mental_meter == 9U && mental_item == 0x11U &&
                mental_phase == 1U)) {
        return 7;
    }

    FirstAidTargetState physical_target{7U, 3U, 0U, 0x51U, 0U};
    std::uint8_t physical_item = 0x13U;
    std::uint8_t physical_phase = 1U;
    const auto physical_patch = use_first_aid_item(
        physical_item, physical_phase, user, physical_target, {});
    if (!expect(physical_patch.result == FirstAidItemResult::used &&
                physical_target.physical_meter == 9U && physical_item == 0U &&
                physical_phase == 0U)) {
        return 8;
    }

    FirstAidTargetState no_mental_need{4U, 10U, 0U, 0x01U, 0U};
    std::uint8_t no_mental_item = 0x11U;
    std::uint8_t no_mental_phase = 4U;
    const auto no_mental = use_first_aid_item(
        no_mental_item, no_mental_phase, user, no_mental_need, {});
    if (!expect(no_mental.result == FirstAidItemResult::target_not_injured &&
                no_mental_phase == 4U && no_mental_item == 0x11U)) {
        return 9;
    }

    std::uint8_t invalid_item = 0x0FU;
    std::uint8_t invalid_phase = 1U;
    if (!expect(use_first_aid_item(invalid_item, invalid_phase, user,
                                   no_mental_need, {})
                        .result == FirstAidItemResult::invalid_item)) {
        return 10;
    }
    return 0;
}

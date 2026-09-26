#include "shadowrun_ghidra/first_aid_conversion.hpp"

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
    return 0;
}

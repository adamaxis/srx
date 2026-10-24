#include "shadowrun_ghidra/menus/first_aid_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

bool read_file(const std::filesystem::path& path,
               std::vector<std::uint8_t>& bytes) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    bytes.assign(std::istreambuf_iterator<char>(input),
                 std::istreambuf_iterator<char>());
    return !input.bad();
}

bool check_source_profile_matrix(const std::vector<std::uint8_t>& rom) {
    using namespace shadowrun::ghidra_conversion;

    constexpr std::size_t kRecordSize = 0x100U;
    constexpr std::size_t kJoshuaRecords = 0x7291AU;
    constexpr std::size_t kRecruitedRecords = 0x72C1AU;
    constexpr std::size_t kRaceMaximums = 0xC22A6U;
    constexpr std::size_t kBodyOffset = 0x77U;
    constexpr std::size_t kBiotechOffset = 0x89U;
    constexpr std::size_t kRaceOffset = 0x9AU;
    constexpr std::size_t kClassOffset = 0x9BU;
    constexpr std::array<std::array<std::uint8_t, 2U>, 13U> kRaceAndClass{{
        {{0U, 2U}}, {{0U, 3U}}, {{0U, 1U}},
        {{2U, 1U}}, {{4U, 2U}}, {{0U, 0U}}, {{2U, 3U}},
        {{3U, 1U}}, {{1U, 3U}}, {{0U, 2U}}, {{1U, 0U}},
        {{1U, 3U}}, {{0U, 2U}},
    }};

    struct Profile {
        std::size_t rom_offset = 0U;
        std::uint8_t race = 0U;
        std::uint8_t archetype = 0U;
        std::uint8_t body = 0U;
        std::uint8_t biotech = 0U;
        std::uint8_t maximum_body = 0U;
    };

    if (rom.size() < kRecruitedRecords + 10U * kRecordSize) {
        std::cerr << "First Aid profile matrix: authoritative ROM is truncated\n";
        return false;
    }

    std::array<Profile, 13U> profiles{};
    for (std::size_t index = 0U; index < profiles.size(); ++index) {
        const auto record_offset = index < 3U
            ? kJoshuaRecords + index * kRecordSize
            : kRecruitedRecords + (index - 3U) * kRecordSize;
        auto& profile = profiles[index];
        profile.rom_offset = record_offset;
        profile.race = rom[record_offset + kRaceOffset];
        profile.archetype = rom[record_offset + kClassOffset];
        profile.body = rom[record_offset + kBodyOffset];
        profile.biotech = rom[record_offset + kBiotechOffset];
        if (profile.race != kRaceAndClass[index][0U] ||
            profile.archetype != kRaceAndClass[index][1U] ||
            profile.race >= 5U) {
            std::cerr << "First Aid profile matrix: race/class mismatch at ROM 0x"
                      << std::hex << record_offset << std::dec << '\n';
            return false;
        }
        profile.maximum_body = rom[kRaceMaximums + profile.race * 6U];
        if (profile.body > profile.maximum_body) {
            std::cerr << "First Aid profile matrix: source Body exceeds race maximum at ROM 0x"
                      << std::hex << record_offset << std::dec << '\n';
            return false;
        }
    }

    const auto expected_difficulty = [](const std::uint8_t physical,
                                        const std::uint8_t body) {
        const std::uint8_t base = physical >= 7U ? 4U
                                  : physical >= 4U ? 6U
                                                   : 8U;
        const auto reduction = static_cast<std::uint8_t>(body / 3U);
        return std::max<std::uint8_t>(
            base > reduction ? static_cast<std::uint8_t>(base - reduction)
                             : 0U,
            2U);
    };
    const auto expected_meter = [](const std::uint8_t meter,
                                   const std::uint8_t delta) {
        return std::min<std::uint8_t>(
            10U, static_cast<std::uint8_t>(meter + delta));
    };

    std::size_t roll_cases = 0U;
    std::size_t fully_treated_cases = 0U;
    std::size_t expected_full_target_cases = 0U;
    for (const auto& user_profile : profiles) {
        const FirstAidUserState user{user_profile.biotech};
        for (const auto& target_profile : profiles) {
            expected_full_target_cases +=
                static_cast<std::size_t>(target_profile.maximum_body) + 1U;
            // Body may rise through the target race's source maximum. This
            // also covers each /3 difficulty-reduction boundary from 0..11.
            for (std::uint8_t body = 0U;
                 body <= target_profile.maximum_body; ++body) {
                for (std::uint8_t physical = 0U; physical <= 10U; ++physical) {
                    for (std::uint8_t mental = 0U; mental <= 10U; ++mental) {
                        if (physical == 10U && mental == 10U) {
                            FirstAidTargetState full{
                                physical, mental, body, 0x51U, 0x1234U};
                            bool roll_called = false;
                            const auto result = apply_first_aid_menu_effect(
                                user, full,
                                [&roll_called](const std::uint8_t,
                                               const std::uint8_t) {
                                    roll_called = true;
                                    return 1U;
                                });
                            if (result.result !=
                                    FirstAidResult::target_already_full ||
                                result.roll.roll_count !=
                                    user_profile.biotech ||
                                roll_called || full.physical_meter != 10U ||
                                full.mental_meter != 10U ||
                                full.flags != 0x51U ||
                                full.continuation != 0x1234U) {
                                std::cerr << "First Aid profile matrix: full-target guard mismatch\n";
                                return false;
                            }
                            ++fully_treated_cases;
                            continue;
                        }

                        const auto trial_count = std::max<std::uint8_t>(
                            user_profile.biotech, 1U);
                        const auto difficulty =
                            expected_difficulty(physical, body);
                        for (std::uint8_t successes = 0U;
                             successes <= trial_count; ++successes) {
                            FirstAidTargetState target{
                                physical, mental, body, 0x51U, 0x1234U};
                            bool roll_called = false;
                            std::uint8_t seen_roll_count = 0xFFU;
                            std::uint8_t seen_difficulty = 0xFFU;
                            const auto result = apply_first_aid_menu_effect(
                                user, target,
                                [&](const std::uint8_t roll_count,
                                    const std::uint8_t seen_threshold) {
                                    roll_called = true;
                                    seen_roll_count = roll_count;
                                    seen_difficulty = seen_threshold;
                                    return successes;
                                });
                            const auto delta = successes == 0U
                                ? 1U
                                : successes;
                            if (result.result != FirstAidResult::treated ||
                                result.roll.roll_count !=
                                    user_profile.biotech ||
                                result.roll.difficulty != difficulty ||
                                !roll_called ||
                                seen_roll_count != user_profile.biotech ||
                                seen_difficulty != difficulty ||
                                result.successes != successes ||
                                result.applied_delta != delta ||
                                target.physical_meter !=
                                    expected_meter(physical, delta) ||
                                target.mental_meter !=
                                    expected_meter(mental, delta) ||
                                target.flags != 0x01U ||
                                target.continuation != 0xB45CU) {
                                std::cerr << "First Aid profile matrix: Medkit roll/effect mismatch"
                                          << " user=0x" << std::hex
                                          << user_profile.rom_offset
                                          << " target=0x"
                                          << target_profile.rom_offset
                                          << std::dec << " biotech="
                                          << static_cast<unsigned>(user_profile.biotech)
                                          << " body=" << static_cast<unsigned>(body)
                                          << " physical=" << static_cast<unsigned>(physical)
                                          << " mental=" << static_cast<unsigned>(mental)
                                          << " successes="
                                          << static_cast<unsigned>(successes)
                                          << '\n';
                                return false;
                            }
                            ++roll_cases;
                        }
                    }
                }
            }
        }
    }

    if (roll_cases == 0U ||
        fully_treated_cases != expected_full_target_cases) {
        std::cerr << "First Aid profile matrix: incomplete cross-profile sweep\n";
        return false;
    }
    std::cout << "First Aid profile matrix: profiles=13 roll_cases="
              << roll_cases << " full_target_cases=" << fully_treated_cases
              << "\n";
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

#ifndef SHADOWRUN_US_ROM_PATH
#error SHADOWRUN_US_ROM_PATH must point to the authoritative US ROM
#endif

    std::vector<std::uint8_t> rom;
    if (!read_file(std::filesystem::path(SHADOWRUN_US_ROM_PATH), rom) ||
        !check_source_profile_matrix(rom)) {
        return 100;
    }

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
    std::uint8_t physical_item = 0x12U;
    std::uint8_t physical_phase = 1U;
    const auto physical_patch = use_first_aid_item(
        physical_item, physical_phase, user, physical_target, {});
    if (!expect(physical_patch.result == FirstAidItemResult::used &&
                physical_target.physical_meter == 9U && physical_item == 0U &&
                physical_phase == 0U)) {
        return 8;
    }

    FirstAidTargetState medkit_target{2U, 2U, 3U, 0x01U, 0U};
    std::uint8_t medkit_item = 0x10U;
    std::uint8_t medkit_phase = 2U;
    const auto no_success_roll = [](const std::uint8_t, const std::uint8_t) {
        return 0U;
    };
    const auto medkit_first_use = use_first_aid_item(
        medkit_item, medkit_phase, user, medkit_target, no_success_roll);
    if (!expect(medkit_first_use.result == FirstAidItemResult::used &&
                medkit_first_use.applied_delta == 1U && medkit_item == 0x10U &&
                medkit_phase == 1U && medkit_target.physical_meter == 3U &&
                medkit_target.mental_meter == 3U)) {
        return 9;
    }
    const auto medkit_final_use = use_first_aid_item(
        medkit_item, medkit_phase, user, medkit_target, no_success_roll);
    if (!expect(medkit_final_use.result == FirstAidItemResult::used &&
                medkit_item == 0U && medkit_phase == 0U &&
                medkit_target.physical_meter == 4U &&
                medkit_target.mental_meter == 4U)) {
        return 10;
    }

    FirstAidTargetState already_healed{10U, 10U, 3U, 0x01U, 0U};
    std::uint8_t full_medkit_item = 0x10U;
    std::uint8_t full_medkit_phase = 3U;
    bool medkit_roll_called = false;
    const auto full_medkit = use_first_aid_item(
        full_medkit_item, full_medkit_phase, user, already_healed,
        [&medkit_roll_called](const std::uint8_t, const std::uint8_t) {
            medkit_roll_called = true;
            return 1U;
        });
    if (!expect(full_medkit.result == FirstAidItemResult::target_not_injured &&
                full_medkit_item == 0x10U && full_medkit_phase == 3U &&
                !medkit_roll_called)) {
        return 11;
    }

    FirstAidTargetState no_mental_need{4U, 10U, 0U, 0x01U, 0U};
    std::uint8_t no_mental_item = 0x11U;
    std::uint8_t no_mental_phase = 4U;
    const auto no_mental = use_first_aid_item(
        no_mental_item, no_mental_phase, user, no_mental_need, {});
    if (!expect(no_mental.result == FirstAidItemResult::target_not_injured &&
                no_mental_phase == 4U && no_mental_item == 0x11U)) {
        return 12;
    }

    std::uint8_t invalid_item = 0x0FU;
    std::uint8_t invalid_phase = 1U;
    if (!expect(use_first_aid_item(invalid_item, invalid_phase, user,
                                   no_mental_need, {})
                        .result == FirstAidItemResult::invalid_item)) {
        return 13;
    }

    std::uint8_t automatic_use_item = 0x13U;
    std::uint8_t automatic_use_phase = 1U;
    if (!expect(use_first_aid_item(automatic_use_item, automatic_use_phase,
                                   user, no_mental_need, {})
                        .result == FirstAidItemResult::invalid_item &&
                automatic_use_item == 0x13U && automatic_use_phase == 1U)) {
        return 14;
    }
    return 0;
}

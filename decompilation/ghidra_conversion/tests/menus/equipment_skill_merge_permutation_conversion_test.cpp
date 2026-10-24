#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

namespace {

using namespace shadowrun::ghidra_conversion;

constexpr std::uint32_t kMaximumLegalSkillRank = 12U;
constexpr std::uint32_t kFirstRejectedSkillRank = 13U;
constexpr std::array<std::uint16_t, 15U> kCurrentValues{{
    0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,
    8U,  9U,  10U, 11U, 12U, 13U, 0xFFU,
}};

bool source_cmp_is_negative(const std::uint8_t left,
                            const std::uint8_t right) {
    const auto result = static_cast<std::uint8_t>(left - right);
    return (result & 0x80U) != 0U;
}

EquipmentActionSkillMergeResult expected_source_merge(
    const std::uint16_t current,
    const std::uint8_t action_code,
    const std::uint8_t skill_81,
    const std::uint8_t skill_82,
    const std::uint8_t skill_83,
    const std::uint8_t skill_84) {
    // Independent ROM 0x12C74 model: byte CMP/BPL bands choose one candidate
    // and the selected skill is merged with the current action value.
    const auto first_band = source_cmp_is_negative(action_code, 0x08U);
    const auto second_band = source_cmp_is_negative(action_code, 0x0BU);
    const auto candidate = first_band
                               ? skill_82
                               : second_band ? skill_83 : skill_84;
    auto selected = skill_81;
    if (source_cmp_is_negative(selected, candidate)) {
        selected = candidate;
    }

    const auto lower = std::min<std::uint16_t>(current, selected);
    const auto merged = static_cast<std::uint16_t>(lower + selected);
    const auto value = merged == 0U ? std::uint16_t{1U} : merged;
    return {value, selected};
}

} // namespace

int main() {
    std::uint64_t checked = 0U;
    for (std::uint32_t action_code = 0U; action_code <= 0xFFU;
         ++action_code) {
        for (std::uint32_t skill_81 = 0U;
             skill_81 <= kFirstRejectedSkillRank; ++skill_81) {
            for (std::uint32_t skill_82 = 0U;
                 skill_82 <= kFirstRejectedSkillRank; ++skill_82) {
                for (std::uint32_t skill_83 = 0U;
                     skill_83 <= kFirstRejectedSkillRank; ++skill_83) {
                    for (std::uint32_t skill_84 = 0U;
                         skill_84 <= kFirstRejectedSkillRank; ++skill_84) {
                        for (const auto current : kCurrentValues) {
                            const auto actual =
                                merge_equipment_action_skill_value(
                                    {current,
                                     static_cast<std::uint8_t>(action_code),
                                     static_cast<std::uint8_t>(skill_81),
                                     static_cast<std::uint8_t>(skill_82),
                                     static_cast<std::uint8_t>(skill_83),
                                     static_cast<std::uint8_t>(skill_84)});
                            const auto expected = expected_source_merge(
                                current,
                                static_cast<std::uint8_t>(action_code),
                                static_cast<std::uint8_t>(skill_81),
                                static_cast<std::uint8_t>(skill_82),
                                static_cast<std::uint8_t>(skill_83),
                                static_cast<std::uint8_t>(skill_84));
                            if (actual.value != expected.value ||
                                actual.selected_skill !=
                                    expected.selected_skill) {
                                std::cerr
                                    << "Atk merge mismatch action="
                                    << action_code << " skills=" << skill_81
                                    << ',' << skill_82 << ',' << skill_83
                                    << ',' << skill_84 << " current="
                                    << current << " actual=" << actual.value
                                    << '/' << static_cast<unsigned>(
                                           actual.selected_skill)
                                    << " expected=" << expected.value << '/'
                                    << static_cast<unsigned>(
                                           expected.selected_skill)
                                    << '\n';
                                return 1;
                            }
                            ++checked;
                        }
                    }
                }
            }
        }
    }

    constexpr std::uint64_t kExpectedCases =
        256ULL * 14ULL * 14ULL * 14ULL * 14ULL * 15ULL;
    if (checked != kExpectedCases) {
        std::cerr << "Atk merge permutation count mismatch: " << checked
                  << '/' << kExpectedCases << '\n';
        return 2;
    }

    std::cout << "equipment_combat_bar_skill_action_cases=" << checked
              << " action_codes=256 legal_skill_ranks=0.."
              << kMaximumLegalSkillRank << " boundary_rank="
              << kFirstRejectedSkillRank << " current_values=0..13,255\n";
    return 0;
}

#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"

#include <tuple>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields record{};
    record.attributes = {10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U};
    record.skills = {20U, 21U, 22U, 23U, 24U, 25U,
                     26U, 27U, 28U, 29U, 30U, 31U};
    record.auxiliary[0U] = 18U;
    record.set_cyberware_essence_ones(19U);
    record.cyberware_flags = static_cast<std::uint16_t>(
        (1U << 6U) | (1U << 8U) | (1U << 10U));

    const auto values = read_cyberware_detail_values(record);
    if (!expect(values.primary_attributes[0U] == 10U &&
                values.primary_attributes[5U] == 15U &&
                values.essence == 17U && values.magic == 18U &&
                values.skills[11U] == 31U &&
                values.essence_secondary == 19U &&
                values.cyberware_flags == record.cyberware_flags)) {
        return 1;
    }

    std::vector<std::tuple<CyberwareDetailLabelSection, std::uint8_t,
                           std::uint16_t>> labels;
    std::vector<std::tuple<CyberwareDetailNumericField, std::uint16_t,
                           std::uint16_t>> numbers;
    std::vector<std::pair<CyberwareDetailModifierField, std::uint16_t>>
        modifier_labels;
    CyberwareDetailCallbacks callbacks{};
    callbacks.render_selected_label =
        [&](const CyberwareDetailLabel& label, const std::uint16_t mode) {
            labels.emplace_back(label.section, label.index, mode);
        };
    callbacks.render_numeric_value =
        [&](const CyberwareDetailNumericField field, const std::uint16_t value,
            const std::uint16_t mode) {
            numbers.emplace_back(field, value, mode);
        };
    callbacks.render_cyberware_modifier_label =
        [&](const CyberwareDetailModifierField group,
            const std::uint16_t mode) {
            modifier_labels.emplace_back(group, mode);
        };

    if (!expect(render_attribute_skill_detail_panel(values, {1U, 2U}, 0x4000U,
                                        callbacks))) {
        return 2;
    }
    if (!expect(labels.size() == 1U &&
                std::get<0>(labels[0]) == CyberwareDetailLabelSection::attribute &&
                std::get<1>(labels[0]) == 6U && std::get<2>(labels[0]) == 0x4000U &&
                numbers.size() == 5U &&
                std::get<0>(numbers[0]) == CyberwareDetailNumericField::essence &&
                std::get<1>(numbers[0]) == 17U &&
                std::get<0>(numbers[1]) ==
                    CyberwareDetailNumericField::essence_secondary &&
                std::get<1>(numbers[1]) == 19U)) {
        return 3;
    }
    if (!expect(modifier_labels.size() == 3U &&
                modifier_labels[0].first ==
                    CyberwareDetailModifierField::quickness &&
                modifier_labels[0].second == 0x6000U &&
                modifier_labels[1].first ==
                    CyberwareDetailModifierField::strength &&
                modifier_labels[2].first ==
                    CyberwareDetailModifierField::body &&
                numbers.size() == 5U &&
                std::get<0>(numbers[2]) ==
                    CyberwareDetailNumericField::quickness_cyberware_modifier &&
                std::get<1>(numbers[2]) == 13U &&
                std::get<0>(numbers[3]) ==
                    CyberwareDetailNumericField::strength_cyberware_modifier &&
                std::get<1>(numbers[3]) == 14U &&
                std::get<0>(numbers[4]) ==
                    CyberwareDetailNumericField::body_cyberware_modifier &&
                std::get<1>(numbers[4]) == 12U)) {
        return 4;
    }

    CyberwareDetailRecordValues overflow_values = values;
    overflow_values.primary_attributes[1U] = 0xFFU;
    overflow_values.primary_attributes[2U] = 0xFFU;
    overflow_values.cyberware_flags = static_cast<std::uint16_t>(1U << 8U);
    labels.clear();
    numbers.clear();
    modifier_labels.clear();
    if (!expect(render_attribute_skill_detail_panel(overflow_values, {0U, 0U}, 0x4000U,
                                        callbacks) &&
                numbers.size() == 3U &&
                std::get<1>(numbers[1]) == 259U &&
                std::get<1>(numbers[2]) == 259U)) {
        return 5;
    }

    labels.clear();
    numbers.clear();
    modifier_labels.clear();
    if (!expect(render_attribute_skill_detail_panel(values, {0U, 9U}, 0x6000U,
                                        callbacks) &&
                labels.size() == 1U &&
                std::get<0>(labels[0]) == CyberwareDetailLabelSection::skill &&
                std::get<1>(labels[0]) == 5U && numbers.size() == 4U &&
                std::get<1>(numbers[0]) == 25U)) {
        return 6;
    }

    for (std::uint8_t column = 0U; column < 2U; ++column) {
        for (std::uint8_t row = 0U; row < 10U; ++row) {
            labels.clear();
            numbers.clear();
            modifier_labels.clear();
            const CyberwareDetailCursor cursor{column, row};
            if (!expect(render_attribute_skill_detail_panel(
                    values, cursor, 0x4000U, callbacks))) {
                return 7;
            }

            const auto attribute_cell = row < 4U;
            const auto selected_index = static_cast<std::uint8_t>(
                attribute_cell ? column * 4U + row
                               : column * 6U + row - 4U);
            const auto expected_section =
                attribute_cell ? CyberwareDetailLabelSection::attribute
                               : CyberwareDetailLabelSection::skill;
            const auto expected_number_count =
                attribute_cell && selected_index == 6U ? 2U : 1U;
            if (labels.size() != 1U ||
                std::get<0>(labels.front()) != expected_section ||
                std::get<1>(labels.front()) != selected_index ||
                std::get<2>(labels.front()) != 0x4000U ||
                numbers.size() != expected_number_count + 3U ||
                modifier_labels.size() != 3U) {
                return 8;
            }

            auto expected_field =
                CyberwareDetailNumericField::selected_attribute_or_skill;
            auto expected_value = static_cast<std::uint16_t>(0U);
            if (!attribute_cell) {
                expected_value = values.skills[selected_index];
            } else if (selected_index < 6U) {
                expected_value = values.primary_attributes[selected_index];
            } else if (selected_index == 6U) {
                expected_field = CyberwareDetailNumericField::essence;
                expected_value = values.essence;
            } else {
                expected_field = CyberwareDetailNumericField::magic;
                expected_value = values.magic;
            }
            if (std::get<0>(numbers.front()) != expected_field ||
                std::get<1>(numbers.front()) != expected_value ||
                std::get<2>(numbers.front()) != 0x4000U) {
                return 9;
            }
            if (attribute_cell && selected_index == 6U &&
                (std::get<0>(numbers[1]) !=
                     CyberwareDetailNumericField::essence_secondary ||
                 std::get<1>(numbers[1]) != values.essence_secondary ||
                 std::get<2>(numbers[1]) != 0x4000U)) {
                return 10;
            }
        }
    }

    auto no_cyberware_values = values;
    no_cyberware_values.cyberware_flags = 0U;
    labels.clear();
    numbers.clear();
    modifier_labels.clear();
    if (!expect(render_attribute_skill_detail_panel(
                    no_cyberware_values, {0U, 0U}, 0x4000U, callbacks) &&
                labels.size() == 1U && numbers.size() == 1U &&
                modifier_labels.empty() &&
                std::get<0>(numbers.front()) ==
                    CyberwareDetailNumericField::selected_attribute_or_skill &&
                std::get<1>(numbers.front()) == 10U)) {
        return 11;
    }

    if (!expect(!render_attribute_skill_detail_panel(values, {2U, 0U}, 0U,
                                                       callbacks))) {
        return 12;
    }
    return 0;
}

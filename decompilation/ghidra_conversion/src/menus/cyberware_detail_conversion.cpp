#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint16_t kCyberwareModifierTextMode = 0x6000U;

void render_numeric(const CyberwareDetailCallbacks& callbacks,
                    const CyberwareDetailNumericField field,
                    const std::uint16_t value, const std::uint16_t text_mode) {
    if (callbacks.render_numeric_value) {
        callbacks.render_numeric_value(field, value, text_mode);
    }
}

} // namespace

CyberwareDetailRecordValues read_cyberware_detail_values(
    const RunnerRecordFields& record) {
    CyberwareDetailRecordValues result{};
    for (std::size_t index = 0; index < result.primary_attributes.size();
         ++index) {
        result.primary_attributes[index] = record.attributes[index];
    }
    // 0x0D5DC's row-6/row-7 paths are not the same as the contiguous
    // Attributes/Skills display array: +0x7E is Essence, +0x7F is Magic, and
    // the secondary Essence number comes from +0x8E.
    result.essence = record.attributes[7U];
    result.magic = record.auxiliary[0U];
    result.skills = record.skills;
    result.essence_secondary = record.auxiliary[3U];
    result.cyberware_flags = record.cyberware_flags;
    return result;
}

bool render_attribute_skill_detail_panel(
    const CyberwareDetailRecordValues& values,
    const CyberwareDetailCursor cursor, const std::uint16_t text_mode,
    const CyberwareDetailCallbacks& callbacks) {
    if (cursor.column >= 2U || cursor.row >= 10U) {
        return false;
    }

    CyberwareDetailLabel label{};
    if (cursor.row < 4U) {
        label.section = CyberwareDetailLabelSection::attribute;
        label.index = static_cast<std::uint8_t>(
            cursor.column * 4U + cursor.row);
    } else {
        label.section = CyberwareDetailLabelSection::skill;
        label.index = static_cast<std::uint8_t>(
            cursor.column * 6U + cursor.row - 4U);
    }

    if (callbacks.render_selected_label) {
        callbacks.render_selected_label(label, text_mode);
    }

    if (label.section == CyberwareDetailLabelSection::skill) {
        render_numeric(callbacks,
                       CyberwareDetailNumericField::selected_attribute_or_skill,
                       values.skills[label.index], text_mode);
    } else if (label.index < 6U) {
        render_numeric(callbacks,
                       CyberwareDetailNumericField::selected_attribute_or_skill,
                       values.primary_attributes[label.index], text_mode);
    } else if (label.index == 6U) {
        // ROM 0xD6EA emits both values for the Essence row.
        render_numeric(callbacks, CyberwareDetailNumericField::essence,
                       values.essence, text_mode);
        render_numeric(callbacks,
                       CyberwareDetailNumericField::essence_secondary,
                       values.essence_secondary, text_mode);
    } else {
        render_numeric(callbacks, CyberwareDetailNumericField::magic,
                       values.magic, text_mode);
    }

    // ROM 0xD732..0xD798 chooses the first installed bit in 5..8.  The
    // selected bit's offset is Quickness (+0x78) plus bit-4.
    for (std::uint8_t bit = 5U; bit <= 8U; ++bit) {
        if ((values.cyberware_flags & (std::uint16_t{1U} << bit)) == 0U) {
            continue;
        }
        const auto modifier = static_cast<std::uint16_t>(
            values.primary_attributes[1U] + bit - 4U);
        if (callbacks.render_cyberware_modifier_label) {
            callbacks.render_cyberware_modifier_label(
                CyberwareDetailModifierField::quickness,
                kCyberwareModifierTextMode);
        }
        render_numeric(callbacks,
                       CyberwareDetailNumericField::quickness_cyberware_modifier,
                       modifier, kCyberwareModifierTextMode);
        if (callbacks.render_cyberware_modifier_label) {
            callbacks.render_cyberware_modifier_label(
                CyberwareDetailModifierField::strength,
                kCyberwareModifierTextMode);
        }
        const auto strength_modifier = static_cast<std::uint16_t>(
            values.primary_attributes[2U] + bit - 4U);
        render_numeric(callbacks,
                       CyberwareDetailNumericField::strength_cyberware_modifier,
                       strength_modifier,
                       kCyberwareModifierTextMode);
        break;
    }

    // ROM 0xD7A0..0xD7E4 chooses the first installed bit in 9..11.  Its
    // displayed value is Body (+0x77) plus bit-8.
    for (std::uint8_t bit = 9U; bit <= 11U; ++bit) {
        if ((values.cyberware_flags & (std::uint16_t{1U} << bit)) == 0U) {
            continue;
        }
        if (callbacks.render_cyberware_modifier_label) {
            callbacks.render_cyberware_modifier_label(
                CyberwareDetailModifierField::body,
                kCyberwareModifierTextMode);
        }
        const auto modifier = static_cast<std::uint16_t>(
            values.primary_attributes[0U] + bit - 8U);
        render_numeric(callbacks,
                       CyberwareDetailNumericField::body_cyberware_modifier,
                       modifier, kCyberwareModifierTextMode);
        break;
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion

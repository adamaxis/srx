#pragma once

#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// The Attributes/Skills screen supplies a column/page and row cursor to ROM
// 0x0D5DC.  The first four rows are attributes; the lower six are skills.
struct CyberwareDetailCursor {
    std::uint8_t column = 0;
    std::uint8_t row = 0;
};

enum class CyberwareDetailLabelSection {
    attribute,
    skill,
};

struct CyberwareDetailLabel {
    CyberwareDetailLabelSection section =
        CyberwareDetailLabelSection::attribute;
    // Attribute indexes are 0..7 and skill indexes are 0..11, matching the
    // two source pointer tables used by 0x0D5DC.
    std::uint8_t index = 0;
};

// The source special-cases Essence and Magic instead of treating the final
// two attribute rows as one contiguous byte array.  Keep those meanings
// explicit so the conversion does not silently reuse the broader menu model.
enum class CyberwareDetailNumericField {
    selected_attribute_or_skill,
    essence,
    magic,
    essence_secondary,
    quickness_cyberware_modifier,
    strength_cyberware_modifier,
    body_cyberware_modifier,
};

enum class CyberwareDetailModifierField {
    quickness,
    strength,
    body,
};

// Values consumed by the detail-panel renderer.  The first six attributes
// come from +0x77..+0x7C, Essence from +0x7E, Magic from +0x7F, skills from
// +0x80..+0x8C, and the second Essence value from +0x8E.  The source also
// reads the installed-cyberware word at +0x90.
struct CyberwareDetailRecordValues {
    std::array<std::uint8_t, 6U> primary_attributes{};
    std::uint8_t essence = 0;
    std::uint8_t magic = 0;
    std::array<std::uint8_t, kRunnerRecordSkillCount> skills{};
    std::uint8_t essence_secondary = 0;
    std::uint16_t cyberware_flags = 0;
};

CyberwareDetailRecordValues read_cyberware_detail_values(
    const RunnerRecordFields& record);

struct CyberwareDetailCallbacks {
    // The caller supplies the source text presentation mode.  This is a
    // typed boundary for the old C0DA call; no source destination address is
    // part of the native conversion.
    std::function<void(const CyberwareDetailLabel&, std::uint16_t)>
        render_selected_label;
    std::function<void(CyberwareDetailNumericField, std::uint16_t,
                       std::uint16_t)>
        render_numeric_value;
    std::function<void(CyberwareDetailModifierField, std::uint16_t)>
        render_cyberware_modifier_label;
};

// ROM 0x0D5DC: render the selected Attributes/Skills cell, the Essence/Magic
// special rows, and the first installed cyberware modifier in each source
// group.  The source scans bits 5..8 and 9..11 in ascending order and stops
// after the first match in each group.
bool render_attribute_skill_detail_panel(
    const CyberwareDetailRecordValues& values,
    CyberwareDetailCursor cursor,
    std::uint16_t text_mode,
    const CyberwareDetailCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion

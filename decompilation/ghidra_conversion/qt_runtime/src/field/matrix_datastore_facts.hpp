#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline scripting::ScriptMatrixDatastoreFacts make_matrix_datastore_facts(
    const std::array<std::uint32_t, 3>& notebook_tip_flags,
    const bool notebook_tip_flags_resolved,
    const std::uint32_t world_event_flags,
    const std::optional<std::uint32_t>& group_item_flags,
    const std::optional<std::uint32_t>& nuyen,
    const std::optional<std::uint8_t>& mr_johnsons_met_flags) {
    scripting::ScriptMatrixDatastoreFacts facts;
    // The ROM's Tips & Clues longwords are FFFC30, FFFC34, and FFFC38.
    // Page 1 is owned by the field dialogue flow; this handler reads pages 2
    // and 3. Preserve unknown loaded-save state as absent facts.
    if (notebook_tip_flags_resolved) {
        facts.tips_page3_flags = notebook_tip_flags[2];
        // ROM 0x55088..0x550EA uses page-2 bits 28..31 as four independent
        // one-time Matrix reports. Keep that save representation in the host.
        const auto page2 = notebook_tip_flags[1];
        facts.system18_project_file_collected = (page2 & (1U << 28U)) != 0U;
        facts.system18_confidential_file_collected =
            (page2 & (1U << 29U)) != 0U;
        facts.system18_financial_file_collected =
            (page2 & (1U << 30U)) != 0U;
        facts.system18_management_file_collected =
            (page2 & (1U << 31U)) != 0U;
        // ROM 0x5533E..0x5536E exposes the UCAS virology report after five
        // research notices, then records the report itself. Keep this source
        // encoding here and present story-level facts to Lua.
        constexpr std::uint32_t kResearchNotices = 0x7C000000U;
        constexpr std::uint32_t kReportCollected = 0x80000000U;
        facts.virology_research_complete =
            (notebook_tip_flags[2] & kResearchNotices) == kResearchNotices;
        facts.virology_report_collected =
            (notebook_tip_flags[2] & kReportCollected) != 0U;
    }
    facts.world_event_flags = world_event_flags;
    facts.group_item_flags = group_item_flags;
    facts.nuyen = nuyen;
    facts.mr_johnsons_met_flags = mr_johnsons_met_flags;
    return facts;
}

inline void apply_matrix_datastore_facts(
    const scripting::ScriptMatrixDatastoreFacts& facts,
    std::array<std::uint32_t, 3>& notebook_tip_flags,
    bool& notebook_tip_flags_resolved,
    std::uint32_t& world_event_flags,
    std::optional<std::uint32_t>& group_item_flags,
    std::optional<std::uint32_t>& nuyen,
    std::optional<std::uint8_t>& mr_johnsons_met_flags) {
    if (facts.tips_page3_flags.has_value()) {
        notebook_tip_flags[2] = *facts.tips_page3_flags;
    }
    if (facts.system18_project_file_collected.value_or(false)) {
        notebook_tip_flags[1] |= 1U << 28U;
    }
    if (facts.system18_confidential_file_collected.value_or(false)) {
        notebook_tip_flags[1] |= 1U << 29U;
    }
    if (facts.system18_financial_file_collected.value_or(false)) {
        notebook_tip_flags[1] |= 1U << 30U;
    }
    if (facts.system18_management_file_collected.value_or(false)) {
        notebook_tip_flags[1] |= 1U << 31U;
    }
    if (facts.virology_report_collected.value_or(false)) {
        notebook_tip_flags[2] |= 0x80000000U;
    }
    if (facts.tips_page3_flags.has_value()) {
        notebook_tip_flags_resolved = true;
    }
    world_event_flags = facts.world_event_flags;
    group_item_flags = facts.group_item_flags;
    nuyen = facts.nuyen;
    mr_johnsons_met_flags = facts.mr_johnsons_met_flags;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

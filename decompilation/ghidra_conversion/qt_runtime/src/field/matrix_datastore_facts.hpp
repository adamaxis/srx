#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline scripting::ScriptMatrixDatastoreFacts make_matrix_datastore_facts(
    const std::array<std::uint32_t, 3>& notebook_tip_flags,
    const bool notebook_tip_flags_resolved,
    const std::optional<std::uint32_t>& group_item_flags,
    const std::optional<std::uint32_t>& nuyen,
    const std::optional<std::uint8_t>& mr_johnsons_met_flags) {
    scripting::ScriptMatrixDatastoreFacts facts;
    // The ROM's Tips & Clues longwords are FFFC30, FFFC34, and FFFC38.
    // Page 1 is owned by field dialogue. Translate the supported Matrix
    // story outcomes from pages 2 and 3 here; Lua only sees named facts.
    if (notebook_tip_flags_resolved) {
        const auto page2 = notebook_tip_flags[1];
        const auto page3 = notebook_tip_flags[2];
        const auto has = [](const std::uint32_t flags, const unsigned bit) {
            return (flags & (std::uint32_t{1U} << bit)) != 0U;
        };

        // ROM 0x55088..0x550EA selects Ito's four reports from FFFC34 bits
        // 28..31 by their named Matrix file categories.
        facts.ito_project_file_collected = has(page2, 28U);
        facts.ito_confidential_file_collected = has(page2, 29U);
        facts.ito_financial_file_collected = has(page2, 30U);
        facts.ito_management_file_collected = has(page2, 31U);

        // ROM 0x5533E..0x5536E requires FFFC38 bits 26..30, then sets bit 31.
        constexpr std::uint32_t kResearchNotices = 0x7C000000U;
        constexpr std::uint32_t kReportCollected = 0x80000000U;
        facts.ucas_virology_research_complete =
            (page3 & kResearchNotices) == kResearchNotices;
        facts.ucas_virology_report_collected =
            (page3 & kReportCollected) != 0U;

        // [C] The remaining fields map supported system/category branches
        // through ROM 0x550F4..0x55578 to named collection outcomes.
        facts.gates_financial_file_collected = has(page3, 0U);
        facts.gates_outdated_file_collected = has(page3, 1U);
        facts.fuchi_management_file_collected = has(page3, 2U);
        facts.fuchi_security_file_collected = has(page3, 3U);
        // ROM 0x55204..0x55220 maps Mitsuhama Security Files to this outcome.
        facts.mitsuhama_security_file_collected = has(page3, 4U);
        facts.lone_star_financial_file_collected = has(page3, 11U);
        facts.ares_confidential_followup_collected = has(page3, 12U);
        facts.hollywood_prisoner_file_collected = has(page3, 13U);
        facts.hollywood_outdated_file_collected = has(page3, 14U);
        facts.renraku_shaman_project_report_collected = has(page3, 16U);
        facts.renraku_management_file_collected = has(page3, 17U);
        facts.renraku_confidential_followup_collected = has(page3, 18U);
        facts.renraku_mako_project_report_collected = has(page3, 19U);
        facts.renraku_michael_project_report_collected = has(page3, 24U);
        facts.renraku_map_project_report_collected = has(page3, 25U);
        facts.aztechnology_confidential_file_collected = has(page3, 26U);
        facts.fuchi_security_followup_collected = has(page3, 27U);
        facts.mitsuhama_confidential_file_collected = has(page3, 28U);
        facts.ares_confidential_file_collected = has(page3, 29U);
        facts.renraku_confidential_file_collected = has(page3, 30U);
        facts.lone_star_prisoner_file_collected = has(page3, 9U);
    }
    // ROM 0x550456..0x550470 checks Group Item FFFC22 bit 8. This is the
    // named Renraku map overlay item in E.GroupItemBit.
    if (group_item_flags.has_value()) {
        facts.renraku_map_overlay_collected =
            (*group_item_flags & (1U << 8U)) != 0U;
    }
    facts.nuyen = nuyen;
    // ROM 0x5502AA..0x5502BE and 0x55037A..0x550392 select the known Johnson
    // branch from FFFC27 bit 3; keep the contact fact semantic in Lua.
    if (mr_johnsons_met_flags.has_value()) {
        facts.johnson_contact_recorded =
            (*mr_johnsons_met_flags & 0x08U) != 0U;
    }
    return facts;
}

inline void apply_matrix_datastore_facts(
    const scripting::ScriptMatrixDatastoreFacts& facts,
    std::array<std::uint32_t, 3>& notebook_tip_flags,
    bool& notebook_tip_flags_resolved,
    std::optional<std::uint32_t>& nuyen) {
    const auto record = [](std::uint32_t& flags, const unsigned bit,
                           const std::optional<bool>& collected) {
        if (collected.value_or(false)) {
            flags |= std::uint32_t{1U} << bit;
        }
    };
    // Collection is one-way in the source handler; merge named outcomes so
    // unrelated Notebook clues remain untouched.
    record(notebook_tip_flags[1], 28U, facts.ito_project_file_collected);
    record(notebook_tip_flags[1], 29U, facts.ito_confidential_file_collected);
    record(notebook_tip_flags[1], 30U, facts.ito_financial_file_collected);
    record(notebook_tip_flags[1], 31U, facts.ito_management_file_collected);
    record(notebook_tip_flags[2], 0U, facts.gates_financial_file_collected);
    record(notebook_tip_flags[2], 1U, facts.gates_outdated_file_collected);
    record(notebook_tip_flags[2], 2U, facts.fuchi_management_file_collected);
    record(notebook_tip_flags[2], 3U, facts.fuchi_security_file_collected);
    record(notebook_tip_flags[2], 4U, facts.mitsuhama_security_file_collected);
    record(notebook_tip_flags[2], 9U, facts.lone_star_prisoner_file_collected);
    record(notebook_tip_flags[2], 11U, facts.lone_star_financial_file_collected);
    record(notebook_tip_flags[2], 12U, facts.ares_confidential_followup_collected);
    record(notebook_tip_flags[2], 13U, facts.hollywood_prisoner_file_collected);
    record(notebook_tip_flags[2], 14U, facts.hollywood_outdated_file_collected);
    record(notebook_tip_flags[2], 16U, facts.renraku_shaman_project_report_collected);
    record(notebook_tip_flags[2], 17U, facts.renraku_management_file_collected);
    record(notebook_tip_flags[2], 18U, facts.renraku_confidential_followup_collected);
    record(notebook_tip_flags[2], 19U, facts.renraku_mako_project_report_collected);
    record(notebook_tip_flags[2], 24U, facts.renraku_michael_project_report_collected);
    record(notebook_tip_flags[2], 25U, facts.renraku_map_project_report_collected);
    record(notebook_tip_flags[2], 26U, facts.aztechnology_confidential_file_collected);
    record(notebook_tip_flags[2], 27U, facts.fuchi_security_followup_collected);
    record(notebook_tip_flags[2], 28U, facts.mitsuhama_confidential_file_collected);
    record(notebook_tip_flags[2], 29U, facts.ares_confidential_file_collected);
    record(notebook_tip_flags[2], 30U, facts.renraku_confidential_file_collected);
    if (facts.ucas_virology_report_collected.value_or(false)) {
        notebook_tip_flags[2] |= 0x80000000U;
    }
    if (facts.ucas_virology_research_complete.has_value()) {
        notebook_tip_flags_resolved = true;
    }
    nuyen = facts.nuyen;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

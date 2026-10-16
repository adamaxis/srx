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
        facts.tips_page2_flags = notebook_tip_flags[1];
        facts.tips_page3_flags = notebook_tip_flags[2];
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
    if (facts.tips_page2_flags.has_value()) {
        notebook_tip_flags[1] = *facts.tips_page2_flags;
    }
    if (facts.tips_page3_flags.has_value()) {
        notebook_tip_flags[2] = *facts.tips_page3_flags;
    }
    if (facts.tips_page2_flags.has_value() &&
        facts.tips_page3_flags.has_value()) {
        notebook_tip_flags_resolved = true;
    }
    world_event_flags = facts.world_event_flags;
    group_item_flags = facts.group_item_flags;
    nuyen = facts.nuyen;
    mr_johnsons_met_flags = facts.mr_johnsons_met_flags;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime

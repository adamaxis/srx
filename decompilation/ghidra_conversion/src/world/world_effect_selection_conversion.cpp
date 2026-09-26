#include "shadowrun_ghidra/world/world_effect_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::size_t> select_world_effect_sequence(
    const std::uint8_t context,
    const WorldEffectSelectionTable& table,
    const WorldEffectRandomBoundary& next_random) {
    if (!next_random || static_cast<std::size_t>(context) >=
                            table.row_by_context.size()) {
        return std::nullopt;
    }

    const auto first_roll = next_random(kWorldEffectSelectionRollBound);
    const auto second_roll = next_random(kWorldEffectSelectionRollBound);
    if (first_roll >= kWorldEffectSelectionRollBound ||
        second_roll >= kWorldEffectSelectionRollBound) {
        return std::nullopt;
    }

    const auto row = static_cast<std::size_t>(table.row_by_context[context]);
    const auto column = static_cast<std::size_t>(first_roll + second_roll);
    const auto entry_index = row * kWorldEffectSelectionColumns + column;
    if (row != 0U && entry_index / kWorldEffectSelectionColumns != row) {
        return std::nullopt;
    }
    if (entry_index >= table.direct_handles.size() ||
        entry_index >= table.indirection_enabled.size()) {
        return std::nullopt;
    }
    if (table.indirection_enabled[entry_index] == 0U ||
        !table.direct_handles[entry_index].has_value()) {
        return std::nullopt;
    }

    auto handle = *table.direct_handles[entry_index];
    if (handle >= table.indirection_targets.size() ||
        !table.indirection_targets[handle].has_value()) {
        return std::nullopt;
    }
    handle = *table.indirection_targets[handle];
    return handle;
}

} // namespace shadowrun::ghidra_conversion

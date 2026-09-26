#include "shadowrun_ghidra/menus/hud_selection_lookup_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::size_t> find_hud_selection_index(
    const std::array<std::uint8_t, 5>& entries,
    const std::uint8_t code) {
    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (entries[index] == code) {
            return index;
        }
    }
    return std::nullopt;
}

bool set_hud_selection_value(
    HudSelectionState& state,
    const std::uint8_t code,
    const std::uint8_t value) {
    const auto index = find_hud_selection_index(state.entry_codes, code);
    if (!index.has_value()) {
        return false;
    }
    state.values[*index] = value;
    return true;
}

bool hud_selection_value_matches(
    const HudSelectionState& state,
    const std::uint8_t code,
    const std::uint8_t value) {
    const auto index = find_hud_selection_index(state.entry_codes, code);
    return index.has_value() && state.values[*index] == value;
}

} // namespace shadowrun::ghidra_conversion

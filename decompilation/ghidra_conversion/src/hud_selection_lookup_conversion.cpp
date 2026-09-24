#include "shadowrun_ghidra/hud_selection_lookup_conversion.hpp"

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

} // namespace shadowrun::ghidra_conversion

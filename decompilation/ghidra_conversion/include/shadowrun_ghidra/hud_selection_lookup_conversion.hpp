#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// ROM 0x17C98: scan the five-entry HUD/action table for a byte code and
// return its zero-based slot index.
std::optional<std::size_t> find_hud_selection_index(
    const std::array<std::uint8_t, 5>& entries,
    std::uint8_t code);

} // namespace shadowrun::ghidra_conversion

#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct HudSelectionState {
    std::array<std::uint8_t, 5> entry_codes{};
    std::array<std::uint8_t, 5> values{};
};

// ROM 0x17C98: scan the five-entry HUD/action table for a byte code and
// return its zero-based slot index.
std::optional<std::size_t> find_hud_selection_index(
    const std::array<std::uint8_t, 5>& entries,
    std::uint8_t code);

// ROM 0x17C84 / 0x17C8E: set or compare the parallel value byte selected by
// the five-entry code table.
bool set_hud_selection_value(
    HudSelectionState& state,
    std::uint8_t code,
    std::uint8_t value);
bool hud_selection_value_matches(
    const HudSelectionState& state,
    std::uint8_t code,
    std::uint8_t value);

} // namespace shadowrun::ghidra_conversion

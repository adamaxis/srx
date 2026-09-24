#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::uint16_t kWorldEffectSelectionRollBound = 5U;
constexpr std::size_t kWorldEffectSelectionColumns = 9U;

struct WorldEffectSelectionTable {
    // The source byte at 0x3312 maps the caller's context byte to a row.
    std::vector<std::uint8_t> row_by_context;
    // Entries are row-major, nine handles per row, matching 0x1D7174.
    std::vector<std::optional<std::size_t>> direct_handles;
    // A nonzero source byte at 0x1D70E4 enables one handle indirection.
    std::vector<std::uint8_t> indirection_enabled;
    std::vector<std::optional<std::size_t>> indirection_targets;
};

using WorldEffectRandomBoundary =
    std::function<std::uint16_t(std::uint16_t bound)>;

// ROM 0x59986: roll twice with bound five, select the corresponding
// context-row/roll-sum entry, suppress disabled entries, and follow the one
// source indirection used by the enabled path. The returned handle is a host
// identity for the extracted effect sequence, not a ROM/RAM address.
std::optional<std::size_t> select_world_effect_sequence(
    std::uint8_t context,
    const WorldEffectSelectionTable& table,
    const WorldEffectRandomBoundary& next_random);

} // namespace shadowrun::ghidra_conversion

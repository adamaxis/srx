#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x12148: use byte arithmetic for the selector index and read the
// entity's action-code table beginning at record offset 0x66.
std::optional<std::uint8_t> read_entity_action_code(
    const std::vector<std::uint8_t>& entity_record,
    std::uint8_t base_index,
    std::uint8_t selector);

} // namespace shadowrun::ghidra_conversion

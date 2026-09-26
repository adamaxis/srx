#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// ROM 0x12148: multiply the row selector by two, add the column with a
// byte-sized operation, and read the entity's action-code table beginning at
// record offset 0x66. The MULU.W high byte is retained across ADD.B.
std::optional<std::uint8_t> read_entity_action_code(
    const std::vector<std::uint8_t>& entity_record,
    std::uint8_t base_index,
    std::uint8_t selector);

} // namespace shadowrun::ghidra_conversion

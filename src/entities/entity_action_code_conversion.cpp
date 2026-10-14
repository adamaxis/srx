#include "shadowrun_ghidra/entities/entity_action_code_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::uint8_t> read_entity_action_code(
    const std::vector<std::uint8_t>& entity_record,
    const std::uint8_t base_index,
    const std::uint8_t selector) {
    // The source keeps the full MULU.W result in D7 and applies the column
    // with ADD.B. Only the low byte wraps; the high byte of selector*2 must
    // remain part of the record offset.
    const auto product = static_cast<std::uint16_t>(selector) * 2U;
    const auto low_byte = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(product) + base_index);
    const auto table_index = static_cast<std::uint16_t>(
        (product & 0xFF00U) | low_byte);
    const auto offset = 0x66U + static_cast<std::size_t>(table_index);
    if (offset >= entity_record.size()) {
        return std::nullopt;
    }
    return entity_record[offset];
}

} // namespace shadowrun::ghidra_conversion

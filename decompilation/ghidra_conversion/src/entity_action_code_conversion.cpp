#include "shadowrun_ghidra/entity_action_code_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::uint8_t> read_entity_action_code(
    const std::vector<std::uint8_t>& entity_record,
    const std::uint8_t base_index,
    const std::uint8_t selector) {
    const auto table_index = static_cast<std::uint8_t>(
        base_index + static_cast<std::uint8_t>(selector * 2U));
    const auto offset = 0x66U + static_cast<std::size_t>(table_index);
    if (offset >= entity_record.size()) {
        return std::nullopt;
    }
    return entity_record[offset];
}

} // namespace shadowrun::ghidra_conversion

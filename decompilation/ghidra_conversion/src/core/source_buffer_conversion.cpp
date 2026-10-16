#include "shadowrun_ghidra/core/source_buffer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void copy_source_word_block(SourceWordBlock& destination,
                            const SourceWordBlock& source) {
    destination = source;
}

std::optional<std::vector<std::uint8_t>> extract_even_bytes(
    const std::vector<std::uint8_t>& table, std::size_t source_element,
    std::size_t count) {
    const auto source_offset = source_element * 2U;
    if (source_element != 0U && source_offset / 2U != source_element) {
        return std::nullopt;
    }
    if (count > 0U &&
        (source_offset >= table.size() ||
         (count - 1U) > (table.size() - 1U - source_offset) / 2U)) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> extracted;
    extracted.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        extracted.push_back(table[source_offset + index * 2U]);
    }
    return extracted;
}

bool copy_even_bytes(std::vector<std::uint8_t>& table,
                     std::size_t destination_element,
                     std::size_t source_element, std::size_t count) {
    const auto destination_offset = destination_element * 2U;
    const auto source_offset = source_element * 2U;
    if ((destination_element != 0U &&
         destination_offset / 2U != destination_element) ||
        (source_element != 0U && source_offset / 2U != source_element)) {
        return false;
    }
    if (count == 0U) {
        return true;
    }
    const auto last_index = count - 1U;
    if (destination_offset >= table.size() || source_offset >= table.size() ||
        last_index > (table.size() - 1U - destination_offset) / 2U ||
        last_index > (table.size() - 1U - source_offset) / 2U) {
        return false;
    }

    for (std::size_t index = 0; index < count; ++index) {
        table[destination_offset + index * 2U] =
            table[source_offset + index * 2U];
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion

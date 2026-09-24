#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kSourceWordBlockLength = 32;
using SourceWordBlock = std::array<std::uint32_t, kSourceWordBlockLength>;

// ROM 0x1358: copy exactly 32 longwords into the source work block.
void copy_source_word_block(SourceWordBlock& destination,
                            const SourceWordBlock& source);

// ROM 0x14CE: extract count bytes from every second byte of a source table.
std::optional<std::vector<std::uint8_t>> extract_even_bytes(
    const std::vector<std::uint8_t>& table, std::size_t source_element,
    std::size_t count);

// ROM 0x1502: copy count bytes between even-byte positions in one table.
// The forward order matches the source loop when ranges overlap.
bool copy_even_bytes(std::vector<std::uint8_t>& table,
                     std::size_t destination_element,
                     std::size_t source_element, std::size_t count);

} // namespace shadowrun::ghidra_conversion

#pragma once

#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct TextLineNormalizationResult {
    std::vector<std::uint8_t> bytes;
    std::size_t source_bytes_consumed = 0;
    bool terminated = false;
};

// ROM 0xC134: copy source text, map the 0x80-0xBF marker range to 0x80,
// stop on a 0xC0-0xFF terminator, and fill the remaining inclusive width with
// spaces. A bounded host input reports an absent terminator instead of
// reading beyond the supplied source.
TextLineNormalizationResult normalize_source_text_line(
    const std::vector<std::uint8_t>& source,
    std::int16_t remaining_width);

} // namespace shadowrun::ghidra_conversion

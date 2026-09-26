#include "shadowrun_ghidra/rendering/text_line_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TextLineNormalizationResult normalize_source_text_line(
    const std::vector<std::uint8_t>& source,
    const std::int16_t remaining_width) {
    TextLineNormalizationResult result{};
    for (const auto value : source) {
        ++result.source_bytes_consumed;
        if ((value & 0x80U) != 0U) {
            if ((value & 0x40U) != 0U) {
                result.terminated = true;
                if (remaining_width >= 0) {
                    result.bytes.insert(
                        result.bytes.end(),
                        static_cast<std::size_t>(remaining_width) + 1U,
                        static_cast<std::uint8_t>(' '));
                }
                return result;
            }
            result.bytes.push_back(0x80U);
            continue;
        }
        result.bytes.push_back(value);
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion

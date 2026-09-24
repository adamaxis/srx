#include "shadowrun_ghidra/compressed_data_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::optional<std::vector<std::uint8_t>> decode_source_byte_stream(
    const std::vector<std::uint8_t>& encoded) {
    if (encoded.size() < 2U) {
        return std::nullopt;
    }

    const auto output_size_minus_one = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(encoded[0]) << 8U) | encoded[1]);
    const auto output_size = static_cast<std::size_t>(output_size_minus_one) + 1U;

    std::vector<std::uint8_t> decoded;
    decoded.reserve(output_size);
    std::size_t input_offset = 2U;
    std::uint8_t flags = 0U;
    unsigned flags_remaining = 0U;

    while (decoded.size() < output_size) {
        if (flags_remaining == 0U) {
            if (input_offset >= encoded.size()) {
                return std::nullopt;
            }
            flags = encoded[input_offset++];
            flags_remaining = 8U;
        }

        const bool is_copy = (flags & 0x80U) != 0U;
        flags = static_cast<std::uint8_t>(flags << 1U);
        --flags_remaining;

        if (!is_copy) {
            if (input_offset >= encoded.size()) {
                return std::nullopt;
            }
            decoded.push_back(encoded[input_offset++]);
            continue;
        }

        if (input_offset >= encoded.size()) {
            return std::nullopt;
        }
        const auto control = encoded[input_offset++];
        if ((control & 0x80U) == 0U) {
            if (input_offset >= encoded.size()) {
                return std::nullopt;
            }
            const auto value = encoded[input_offset++];
            const auto count = static_cast<std::size_t>(
                (control & 0x7FU) + 3U);
            if (decoded.size() + count > output_size) {
                return std::nullopt;
            }
            decoded.insert(decoded.end(), count, value);
            continue;
        }

        if (input_offset >= encoded.size()) {
            return std::nullopt;
        }
        const auto distance_low = encoded[input_offset++];
        const auto distance = static_cast<std::size_t>(
            (static_cast<std::size_t>(distance_low) << 3U) |
            ((control >> 4U) & 0x07U));
        const auto count = static_cast<std::size_t>((control & 0x0FU) + 3U);
        if (distance == 0U || distance > decoded.size() ||
            decoded.size() + count > output_size) {
            return std::nullopt;
        }

        for (std::size_t index = 0; index < count; ++index) {
            decoded.push_back(decoded[decoded.size() - distance]);
        }
    }

    return decoded;
}

} // namespace shadowrun::ghidra_conversion

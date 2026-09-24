#include "shadowrun_ghidra/coordinate_data_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

std::int16_t read_signed_word(const std::vector<std::uint8_t>& bytes,
                              std::size_t offset) {
    const auto value = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        bytes[offset + 1U]);
    return static_cast<std::int16_t>(value);
}

std::int16_t add_offset(std::int16_t base, std::int16_t offset) {
    return static_cast<std::int16_t>(
        static_cast<std::uint16_t>(base) +
        static_cast<std::uint16_t>(offset));
}

} // namespace

std::optional<std::vector<std::int16_t>> decode_source_coordinate_stream(
    const std::vector<std::uint8_t>& encoded) {
    if (encoded.size() < 2U) {
        return std::nullopt;
    }

    const auto base = read_signed_word(encoded, 0U);
    std::size_t input_offset = 2U;
    std::vector<std::int16_t> decoded;

    while (true) {
        if (input_offset >= encoded.size()) {
            return std::nullopt;
        }
        const auto command = encoded[input_offset++];
        if (command == 0U) {
            return decoded;
        }

        const auto count = static_cast<std::size_t>((command & 0x1FU) + 1U);
        if (command < 0xC1U) {
            const auto direction = command < 0x41U
                ? static_cast<std::int16_t>(-1)
                : command < 0x81U
                    ? static_cast<std::int16_t>(0)
                    : static_cast<std::int16_t>(1);
            std::int16_t offset = 0;
            if ((command & 0x20U) == 0U) {
                if (input_offset >= encoded.size()) {
                    return std::nullopt;
                }
                offset = static_cast<std::int16_t>(encoded[input_offset++]);
            } else {
                if (input_offset + 1U >= encoded.size()) {
                    return std::nullopt;
                }
                offset = read_signed_word(encoded, input_offset);
                input_offset += 2U;
            }

            auto value = add_offset(base, offset);
            for (std::size_t index = 0; index < count; ++index) {
                decoded.push_back(value);
                value = static_cast<std::int16_t>(value + direction);
            }
            continue;
        }

        std::int16_t offset = 0;
        if ((command & 0x20U) == 0U) {
            if (input_offset >= encoded.size()) {
                return std::nullopt;
            }
            offset = static_cast<std::int16_t>(encoded[input_offset++]);
        } else {
            if (input_offset + 1U >= encoded.size()) {
                return std::nullopt;
            }
            offset = read_signed_word(encoded, input_offset);
            input_offset += 2U;
        }

        decoded.insert(decoded.end(), count, add_offset(base, offset));
    }
}

} // namespace shadowrun::ghidra_conversion
